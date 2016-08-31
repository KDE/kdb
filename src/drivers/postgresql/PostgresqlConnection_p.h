/* This file is part of the KDE project
   Copyright (C) 2005 Adam Pigg <adam@piggz.co.uk>
   Copyright (C) 2010-2016 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KDB_POSTGRESQLCONNECTION_P_H
#define KDB_POSTGRESQLCONNECTION_P_H

#include "KDbConnection_p.h"
#include "PostgresqlConnection.h"
#include "PostgresqlDriver.h"
#include "KDbResult.h"
#include "KDbSqlField.h"
#include "KDbSqlRecord.h"
#include "KDbSqlResult.h"
#include "KDbSqlString.h"

#include <QString>

#include <libpq-fe.h>

class KDbEscapedString;

class PostgresqlConnectionInternal : public KDbConnectionInternal
{
public:
    explicit PostgresqlConnectionInternal(KDbConnection *connection);

    virtual ~PostgresqlConnectionInternal();

    //! Executes query for a raw SQL statement @a sql on the database
    PGresult* executeSQL(const KDbEscapedString& sql);

    static QString serverResultName(int resultCode);

    void storeResultAndClear(KDbResult *result, PGresult **pgResult, ExecStatusType execStatus);

    void storeResult(KDbResult *result);

    //! @return true if status of connection is "OK".
    /*! From http://www.postgresql.org/docs/8.4/static/libpq-status.html:
        "Only two of these are seen outside of an asynchronous connection procedure:
         CONNECTION_OK and CONNECTION_BAD." */
    inline bool connectionOK() { return CONNECTION_OK == PQstatus(conn); }

    PGconn *conn;
    bool unicode;
    QByteArray escapingBuffer;
};

//! Internal PostgreSQL cursor data.
/*! Provides a low-level abstraction for iterating over result sets. */
class PostgresqlCursorData : public PostgresqlConnectionInternal
{
public:
    explicit PostgresqlCursorData(KDbConnection* connection);
    virtual ~PostgresqlCursorData();

    PGresult* res;
    ExecStatusType resultStatus;
};

class PostgresqlSqlField : public KDbSqlField
{
public:
    inline PostgresqlSqlField(const PGresult *r, int n) : result(r), number(n) {
    }
    //! @return column name
    inline QString name() Q_DECL_OVERRIDE {
        //! @todo UTF8?
        return QString::fromLatin1(PQfname(result, number));
    }
    inline int type() Q_DECL_OVERRIDE {
        return static_cast<int>(PQftype(result, number));
    }
    inline int length() Q_DECL_OVERRIDE {
        return PostgresqlDriver::pqfmodToLength(PQfmod(result, number));
    }
    const PGresult * const result;
    const int number;
};

class PostgresqlSqlRecord : public KDbSqlRecord
{
public:
    inline PostgresqlSqlRecord(const PGresult *res, int r) : result(res), record(r) {
    }
    inline ~PostgresqlSqlRecord() {
    }
    inline QString stringValue(int index) Q_DECL_OVERRIDE {
        return PQgetisnull(result, record, index)
                ? QString()
                : QString::fromUtf8(PQgetvalue(result, record, index),
                                    PQgetlength(result, record, index));
    }
    inline KDbSqlString cstringValue(int index) Q_DECL_OVERRIDE {
        return PQgetisnull(result, record, index)
                ? KDbSqlString()
                : KDbSqlString(PQgetvalue(result, record, index),
                               PQgetlength(result, record, index));
    }
    inline QByteArray toByteArray(int index) Q_DECL_OVERRIDE {
        return PQgetisnull(result, record, index)
                ? QByteArray()
                : QByteArray(PQgetvalue(result, record, index),
                             PQgetlength(result, record, index));
    }

protected:
    const PGresult * const result;
    const int record;
};

class PostgresqlSqlResult : public KDbSqlResult
{
public:
    inline PostgresqlSqlResult(PostgresqlConnection *c, PGresult* r, ExecStatusType status)
        : conn(c), result(r), resultStatus(status), recordToFetch(0), recordsCount(PQntuples(r))
    {
        Q_ASSERT(c);
    }

    inline ~PostgresqlSqlResult() {
        PQclear(result);
    }

    inline KDbConnection *connection() const Q_DECL_OVERRIDE {
        return conn;
    }

    inline int fieldsCount() Q_DECL_OVERRIDE {
        return PQnfields(result);
    }

    inline KDbSqlField *field(int index) Q_DECL_OVERRIDE Q_REQUIRED_RESULT {
        return new PostgresqlSqlField(result, index);
    }

    KDbField *createField(const QString &tableName, int index) Q_DECL_OVERRIDE Q_REQUIRED_RESULT;

    inline KDbSqlRecord* fetchRecord() Q_DECL_OVERRIDE Q_REQUIRED_RESULT {
        return recordToFetch < recordsCount
               ? new PostgresqlSqlRecord(result, recordToFetch++)
               : nullptr;
    }

    inline KDbResult lastResult() Q_DECL_OVERRIDE {
        KDbResult r;
        if (resultStatus == PGRES_TUPLES_OK || resultStatus == PGRES_COMMAND_OK) {
            return r;
        }
        QByteArray msg(PQresultErrorMessage(result));
        if (msg.endsWith('\n')) {
            msg.chop(1);
        }
        r.setServerMessage(QString::fromLatin1(msg));
        r.setServerErrorCode(resultStatus);
        return r;
    }

    //! @return the oid of the last insert - only works if there was insert of 1 row
    inline quint64 lastInsertRecordId() Q_DECL_OVERRIDE {
        // InvalidOid == 0 means error
        const Oid oid = PQoidValue(result);
        return oid == 0 ? std::numeric_limits<quint64>::max() : static_cast<quint64>(oid);
    }

protected:
    //! @return a KDb type for PostgreSQL type
    //! @todo prompt user if necessary?
    KDbField::Type type(const QString& tableName, PostgresqlSqlField *field);

    PostgresqlConnection* const conn;
    PGresult* result;
    ExecStatusType resultStatus;
    int recordToFetch;
    int recordsCount;
};

#endif

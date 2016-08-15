/* This file is part of the KDE project
   Copyright (C) 2004 Martin Ellis <martin.ellis@kdemail.net>
   Copyright (C) 2004-2016 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_MYSQLCONNECTION_P_H
#define KDB_MYSQLCONNECTION_P_H

#include "KDbConnection_p.h"
#include "MysqlConnection.h"
#include <KDbSqlField>
#include <KDbSqlRecord>
#include <KDbSqlResult>

#include <mysql/mysql_version.h>
#include <mysql/mysql.h>

class KDbConnectionData;
class KDbEscapedString;
class KDbResult;

//! Internal MySQL connection data.
/*! Provides a low-level API for accessing MySQL databases, that can
    be shared by any module that needs direct access to the underlying
    database.  Used by the KDb and migration drivers.
    @todo fix the above note about migration...
 */
class MysqlConnectionInternal : public KDbConnectionInternal
{
public:
    explicit MysqlConnectionInternal(KDbConnection* connection);
    virtual ~MysqlConnectionInternal();

    //! Connects to a MySQL database
    /*! Connects to the MySQL server on host as the given user using the specified
        password.  If host is "localhost", then a socket on the local file system
        can be specified to connect to the server (several defaults will be tried if
        none is specified).  If the server is on a remote machine, then a port is
        the port that the remote server is listening on.
     */
    bool db_connect(const KDbConnectionData& data);

    //! Disconnects from the database
    bool db_disconnect();

    //! Selects a database that is about to be used
    bool useDatabase(const QString &dbName = QString());

    //! Executes query for a raw SQL statement @a sql
    bool executeSQL(const KDbEscapedString& sql);

    static QString serverResultName(MYSQL *mysql);

    void storeResult(KDbResult *result);

    MYSQL *mysql;
    bool mysql_owned; //!< true if mysql pointer should be freed on destruction
    int res; //!< result code of last operation on server
    //! Get lower_case_table_name variable value so we know if there's case sensitivity supported for table and database names
    bool lowerCaseTableNames;
    //! Server version known after successfull connection.
    //! Equal to major_version*10000 + release_level*100 + sub_version
    //! 0 if not known.
    //! See https://dev.mysql.com/doc/refman/5.7/en/mysql-get-server-version.html
    //! @todo store in Connection base class as a property or as public server info
    unsigned long serverVersion;
};

//! Internal MySQL cursor data.
/*! Provides a low-level abstraction for iterating over MySql result sets. */
class MysqlCursorData : public MysqlConnectionInternal
{
public:
    explicit MysqlCursorData(KDbConnection* connection);
    virtual ~MysqlCursorData();

    MYSQL_RES *mysqlres;
    MYSQL_ROW mysqlrow;
    unsigned long *lengths;
    qint64 numRows;
};

class MysqlSqlField : public KDbSqlField
{
public:
    inline MysqlSqlField(MYSQL_FIELD *f) : data(f) {
    }
    //! @return column name
    inline QString name() Q_DECL_OVERRIDE {
        //! @todo UTF8?
        return QString::fromLatin1(data->name);
    }
    inline int type() Q_DECL_OVERRIDE {
        return data->type;
    }
    inline int length() Q_DECL_OVERRIDE {
        return data->length;
    }
    MYSQL_FIELD *data;
};

class MysqlSqlRecord : public KDbSqlRecord
{
public:
    inline MysqlSqlRecord(MYSQL_ROW r, unsigned long* len) : record(r), lengths(len) {
    }
    inline ~MysqlSqlRecord() {
    }
    inline QString stringValue(int index) Q_DECL_OVERRIDE {
        return QString::fromUtf8(record[index], lengths[index]);
    }
    inline KDbSqlString cstringValue(int index) Q_DECL_OVERRIDE {
        return KDbSqlString(record[index], lengths[index]);
    }
    inline QByteArray toByteArray(int index) Q_DECL_OVERRIDE {
        return QByteArray(record[index], lengths[index]);
    }

protected:
    MYSQL_ROW record;
    unsigned long* lengths;
};

class MysqlSqlResult : public KDbSqlResult
{
public:
    inline MysqlSqlResult(MysqlConnection *c, MYSQL_RES *d)
        : conn(c), data(d), fields(nullptr)
    {
        Q_ASSERT(c);
        Q_ASSERT(d);
    }

    inline ~MysqlSqlResult() {
        mysql_free_result(data);
    }

    inline int fieldsCount() Q_DECL_OVERRIDE Q_REQUIRED_RESULT {
        return mysql_num_fields(data);
    }

    inline KDbSqlField *field(int index) Q_DECL_OVERRIDE Q_REQUIRED_RESULT {
        if (!fields) {
            fields = mysql_fetch_fields(data);
        }
        return new MysqlSqlField(fields + index);
    }

    KDbField *createField(const QString &tableName, int index) Q_DECL_OVERRIDE Q_REQUIRED_RESULT;

    inline KDbSqlRecord* fetchRecord() Q_DECL_OVERRIDE Q_REQUIRED_RESULT {
        MYSQL_ROW row = mysql_fetch_row(data);
        if (!row) {
            return nullptr;
        }
        unsigned long* lengths = mysql_fetch_lengths(data);
        return new MysqlSqlRecord(row, lengths);
    }

    inline KDbResult lastResult() Q_DECL_OVERRIDE {
        KDbResult res;
        const int err = mysql_errno(conn->d->mysql);
        if (err != 0) {
            res.setCode(ERR_OTHER);
            res.setServerErrorCode(err);
        }
        return res;
    }

protected:
    //! @return a KDb type for MySQL type
    //! @todo prompt user if necessary?
    KDbField::Type type(const QString& tableName, MysqlSqlField *field);

    //! @return a KDb BLOB-related type for MySQL type
    /*! Distinguishes between a BLOB and a TEXT types.
        MySQL uses the same field type to identify BLOB and TEXT fields.
        This method queries the server to find out if a field is a binary
        field or a text field. It also considers the length of CHAR and VARCHAR
        fields to see whether Text or LongText is the appropriate Kexi field type.
        Assumes fld is a CHAR, VARCHAR, one of the BLOBs or TEXTs.
        Returns KDbField::Text, KDbField::LongText or KDbField::BLOB. */
    KDbField::Type blobType(const QString& tableName, MysqlSqlField *field);

    MysqlConnection * const conn;
    MYSQL_RES * const data;
    MYSQL_FIELD *fields;
};

#endif

/* This file is part of the KDE project
   Copyright (C) 2003 Adam Pigg <adam@piggz.co.uk>
   Copyright (C) 2010 Jarosław Staniek <staniek@kde.org>

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

#ifndef POSTGRESQLCONNECTION_H
#define POSTGRESQLCONNECTION_H

#include <QStringList>

#include <Predicate/Connection.h>

namespace Predicate
{

class PostgresqlConnectionInternal;

//! @internal
class PostgresqlTransactionData : public TransactionData
{
public:
    PostgresqlTransactionData(Connection *conn);
    ~PostgresqlTransactionData();
};

class PostgresqlConnection : public Connection
{
public:
    virtual ~PostgresqlConnection();

    virtual Cursor* prepareQuery(const EscapedString& statement, uint cursor_options = 0);
    virtual Cursor* prepareQuery(QuerySchema* query, uint cursor_options = 0);

    virtual PreparedStatementInterface* prepareStatementInternal();

    /*! Connection-specific string escaping.  */
    virtual EscapedString escapeString(const QString& str) const;
    virtual EscapedString escapeString(const QByteArray& str) const;

protected:
    /*! Used by driver */
    PostgresqlConnection(Driver *driver, const ConnectionData& connData);

    virtual bool drv_isDatabaseUsed() const;
    virtual bool drv_connect(Predicate::ServerVersionInfo* version);
    virtual bool drv_disconnect();
    virtual bool drv_getDatabasesList(QStringList* list);
    virtual bool drv_createDatabase(const QString &dbName = QString());
    //! Uses database. Note that if data().localSocketFileName() is not empty,
    //! only directory path is used for connecting; the local socket's filename stays ".s.PGSQL.5432".
    virtual bool drv_useDatabase(const QString &dbName = QString(), bool *cancelled = 0,
                                 MessageHandler* msgHandler = 0);
    virtual bool drv_closeDatabase();
    virtual bool drv_dropDatabase(const QString &dbName = QString());
    virtual bool drv_executeSQL(const EscapedString& statement);
    virtual quint64 drv_lastInsertRecordId();

    //! Implemented for Resultable
    virtual QString serverResultName() const;
//    virtual void drv_clearServerResult();

//TODO: move this somewhere to low level class (MIGRATION?)
    virtual bool drv_getTablesList(QStringList* list);
//TODO: move this somewhere to low level class (MIGRATION?)
    virtual bool drv_containsTable(const QString &tableName);

//pred    virtual TransactionData* drv_beginTransaction();
//pred    virtual bool drv_commitTransaction(TransactionData *);
//pred    virtual bool drv_rollbackTransaction(TransactionData *);

    PostgresqlConnectionInternal *d;

    //! temporary solution for executeSQL()...
//pred    PostgresqlTransactionData *m_trans;

    friend class PostgresqlDriver;
    friend class PostgresqlCursorData;
    friend class PostgresqlTransactionData;
};
}
#endif

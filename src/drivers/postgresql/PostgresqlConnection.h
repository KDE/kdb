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

#include "KDbConnection.h"

class PostgresqlConnectionInternal;

//! @internal
class PostgresqlTransactionData : public KDbTransactionData
{
public:
    explicit PostgresqlTransactionData(KDbConnection *conn);
    ~PostgresqlTransactionData();
};

class PostgresqlConnection : public KDbConnection
{
public:
    virtual ~PostgresqlConnection();

    //! @return a new query based on a query statement
    virtual KDbCursor* prepareQuery(const KDbEscapedString& sql, uint cursor_options = 0);

    //! @return a new query based on a query object
    virtual KDbCursor* prepareQuery(KDbQuerySchema* query, uint cursor_options = 0);

    virtual KDbPreparedStatementInterface* prepareStatementInternal();

    /*! Connection-specific string escaping.  */
    virtual KDbEscapedString escapeString(const QString& str) const;
    virtual KDbEscapedString escapeString(const QByteArray& str) const;

protected:
    /*! Used by driver */
    PostgresqlConnection(KDbDriver *driver, const KDbConnectionData& connData);

    //! @return true if currently connected to a database, ignoring the m_is_connected flag.
    virtual bool drv_isDatabaseUsed() const;
    //! Noop: we tell we are connected, but we wont actually connect until we use a database.
    virtual bool drv_connect();
    virtual bool drv_getServerVersion(KDbServerVersionInfo* version);
    //! Noop: we tell we have disconnected, but it is actually handled by closeDatabase.
    virtual bool drv_disconnect();
    //! @return a list of database names
    virtual bool drv_getDatabasesList(QStringList* list);
    //! Create a new database
    virtual bool drv_createDatabase(const QString &dbName = QString());
    //! Uses database. Note that if data().localSocketFileName() is not empty,
    //! only directory path is used for connecting; the local socket's filename stays ".s.PGSQL.5432".
    virtual bool drv_useDatabase(const QString &dbName = QString(), bool *cancelled = 0,
                                 KDbMessageHandler* msgHandler = 0);
    //! Close the database connection
    virtual bool drv_closeDatabase();
    //! Drops the given database
    virtual bool drv_dropDatabase(const QString &dbName = QString());
    //! Executes an SQL statement
    virtual bool drv_executeSQL(const KDbEscapedString& sql);
    //! @return the oid of the last insert - only works if sql was insert of 1 row
    virtual quint64 drv_lastInsertRecordId();

    //! Implemented for KDbResultable
    virtual QString serverResultName() const;

//! @todo move this somewhere to low level class (MIGRATION?)
    virtual bool drv_getTablesList(QStringList* list);
//! @todo move this somewhere to low level class (MIGRATION?)
    virtual bool drv_containsTable(const QString &tableName);

    PostgresqlConnectionInternal *d;

    friend class PostgresqlDriver;
    friend class PostgresqlCursorData;
    friend class PostgresqlTransactionData;
};

#endif

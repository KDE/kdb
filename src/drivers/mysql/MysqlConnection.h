/* This file is part of the KDE project
   Copyright (C) 2002 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2003 Joseph Wenninger<jowenn@kde.org>
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

#ifndef KDB_MYSQLCONNECTION_H
#define KDB_MYSQLCONNECTION_H

#include <QStringList>

#include "KDbConnection.h"

class MysqlConnectionInternal;

/*! @short Provides database connection, allowing queries and data modification.
*/
class MysqlConnection : public KDbConnection
{
    Q_DECLARE_TR_FUNCTIONS(MysqlConnection)
public:
    virtual ~MysqlConnection();

    KDbCursor* prepareQuery(const KDbEscapedString& sql,
                            KDbCursor::Options options = KDbCursor::Option::None) Q_DECL_OVERRIDE Q_REQUIRED_RESULT;
    KDbCursor* prepareQuery(KDbQuerySchema* query,
                            KDbCursor::Options options = KDbCursor::Option::None) Q_DECL_OVERRIDE Q_REQUIRED_RESULT;

    KDbPreparedStatementInterface* prepareStatementInternal() Q_DECL_OVERRIDE Q_REQUIRED_RESULT;

protected:
    /*! Used by driver */
    MysqlConnection(KDbDriver *driver, const KDbConnectionData& connData,
                    const KDbConnectionOptions &options);

    virtual bool drv_connect();
    virtual bool drv_getServerVersion(KDbServerVersionInfo* version);
    virtual bool drv_disconnect();
    virtual bool drv_getDatabasesList(QStringList* list);
    //! reimplemented using "SHOW DATABASES LIKE..." because MySQL stores db names in lower case.
    virtual bool drv_databaseExists(const QString &dbName, bool ignoreErrors = true);
    virtual bool drv_createDatabase(const QString &dbName = QString());
    virtual bool drv_useDatabase(const QString &dbName = QString(), bool *cancelled = 0,
                                 KDbMessageHandler* msgHandler = 0);
    virtual bool drv_closeDatabase();
    virtual bool drv_dropDatabase(const QString &dbName = QString());
    virtual KDbSqlResult* drv_executeSQL(const KDbEscapedString& sql) Q_REQUIRED_RESULT;
    virtual bool drv_executeVoidSQL(const KDbEscapedString& sql);

    //! Implemented for KDbResultable
    virtual QString serverResultName() const;

//! @todo move this somewhere to low level class (MIGRATION?)
    virtual tristate drv_containsTable(const QString &tableName);

    void storeResult();

    MysqlConnectionInternal* const d;

    friend class MysqlDriver;
    friend class MysqlCursorData;
    friend class MysqlSqlResult;
private:
    Q_DISABLE_COPY(MysqlConnection)
};

#endif

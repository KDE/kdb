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
    ~MysqlConnection() override;

    Q_REQUIRED_RESULT KDbCursor *prepareQuery(const KDbEscapedString &sql,
                                              KDbCursor::Options options
                                              = KDbCursor::Option::None) override;
    Q_REQUIRED_RESULT KDbCursor *prepareQuery(KDbQuerySchema *query,
                                              KDbCursor::Options options
                                              = KDbCursor::Option::None) override;

    Q_REQUIRED_RESULT KDbPreparedStatementInterface *prepareStatementInternal() override;

protected:
    /*! Used by driver */
    MysqlConnection(KDbDriver *driver, const KDbConnectionData& connData,
                    const KDbConnectionOptions &options);

    bool drv_connect() override;
    bool drv_getServerVersion(KDbServerVersionInfo* version) override;
    bool drv_disconnect() override;
    bool drv_getDatabasesList(QStringList* list) override;
    //! reimplemented using "SHOW DATABASES LIKE..." because MySQL stores db names in lower case.
    bool drv_databaseExists(const QString &dbName, bool ignoreErrors = true) override;
    bool drv_createDatabase(const QString &dbName = QString()) override;
    bool drv_useDatabase(const QString &dbName = QString(), bool *cancelled = nullptr,
                                 KDbMessageHandler* msgHandler = nullptr) override;
    bool drv_closeDatabase() override;
    bool drv_dropDatabase(const QString &dbName = QString()) override;
    Q_REQUIRED_RESULT KDbSqlResult *drv_prepareSql(const KDbEscapedString &sql) override;
    bool drv_executeSql(const KDbEscapedString& sql) override;

    //! Implemented for KDbResultable
    QString serverResultName() const override;

//! @todo move this somewhere to low level class (MIGRATION?)
    tristate drv_containsTable(const QString &tableName) override;

    void storeResult();

    MysqlConnectionInternal* const d;

    friend class MysqlDriver;
    friend class MysqlCursorData;
    friend class MysqlSqlResult;
private:
    Q_DISABLE_COPY(MysqlConnection)
};

#endif

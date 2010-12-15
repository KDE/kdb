/* This file is part of the KDE project
   Copyright (C) 2002 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2003 Joseph Wenninger<jowenn@kde.org>
   Copyright (C) 2004-2010 Jarosław Staniek <staniek@kde.org>

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

#ifndef MYSQLCONNECTION_H
#define MYSQLCONNECTION_H

#include <qstringlist.h>

#include <Predicate/Connection.h>
#include "MysqlCursor.h"

namespace Predicate
{

class MysqlConnectionInternal;

/*! @short Provides database connection, allowing queries and data modification.
*/
class MysqlConnection : public Connection
{
public:
    virtual ~MysqlConnection();

    virtual Cursor* prepareQuery(const EscapedString& statement, uint cursor_options = 0);
    virtual Cursor* prepareQuery(QuerySchema* query, uint cursor_options = 0);

    virtual PreparedStatementInterface* prepareStatementInternal();

protected:
    /*! Used by driver */
    MysqlConnection(Driver *driver, const ConnectionData& connData);

    virtual bool drv_connect(Predicate::ServerVersionInfo* version);
    virtual bool drv_disconnect();
    virtual bool drv_getDatabasesList(QStringList* list);
    //! reimplemented using "SHOW DATABASES LIKE..." because MySQL stores db names in lower case.
    virtual bool drv_databaseExists(const QString &dbName, bool ignoreErrors = true);
    virtual bool drv_createDatabase(const QString &dbName = QString());
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

    MysqlConnectionInternal* d;

    friend class MysqlDriver;
    friend class MysqlCursorData;
};

}

#endif

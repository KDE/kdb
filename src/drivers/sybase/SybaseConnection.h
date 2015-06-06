/* This file is part of the KDE project
   Copyright (C) 2007 Sharan Rao <sharanrao@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this program; see the file COPYING. If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef SYBASECONNECTION_H
#define SYBASECONNECTION_H

#include <qstringlist.h>

#include "KDbConnection.h"
#include "SybaseCursor.h"

class SybaseConnectionInternal;

/*! @short Provides database connection, allowing queries and data modification.
*/
class SybaseConnection : public KDbConnection
{
public:
    virtual ~SybaseConnection();

    virtual KDbCursor* prepareQuery(const KDbEscapedString& sql, uint cursor_options = 0);
    virtual KDbCursor* prepareQuery(KDbQuerySchema* query, uint cursor_options = 0);

    virtual KDbPreparedStatement prepareStatement(KDbPreparedStatement::StatementType type,
            KDbFieldList* fields);

protected:

    /*! Used by driver */
    SybaseConnection(KDbDriver *driver, const ConnectionData& connData);

    virtual bool drv_connect(KDbServerVersionInfo* version);
    virtual bool drv_disconnect();
    virtual bool drv_getDatabasesList(QStringList* list);
    virtual bool drv_createDatabase(const QString &dbName = QString());
    virtual bool drv_useDatabase(const QString &dbName = QString(), bool *cancelled = 0,
                                 KDbMessageHandler* msgHandler = 0);
    virtual bool drv_closeDatabase();
    virtual bool drv_dropDatabase(const QString &dbName = QString());
    virtual bool drv_executeSQL(const KDbEscapedString& sql);
    virtual quint64 drv_lastInsertRecordId();

    //! Implemented for KDbResultable
    virtual QString serverResultName() const;
//    virtual void drv_clearServerResult();

    //! @todo move this somewhere to low level class (MIGRATION?)
    virtual bool drv_getTablesList(QStringList* list);
    //! @todo move this somewhere to low level class (MIGRATION?)
    virtual bool drv_containsTable(const QString &tableName);

    virtual bool drv_beforeInsert(const QString& table, KDbFieldList* fields);
    virtual bool drv_afterInsert(const QString& table, KDbFieldList* fields);

    virtual bool drv_beforeUpdate(const QString& table, KDbFieldList* fields);
    virtual bool drv_afterUpdate(const QString& table, KDbFieldList* fields);

    SybaseConnectionInternal* d;

    friend class SybaseDriver;
    friend class SybaseCursor;
};

#endif

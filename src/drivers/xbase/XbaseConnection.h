/* This file is part of the KDE project
   Copyright (C) 2008 Sharan Rao <sharanrao@gmail.com>

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

#ifndef XBASECONNECTION_H
#define XBASECONNECTION_H

#include <QStringList>

#include "KDbConnection.h"
#include "XbaseCursor.h"

class xBaseConnectionInternal;

/*! @short Provides database connection, allowing queries and data modification.
*/
class xBaseConnection : public KDbConnection
{
public:
    virtual ~xBaseConnection();

    Q_REQUIRED_RESULT KDbCursor *prepareQuery(const KDbEscapedString &sql,
                                              int cursor_options = 0) override;
    Q_REQUIRED_RESULT KDbCursor *prepareQuery(KDbQuerySchema *query,
                                              int cursor_options = 0) override;

    //! @todo returns @c nullptr for now
    Q_REQUIRED_RESULT KDbPreparedStatementInterface *prepareStatementInternal() override;

protected:

    /*! Used by driver */
    xBaseConnection(KDbDriver *driver, KDbDriver* internalDriver, const KDbConnectionData& connData);

    virtual bool drv_connect(KDbServerVersionInfo* version);
    virtual bool drv_disconnect();
    virtual bool drv_getDatabasesList(QStringList* list);
    virtual bool drv_createDatabase( const QString &dbName = QString() );
    virtual bool drv_useDatabase( const QString &dbName = QString(), bool *cancelled = 0,
      KDbMessageHandler* msgHandler = 0 );
    virtual bool drv_closeDatabase();
    virtual bool drv_dropDatabase(const QString &dbName = QString());
    virtual bool drv_executeSql(const KDbEscapedString& sql);
    virtual quint64 drv_lastInsertRecordId();

    //! Implemented for KDbResultable
    virtual QString serverResultName() const;
//    virtual void drv_clearServerResult();

    //! @todo move this somewhere to low level class (MIGRATION?)
    virtual bool drv_getTablesList(QStringList* list);
    //! @todo move this somewhere to low level class (MIGRATION?)
    virtual bool drv_containsTable(const QString &tableName);

    xBaseConnectionInternal* d;

    friend class xBaseDriver;
    friend class xBaseCursor;
};

#endif

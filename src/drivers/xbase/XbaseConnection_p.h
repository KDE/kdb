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

#ifndef KDB_XBASECLIENT_P_H
#define KDB_XBASECLIENT_P_H

#include <QPointer>

#include "KDbConnection_p.h"

class KDbConnectionData;

//! Internal xBase connection data.
/*! Provides a low-level API for accessing xBase databases, that can
be shared by any module that needs direct access to the underlying
database.  Used by the KDb drivers.
*/
class xBaseConnectionInternal : public KDbConnectionInternal
{
  public:
    xBaseConnectionInternal(KDbConnection* connection, KDbDriver* internalDriver);
    virtual ~xBaseConnectionInternal();

    //! Connects to a xBase database
    bool db_connect(const KDbConnectionData& data);

    //! Disconnects from the database
    bool db_disconnect(const KDbConnectionData& data);

    //! Selects a database that is about to be used
    bool useDatabase(const QString &dbName = QString());

    //! Execute SQL statement on the database
    bool executeSql(const KDbEscapedString& sql);

    //! Stores last operation's result
    virtual void storeResult();

    QPointer<KDbDriver> internalDriver;
    QPointer<KDbConnection> internalConn;
    QString tempDatabase;

    QHash<QString,QString> dbMap;

    QString errmsg; //!< server-specific message of last operation
    int res; //!< result code of last operation on server
};

#endif

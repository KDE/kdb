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

#ifndef KDB_SYBASECLIENT_P_H
#define KDB_SYBASECLIENT_P_H

#include <QMap>

#include "KDbConnection_p.h"

//#include <config.h>
#include <sqlfront.h>
#include <sqldb.h>

class KDbConnectionData;

//! Internal Sybase connection data.
/*! Provides a low-level API for accessing Sybase databases, that can
    be shared by any module that needs direct access to the underlying
    database.  Used by the KDb and KexiMigration drivers.
 */
class SybaseConnectionInternal : public KDbConnectionInternal
{

public:
    explicit SybaseConnectionInternal(KDbConnection* connection);
    virtual ~SybaseConnectionInternal();

    //! Connects to a Sybase database
    /*! Connects to the Sybase server on host as the given user using the specified
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

    //! Executes query for a raw SQL statement @a sql on the database
    bool executeSql(const KDbEscapedString& sql);

    //! Stores last operation's result
    virtual void storeResult();

    //! Escapes a table, database or column name
    QString escapeIdentifier(const QString& str) const;

    // message handler called by call back function
    void messageHandler(DBINT msgno, int msgstate, int severity, char* msgtext
                        , char* srvname, char* procname, int line);

    // dbProcess-KDbConnection map
    static QMap<DBPROCESS*, SybaseConnectionInternal*> dbProcessConnectionMap;

    // Server specific stuff
    DBPROCESS *dbProcess;

    bool sybase_owned; //!< true if dbprocess should be closed on destruction
    QString errmsg; //!< server-specific message of last operation
    int res; //!< result code of last operation on server

};


//! Internal Sybase cursor data.
/*! Provides a low-level abstraction for iterating over Sybase result sets. */
class SybaseCursorData : public SybaseConnectionInternal
{
public:
    explicit SybaseCursorData(KDbConnection* connection);
    virtual ~SybaseCursorData();

    //unsigned long *lengths;
    unsigned long numRows;
};

#endif

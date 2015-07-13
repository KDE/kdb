/* This file is part of the KDE project
   Copyright (C) 2004 Martin Ellis <martin.ellis@kdemail.net>

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

#ifndef KDB_MYSQLCLIENT_P_H
#define KDB_MYSQLCLIENT_P_H

#include "KDbConnection_p.h"

#include <QString>

#ifdef Q_OS_WIN
#include <my_Global.h>
#endif
#include <mysql_version.h>
#include <mysql.h>

typedef struct st_mysql MYSQL;
#undef bool

class KDbConnectionData;
class KDbEscapedString;

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

    //! Escapes a table, database or column name
    QString escapeIdentifier(const QString& str) const;

    static QString serverResultName(MYSQL *mysql);

    MYSQL *mysql;
    bool mysql_owned; //!< true if mysql pointer should be freed on destruction
    int res; //!< result code of last operation on server
    //! Get lower_case_table_name variable value so we know if there's case sensitivity supported for table and database names
    bool lowerCaseTableNames;
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

#endif

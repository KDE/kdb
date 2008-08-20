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

#ifndef PREDICATE_MYSQLCLIENT_P_H
#define PREDICATE_MYSQLCLIENT_P_H

#include <Predicate/Connection_p.h>

#ifdef Q_WS_WIN
#include <my_Global.h>
#endif
#include <mysql_version.h>
#include <mysql.h>

typedef struct st_mysql MYSQL;
#undef bool

#ifdef MYSQLMIGRATE_H
#define NAMESPACE KexiMigration
#else
#define NAMESPACE KexiDB
#endif

namespace Predicate
{
class ConnectionData;
}

namespace NAMESPACE
{

//! Internal MySQL connection data.
/*! Provides a low-level API for accessing MySQL databases, that can
    be shared by any module that needs direct access to the underlying
    database.  Used by the Predicate and KexiMigration drivers.
 */
class MySqlConnectionInternal : public Predicate::ConnectionInternal
{
public:
    MySqlConnectionInternal(Predicate::Connection* connection);
    virtual ~MySqlConnectionInternal();

    //! Connects to a MySQL database
    bool db_connect(const Predicate::ConnectionData& data);

    //! Disconnects from the database
    bool db_disconnect();

    //! Selects a database that is about to be used
    bool useDatabase(const QString &dbName = QString());

    //! Execute SQL statement on the database
    bool executeSQL(const QString& statement);

    //! Stores last operation's result
    virtual void storeResult();

    //! Escapes a table, database or column name
    QString escapeIdentifier(const QString& str) const;

    MYSQL *mysql;
    bool mysql_owned; //!< true if mysql pointer should be freed on destruction
    QString errmsg; //!< server-specific message of last operation
    int res; //!< result code of last operation on server
};


//! Internal MySQL cursor data.
/*! Provides a low-level abstraction for iterating over MySql result sets. */
class MySqlCursorData : public MySqlConnectionInternal
{
public:
    MySqlCursorData(Predicate::Connection* connection);
    virtual ~MySqlCursorData();

    MYSQL_RES *mysqlres;
    MYSQL_ROW mysqlrow;
    unsigned long *lengths;
    unsigned long numRows;
};

}

#endif

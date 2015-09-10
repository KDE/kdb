/* This file is part of the KDE project
   Copyright (C) 2004-2012 Jarosław Staniek <staniek@kde.org>
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

#include "MysqlConnection_p.h"
#include "MysqlConnection.h"
#include "mysql_debug.h"

#include "KDbConnectionData.h"

#include <QByteArray>
#include <QStringList>
#include <QFile>

MysqlConnectionInternal::MysqlConnectionInternal(KDbConnection* connection)
        : KDbConnectionInternal(connection)
        , mysql(0)
        , mysql_owned(true)
        , res(0)
        , lowerCaseTableNames(false)
        , serverVersion(0)
{
}

MysqlConnectionInternal::~MysqlConnectionInternal()
{
    if (mysql_owned && mysql) {
        db_disconnect();
    }
}

bool MysqlConnectionInternal::db_connect(const KDbConnectionData& data)
{
    if (!(mysql = mysql_init(mysql)))
        return false;

    mysqlDebug();
    QByteArray localSocket;
    QByteArray hostName = QFile::encodeName(data.hostName());
    if (hostName.isEmpty() || 0 == qstricmp(hostName.constData(), "localhost")) {
        if (data.useLocalSocketFile()) {
            if (data.localSocketFileName().isEmpty()) {
                //! @todo move the list of default sockets to a generic method
                QStringList sockets;
#ifndef Q_OS_WIN
                sockets
                    << QLatin1String("/var/lib/mysql/mysql.sock")
                    << QLatin1String("/var/run/mysqld/mysqld.sock")
                    << QLatin1String("/var/run/mysql/mysql.sock")
                    << QLatin1String("/tmp/mysql.sock");

                foreach(const QString& socket, sockets) {
                    if (QFile(socket).exists()) {
                        localSocket = socket.toLatin1();
                        break;
                    }
                }
#endif
            } else
                localSocket = QFile::encodeName(data.localSocketFileName());
        } else {
            //we're not using local socket
            hostName = "127.0.0.1"; //this will force mysql to connect to localhost
        }
    }

    /*! @todo is latin1() encoding here valid? what about using UTF for passwords? */
    const QByteArray userName(data.userName().toLatin1());
    const QByteArray password(data.password().toLatin1());
    int client_flag = 0; //!< @todo support client_flag?
    if (mysql_real_connect(mysql, hostName.isEmpty() ? 0 : hostName.constData(),
                           data.userName().isEmpty() ? 0 : userName.constData(),
                           data.password().isNull() ? 0 : password.constData(),
                           0,
                           data.port(), localSocket.isEmpty() ? 0 : localSocket.constData(),
                           client_flag))
    {
        serverVersion = mysql_get_server_version(mysql);
        return true;
    }
    return false;
}

bool MysqlConnectionInternal::db_disconnect()
{
    mysql_close(mysql);
    mysql = 0;
    serverVersion = 0;
    mysqlDebug();
    return true;
}

bool MysqlConnectionInternal::useDatabase(const QString &dbName)
{
//! @todo is here escaping needed?
    if (!executeSQL(KDbEscapedString("USE ") + escapeIdentifier(dbName))) {
        return false;
    }
    if (!executeSQL(KDbEscapedString("SET SESSION sql_mode='TRADITIONAL'"))) {
        // needed to turn warnings about trimming string values into SQL errors
        return false;
    }
    return true;
}

bool MysqlConnectionInternal::executeSQL(const KDbEscapedString& sql)
{
    return 0 == mysql_real_query(mysql, sql.constData(), sql.length());
}

QString MysqlConnectionInternal::escapeIdentifier(const QString& str) const
{
    return QString(str).replace(QLatin1Char('`'), QLatin1Char('\''));
}

//static
QString MysqlConnectionInternal::serverResultName(MYSQL *mysql)
{
    //! @todo use mysql_stmt_sqlstate() for prepared statements
    return QString::fromLatin1(mysql_sqlstate(mysql));
}

//--------------------------------------

MysqlCursorData::MysqlCursorData(KDbConnection* connection)
        : MysqlConnectionInternal(connection)
        , mysqlres(0)
        , mysqlrow(0)
        , lengths(0)
        , numRows(0)
{
    mysql_owned = false;
    mysql = static_cast<MysqlConnection*>(connection)->d->mysql;
}

MysqlCursorData::~MysqlCursorData()
{
}


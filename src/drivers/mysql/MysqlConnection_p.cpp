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

#include <QByteArray>
#include <QStringList>
#include <QFile>


#include "MysqlConnection_p.h"
#include "MysqlConnection.h"

#include "KDbConnectionData.h"


MysqlConnectionInternal::MysqlConnectionInternal(KDbConnection* connection)
        : KDbConnectionInternal(connection)
        , mysql(0)
        , mysql_owned(true)
        , res(0)
        , lowerCaseTableNames(false)
{
}

MysqlConnectionInternal::~MysqlConnectionInternal()
{
    if (mysql_owned && mysql) {
        mysql_close(mysql);
        mysql = 0;
    }
}

void MysqlConnectionInternal::storeResult()
{
    setServerResultCode(mysql_errno(mysql));
    setServerMessage(QLatin1String(mysql_error(mysql)));
}

bool MysqlConnectionInternal::db_connect(const ConnectionData& data)
{
    if (!(mysql = mysql_init(mysql)))
        return false;

    KDbDrvDbg;
    QByteArray localSocket;
    QString hostName = data.hostName();
    if (   hostName.isEmpty()
        || 0 == QString::compare(hostName, QLatin1String("localhost"), Qt::CaseInsensitive))
    {
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
            hostName = QLatin1String("127.0.0.1"); //this will force mysql to connect to localhost
        }
    }

    /*! @todo is latin1() encoding here valid? what about using UTF for passwords? */
    QByteArray pwd(data.password().isNull() ? QByteArray() : data.password().toLatin1());
    mysql_real_connect(mysql, hostName.toLatin1(), data.userName().toLatin1(),
                       pwd.constData(), 0, data.port(), localSocket, 0);
    if (mysql_errno(mysql) == 0)
        return true;

    storeResult(); //store error msg, if any - can be destroyed after disconnect()
    db_disconnect();
    return false;
}

bool MysqlConnectionInternal::db_disconnect()
{
    mysql_close(mysql);
    mysql = 0;
    KDbDrvDbg;
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
    if (mysql_real_query(mysql, sql.constData(), sql.length()) == 0)
        return true;

    storeResult();
    return false;
}

QString MysqlConnectionInternal::escapeIdentifier(const QString& str) const
{
    return QString(str).replace(QLatin1Char('`'), QLatin1Char('\''));
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


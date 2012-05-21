/* This file is part of the KDE project
   Copyright (C) 2005 Adam Pigg <adam@piggz.co.uk>
   Copyright (C) 2010 Jarosław Staniek <staniek@kde.org>

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

#include "PostgresqlConnection_p.h"
#include "PostgresqlConnection.h"
#include <QtDebug>

using namespace Predicate;

PostgresqlConnectionInternal::PostgresqlConnectionInternal(Connection *conn)
        : ConnectionInternal(conn)
        , conn(0)
        , unicode(true) // will be set in PostgresqlConnection::drv_useDatabase()
        , res(0)
{
    setServerResultCode(-1);
    escapingBuffer.reserve(0x8000);
}

PostgresqlConnectionInternal::~PostgresqlConnectionInternal()
{

}

void PostgresqlConnectionInternal::storeResult()
{
    QString msg = QLatin1String(PQerrorMessage(conn));
    if (msg.endsWith(QLatin1Char('\n'))) {
        msg.chop(1);
    }
    setServerMessage(msg);
/*    if (d->res) {
        setServerResultCode(PQresultStatus(d->res));
    }
    else {
        setServerResultCode(-1);
    }*/
}

bool PostgresqlConnectionInternal::executeSQL(const EscapedString& statement, ExecStatusType expectedStatus)
{
    if (res) { // for sanity
        PQclear(res);
    }
//! @todo consider using binary mode with PQexecParams()
    res = PQexec(conn, statement.toByteArray().constData());
    if (PQresultStatus(res) != expectedStatus) {
        setServerResultCode(PQresultStatus(res));
        storeResult();
        PQclear(res);
        return false;
    }
    else {
        setServerResultCode(-1);
    }
    return true;
}

//--------------------------------------

PostgresqlCursorData::PostgresqlCursorData(Connection* connection)
        : PostgresqlConnectionInternal(connection)
{
    conn = static_cast<PostgresqlConnection*>(connection)->d->conn;
}

PostgresqlCursorData::~PostgresqlCursorData()
{
}

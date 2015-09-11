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

PostgresqlConnectionInternal::PostgresqlConnectionInternal(KDbConnection *_conn)
        : KDbConnectionInternal(_conn)
        , conn(0)
        , unicode(true) // will be set in PostgresqlConnection::drv_useDatabase()
        , res(0)
{
    escapingBuffer.reserve(0x8000);
}

PostgresqlConnectionInternal::~PostgresqlConnectionInternal()
{
}

//static
QString PostgresqlConnectionInternal::serverResultName(int resultCode)
{
    return QString::fromLatin1(PQresStatus(static_cast<ExecStatusType>(resultCode)));
}

void PostgresqlConnectionInternal::storeResult(KDbResult *result)
{
    QByteArray msg(PQerrorMessage(conn));
    if (msg.endsWith('\n')) {
        msg.chop(1);
    }
    result->setServerMessage(QString::fromLatin1(msg));
    if (res) {
        result->setServerErrorCode(PQresultStatus(res));
        PQclear(res);
        res = 0;
    }
}

bool PostgresqlConnectionInternal::executeSQL(const KDbEscapedString& sql)
{
//! @todo consider using binary mode with PQexecParams()
    res = PQexec(conn, sql.toByteArray().constData());
    ExecStatusType resultType = PQresultStatus(res);
    return resultType == PGRES_COMMAND_OK || resultType == PGRES_TUPLES_OK;
}

//--------------------------------------

PostgresqlCursorData::PostgresqlCursorData(KDbConnection* connection)
        : PostgresqlConnectionInternal(connection)
{
    conn = static_cast<PostgresqlConnection*>(connection)->d->conn;
}

PostgresqlCursorData::~PostgresqlCursorData()
{
}

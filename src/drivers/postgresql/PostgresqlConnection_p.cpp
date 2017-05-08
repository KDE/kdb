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

PostgresqlConnectionInternal::PostgresqlConnectionInternal(KDbConnection *_conn)
        : KDbConnectionInternal(_conn)
        , conn(nullptr)
        , unicode(true) // will be set in PostgresqlConnection::drv_useDatabase()
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

void PostgresqlConnectionInternal::storeResultAndClear(KDbResult *result, PGresult **pgResult,
                                                       ExecStatusType execStatus)
{
    QByteArray msg(PQresultErrorMessage(*pgResult));
    if (msg.endsWith('\n')) {
        msg.chop(1);
    }
    result->setServerMessage(QString::fromLatin1(msg));
    if (*pgResult) {
        result->setServerErrorCode(execStatus);
        PQclear(*pgResult);
        *pgResult = nullptr;
    }
}

void PostgresqlConnectionInternal::storeResult(KDbResult *result)
{
    QByteArray msg(PQerrorMessage(conn));
    if (msg.endsWith('\n')) {
        msg.chop(1);
    }
    result->setServerMessage(QString::fromLatin1(msg));
}

PGresult* PostgresqlConnectionInternal::executeSql(const KDbEscapedString& sql)
{
//! @todo consider using binary mode with PQexecParams()
    return PQexec(conn, sql.toByteArray().constData());
}

//--------------------------------------

PostgresqlCursorData::PostgresqlCursorData(KDbConnection* connection)
        : PostgresqlConnectionInternal(connection), res(nullptr), resultStatus(PGRES_FATAL_ERROR)
{
    conn = static_cast<PostgresqlConnection*>(connection)->d->conn;
}

PostgresqlCursorData::~PostgresqlCursorData()
{
}

KDbField* PostgresqlSqlResult::createField(const QString &tableName, int index)
{
    Q_UNUSED(tableName)
    QScopedPointer<PostgresqlSqlField> f(static_cast<PostgresqlSqlField*>(field(index)));
    if (!f) {
        return nullptr;
    }
    const QString caption(f->name());
    QString realFieldName(KDb::stringToIdentifier(caption.toLower()));
    const PostgresqlDriver *pgdriver = static_cast<const PostgresqlDriver*>(conn->driver());
    const KDbField::Type kdbType = pgdriver->pgsqlToKDbType(
                PQftype(result, index), PQfmod(result, index), nullptr);
    KDbField *kdbField = new KDbField(realFieldName, kdbType);
    kdbField->setCaption(caption);
    if (KDbField::isTextType(kdbType)) {
        const int len = f->length();
        if (len != -1) {
            kdbField->setMaxLength(len);
        }
    }
    //! @todo use information_schema.table_constraints to get constraints
    //copyConstraints(...);
    //! @todo use information_schema.columns to get options
    //copyOptions(...);
    return kdbField;
}

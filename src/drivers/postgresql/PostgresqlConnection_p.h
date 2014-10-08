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

#ifndef POSTGRESQLSQLCONNECTIONINTERNAL_H
#define POSTGRESQLSQLCONNECTIONINTERNAL_H

#include <Predicate/Private/Connection>

#include <libpq-fe.h>

namespace Predicate
{
class PostgresqlConnectionInternal : public ConnectionInternal
{
public:
    explicit PostgresqlConnectionInternal(Connection *conn);

    virtual ~PostgresqlConnectionInternal();

    //! Execute SQL statement on the database
    //! @a expectedStatus can be PGRES_COMMAND_OK for command
    //! not returning tuples, e.g. CREATE and PGRES_TUPLES_OK for command returning tuples, e.g. SELECT.
    bool executeSQL(const EscapedString& statement, ExecStatusType expectedStatus);

    //! stores last result's message
    virtual void storeResult();

    //! @return true if status of connection is "OK".
    /*! From http://www.postgresql.org/docs/8.4/static/libpq-status.html:
        "Only two of these are seen outside of an asynchronous connection procedure:
         CONNECTION_OK and CONNECTION_BAD." */
    inline bool connectionOK() { return CONNECTION_OK == PQstatus(conn); }

    QString parameter(const char *paramName) { return QLatin1String(PQparameterStatus(conn, paramName)); }

    PGconn *conn;
    bool unicode;
    PGresult *res;
    QByteArray escapingBuffer;

    QString errmsg; //!< server-specific message of last operation
    int resultCode; //!< result code of last operation on server
};

//! Internal PostgreSQL cursor data.
/*! Provides a low-level abstraction for iterating over result sets. */
class PostgresqlCursorData : public PostgresqlConnectionInternal
{
public:
    explicit PostgresqlCursorData(Connection* connection);
    virtual ~PostgresqlCursorData();
};

}
#endif

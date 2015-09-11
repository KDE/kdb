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

#ifndef KDB_POSTGRESQLCONNECTION_P_H
#define KDB_POSTGRESQLCONNECTION_P_H

#include "KDbConnection_p.h"

#include <QString>

#include <libpq-fe.h>

class KDbEscapedString;
class KDbResult;

class PostgresqlConnectionInternal : public KDbConnectionInternal
{
public:
    explicit PostgresqlConnectionInternal(KDbConnection *connection);

    virtual ~PostgresqlConnectionInternal();

    //! Executes query for a raw SQL statement @a sql on the database
    bool executeSQL(const KDbEscapedString& sql);

    static QString serverResultName(int resultCode);

    void storeResult(KDbResult *result);

    //! @return true if status of connection is "OK".
    /*! From http://www.postgresql.org/docs/8.4/static/libpq-status.html:
        "Only two of these are seen outside of an asynchronous connection procedure:
         CONNECTION_OK and CONNECTION_BAD." */
    inline bool connectionOK() { return CONNECTION_OK == PQstatus(conn); }

    PGconn *conn;
    bool unicode;
    PGresult *res;
    QByteArray escapingBuffer;
};

//! Internal PostgreSQL cursor data.
/*! Provides a low-level abstraction for iterating over result sets. */
class PostgresqlCursorData : public PostgresqlConnectionInternal
{
public:
    explicit PostgresqlCursorData(KDbConnection* connection);
    virtual ~PostgresqlCursorData();
};

#endif

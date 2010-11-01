/* This file is part of the KDE project
   Copyright (C) 2005 Adam Pigg <adam@piggz.co.uk>

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

#include <Predicate/Connection_p.h>

#include <libpq-fe.h>

/**
  @author Adam Pigg <adam@piggz.co.uk>
*/
namespace Predicate
{
class PostgresqlConnectionInternal : public ConnectionInternal
{
public:
    explicit PostgresqlConnectionInternal(Connection *conn);

    virtual ~PostgresqlConnectionInternal();

    //! stores last result's message
    virtual void storeResult();

    PGconn *conn;

    Predicate::ServerVersionInfo *version; //!< this is set in drv_connect(), so we can use it in drv_useDatabase()
    //!< because pgsql really connects after "USE".

    QString errmsg; //!< server-specific message of last operation
    int resultCode; //!< result code of last operation on server
};

#if 0 //pred
//! @internal
class PostgresqlTransactionData : public TransactionData
{
public:
    PostgresqlTransactionData(Connection *conn, bool nontransaction);
    ~PostgresqlTransactionData();
    pqxx::transaction_base *data;
};
#endif

//! Internal PostgreSQL cursor data.
/*! Provides a low-level abstraction for iterating over result sets. */
class PostgresqlCursorData : public PostgresqlConnectionInternal
{
public:
    explicit PostgresqlCursorData(Predicate::Connection* connection);
    virtual ~MysqlCursorData();

    MYSQL_RES *mysqlres;
    MYSQL_ROW mysqlrow;
    unsigned long *lengths;
    unsigned long numRows;
};

}
#endif

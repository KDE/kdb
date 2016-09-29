/* This file is part of the KDE project
   Copyright (C) 2008-2010 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KDB_PREPAREDSTATEMENT_IFACE_H
#define KDB_PREPAREDSTATEMENT_IFACE_H

#include <QSharedData>

#include "KDbResult.h"
#include "KDbPreparedStatement.h"

class KDbSqlResult;

//! Prepared statement interface for backend-dependent implementations.
class KDB_EXPORT KDbPreparedStatementInterface : public KDbResultable
{
protected:
    KDbPreparedStatementInterface() {}
    virtual ~KDbPreparedStatementInterface() {}

    /*! For implementation. Initializes the prepared statement in a backend-dependent way
        using recently generated @a sql statement.
        It should be guaranteed that @a sql is valid and not empty.
        For example sqlite3_prepare() is used for SQLite.
        This is called only when d->dirty == true is encountered on execute(),
        i.e. when attributes of the object (like WHERE field names) change. */
    virtual bool prepare(const KDbEscapedString& sql) = 0;

    //! For implementation, executes the prepared statement
    //! Type of statement is specified by the @a type parameter.
    //! @a selectFieldList specifies fields for SELECT statement.
    //! @a insertFieldList is set to list of fields in INSERT statement.
    //! Parameters @a parameters are passed to the statement, usually using binding.
    //! The value pointed by @a resultOwned is set to true if the returned SQL result is owned
    //! by the prepared statement object. This is expected and the default behaviour.
    //! If the value pointed by @a resultOwned is set to @c false, the KDbSqlResult object
    //! will be deleted by the KDbPreparedStatement object before returning.
    virtual KDbSqlResult* execute(
        KDbPreparedStatement::Type type,
        const KDbField::List& selectFieldList,
        KDbFieldList* insertFieldList,
        const KDbPreparedStatementParameters& parameters, bool *resultOwned) Q_REQUIRED_RESULT = 0;

    friend class KDbConnection;
    friend class KDbPreparedStatement;
private:
    Q_DISABLE_COPY(KDbPreparedStatementInterface)
};

#endif

/* This file is part of the KDE project
   Copyright (C) 2005-2012 Jarosław Staniek <staniek@kde.org>

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

#ifndef PREDICATE_SQLITEPREPAREDSTATEMENT_H
#define PREDICATE_SQLITEPREPAREDSTATEMENT_H

#include <Predicate/Interfaces/PreparedStatementInterface>
#include "SqliteConnection_p.h"

namespace Predicate
{
class Field;

/*! Implementation of prepared statements for the SQLite driver. */
class SQLitePreparedStatement : public PreparedStatementInterface, public SQLiteConnectionInternal
{
public:
    explicit SQLitePreparedStatement(ConnectionInternal* conn);

    virtual ~SQLitePreparedStatement();

protected:
    virtual bool prepare(const EscapedString& statement);

    virtual bool execute(
        PreparedStatement::Type type,
        const Field::List& selectFieldList,
        FieldList& insertFieldList,
        const PreparedStatementParameters& parameters);

    bool bindValue(Field *field, const QVariant& value, int arg);

    sqlite3_stmt *m_handle;
};

}

#endif

/* This file is part of the KDE project
   Copyright (C) 2005 Adam Pigg <adam@piggz.co.uk>
   Copyright (C) 2010-2012 Jarosław Staniek <staniek@kde.org>

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

#include "PostgresqlPreparedStatement.h"
#include <QtDebug>
using namespace Predicate;

PostgresqlPreparedStatement::PostgresqlPreparedStatement(ConnectionInternal* conn)
        : PreparedStatementInterface()
        , PostgresqlConnectionInternal(conn->connection)
{
}


PostgresqlPreparedStatement::~PostgresqlPreparedStatement()
{
}

bool PostgresqlPreparedStatement::prepare(const EscapedString& statement)
{
    Q_UNUSED(statement);
    return true;
}

bool PostgresqlPreparedStatement::execute(
    PreparedStatement::Type type,
    const Field::List& selectFieldList,
    FieldList& insertFieldList,
    const PreparedStatementParameters& parameters)
{
    Q_UNUSED(selectFieldList);
    if (type == PreparedStatement::InsertStatement) {
        const int missingValues = insertFieldList.fieldCount() - parameters.count();
        PreparedStatementParameters myParameters(parameters);
        if (missingValues > 0) {
    //! @todo can be more efficient
            for (int i = 0; i < missingValues; i++) {
                myParameters.append(QVariant());
            }
        }
        return connection->insertRecord(&insertFieldList, myParameters);
    }
//! @todo support select
    return false;  
}

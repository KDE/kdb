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

#ifndef KDB_POSTGRESQLPREPAREDSTATEMENT_H
#define KDB_POSTGRESQLPREPAREDSTATEMENT_H

#include "KDbPreparedStatementInterface.h"
#include "PostgresqlConnection_p.h"

class PostgresqlPreparedStatement : public KDbPreparedStatementInterface, public PostgresqlConnectionInternal
{
public:
    explicit PostgresqlPreparedStatement(PostgresqlConnectionInternal* conn);

    virtual ~PostgresqlPreparedStatement();

    virtual bool prepare(const KDbEscapedString& sql);

    virtual bool execute(
        KDbPreparedStatement::Type type,
        const KDbField::List& selectFieldList,
        KDbFieldList* insertFieldList,
        const KDbPreparedStatementParameters& parameters);
};

#endif

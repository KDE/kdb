/* This file is part of the KDE project
   Copyright (C) 2006 Jarosław Staniek <staniek@kde.org>

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

#ifndef MYSQLPREPAREDSTATEMENT_H
#define MYSQLPREPAREDSTATEMENT_H

#include <Predicate/Interfaces/PreparedStatementInterface.h>
#include "MysqlConnection_p.h"

//todo 1.1 - unfinished: #define PREDICATE_USE_MYSQL_STMT
// for 1.0 we're using unoptimized version

namespace Predicate
{

/*! Implementation of prepared statements for MySQL driver. */
class MysqlPreparedStatement : public PreparedStatementInterface, public MysqlConnectionInternal
{
public:
    MysqlPreparedStatement(ConnectionInternal& conn);

    virtual ~MysqlPreparedStatement();

protected:
    virtual bool prepare(const QByteArray& statement);

    virtual bool execute(
        PreparedStatement::Type type,
        const Field::List& fieldList,
        const PreparedStatement::Arguments &args);

    bool init();
    void done();

#ifdef PREDICATE_USE_MYSQL_STMT
    bool bindValue(Field *field, const QVariant& value, int arg);
    int m_realParamCount;
    MYSQL_STMT *m_statement;
    MYSQL_BIND *m_mysqlBind;
#endif
    QByteArray m_tempStatementString;
    bool m_resetRequired;
};
}
#endif

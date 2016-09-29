/* This file is part of the KDE project
   Copyright (C) 2006-2010 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_MYSQLPREPAREDSTATEMENT_H
#define KDB_MYSQLPREPAREDSTATEMENT_H

#include "KDbPreparedStatementInterface.h"
#include "MysqlConnection_p.h"

//! @todo 3.1 - unfinished: #define KDB_USE_MYSQL_STMT; for 3.0 we're using unoptimized version

/*! Implementation of prepared statements for MySQL driver. */
class MysqlPreparedStatement : public KDbPreparedStatementInterface, public MysqlConnectionInternal
{
public:
    explicit MysqlPreparedStatement(MysqlConnectionInternal* conn);

    virtual ~MysqlPreparedStatement();

private:
    virtual bool prepare(const KDbEscapedString& sql);

    virtual KDbSqlResult* execute(
        KDbPreparedStatement::Type type,
        const KDbField::List& selectFieldList,
        KDbFieldList* insertFieldList,
        const KDbPreparedStatementParameters& parameters,
        bool *resultOwned) Q_REQUIRED_RESULT;

    bool init();
    void done();

#ifdef KDB_USE_MYSQL_STMT
    bool bindValue(KDbField *field, const QVariant& value, int arg);
    int m_realParamCount;
    MYSQL_STMT *m_statement;
    MYSQL_BIND *m_mysqlBind;
#endif
    KDbEscapedString m_tempStatementString;
    bool m_resetRequired;
    Q_DISABLE_COPY(MysqlPreparedStatement)
};

#endif

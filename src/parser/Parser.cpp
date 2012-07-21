/* This file is part of the KDE project
   Copyright (C) 2003 Lucijan Busch <lucijan@kde.org>
   Copyright (C) 2004-2012 Jarosław Staniek <staniek@kde.org>

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

#include <Predicate/Connection>
#include <Predicate/TableSchema>
#include <Predicate/Tools/Static>
#include "Parser.h"
#include "Parser_p.h"
#include "SqlParser.h"

/*moved to Driver
#include "tokens.cpp"
PREDICATE_GLOBAL_STATIC_WITH_ARGS(StaticSetOfStrings, _reservedKeywords, (_tokens))
*/

//--------------------

using namespace Predicate;

//! Cache
class ParserStatic
{
public:
    ParserStatic()
     : operationStrings((QLatin1String[]){
            QLatin1String("None"),
            QLatin1String("Error"),
            QLatin1String("CreateTable"),
            QLatin1String("AlterTable"),
            QLatin1String("Select"),
            QLatin1String("Insert"),
            QLatin1String("Update"),
            QLatin1String("Delete")})
    {
    }
    const QLatin1String operationStrings[8];
};

PREDICATE_GLOBAL_STATIC(ParserStatic, Predicate_parserStatic)

Parser::Parser(Connection *db)
        : d(new Private)
{
    d->db = db;
}

Parser::~Parser()
{
    delete d;
}

Parser::OPCode Parser::operation() const
{
    return (OPCode)d->operation;
}

QString Parser::operationString() const
{
    Q_ASSERT(size_t(d->operation) < sizeof(Predicate_parserStatic->operationStrings));
    return Predicate_parserStatic->operationStrings[d->operation];
}

TableSchema *Parser::table()
{
    TableSchema *t = d->table; d->table = 0; return t;
}

QuerySchema *Parser::query()
{
    QuerySchema *s = d->select; d->select = 0; return s;
}

Connection *Parser::db() const
{
    return d->db;
}

ParserError Parser::error() const
{
    return d->error;
}

EscapedString Parser::statement() const
{
    return d->statement;
}

void Parser::setOperation(OPCode op)
{
    d->operation = op;
}

QuerySchema *Parser::select() const
{
    return d->select;
}

void Parser::setError(const ParserError &err)
{
    d->error = err;
}

void
Parser::createTable(const char *t)
{
    if (d->table)
        return;

    d->table = new Predicate::TableSchema(QLatin1String(t));
}

void
Parser::setQuerySchema(QuerySchema *query)
{
    if (d->select)
        delete d->select;

    d->select = query;
}

void Parser::init()
{
    if (d->initialized)
        return;
    // nothing to do
    d->initialized = true;
}

/*moved to Driver
bool Parser::isReservedKeyword(const QByteArray& str)
{
  return _reservedKeywords->contains(str.toUpper());
}*/

bool
Parser::parse(const EscapedString &statement)
{
    init();
    clear();
    d->statement = statement;

    Predicate::Parser *oldParser = parser;
    Predicate::Field *oldField = field;
    bool res = parseData(this, statement.toByteArray());
    parser = oldParser;
    field = oldField;
    return res;
}

void
Parser::clear()
{
    d->clear();
}

//-------------------------------------

ParserError::ParserError()
        : m_position(-1)
{
// m_isNull = true;
}

ParserError::ParserError(const QString &type, const QString &message, const QByteArray &token,
                         int position)
{
    m_type = type;
    m_message = message;
    m_token = token;
    m_position = position;
}

ParserError::~ParserError()
{
}

QDebug operator<<(QDebug dbg, const Predicate::ParserError& error)
{
    return dbg.space() << "Predicate:ParserError: type=" << error.type() << "message=" << error.message()
                       << "pos=" << error.position() << ")";
}

//-------------------------------------

ParseInfo::ParseInfo(Predicate::QuerySchema *query)
 : d(new Private)
{
    d->querySchema = query;
}

ParseInfo::~ParseInfo()
{
    delete d;
}

QList<int> ParseInfo::tablesAndAliasesForName(const QString &tableOrAliasName) const
{
    const QList<int> *list = d->repeatedTablesAndAliases.value(tableOrAliasName);
    return list ? *list : QList<int>();
}

QuerySchema* ParseInfo::querySchema() const
{
    return d->querySchema;
}

QString ParseInfo::errorMessage() const
{
    return d->errorMessage;
}

QString ParseInfo::errorDescription() const
{
    return d->errorDescription;
}

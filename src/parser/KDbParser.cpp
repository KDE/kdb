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

#include "KDbParser.h"
#include "KDbParser_p.h"
#include "generated/sqlparser.h"

#include "KDbConnection.h"
#include "KDbTableSchema.h"

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

Q_GLOBAL_STATIC(ParserStatic, KDb_parserStatic)

KDbParser::KDbParser(KDbConnection *db)
        : d(new Private)
{
    d->db = db;
}

KDbParser::~KDbParser()
{
    delete d;
}

KDbParser::OPCode KDbParser::operation() const
{
    return (OPCode)d->operation;
}

QString KDbParser::operationString() const
{
    Q_ASSERT(size_t(d->operation) < sizeof(KDb_parserStatic->operationStrings));
    return KDb_parserStatic->operationStrings[d->operation];
}

KDbTableSchema *KDbParser::table()
{
    KDbTableSchema *t = d->table; d->table = 0; return t;
}

KDbQuerySchema *KDbParser::query()
{
    KDbQuerySchema *s = d->select; d->select = 0; return s;
}

KDbConnection *KDbParser::connection() const
{
    return d->db;
}

KDbParserError KDbParser::error() const
{
    return d->error;
}

KDbEscapedString KDbParser::statement() const
{
    return d->sql;
}

void KDbParser::setOperation(OPCode op)
{
    d->operation = op;
}

KDbQuerySchema *KDbParser::select() const
{
    return d->select;
}

void KDbParser::setError(const KDbParserError &err)
{
    d->error = err;
}

void KDbParser::createTable(const QByteArray &name)
{
    if (d->table)
        return;

    d->table = new KDbTableSchema(QLatin1String(name));
}

void KDbParser::setQuerySchema(KDbQuerySchema *query)
{
    if (d->select)
        delete d->select;

    d->select = query;
}

void KDbParser::init()
{
    if (d->initialized)
        return;
    // nothing to do
    d->initialized = true;
}

bool KDbParser::parse(const KDbEscapedString &sql)
{
    init();
    clear();
    d->sql = sql;

    KDbParser *oldParser = globalParser;
    KDbField *oldField = globalField;
    bool res = parseData(this, sql.toByteArray().constData());
    globalParser = oldParser;
    globalField = oldField;
    return res;
}

void KDbParser::clear()
{
    d->clear();
}

//-------------------------------------

KDbParserError::KDbParserError()
        : m_position(-1)
{
}

KDbParserError::KDbParserError(const QString &type, const QString &message, const QByteArray &token,
                         int position)
{
    m_type = type;
    m_message = message;
    m_token = token;
    m_position = position;
}

KDbParserError::~KDbParserError()
{
}

QDebug operator<<(QDebug dbg, const KDbParserError& error)
{
    if (error.type().isEmpty() && error.message().isEmpty()) {
        return dbg.space() << "KDb:KDbParserError: None";
    }
    return dbg.space() << "KDb:KDbParserError: type=" << error.type() << "message=" << error.message()
                       << "pos=" << error.position() << ")";
}

/* This file is part of the KDE project
   Copyright (C) 2003 Lucijan Busch <lucijan@kde.org>
   Copyright (C) 2004-2017 Jarosław Staniek <staniek@kde.org>

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

#include <vector>

bool parseData(KDbParser *p, const KDbEscapedString &sql);

//! Cache
class ParserStatic
{
public:
    ParserStatic()
     : statementTypeStrings({
            QLatin1String("None"),
            QLatin1String("Select"),
            QLatin1String("CreateTable"),
            QLatin1String("AlterTable"),
            QLatin1String("Insert"),
            QLatin1String("Update"),
            QLatin1String("Delete")})
    {
    }
    const std::vector<QString> statementTypeStrings;
private:
    Q_DISABLE_COPY(ParserStatic)
};

Q_GLOBAL_STATIC(ParserStatic, KDb_parserStatic)

KDbParser::KDbParser(KDbConnection *connection)
        : d(new KDbParserPrivate)
{
    d->connection = connection;
}

KDbParser::~KDbParser()
{
    delete d;
}

KDbParser::StatementType KDbParser::statementType() const
{
    return d->statementType;
}

QString KDbParser::statementTypeString() const
{
    Q_ASSERT(size_t(d->statementType) < sizeof(KDb_parserStatic->statementTypeStrings));
    return KDb_parserStatic->statementTypeStrings[d->statementType];
}

KDbTableSchema *KDbParser::table()
{
    KDbTableSchema *t = d->table;
    d->table = nullptr;
    return t;
}

KDbQuerySchema *KDbParser::query()
{
    KDbQuerySchema *s = d->query;
    d->query = nullptr;
    return s;
}

KDbConnection *KDbParser::connection() const
{
    return d->connection;
}

KDbParserError KDbParser::error() const
{
    return d->error;
}

KDbEscapedString KDbParser::statement() const
{
    return d->sql;
}

void KDbParser::init()
{
    if (d->initialized)
        return;
    // nothing to do
    d->initialized = true;
}

bool KDbParser::parse(const KDbEscapedString &sql, KDbQuerySchema *query)
{
    init();
    reset();
    d->sql = sql;
    d->query = query;

    KDbParser *oldParser = globalParser;
    KDbField *oldField = globalField;
    bool res = parseData(this, sql);
    globalParser = oldParser;
    globalField = oldField;
    return res;
}

void KDbParser::reset()
{
    d->reset();
}

//-------------------------------------

class Q_DECL_HIDDEN KDbParserError::Private
{
public:
    Private() {}
    Private(const Private &other) {
        copy(other);
    }
#define KDbParserErrorPrivateArgs(o) std::tie(o.type, o.message, o.token, o.position)
    void copy(const Private &other) {
        KDbParserErrorPrivateArgs((*this)) = KDbParserErrorPrivateArgs(other);
    }
    bool operator==(const Private &other) const {
        return KDbParserErrorPrivateArgs((*this)) == KDbParserErrorPrivateArgs(other);
    }
    QString type;
    QString message;
    QByteArray token;
    int position = -1;
};

KDbParserError::KDbParserError()
    : d(new Private)
{
}

KDbParserError::KDbParserError(const QString &type, const QString &message, const QByteArray &token,
                               int position)
    : d(new Private)
{
    d->type = type;
    d->message = message;
    d->token = token;
    d->position = position;
}

KDbParserError::KDbParserError(const KDbParserError &other)
    : d(new Private(*other.d))
{
    *d = *other.d;
}

KDbParserError::~KDbParserError()
{
    delete d;
}

KDbParserError& KDbParserError::operator=(const KDbParserError &other)
{
    if (this != &other) {
        d->copy(*other.d);
    }
    return *this;
}

bool KDbParserError::operator==(const KDbParserError &other) const
{
    return *d == *other.d;
}

QString KDbParserError::type() const
{
    return d->type;
}

QString KDbParserError::message() const
{
    return d->message;
}

int KDbParserError::position() const
{
    return d->position;
}

QDebug operator<<(QDebug dbg, const KDbParserError& error)
{
    if (error.type().isEmpty() && error.message().isEmpty()) {
        return dbg.space() << "KDb:KDbParserError: None";
    }
    return dbg.space() << "KDb:KDbParserError: type=" << error.type() << "message=" << error.message()
                       << "pos=" << error.position() << ")";
}

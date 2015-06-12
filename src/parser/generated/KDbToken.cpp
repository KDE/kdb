/****************************************************************************
 * Created by generate_parser_code.sh
 * WARNING! All changes made in this file will be lost!
 ****************************************************************************/
/* This file is part of the KDE project
   Copyright (C) 2015 Jarosław Staniek <staniek@kde.org>

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

#include "KDbToken.h"
#include "sqlparser.h"

KDbToken::KDbToken(char charToken)
    : v(g_tokenName(charToken) == 0 ? 0 : charToken)
{
}

QString KDbToken::name() const
{
    if (!isValid()) {
        return QLatin1String("<INVALID_TOKEN>");
    }
    if (v > maxCharTokenValue) {
        return QLatin1String(g_tokenName(v));
    }
    if (isprint(v)) {
        return QString(QLatin1Char(char(v)));
    }
    else {
        return QLatin1String(QByteArray::number(v));
    }
}

QString KDbToken::toString() const
{
    if (toChar() > 0) {
        return name();
    }
    // other arithmetic operations: << >>
    // NOTE: include cases that have toString() != name()
    switch (v) {
    case ::BITWISE_SHIFT_RIGHT: return QLatin1String(">>");
    case ::BITWISE_SHIFT_LEFT: return QLatin1String("<<");
        // other relational operations: <= >= <> (or !=) LIKE IN
    case ::NOT_EQUAL: return QLatin1String("<>");
    case ::NOT_EQUAL2: return QLatin1String("!=");
    case ::LESS_OR_EQUAL: return QLatin1String("<=");
    case ::GREATER_OR_EQUAL: return QLatin1String(">=");
    case ::NOT_LIKE: return QLatin1String("NOT LIKE");
    case ::SQL_IN: return QLatin1String("IN");
        // other logical operations: OR (or ||) AND (or &&) XOR
    case ::SIMILAR_TO: return QLatin1String("SIMILAR TO");
    case ::NOT_SIMILAR_TO: return QLatin1String("NOT SIMILAR TO");
        // other string operations: || (as CONCATENATION)
    case ::CONCATENATION: return QLatin1String("||");
        // SpecialBinary "pseudo operators":
        /* not handled here */
    default:;
    }
    const QString s = name();
    if (!s.isEmpty()) {
        return s;
    }
    return QString::fromLatin1("<INVALID_TOKEN#%1> ").arg(v);
}

//static
QString KDbToken::toString(KDbToken token)
{
    return token.toString();
}

KDB_EXPORT QDebug operator<<(QDebug dbg, KDbToken token)
{
    dbg.nospace() << token.name().toLatin1().constData();
    return dbg.space();
}

static QList<KDbToken> g_allTokens;

//static
QList<KDbToken> KDbToken::allTokens()
{
    if (g_allTokens.isEmpty()) {
        for (int i = 0; i < KDbToken::maxTokenValue; ++i) {
            if (g_tokenName(i) != 0) {
                g_allTokens.append(i);
            }
        }
    }
    return g_allTokens;
}
const KDbToken KDbToken::SQL_TYPE(::SQL_TYPE);
const KDbToken KDbToken::AS(::AS);
const KDbToken KDbToken::AS_EMPTY(::AS_EMPTY);
const KDbToken KDbToken::ASC(::ASC);
const KDbToken KDbToken::AUTO_INCREMENT(::AUTO_INCREMENT);
const KDbToken KDbToken::BIT(::BIT);
const KDbToken KDbToken::BITWISE_SHIFT_LEFT(::BITWISE_SHIFT_LEFT);
const KDbToken KDbToken::BITWISE_SHIFT_RIGHT(::BITWISE_SHIFT_RIGHT);
const KDbToken KDbToken::BY(::BY);
const KDbToken KDbToken::CHARACTER_STRING_LITERAL(::CHARACTER_STRING_LITERAL);
const KDbToken KDbToken::CONCATENATION(::CONCATENATION);
const KDbToken KDbToken::CREATE(::CREATE);
const KDbToken KDbToken::DESC(::DESC);
const KDbToken KDbToken::DISTINCT(::DISTINCT);
const KDbToken KDbToken::DOUBLE_QUOTED_STRING(::DOUBLE_QUOTED_STRING);
const KDbToken KDbToken::FROM(::FROM);
const KDbToken KDbToken::JOIN(::JOIN);
const KDbToken KDbToken::KEY(::KEY);
const KDbToken KDbToken::LEFT(::LEFT);
const KDbToken KDbToken::LESS_OR_EQUAL(::LESS_OR_EQUAL);
const KDbToken KDbToken::GREATER_OR_EQUAL(::GREATER_OR_EQUAL);
const KDbToken KDbToken::SQL_NULL(::SQL_NULL);
const KDbToken KDbToken::SQL_IS(::SQL_IS);
const KDbToken KDbToken::SQL_IS_NULL(::SQL_IS_NULL);
const KDbToken KDbToken::SQL_IS_NOT_NULL(::SQL_IS_NOT_NULL);
const KDbToken KDbToken::ORDER(::ORDER);
const KDbToken KDbToken::PRIMARY(::PRIMARY);
const KDbToken KDbToken::SELECT(::SELECT);
const KDbToken KDbToken::INTEGER_CONST(::INTEGER_CONST);
const KDbToken KDbToken::REAL_CONST(::REAL_CONST);
const KDbToken KDbToken::RIGHT(::RIGHT);
const KDbToken KDbToken::SQL_ON(::SQL_ON);
const KDbToken KDbToken::DATE_CONST(::DATE_CONST);
const KDbToken KDbToken::DATETIME_CONST(::DATETIME_CONST);
const KDbToken KDbToken::TIME_CONST(::TIME_CONST);
const KDbToken KDbToken::TABLE(::TABLE);
const KDbToken KDbToken::IDENTIFIER(::IDENTIFIER);
const KDbToken KDbToken::IDENTIFIER_DOT_ASTERISK(::IDENTIFIER_DOT_ASTERISK);
const KDbToken KDbToken::QUERY_PARAMETER(::QUERY_PARAMETER);
const KDbToken KDbToken::VARCHAR(::VARCHAR);
const KDbToken KDbToken::WHERE(::WHERE);
const KDbToken KDbToken::SQL(::SQL);
const KDbToken KDbToken::SQL_TRUE(::SQL_TRUE);
const KDbToken KDbToken::SQL_FALSE(::SQL_FALSE);
const KDbToken KDbToken::UNION(::UNION);
const KDbToken KDbToken::SCAN_ERROR(::SCAN_ERROR);
const KDbToken KDbToken::AND(::AND);
const KDbToken KDbToken::BETWEEN(::BETWEEN);
const KDbToken KDbToken::NOT_BETWEEN(::NOT_BETWEEN);
const KDbToken KDbToken::EXCEPT(::EXCEPT);
const KDbToken KDbToken::SQL_IN(::SQL_IN);
const KDbToken KDbToken::INTERSECT(::INTERSECT);
const KDbToken KDbToken::LIKE(::LIKE);
const KDbToken KDbToken::ILIKE(::ILIKE);
const KDbToken KDbToken::NOT_LIKE(::NOT_LIKE);
const KDbToken KDbToken::NOT(::NOT);
const KDbToken KDbToken::NOT_EQUAL(::NOT_EQUAL);
const KDbToken KDbToken::NOT_EQUAL2(::NOT_EQUAL2);
const KDbToken KDbToken::OR(::OR);
const KDbToken KDbToken::SIMILAR_TO(::SIMILAR_TO);
const KDbToken KDbToken::NOT_SIMILAR_TO(::NOT_SIMILAR_TO);
const KDbToken KDbToken::XOR(::XOR);
const KDbToken KDbToken::UMINUS(::UMINUS);
const KDbToken KDbToken::BETWEEN_AND(0x1001);
const KDbToken KDbToken::NOT_BETWEEN_AND(0x1002);

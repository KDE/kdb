/* This file is part of the KDE project
   Copyright (C) 2003-2011 Jarosław Staniek <staniek@kde.org>

   Based on nexp.cpp : Parser module of Python-like language
   (C) 2001 Jarosław Staniek, MIMUW (www.mimuw.edu.pl)

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

#include <Predicate/Expression>
#include <Predicate/Utils>
#include <Predicate/QuerySchema>
#include <Predicate/Tools/Static>
#include "parser/SqlParser.h"

#include <ctype.h>

using namespace Predicate;

UnaryExpressionData::UnaryExpressionData()
 : ExpressionData()
{
    ExpressionDebug << "UnaryExpressionData" << ref;
}

UnaryExpressionData::~UnaryExpressionData()
{
    ExpressionDebug << "~UnaryExpressionData" << ref;
}

UnaryExpressionData* UnaryExpressionData::clone()
{
    ExpressionDebug << "UnaryExpressionData::clone" << *this;
    return new UnaryExpressionData(*this);
}

QDebug UnaryExpressionData::debug(QDebug dbg) const
{
    dbg.nospace() << "UnaryExp('"
           << Expression::tokenToDebugString(token) << "', ";
    ExplicitlySharedExpressionDataPointer a = arg();
    if (a.data())
        dbg.nospace() << *a;
    else
        dbg.nospace() << "<NONE>";
    dbg.nospace() << QString::fromLatin1(",type=%1)").arg(Driver::defaultSQLTypeName(type()));
    return dbg.space();
}

EscapedString UnaryExpressionData::toString(QuerySchemaParameterValueListIterator* params) const
{
    ExplicitlySharedExpressionDataPointer a = arg();

    if (token == '(') //parentheses (special case)
        return "(" + (a.constData() ? a->toString(params) : EscapedString("<NULL>")) + ")";
    if (token < 255 && isprint(token))
        return Expression::tokenToDebugString(token)
                + (a.constData() ? a->toString(params) : EscapedString("<NULL>"));
    if (token == NOT)
        return "NOT " + (a.constData() ? a->toString(params) : EscapedString("<NULL>"));
    if (token == SQL_IS_NULL)
        return (a.constData() ? a->toString(params) : EscapedString("<NULL>")) + " IS NULL";
    if (token == SQL_IS_NOT_NULL)
        return (a.constData() ? a->toString(params) : EscapedString("<NULL>")) + " IS NOT NULL";
    return EscapedString("{INVALID_OPERATOR#%1} ")
        .arg(token) + (a.constData() ? a->toString(params) : EscapedString("<NULL>"));
}

void UnaryExpressionData::getQueryParameters(QuerySchemaParameterList& params)
{
    ExplicitlySharedExpressionDataPointer a = arg();
    if (a.constData())
        a->getQueryParameters(params);
}

Field::Type UnaryExpressionData::type() const
{
    ExplicitlySharedExpressionDataPointer a = arg();
    if (!a.constData())
        return Field::InvalidType;

    //NULL IS NOT NULL : BOOLEAN
    //NULL IS NULL : BOOLEAN
    switch (token) {
    case SQL_IS_NULL:
    case SQL_IS_NOT_NULL:
        return Field::Boolean;
    }

    const Field::Type t = a->type();
    if (t == Field::Null)
        return Field::Null;
    if (token == NOT)
        return Field::Boolean;

    return t;
}

bool UnaryExpressionData::validate(ParseInfo *parseInfo)
{
    ExplicitlySharedExpressionDataPointer a = arg();
    if (!a.constData())
        return false;

    if (!ExpressionData::validate(parseInfo))
        return false;

    if (!a->validate(parseInfo))
        return false;

//! @todo compare types... e.g. NOT applied to Text makes no sense...

    // update type
#warning TODO
#if 0 // TODO
    if (a->toQueryParameter()) {
        a->toQueryParameter()->setType(type());
    }
#endif

    return true;
#if 0
    Expression *n = l.at(0);

    n->check();
    /*typ wyniku:
        const bool dla "NOT <bool>" (negacja)
        int dla "# <str>" (dlugosc stringu)
        int dla "+/- <int>"
        */
    if (is(NOT) && n->nodeTypeIs(TYP_BOOL)) {
        node_type = new NConstType(TYP_BOOL);
    } else if (is('#') && n->nodeTypeIs(TYP_STR)) {
        node_type = new NConstType(TYP_INT);
    } else if ((is('+') || is('-')) && n->nodeTypeIs(TYP_INT)) {
        node_type = new NConstType(TYP_INT);
    } else {
        ERR("Niepoprawny argument typu '%s' dla operatora '%s'",
            n->nodeTypeName(), is(NOT) ? QString("not") : QChar(typ()));
    }
#endif
}

//=========================================

UnaryExpression::UnaryExpression()
 : Expression(new UnaryExpressionData)
{
    ExpressionDebug << "UnaryExpression() ctor" << *this;
}

UnaryExpression::UnaryExpression(int token, const Expression& arg)
        : Expression(new UnaryExpressionData, UnaryExpressionClass, token)
{
    if (!arg.isNull())
        appendChild(arg);
}

UnaryExpression::UnaryExpression(ExpressionData* data)
 : Expression(data)
{
    ExpressionDebug << "UnaryExpression(ExpressionData*) ctor" << *this;
}

UnaryExpression::UnaryExpression(const UnaryExpression& expr)
        : Expression(expr)
{
}

UnaryExpression::~UnaryExpression()
{
}

Expression UnaryExpression::arg() const
{
    return Expression(d->convertConst<UnaryExpressionData>()->arg().data());
}

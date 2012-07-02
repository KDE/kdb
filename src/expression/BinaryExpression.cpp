/* This file is part of the KDE project
   Copyright (C) 2003-2012 Jarosław Staniek <staniek@kde.org>

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

BinaryExpressionData::BinaryExpressionData()
 : ExpressionData()
{
    ExpressionDebug << "BinaryExpressionData" << ref;
}

BinaryExpressionData::~BinaryExpressionData()
{
}

bool BinaryExpressionData::validate(ParseInfo& parseInfo)
{
    if (!ExpressionData::validate(parseInfo) || children.count() != 2)
        return false;

    if (!left()->validate(parseInfo))
        return false;
    if (!right()->validate(parseInfo))
        return false;

//! @todo compare types..., BITWISE_SHIFT_RIGHT requires integers, etc...

    //update type for query parameters
#warning TODO
#if 0
    if (left()->isQueryParameter()) {
        QueryParameterExpression queryParameter = left()->toQueryParameter();
        queryParameter->setType(left()->type());
    }
    if (right()->isQueryParameter()) {
        QueryParameterExpression queryParameter = right()->toQueryParameter();
        queryParameter->setType(right()->type());
    }
#endif
    return true;
}

Field::Type BinaryExpressionData::type() const
{
    if (children.count() != 2)
        return Field::InvalidType;
    const Field::Type lt = left()->type();
    const Field::Type rt = right()->type();
    if (lt == Field::InvalidType || rt == Field::InvalidType)
        return Field::InvalidType;
    if (lt == Field::Null || rt == Field::Null) {
        if (token != OR) //note that NULL OR something != NULL
            return Field::Null;
    }

    switch (token) {
    case BITWISE_SHIFT_RIGHT:
    case BITWISE_SHIFT_LEFT:
    case CONCATENATION:
        return lt;
    }

    const bool ltInt = Field::isIntegerType(lt);
    const bool rtInt = Field::isIntegerType(rt);
    if (ltInt && rtInt)
        return Predicate::maximumForIntegerTypes(lt, rt);

    if (Field::isFPNumericType(lt) && (rtInt || lt == rt))
        return lt;
    if (Field::isFPNumericType(rt) && (ltInt || lt == rt))
        return rt;

    return Field::Boolean;
}

BinaryExpressionData* BinaryExpressionData::clone()
{
    ExpressionDebug << "BinaryExpressionData::clone" << *this;
    return new BinaryExpressionData(*this);
}

QDebug BinaryExpressionData::debug(QDebug dbg) const
{
    dbg.nospace() << "BinaryExp(class="
        << expressionClassName(expressionClass)
        << ",";
    if (children.count() == 2 && left().constData()) {
        dbg.nospace() << *left();
    }
    else {
        dbg.nospace() << "<NONE>";
    }
    dbg.nospace() << "," << Expression::tokenToDebugString(token) << ",";
    if (children.count() == 2 && right().constData()) {
        dbg.nospace() << *left();
    }
    else {
        dbg.nospace() << "<NONE>";
    }
    dbg.nospace() << ",type=" << Driver::defaultSQLTypeName(type()) << ")";
    return dbg.space();
}

QString BinaryExpressionData::tokenToString() const
{
    if (token < 255 && isprint(token))
        return Expression::tokenToDebugString(token);
    // other arithmetic operations: << >>
    switch (token) {
    case BITWISE_SHIFT_RIGHT: return QLatin1String(">>");
    case BITWISE_SHIFT_LEFT: return QLatin1String("<<");
        // other relational operations: <= >= <> (or !=) LIKE IN
    case NOT_EQUAL: return QLatin1String("<>");
    case NOT_EQUAL2: return QLatin1String("!=");
    case LESS_OR_EQUAL: return QLatin1String("<=");
    case GREATER_OR_EQUAL: return QLatin1String(">=");
    case LIKE: return QLatin1String("LIKE");
    case SQL_IN: return QLatin1String("IN");
        // other logical operations: OR (or ||) AND (or &&) XOR
    case SIMILAR_TO: return QLatin1String("SIMILAR TO");
    case NOT_SIMILAR_TO: return QLatin1String("NOT SIMILAR TO");
    case OR: return QLatin1String("OR");
    case AND: return QLatin1String("AND");
    case XOR: return QLatin1String("XOR");
        // other string operations: || (as CONCATENATION)
    case CONCATENATION: return QLatin1String("||");
        // SpecialBinary "pseudo operators":
        /* not handled here */
    default:;
    }
    return QString::fromLatin1("{INVALID_BINARY_OPERATOR#%1} ").arg(token);
}

EscapedString BinaryExpressionData::toString(QuerySchemaParameterValueListIterator* params) const
{
#define INFIX(a) \
    (left().constData() ? left()->toString(params) : EscapedString("<NULL>")) \
    + " " + a + " " + (right().constData() ? right()->toString(params) : EscapedString("<NULL>"))
    return INFIX(tokenToString());
#undef INFIX
}

void BinaryExpressionData::getQueryParameters(QuerySchemaParameterList& params)
{
    if (left().constData())
        left()->getQueryParameters(params);
    if (right().constData())
        right()->getQueryParameters(params);
}

//=========================================

static ExpressionClass classForArgs(ExpressionClass aClass,
                                    const Expression& leftExpr,
                                    const Expression& rightExpr)
{
    bool ok = true;
    if (leftExpr.isNull()) {
        qWarning() << "BinaryExpression set to null because left argument is not specified";
        ok = false;
    }
    if (rightExpr.isNull()) {
        qWarning() << "BinaryExpression set to null because right argument is not specified";
        ok = false;
    }
    return ok ? aClass : UnknownExpressionClass;
}

BinaryExpression::BinaryExpression()
 : Expression(new BinaryExpressionData)
{
    insertEmptyChild(0);
    insertEmptyChild(1);
    ExpressionDebug << "BinaryExpression() ctor" << *this;
}

BinaryExpression::BinaryExpression(ExpressionClass aClass,
                                   const Expression& leftExpr,
                                   int token,
                                   const Expression& rightExpr)
    : Expression(new BinaryExpressionData, classForArgs(aClass, leftExpr, rightExpr), token)
{
    appendChild(leftExpr.d);
    appendChild(rightExpr.d);
}

BinaryExpression::BinaryExpression(const BinaryExpression& expr)
        : Expression(expr)
{
}

BinaryExpression::BinaryExpression(ExpressionData* data)
 : Expression(data)
{
    ExpressionDebug << "BinaryExpression(ExpressionData*) ctor" << *this;
//    insertEmptyChild(0);
//    insertEmptyChild(1);
}

BinaryExpression::~BinaryExpression()
{
}

Expression BinaryExpression::left() const
{
    return Expression(d->children.at(0).data());
}

void BinaryExpression::setLeft(const Expression& leftExpr)
{
    d->children[0] = leftExpr.d;
}

Expression BinaryExpression::right() const
{
    return Expression(d->children.at(1).data());
}

void BinaryExpression::setRight(const Expression& rightExpr)
{
    d->children[1] = rightExpr.d;
}

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
#include "Expression_p.h"
#include <Predicate/Utils>
#include <Predicate/QuerySchema>
#include "parser/SqlParser.h"
#include "parser/Parser_p.h"
#include <Predicate/Tools/Static>

#include <ctype.h>

using namespace Predicate;

BinaryExpressionData::BinaryExpressionData()
 : ExpressionData()
{
    qDebug() << "BinaryExpressionData" << ref;
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
    qDebug() << "BinaryExpressionData::clone" << *this;
    return new BinaryExpressionData(*this);
}

QDebug BinaryExpressionData::debug(QDebug dbg) const
{
    dbg.nospace() << "BinaryExp(class="
        << expressionClassName(expressionClass)
        << ",";
    if (left().constData()) {
        dbg.nospace() << *left();
    }
    else {
        dbg.nospace() << "<NONE>";
    }
    dbg.nospace() << "," << Expression::tokenToDebugString(token) << ",";
    if (right().constData()) {
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
    case BITWISE_SHIFT_RIGHT: return ">>";
    case BITWISE_SHIFT_LEFT: return "<<";
        // other relational operations: <= >= <> (or !=) LIKE IN
    case NOT_EQUAL: return "<>";
    case NOT_EQUAL2: return "!=";
    case LESS_OR_EQUAL: return "<=";
    case GREATER_OR_EQUAL: return ">=";
    case LIKE: return "LIKE";
    case SQL_IN: return "IN";
        // other logical operations: OR (or ||) AND (or &&) XOR
    case SIMILAR_TO: return "SIMILAR TO";
    case NOT_SIMILAR_TO: return "NOT SIMILAR TO";
    case OR: return "OR";
    case AND: return "AND";
    case XOR: return "XOR";
        // other string operations: || (as CONCATENATION)
    case CONCATENATION: return "||";
        // SpecialBinary "pseudo operators":
        /* not handled here */
    default:;
    }
    return QString("{INVALID_BINARY_OPERATOR#%1} ").arg(token);
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
    qDebug() << "BinaryExpression() ctor" << *this;
}

BinaryExpression::BinaryExpression(ExpressionClass aClass,
                                   const Expression& leftExpr,
                                   int token,
                                   const Expression& rightExpr)
    : Expression(new BinaryExpressionData, classForArgs(aClass, leftExpr, rightExpr), token)
{
}

BinaryExpression::BinaryExpression(const BinaryExpression& expr)
        : Expression(expr)
{
}

BinaryExpression::BinaryExpression(ExpressionData* data)
 : Expression(data)
{
    qDebug() << "BinaryExpression(ExpressionData*) ctor" << *this;
    appendChild(Expression());
    appendChild(Expression());
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

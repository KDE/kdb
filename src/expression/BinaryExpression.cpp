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

bool BinaryExpressionData::validateInternal(ParseInfo *parseInfo, CallStack* callStack)
{
    if (children.count() != 2)
        return false;

    if (!left()->validate(parseInfo, callStack))
        return false;
    if (!right()->validate(parseInfo, callStack))
        return false;

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
    return typeInternal(callStack) != Field::InvalidType;
}

Field::Type BinaryExpressionData::typeInternal(CallStack* callStack) const
{
    if (children.count() != 2 || expressionClass == UnknownExpressionClass)
        return Field::InvalidType;
    const Field::Type lt = left()->type(callStack);
    const Field::Type rt = right()->type(callStack);
    if (lt == Field::InvalidType || rt == Field::InvalidType)
        return Field::InvalidType;

    const bool ltNull = lt == Field::Null;
    const bool rtNull = rt == Field::Null;
    const bool ltText = Field::isTextType(lt);
    const bool rtText = Field::isTextType(rt);
    const bool ltInt = Field::isIntegerType(lt);
    const bool rtInt = Field::isIntegerType(rt);
    const bool ltFP = Field::isFPNumericType(lt);
    const bool rtFP = Field::isFPNumericType(rt);
    const bool ltBool = lt == Field::Boolean;
    const bool rtBool = rt == Field::Boolean;
    const Field::TypeGroup ltGroup = Field::typeGroup(lt);
    const Field::TypeGroup rtGroup = Field::typeGroup(rt);

    if (ltNull || rtNull) {
        switch (token) {
        case OR: // NULL OR something == something
            if (ltNull) {
                return rtBool ? Field::Boolean : Field::InvalidType;
            }
            else if (rtNull) {
                return ltBool ? Field::Boolean : Field::InvalidType;
            }
        default:
            return Field::Null;
        }
    }

    switch (token) {
    case OR:
    case AND:
    case XOR:
        return (ltBool && rtBool) ? Field::Boolean : Field::InvalidType;
    case CONCATENATION:
        if (lt == Field::Text && rt == Field::Text) {
            return Field::Text;
        }
        else if (ltText && rtText) {
            return Field::LongText;
        }
        else if ((ltText && rtNull)
            || (rtText && ltNull))
        {
            return Field::Null;
        }
        return Field::InvalidType;
    default:;
    }

    if (expressionClass == RelationalExpressionClass) {
        if ((ltText && rtText)
            || (ltInt && rtInt)
            || (ltFP && rtFP) || (ltInt && rtFP) || (ltFP && rtInt)
            || (ltBool && rtBool) || (ltBool && rtInt) || (ltInt && rtBool)
            || (ltBool && rtFP) || (ltFP && rtBool)
            || (ltGroup == Field::DateTimeGroup && rtGroup == Field::DateTimeGroup))
        {
            return Field::Boolean;
        }
        return Field::InvalidType;
    }

    if (expressionClass == ArithmeticExpressionClass) {
        if (ltInt && rtInt) {
            /* From documentation of Predicate::maximumForIntegerTypes():
             In case of ArithmeticExpressionClass:
             returned type may not fit to the result of evaluated expression that involves the arguments.
             For example, 100 is within Byte type, maximumForIntegerTypes(Byte, Byte) is Byte but result
             of 100 * 100 exceeds the range of Byte.

             Solution: for types smaller than Integer (e.g. Byte and ShortInteger) we are returning
             Integer type.
            */
            Field::Type t = Predicate::maximumForIntegerTypes(lt, rt);
            if (t == Field::Byte || t == Field::ShortInteger) {
                return Field::Integer;
            }
            return t;
        }

        switch (token) {
        case '&':
        case BITWISE_SHIFT_RIGHT:
        case BITWISE_SHIFT_LEFT:
            if (ltFP && rtFP) {
                return Field::Integer;
            }
            else if (ltFP && rtInt) { // inherit from right
                return rt;
            }
            else if (rtFP && ltInt) { // inherit from left
                return lt;
            }
            break;
        default:;
        }

        /* inherit floating point (Float or Double) type */
        if (ltFP && (rtInt || lt == rt))
            return lt;
        if (rtFP && (ltInt || lt == rt))
            return rt;
    }
    return Field::InvalidType;
}

BinaryExpressionData* BinaryExpressionData::clone()
{
    ExpressionDebug << "BinaryExpressionData::clone" << *this;
    return new BinaryExpressionData(*this);
}

void BinaryExpressionData::debugInternal(QDebug dbg, CallStack* callStack) const
{
    dbg.nospace() << "BinaryExp(class="
        << expressionClassName(expressionClass)
        << ",";
    if (children.count() == 2 && left().constData()) {
        left()->debug(dbg, callStack);
    }
    else {
        dbg.nospace() << "<NONE>";
    }
    dbg.nospace() << "," << Expression::tokenToDebugString(token) << ",";
    if (children.count() == 2 && right().constData()) {
        right()->debug(dbg, callStack);
    }
    else {
        dbg.nospace() << "<NONE>";
    }
    dbg.nospace() << ",type=" << Driver::defaultSQLTypeName(type()) << ")";
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

EscapedString BinaryExpressionData::toStringInternal(QuerySchemaParameterValueListIterator* params,
                                                     CallStack* callStack) const
{
#define INFIX(a) \
    (left().constData() ? left()->toString(params, callStack) : EscapedString("<NULL>")) \
    + " " + a + " " + (right().constData() ? right()->toString(params, callStack) : EscapedString("<NULL>"))
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

static ExpressionClass classForArgs(const Expression& leftExpr,
                                    int token,
                                    const Expression& rightExpr)
{
    if (leftExpr.isNull()) {
        qWarning() << "BinaryExpression set to null because left argument is not specified";
        return UnknownExpressionClass;
    }
    if (rightExpr.isNull()) {
        qWarning() << "BinaryExpression set to null because right argument is not specified";
        return UnknownExpressionClass;
    }
    return Expression::classForToken(token);
}

BinaryExpression::BinaryExpression()
 : Expression(new BinaryExpressionData)
{
    insertEmptyChild(0);
    insertEmptyChild(1);
    ExpressionDebug << "BinaryExpression() ctor" << *this;
}

BinaryExpression::BinaryExpression(const Expression& leftExpr,
                                   int token,
                                   const Expression& rightExpr)
    : Expression(new BinaryExpressionData, classForArgs(leftExpr, token, rightExpr), token)
{
    if (isNull()) {
        insertEmptyChild(0);
        insertEmptyChild(1);
    }
    else {
        appendChild(leftExpr.d);
        appendChild(rightExpr.d);
    }
}

BinaryExpression::BinaryExpression(const BinaryExpression& expr)
        : Expression(expr)
{
}

BinaryExpression::BinaryExpression(ExpressionData* data)
 : Expression(data)
{
    ExpressionDebug << "BinaryExpression(ExpressionData*) ctor" << *this;
    insertEmptyChild(0);
    insertEmptyChild(1);
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
    Expression::setLeftOrRight(leftExpr, 0);
}

Expression BinaryExpression::right() const
{
    return Expression(d->children.at(1).data());
}

void BinaryExpression::setRight(const Expression& rightExpr)
{
    Expression::setLeftOrRight(rightExpr, 1);
}

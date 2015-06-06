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

#include "KDbExpression.h"
#include "KDb.h"
#include "KDbQuerySchema.h"
#include "KDbDriver.h"
#include "kdb_debug.h"
#include "generated/sqlparser.h"

KDbBinaryExpressionData::KDbBinaryExpressionData()
 : KDbExpressionData()
{
    ExpressionDebug << "BinaryExpressionData" << ref;
}

KDbBinaryExpressionData::~KDbBinaryExpressionData()
{
}

bool KDbBinaryExpressionData::validateInternal(KDbParseInfo *parseInfo, KDb::ExpressionCallStack* callStack)
{
    if (children.count() != 2)
        return false;

    if (!left()->validate(parseInfo, callStack))
        return false;
    if (!right()->validate(parseInfo, callStack))
        return false;

    //update type for query parameters
//! @todo IMPORTANT: update type for query parameters
#if 0
    if (left()->isQueryParameter()) {
        KDbQueryParameterExpression queryParameter = left()->toQueryParameter();
        queryParameter->setType(left()->type());
    }
    if (right()->isQueryParameter()) {
        KDbQueryParameterExpression queryParameter = right()->toQueryParameter();
        queryParameter->setType(right()->type());
    }
#endif
    return typeInternal(callStack) != KDbField::InvalidType;
}

KDbField::Type KDbBinaryExpressionData::typeInternal(KDb::ExpressionCallStack* callStack) const
{
    if (children.count() != 2 || expressionClass == KDb::UnknownExpression)
        return KDbField::InvalidType;
    const KDbField::Type lt = left()->type(callStack);
    const KDbField::Type rt = right()->type(callStack);
    if (lt == KDbField::InvalidType || rt == KDbField::InvalidType)
        return KDbField::InvalidType;

    const bool ltNull = lt == KDbField::Null;
    const bool rtNull = rt == KDbField::Null;
    const bool ltText = KDbField::isTextType(lt);
    const bool rtText = KDbField::isTextType(rt);
    const bool ltInt = KDbField::isIntegerType(lt);
    const bool rtInt = KDbField::isIntegerType(rt);
    const bool ltFP = KDbField::isFPNumericType(lt);
    const bool rtFP = KDbField::isFPNumericType(rt);
    const bool ltBool = lt == KDbField::Boolean;
    const bool rtBool = rt == KDbField::Boolean;
    const KDbField::TypeGroup ltGroup = KDbField::typeGroup(lt);
    const KDbField::TypeGroup rtGroup = KDbField::typeGroup(rt);

    if (ltNull || rtNull) {
        switch (token) {
        case OR: // NULL OR something == something
            if (ltNull) {
                return rtBool ? KDbField::Boolean : KDbField::InvalidType;
            }
            else if (rtNull) {
                return ltBool ? KDbField::Boolean : KDbField::InvalidType;
            }
        default:
            return KDbField::Null;
        }
    }

    switch (token) {
    case OR:
    case AND:
    case XOR:
        return (ltBool && rtBool) ? KDbField::Boolean : KDbField::InvalidType;
    case CONCATENATION:
        if (lt == KDbField::Text && rt == KDbField::Text) {
            return KDbField::Text;
        }
        else if (ltText && rtText) {
            return KDbField::LongText;
        }
        else if ((ltText && rtNull)
            || (rtText && ltNull))
        {
            return KDbField::Null;
        }
        return KDbField::InvalidType;
    default:;
    }

    if (expressionClass == KDb::RelationalExpression) {
        if ((ltText && rtText)
            || (ltInt && rtInt)
            || (ltFP && rtFP) || (ltInt && rtFP) || (ltFP && rtInt)
            || (ltBool && rtBool) || (ltBool && rtInt) || (ltInt && rtBool)
            || (ltBool && rtFP) || (ltFP && rtBool)
            || (ltGroup == KDbField::DateTimeGroup && rtGroup == KDbField::DateTimeGroup))
        {
            return KDbField::Boolean;
        }
        return KDbField::InvalidType;
    }

    if (expressionClass == KDb::ArithmeticExpression) {
        if (ltInt && rtInt) {
            /* From documentation of KDb::maximumForIntegerTypes():
             In case of KDb::ArithmeticExpression:
             returned type may not fit to the result of evaluated expression that involves the arguments.
             For example, 100 is within Byte type, maximumForIntegerTypes(Byte, Byte) is Byte but result
             of 100 * 100 exceeds the range of Byte.

             Solution: for types smaller than Integer (e.g. Byte and ShortInteger) we are returning
             Integer type.
            */
            KDbField::Type t = KDb::maximumForIntegerTypes(lt, rt);
            if (t == KDbField::Byte || t == KDbField::ShortInteger) {
                return KDbField::Integer;
            }
            return t;
        }

        switch (token) {
        case '&':
        case BITWISE_SHIFT_RIGHT:
        case BITWISE_SHIFT_LEFT:
            if (ltFP && rtFP) {
                return KDbField::Integer;
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
    return KDbField::InvalidType;
}

KDbBinaryExpressionData* KDbBinaryExpressionData::clone()
{
    ExpressionDebug << "BinaryExpressionData::clone" << *this;
    return new KDbBinaryExpressionData(*this);
}

void KDbBinaryExpressionData::debugInternal(QDebug dbg, KDb::ExpressionCallStack* callStack) const
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
    dbg.nospace() << "," << KDbExpression::tokenToDebugString(token) << ",";
    if (children.count() == 2 && right().constData()) {
        right()->debug(dbg, callStack);
    }
    else {
        dbg.nospace() << "<NONE>";
    }
    dbg.nospace() << ",type=" << KDbDriver::defaultSQLTypeName(type()) << ")";
}

QString KDbBinaryExpressionData::tokenToString() const
{
    if (token < 255 && isprint(token))
        return KDbExpression::tokenToDebugString(token);
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
    case NOT_LIKE: return QLatin1String("NOT LIKE");
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

KDbEscapedString KDbBinaryExpressionData::toStringInternal(KDbQuerySchemaParameterValueListIterator* params,
                                                     KDb::ExpressionCallStack* callStack) const
{
#define INFIX(a) \
    (left().constData() ? left()->toString(params, callStack) : KDbEscapedString("<NULL>")) \
    + " " + a + " " + (right().constData() ? right()->toString(params, callStack) : KDbEscapedString("<NULL>"))
    return INFIX(tokenToString());
#undef INFIX
}

void KDbBinaryExpressionData::getQueryParameters(QList<KDbQuerySchemaParameter>& params)
{
    if (left().constData())
        left()->getQueryParameters(params);
    if (right().constData())
        right()->getQueryParameters(params);
}

ExplicitlySharedExpressionDataPointer KDbBinaryExpressionData::left() const
{
    return (children.count() > 0) ? children.at(0) : ExplicitlySharedExpressionDataPointer();
}
ExplicitlySharedExpressionDataPointer KDbBinaryExpressionData::right() const
{
    return (children.count() > 1) ? children.at(1) : ExplicitlySharedExpressionDataPointer();
}

//=========================================

static KDb::ExpressionClass classForArgs(const KDbExpression& leftExpr,
                                    int token,
                                    const KDbExpression& rightExpr)
{
    if (leftExpr.isNull()) {
        qCWarning(KDB_LOG) << "KDbBinaryExpression set to null because left argument is not specified";
        return KDb::UnknownExpression;
    }
    if (rightExpr.isNull()) {
        qCWarning(KDB_LOG) << "KDbBinaryExpression set to null because right argument is not specified";
        return KDb::UnknownExpression;
    }
    return KDbExpression::classForToken(token);
}

KDbBinaryExpression::KDbBinaryExpression()
 : KDbExpression(new KDbBinaryExpressionData)
{
    ExpressionDebug << "KDbBinaryExpression() ctor" << *this;
}

KDbBinaryExpression::KDbBinaryExpression(const KDbExpression& leftExpr,
                                   int token,
                                   const KDbExpression& rightExpr)
    : KDbExpression(new KDbBinaryExpressionData, classForArgs(leftExpr, token, rightExpr), token)
{
    if (!isNull()) {
        appendChild(leftExpr.d);
        appendChild(rightExpr.d);
    }
}

KDbBinaryExpression::KDbBinaryExpression(const KDbBinaryExpression& expr)
        : KDbExpression(expr)
{
}

KDbBinaryExpression::KDbBinaryExpression(KDbExpressionData* data)
 : KDbExpression(data)
{
    ExpressionDebug << "KDbBinaryExpression(KDbExpressionData*) ctor" << *this;
}

KDbBinaryExpression::KDbBinaryExpression(const ExplicitlySharedExpressionDataPointer &ptr)
    : KDbExpression(ptr)
{
}

KDbBinaryExpression::~KDbBinaryExpression()
{
}

KDbExpression KDbBinaryExpression::left() const
{
    return (d->children.count() > 0) ? KDbExpression(d->children.at(0)) : KDbExpression();
}

void KDbBinaryExpression::setLeft(const KDbExpression& leftExpr)
{
    KDbExpression::setLeftOrRight(leftExpr, 0);
}

KDbExpression KDbBinaryExpression::right() const
{
    return (d->children.count() > 1) ? KDbExpression(d->children.at(1)) : KDbExpression();
}

void KDbBinaryExpression::setRight(const KDbExpression& rightExpr)
{
    KDbExpression::setLeftOrRight(rightExpr, 1);
}

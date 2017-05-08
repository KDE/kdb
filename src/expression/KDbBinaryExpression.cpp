/* This file is part of the KDE project
   Copyright (C) 2003-2016 Jarosław Staniek <staniek@kde.org>

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
#include "KDbParser_p.h"
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
    if (typeInternal(callStack) == KDbField::InvalidType) {
        parseInfo->setErrorMessage(tr("Incompatible types of arguments"));
        parseInfo->setErrorDescription(
                           tr("Expression \"%1\" requires compatible types of arguments. "
                              "Specified arguments are of type %2 and %3.",
                              "Binary expression arguments type error")
                              .arg(toStringInternal(nullptr, nullptr, callStack).toString(),
                                   KDbField::typeName(left()->type()),
                                   KDbField::typeName(right()->type())));
        return false;
    }
    return true;
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
    const bool lAny = left()->convertConst<KDbQueryParameterExpressionData>();
    const bool rAny = right()->convertConst<KDbQueryParameterExpressionData>();

    if (ltNull || rtNull) {
        switch (token.value()) {
        //! @todo add general support, e.g. for "NULL AND (1 == 1)"; for now we only support
        //! constants because there's no evaluation and operations with NULL depend on whether we have TRUE or FALSE
        //! See http://www.postgresql.org/docs/9.4/static/functions-logical.html
        //!     https://dev.mysql.com/doc/refman/5.0/en/logical-operators.html
        case OR: {
            const KDbConstExpressionData *leftConst = left()->convertConst<KDbConstExpressionData>();
            const KDbConstExpressionData *rightConst = right()->convertConst<KDbConstExpressionData>();
            if ((ltBool && leftConst && leftConst->value.toBool())       // true OR NULL is true
                || (rtBool && rightConst && rightConst->value.toBool())) // NULL OR true is true
            {
                return KDbField::Boolean;
            }
            else if ((ltBool && leftConst && !leftConst->value.toBool())  // false OR NULL is NULL
                     || (rtBool && rightConst && !rightConst->value.toBool())  // NULL OR false is NULL
                     || lAny  // Any OR NULL may be NULL
                     || rAny) // NULL OR Any may be NULL
                //! @todo Any OR NULL may be also TRUE -- but this needs support of fuzzy/multivalue types
                //! @todo NULL OR Any may be also TRUE -- but this needs support of fuzzy/multivalue types
            {
                return KDbField::Null;
            }
            break;
        }
        case AND: {
            const KDbConstExpressionData *leftConst = left()->convertConst<KDbConstExpressionData>();
            const KDbConstExpressionData *rightConst = right()->convertConst<KDbConstExpressionData>();
            if ((ltBool && leftConst && !leftConst->value.toBool())       // false AND NULL is false
                || (rtBool && rightConst && !rightConst->value.toBool())) // NULL AND false is false
            {
                return KDbField::Boolean;
            }
            else if ((ltBool && leftConst && leftConst->value.toBool())       // true AND NULL is NULL
                     || (rtBool && rightConst && rightConst->value.toBool()) // NULL AND true is NULL
                     || lAny  // Any AND NULL may be NULL
                     || rAny) // NULL AND Any may be NULL
                //! @todo Any AND NULL may be also FALSE -- but this needs support of fuzzy/multivalue types
                //! @todo NULL AND Any may be also FALSE -- but this needs support of fuzzy/multivalue types
            {
                return KDbField::Null;
            }
            break;
        }
        case XOR: {// Logical XOR. Returns NULL if either operand is NULL. For non-NULL operands,
                   // evaluates to 1 if an odd number of operands is nonzero, otherwise 0 is returned.
                   // a XOR b is mathematically equal to (a AND (NOT b)) OR ((NOT a) and b).
                   // https://dev.mysql.com/doc/refman/5.0/en/logical-operators.html#operator_xor
            return KDbField::Null;
        }
        default:
            return KDbField::Null;
        }
    }

    switch (token.value()) {
    case OR:
    case AND:
    case XOR: {
        if (ltNull && rtNull) {
            return KDbField::Null;
        } else if ((ltBool && rtBool)
                   || (ltBool && rAny)
                   || (lAny && rtBool)
                   || (lAny && rAny))
        {
            return KDbField::Boolean;
        }
        return KDbField::InvalidType;
    }
    case '+':
    case CONCATENATION:
        if (lt == KDbField::Text && rt == KDbField::Text) {
            return KDbField::Text;
        }
        else if ((ltText && rtText)
                 || (ltText && rAny)
                 || (lAny && rtText))
        {
            return KDbField::LongText;
        }
        else if ((ltText && rtNull)
            || (ltNull && rtText)
            || (lAny && rtNull)
            || (ltNull && rAny))
        {
            return KDbField::Null;
        } else if (token.value() == CONCATENATION) {
            if (lAny && rAny) {
                return KDbField::LongText;
            }
            return KDbField::InvalidType;
        }
        break; // '+' can still be handled below for non-text types
    default:;
    }

    if (expressionClass == KDb::RelationalExpression) {
        if ((ltText && rtText)
            || (ltText && rAny)
            || (lAny && rtText)
            || (lAny && rAny)

            || (ltInt && rtInt)
            || (ltInt && rAny)
            || (lAny && rtInt)

            || (ltFP && rtFP) || (ltInt && rtFP) || (ltFP && rtInt)
            || (ltFP && rAny) || (lAny && rtFP)

            || (ltBool && rtBool) || (ltBool && rtInt) || (ltInt && rtBool)
            || (ltBool && rAny) || (lAny && rtBool)

            || (ltBool && rtFP) || (ltFP && rtBool)

            || (ltGroup == KDbField::DateTimeGroup && rtGroup == KDbField::DateTimeGroup)
            || (ltGroup == KDbField::DateTimeGroup && rAny)
            || (lAny && rtGroup == KDbField::DateTimeGroup))
        {
            return KDbField::Boolean;
        }
        return KDbField::InvalidType;
    }

    if (expressionClass == KDb::ArithmeticExpression) {
        if (lAny && rAny) {
            return KDbField::Integer;
        } else if ((ltInt && rtInt)
                   || (ltInt && rAny)
                   || (lAny && rtInt))
        {
            /* From documentation of KDb::maximumForIntegerFieldTypes():
             In case of KDb::ArithmeticExpression:
             returned type may not fit to the result of evaluated expression that involves the arguments.
             For example, 100 is within Byte type, maximumForIntegerFieldTypes(Byte, Byte) is Byte but result
             of 100 * 100 exceeds the range of Byte.

             Solution: for types smaller than Integer (e.g. Byte and ShortInteger) we are returning
             Integer type.
            */
            KDbField::Type t;
            if (lAny) {
                t = rt;
            } else if (rAny) {
                t = lt;
            } else {
                t = KDb::maximumForIntegerFieldTypes(lt, rt);
            }
            if (t == KDbField::Byte || t == KDbField::ShortInteger) {
                return KDbField::Integer;
            }
            return t;
        }

        switch (token.value()) {
        case '&':
        case BITWISE_SHIFT_RIGHT:
        case BITWISE_SHIFT_LEFT:
            if ((ltFP && rtFP)
                 || (ltFP && rAny)  //! @todo can be other Integer too
                 || (lAny && rtFP)) //! @todo can be other Integer too
            {
                return KDbField::Integer;
            }
            else if ((ltFP && rtInt) // inherit from right
                     || (lAny && rtInt))
            {
                return rt;
            }
            else if ((ltInt && rtFP) // inherit from left
                     || (ltInt && rAny))
            {
                return lt;
            }
            break;
        default:;
        }

        /* inherit floating point (Float or Double) type */
        if (ltFP && (rtInt || lt == rt || rAny))
            return lt;
        if (rtFP && (ltInt || lt == rt || lAny))
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
    dbg.nospace() << "," << token << ",";
    if (children.count() == 2 && right().constData()) {
        right()->debug(dbg, callStack);
    }
    else {
        dbg.nospace() << "<NONE>";
    }
    dbg.nospace() << ",type=" << KDbDriver::defaultSqlTypeName(type()) << ")";
}

KDbEscapedString KDbBinaryExpressionData::toStringInternal(
                                        const KDbDriver *driver,
                                        KDbQuerySchemaParameterValueListIterator* params,
                                        KDb::ExpressionCallStack* callStack) const
{
    switch (token.value()) {
    case '+':
    case CONCATENATION: {
        if (driver && KDbField::isTextType(type())) {
            const KDbBinaryExpression binaryExpr(const_cast<KDbBinaryExpressionData*>(this));
            return driver->concatenateFunctionToString(binaryExpr, params, callStack);
        }
        break;
    }
    default:;
    }

#define INFIX(a) \
    (left().constData() ? left()->toString(driver, params, callStack) : KDbEscapedString("<NULL>")) \
    + " " + a + " " + (right().constData() ? right()->toString(driver, params, callStack) : KDbEscapedString("<NULL>"))
    return INFIX(token.toString(driver));
#undef INFIX
}

void KDbBinaryExpressionData::getQueryParameters(QList<KDbQuerySchemaParameter>* params)
{
    Q_ASSERT(params);
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
                                         KDbToken token,
                                         const KDbExpression& rightExpr)
{
    if (leftExpr.isNull()) {
        kdbWarning() << "KDbBinaryExpression set to null because left argument is not specified";
        return KDb::UnknownExpression;
    }
    if (rightExpr.isNull()) {
        kdbWarning() << "KDbBinaryExpression set to null because right argument is not specified";
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
                                         KDbToken token,
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

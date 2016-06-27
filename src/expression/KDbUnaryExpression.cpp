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
#include "kdb_debug.h"
#include "generated/sqlparser.h"

KDbUnaryExpressionData::KDbUnaryExpressionData()
 : KDbExpressionData()
{
    ExpressionDebug << "UnaryExpressionData" << ref;
}

KDbUnaryExpressionData::~KDbUnaryExpressionData()
{
    ExpressionDebug << "~UnaryExpressionData" << ref;
}

KDbUnaryExpressionData* KDbUnaryExpressionData::clone()
{
    ExpressionDebug << "UnaryExpressionData::clone" << *this;
    return new KDbUnaryExpressionData(*this);
}

void KDbUnaryExpressionData::debugInternal(QDebug dbg, KDb::ExpressionCallStack* callStack) const
{
    dbg.nospace() << "UnaryExp("
           << token << ",";
    if (children.isEmpty()) {
        dbg.nospace() << "<NONE>";
    }
    else {
        ExplicitlySharedExpressionDataPointer a = arg();
        if (a.data()) {
            a->debug(dbg, callStack);
        }
        else {
            dbg.nospace() << "<NONE>";
        }
    }
    dbg.nospace() << qPrintable(QString::fromLatin1(",type=%1)")
        .arg(KDbDriver::defaultSQLTypeName(type())));
}

KDbEscapedString KDbUnaryExpressionData::toStringInternal(
                                        const KDbDriver *driver,
                                        KDbQuerySchemaParameterValueListIterator* params,
                                        KDb::ExpressionCallStack* callStack) const
{
    Q_UNUSED(callStack);
    ExplicitlySharedExpressionDataPointer a = arg();
    KDbEscapedString aString = a.constData()
            ? a->toString(driver, params, callStack) : KDbEscapedString("<NULL>");
    if (token == '(') { //parentheses (special case)
        return "(" + aString + ")";
    }
    if (token.toChar() > 0) {
        return token.toString(driver) + aString;
    }
    switch (token.value()) {
    case NOT:
        return "NOT " + aString;
    case SQL_IS_NULL:
        return aString + " IS NULL";
    case SQL_IS_NOT_NULL:
        return aString + " IS NOT NULL";
    }
    return KDbEscapedString("%1 %2").arg(token.toString(driver)).arg(aString);
}

void KDbUnaryExpressionData::getQueryParameters(QList<KDbQuerySchemaParameter>* params)
{
    Q_ASSERT(params);
    ExplicitlySharedExpressionDataPointer a = arg();
    if (a.constData())
        a->getQueryParameters(params);
}

KDbField::Type KDbUnaryExpressionData::typeInternal(KDb::ExpressionCallStack* callStack) const
{
    if (children.isEmpty()) {
        return KDbField::InvalidType;
    }
    ExplicitlySharedExpressionDataPointer a = arg();
    if (!a.constData())
        return KDbField::InvalidType;

    //NULL IS NOT NULL : BOOLEAN
    //NULL IS NULL : BOOLEAN
    switch (token.value()) {
    case SQL_IS_NULL:
    case SQL_IS_NOT_NULL:
        //! @todo queryParameterExpressionData->m_type still is Text but can be any type
        return KDbField::Boolean;
    }

    KDbQueryParameterExpressionData *queryParameterExpressionData = a->convert<KDbQueryParameterExpressionData>();
    if (queryParameterExpressionData) {
        switch (token.value()) {
        case '+':
        case '-':
        case '~':
            queryParameterExpressionData->m_type = KDbField::Integer;
            break;
        case '!':
        case NOT:
            queryParameterExpressionData->m_type = KDbField::Boolean;
            break;
        default:;
        }
    }
    KDbField::Type t = a->type(callStack);
    if (t == KDbField::Null)
        return KDbField::Null;
    if (token == KDbToken::NOT) {
        return t == KDbField::Boolean ? KDbField::Boolean : KDbField::InvalidType;
    }
    return t;
}

bool KDbUnaryExpressionData::validateInternal(KDbParseInfo *parseInfo, KDb::ExpressionCallStack* callStack)
{
    ExplicitlySharedExpressionDataPointer a = arg();
    if (!a.constData())
        return false;

    if (!a->validate(parseInfo, callStack))
        return false;

//! @todo compare types... e.g. NOT applied to Text makes no sense...

    // update type
//! @todo IMPORTANT: update type
#if 0
    if (a->toQueryParameter()) {
        a->toQueryParameter()->setType(type());
    }
#endif

    return typeInternal(callStack) != KDbField::InvalidType;
#if 0
    KDbExpression *n = l.at(0);

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

KDbUnaryExpression::KDbUnaryExpression()
 : KDbExpression(new KDbUnaryExpressionData)
{
    ExpressionDebug << "KDbUnaryExpression() ctor" << *this;
}

KDbUnaryExpression::KDbUnaryExpression(KDbToken token, const KDbExpression& arg)
        : KDbExpression(new KDbUnaryExpressionData, KDb::UnaryExpression, token)
{
    appendChild(arg.d);
}

KDbUnaryExpression::KDbUnaryExpression(KDbExpressionData* data)
 : KDbExpression(data)
{
    ExpressionDebug << "KDbUnaryExpression(KDbExpressionData*) ctor" << *this;
}

KDbUnaryExpression::KDbUnaryExpression(const KDbUnaryExpression& expr)
        : KDbExpression(expr)
{
}

KDbUnaryExpression::KDbUnaryExpression(const ExplicitlySharedExpressionDataPointer &ptr)
    : KDbExpression(ptr)
{
}

KDbUnaryExpression::~KDbUnaryExpression()
{
}

KDbExpression KDbUnaryExpression::arg() const
{
    return KDbExpression(d->convertConst<KDbUnaryExpressionData>()->arg());
}

void KDbUnaryExpression::setArg(const KDbExpression &arg)
{
    if (!d->children.isEmpty()) {
        removeChild(0);
    }
    insertChild(0, arg);
}

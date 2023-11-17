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
#include "KDbDriver.h"
#include "KDbQuerySchema.h"
#include "KDbParser_p.h"
#include "kdb_debug.h"

KDbNArgExpressionData::KDbNArgExpressionData()
 : KDbExpressionData()
{
    ExpressionDebug << "NArgExpressionData" << ref;
}

KDbNArgExpressionData::~KDbNArgExpressionData()
{
    ExpressionDebug << "~NArgExpressionData" << ref;
}

KDbField::Type KDbNArgExpressionData::typeInternal(KDb::ExpressionCallStack *callStack) const
{
    if (token == KDbToken::BETWEEN_AND || token == KDbToken::NOT_BETWEEN_AND) {
        for (int i = 0; i < children.count(); i++) {
            KDbField::Type t = children.at(0)->type(callStack);
            if (t == KDbField::InvalidType || t == KDbField::Null) {
                return t;
            }
        }
        return KDbField::Boolean;
    }
    return KDbField::Tuple;
}

KDbNArgExpressionData* KDbNArgExpressionData::clone()
{
    ExpressionDebug << "NArgExpressionData::clone" << *this;
    return new KDbNArgExpressionData(*this);
}

bool KDbNArgExpressionData::validateInternal(KDbParseInfo *parseInfo, KDb::ExpressionCallStack* callStack)
{
    for(ExplicitlySharedExpressionDataPointer data : std::as_const(children)) {
        if (!data->validate(parseInfo, callStack))
            return false;
    }

    if (token == KDbToken::BETWEEN_AND || token == KDbToken::NOT_BETWEEN_AND) {
        if (children.count() != 3) {
            parseInfo->setErrorMessage(tr("Three arguments required"));
            parseInfo->setErrorDescription(
                tr("%1 operator requires exactly three arguments.", "BETWEEN..AND error")
                                  .arg(QLatin1String("BETWEEN...AND")));
            return false;
        }
        const KDbField::Type t0 = children[0]->type();
        const KDbField::Type t1 = children[1]->type();
        const KDbField::Type t2 = children[2]->type();
        if (t0 == KDbField::Null || t1 == KDbField::Null || t2 == KDbField::Null) {
            return true;
        }
        if (!(   !KDbField::isNumericType(t0)
              || !KDbField::isNumericType(t1)
              || !KDbField::isNumericType(t2)))
        {
            return true;
        } else if (!(   !KDbField::isTextType(t0)
                     || !KDbField::isTextType(t1)
                     || !KDbField::isTextType(t2)))
        {
            return true;
        }

        if (t0 == t1 && t1 == t2) {
            return true;
        }

        parseInfo->setErrorMessage(tr("Incompatible types of arguments"));
        parseInfo->setErrorDescription(
                           tr("Operator \"%1\" requires compatible types of arguments. "
                              "Specified arguments are of type %2, %3, %4.",
                              "BETWEEN..AND arguments type error")
                              .arg(QLatin1String("BETWEEN..AND"),
                                   KDbField::typeName(t0),
                                   KDbField::typeName(t1),
                                   KDbField::typeName(t2)));
        return false;
    }
    return true;
}

void KDbNArgExpressionData::debugInternal(QDebug dbg, KDb::ExpressionCallStack* callStack) const
{
    dbg.nospace() << "NArgExp("
        << token.name() << ", class=" << expressionClassName(expressionClass);
    for(ExplicitlySharedExpressionDataPointer data : std::as_const(children)) {
        dbg.nospace() << ", ";
        data->debug(dbg, callStack);
    }
    dbg.nospace() << ",type=" << KDbDriver::defaultSqlTypeName(type()) << ")";
}

KDbEscapedString KDbNArgExpressionData::toStringInternal(
                                        const KDbDriver *driver,
                                        KDbQuerySchemaParameterValueListIterator* params,
                                        KDb::ExpressionCallStack* callStack) const
{
    if (token == KDbToken::BETWEEN_AND && children.count() == 3) {
        return children[0]->toString(driver, params, callStack) + " BETWEEN "
                + children[1]->toString(driver, params, callStack) + " AND "
                + children[2]->toString(driver, params, callStack);
    }
    if (token == KDbToken::NOT_BETWEEN_AND && children.count() == 3) {
        return children[0]->toString(driver, params, callStack) + " NOT BETWEEN "
                + children[1]->toString(driver, params, callStack) + " AND "
                + children[2]->toString(driver, params, callStack);
    }

    KDbEscapedString s;
    s.reserve(256);
    for(ExplicitlySharedExpressionDataPointer data : std::as_const(children)) {
        if (!s.isEmpty())
            s += ", ";
        s += data->toString(driver, params, callStack);
    }
    return s;
}

void KDbNArgExpressionData::getQueryParameters(QList<KDbQuerySchemaParameter>* params)
{
    Q_ASSERT(params);
    for(ExplicitlySharedExpressionDataPointer data : std::as_const(children)) {
        data->getQueryParameters(params);
    }
}

bool KDbNArgExpressionData::containsInvalidArgument() const
{
    for(const ExplicitlySharedExpressionDataPointer &data : std::as_const(children)) {
        const KDbField::Type type = data->type();
        if (type == KDbField::InvalidType) {
            return true;
        }
    }
    return false;
}

bool KDbNArgExpressionData::containsNullArgument() const
{
    for(const ExplicitlySharedExpressionDataPointer &data : std::as_const(children)) {
        const KDbField::Type type = data->type();
        if (type == KDbField::Null) {
            return true;
        }
    }
    return false;
}

//=========================================

KDbNArgExpression::KDbNArgExpression()
 : KDbExpression(new KDbNArgExpressionData)
{
    ExpressionDebug << "KDbNArgExpression() ctor" << *this;
}

KDbNArgExpression::KDbNArgExpression(KDbExpressionData* data)
 : KDbExpression(data)
{
    ExpressionDebug << "KDbNArgExpression(KDbExpressionData*) ctor" << *this;
}

KDbNArgExpression::KDbNArgExpression(KDb::ExpressionClass aClass, KDbToken token)
        : KDbExpression(new KDbNArgExpressionData, aClass, token)
{
    ExpressionDebug << "KDbNArgExpression(KDb::ExpressionClass, int) ctor" << *this;
}

KDbNArgExpression::KDbNArgExpression(const KDbNArgExpression& expr)
        : KDbExpression(expr)
{
}

KDbNArgExpression::KDbNArgExpression(const ExplicitlySharedExpressionDataPointer &ptr)
    : KDbExpression(ptr)
{
}

KDbNArgExpression::~KDbNArgExpression()
{
}

void KDbNArgExpression::append(const KDbExpression& expr)
{
    appendChild(expr);
}

void KDbNArgExpression::prepend(const KDbExpression& expr)
{
    prependChild(expr);
}

KDbExpression KDbNArgExpression::arg(int i) const
{
    return KDbExpression(d->children.value(i));
}

void KDbNArgExpression::insert(int i, const KDbExpression& expr)
{
    insertChild(i, expr);
}

void KDbNArgExpression::replace(int i, const KDbExpression& expr)
{
    if (!checkBeforeInsert(expr.d))
        return;
    if (i < 0 || i > d->children.count())
        return;
    d->children.at(i)->parent.reset();
    d->children.replace(i, expr.d);
    expr.d->parent = d;
}

bool KDbNArgExpression::remove(const KDbExpression& expr)
{
    return removeChild(expr);
}

void KDbNArgExpression::removeAt(int i)
{
    removeChild(i);
}

KDbExpression KDbNArgExpression::takeAt(int i)
{
    return takeChild(i);
}

int KDbNArgExpression::indexOf(const KDbExpression& expr, int from) const
{
    return indexOfChild(expr, from);
}

int KDbNArgExpression::lastIndexOf(const KDbExpression& expr, int from) const
{
    return lastIndexOfChild(expr, from);
}

int KDbNArgExpression::argCount() const
{
    return d->children.count();
}

bool KDbNArgExpression::isEmpty() const
{
    return d->children.isEmpty();
}

bool KDbNArgExpression::containsInvalidArgument() const
{
    return d->convert<KDbNArgExpressionData>()->containsInvalidArgument();
}

bool KDbNArgExpression::containsNullArgument() const
{
    return d->convert<KDbNArgExpressionData>()->containsNullArgument();
}

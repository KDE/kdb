/* This file is part of the KDE project
   Copyright (C) 2003-2015 Jarosław Staniek <staniek@kde.org>
   Copyright (C) 2014 Radoslaw Wicik <radoslaw@wicik.pl>

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
 * Boston, MA 02110-1301, USA.
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 */

#include "KDbExpression.h"
#include "KDb.h"
#include "KDbDriver.h"
#include "KDbQuerySchema.h"
#include "KDbParser_p.h"
#include "kdb_debug.h"
#include "generated/sqlparser.h"

#include <vector>

//! @internal A cache
class KDbExpressionClassNames
{
public:
    KDbExpressionClassNames()
     : names({
            QLatin1String("Unknown"),
            QLatin1String("Unary"),
            QLatin1String("Arithm"),
            QLatin1String("Logical"),
            QLatin1String("Relational"),
            QLatin1String("SpecialBinary"),
            QLatin1String("Const"),
            QLatin1String("Variable"),
            QLatin1String("Function"),
            QLatin1String("Aggregation"),
            QLatin1String("FieldList"),
            QLatin1String("TableList"),
            QLatin1String("ArgumentList"),
            QLatin1String("QueryParameter")})
    {
    }
    const std::vector<QString> names;
};

Q_GLOBAL_STATIC(KDbExpressionClassNames, KDb_expressionClassNames)

KDB_EXPORT QString expressionClassName(KDb::ExpressionClass c)
{
    Q_ASSERT(size_t(c) < KDb_expressionClassNames->names.size());
    return KDb_expressionClassNames->names[c];
}

KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbExpression& expr)
{
    KDb::ExpressionCallStack callStack;
    return expr.debug(dbg.nospace(), &callStack);
}

//=========================================

KDbExpressionData::KDbExpressionData()
    : expressionClass(KDb::UnknownExpression)
{
    //ExpressionDebug << "KDbExpressionData" << ref;
}

/*Data(const Data& other)
: QSharedData(other)
, token(other.token)
, expressionClass(other.expressionClass)
, parent(other.parent)
, children(other.children)
{
    ExpressionDebug << "KDbExpressionData" << ref;
}*/

KDbExpressionData::~KDbExpressionData()
{
    //ExpressionDebug << "~KDbExpressionData" << ref;
}

KDbExpressionData* KDbExpressionData::clone()
{
    ExpressionDebug << "KDbExpressionData::clone" << *this;
    return new KDbExpressionData(*this);
}

KDbField::Type KDbExpressionData::typeInternal(KDb::ExpressionCallStack* callStack) const
{
    Q_UNUSED(callStack);
    return KDbField::InvalidType;
}

KDbField::Type KDbExpressionData::type(KDb::ExpressionCallStack* callStack) const
{
    if (!addToCallStack(nullptr, callStack)) {
        return KDbField::InvalidType;
    }
    const KDbField::Type t = typeInternal(callStack);
    callStack->removeLast();
    return t;
}

KDbField::Type KDbExpressionData::type() const
{
    KDb::ExpressionCallStack callStack;
    return type(&callStack);
}

bool KDbExpressionData::isValid() const
{
    return type() != KDbField::InvalidType;
}

bool KDbExpressionData::isTextType() const
{
    return KDbField::isTextType(type());
}

bool KDbExpressionData::isIntegerType() const
{
    return KDbField::isIntegerType(type());
}

bool KDbExpressionData::isNumericType() const
{
    return KDbField::isNumericType(type());
}

bool KDbExpressionData::isFPNumericType() const
{
    return KDbField::isFPNumericType(type());
}

bool KDbExpressionData::isDateTimeType() const
{
    return KDbField::isDateTimeType(type());
}

bool KDbExpressionData::validate(KDbParseInfo *parseInfo)
{
    KDb::ExpressionCallStack callStack;
    return validate(parseInfo, &callStack);
}

bool KDbExpressionData::validate(KDbParseInfo *parseInfo, KDb::ExpressionCallStack* callStack)
{
    if (!addToCallStack(nullptr, callStack)) {
        return false;
    }
    bool result = validateInternal(parseInfo, callStack);
    callStack->removeLast();
    return result;
}

bool KDbExpressionData::validateInternal(KDbParseInfo *parseInfo, KDb::ExpressionCallStack* callStack)
{
    Q_UNUSED(parseInfo);
    Q_UNUSED(callStack);
    return true;
}

KDbEscapedString KDbExpressionData::toString(
                                       const KDbDriver *driver,
                                       KDbQuerySchemaParameterValueListIterator* params,
                                       KDb::ExpressionCallStack* callStack) const
{
    const bool owned = !callStack;
    if (owned) {
        callStack = new KDb::ExpressionCallStack();
    }
    if (!addToCallStack(nullptr, callStack)) {
        if (owned) {
            delete callStack;
        }
        return KDbEscapedString("<CYCLE!>");
    }
    KDbEscapedString s = toStringInternal(driver, params, callStack);
    callStack->removeLast();
    if (owned) {
        delete callStack;
    }
    return s;
}

KDbEscapedString KDbExpressionData::toStringInternal(
                                          const KDbDriver *driver,
                                          KDbQuerySchemaParameterValueListIterator* params,
                                          KDb::ExpressionCallStack* callStack) const
{
    Q_UNUSED(driver);
    Q_UNUSED(params);
    Q_UNUSED(callStack);
    return KDbEscapedString("<UNKNOWN!>");
}

void KDbExpressionData::getQueryParameters(QList<KDbQuerySchemaParameter>* params)
{
    Q_UNUSED(params);
}

bool KDbExpressionData::addToCallStack(QDebug *dbg, QList<const KDbExpressionData*>* callStack) const
{
    if (callStack->contains(this)) {
        if (dbg)
            dbg->nospace() << "<CYCLE!>";
        kdbWarning() << "Cycle detected in"
            << expressionClassName(expressionClass) << token.value();
        return false;
    }
    callStack->append(this);
    return true;
}

QDebug KDbExpressionData::debug(QDebug dbg, KDb::ExpressionCallStack* callStack) const
{
    if (!addToCallStack(&dbg, callStack)) {
        return dbg.nospace();
    }
    debugInternal(dbg, callStack);
    callStack->removeLast();
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const KDbExpressionData& expr)
{
    KDb::ExpressionCallStack callStack;
    return expr.debug(dbg.nospace(), &callStack);
}

void KDbExpressionData::debugInternal(QDebug dbg, KDb::ExpressionCallStack* callStack) const
{
    Q_UNUSED(callStack);
    dbg.nospace() << QString::fromLatin1("Exp(%1,type=%2)")
                   .arg(token.value()).arg(KDbDriver::defaultSqlTypeName(type()));
}

//=========================================

KDbExpression::KDbExpression()
    : d(new KDbExpressionData)
{
    ExpressionDebug << "KDbExpression ctor ()" << *this << d->ref;
}

KDbExpression::KDbExpression(KDbExpressionData* data, KDb::ExpressionClass aClass, KDbToken token)
    : d(data)
{
    d->expressionClass = aClass;
    d->token = token;
}

KDbExpression::KDbExpression(KDbExpressionData* data)
    : d(data)
{
    ExpressionDebug << "KDbExpression ctor (KDbExpressionData*)" << *this;
}

KDbExpression::KDbExpression(const ExplicitlySharedExpressionDataPointer &ptr)
    : d(ptr ? ptr : ExplicitlySharedExpressionDataPointer(new KDbExpressionData))
{
}

KDbExpression::~KDbExpression()
{
    //kdbDebug() << *this << d->ref;
    if (d->parent && d->ref == 1) {
         d->parent->children.removeOne(d);
    }
}

bool KDbExpression::isNull() const
{
    return d->expressionClass == KDb::UnknownExpression;
}

KDbExpression KDbExpression::clone() const
{
    return KDbExpression(d->clone());
}

KDbToken KDbExpression::token() const
{
    return d->token;
}

void KDbExpression::setToken(KDbToken token)
{
    d->token = token;
}

KDb::ExpressionClass KDbExpression::expressionClass() const
{
    return d->expressionClass;
}

void KDbExpression::setExpressionClass(KDb::ExpressionClass aClass)
{
    d->expressionClass = aClass;
}

bool KDbExpression::validate(KDbParseInfo *parseInfo)
{
    return d->validate(parseInfo);
}

KDbField::Type KDbExpression::type() const
{
    return d->type();
}

bool KDbExpression::isValid() const
{
    return d->isValid();
}

bool KDbExpression::isTextType() const
{
    return d->isTextType();
}

bool KDbExpression::isIntegerType() const
{
    return d->isIntegerType();
}

bool KDbExpression::isNumericType() const
{
    return d->isNumericType();
}

bool KDbExpression::isFPNumericType() const
{
    return d->isFPNumericType();
}

bool KDbExpression::isDateTimeType() const
{
    return d->isDateTimeType();
}

KDbExpression KDbExpression::parent() const
{
    return d->parent.data() ? KDbExpression(d->parent) : KDbExpression();
}

QList<ExplicitlySharedExpressionDataPointer> KDbExpression::children() const
{
    return d->children;
}

void KDbExpression::appendChild(const KDbExpression& child)
{
    appendChild(child.d);
}

void KDbExpression::prependChild(const KDbExpression& child)
{
    if (!checkBeforeInsert(child.d))
        return;
    d->children.prepend(child.d);
    child.d->parent = d;
}

void KDbExpression::insertChild(int i, const KDbExpression& child)
{
    if (!checkBeforeInsert(child.d))
        return;
    if (i < 0 || i > d->children.count())
        return;
    d->children.insert(i, child.d);
    child.d->parent = d;
}

void KDbExpression::insertEmptyChild(int i)
{
    if (i < 0 || i > d->children.count())
        return;
    KDbExpression child;
    d->children.insert(i, child.d);
    child.d->parent = d;
}

bool KDbExpression::removeChild(const KDbExpression& child)
{
    if (isNull() || child.isNull())
        return false;
    child.d->parent.reset(); // no longer parent
    return d->children.removeOne(child.d);
}

void KDbExpression::removeChild(int i)
{
    if (isNull())
        return;
    if (i < 0 || i >= d->children.count())
        return;
    //kdbDebug() << d->children.count() << d->children.at(i);
    d->children.removeAt(i);
}

KDbExpression KDbExpression::takeChild(int i)
{
    if (isNull())
        return KDbExpression();
    if (i < 0 || i >= d->children.count())
        return KDbExpression();
    ExplicitlySharedExpressionDataPointer child = d->children.takeAt(i);
    if (!child)
        return KDbExpression();
    child->parent.reset();
    return KDbExpression(child);
}

int KDbExpression::indexOfChild(const KDbExpression& child, int from) const
{
    return d->children.indexOf(child.d, from);
}

int KDbExpression::lastIndexOfChild(const KDbExpression& child, int from) const
{
    return d->children.lastIndexOf(child.d, from);
}

bool KDbExpression::checkBeforeInsert(const ExplicitlySharedExpressionDataPointer& child)
{
    if (!child)
        return false;
    if (d == child) // expression cannot be own child
        return false;
    if (child->parent == d) // cannot insert child twice
        return false;
    if (child->parent) // remove from old parent
        child->parent->children.removeOne(child);
    return true;
}

void KDbExpression::appendChild(const ExplicitlySharedExpressionDataPointer& child)
{
    if (!checkBeforeInsert(child))
        return;
    d->children.append(child);
    child->parent = d;
}

KDbEscapedString KDbExpression::toString(const KDbDriver *driver,
                                         KDbQuerySchemaParameterValueListIterator* params,
                                         KDb::ExpressionCallStack* callStack) const
{
    if (isNull())
        return KDbEscapedString("<UNKNOWN!>");
    return d->toString(driver, params, callStack);
}

void KDbExpression::getQueryParameters(QList<KDbQuerySchemaParameter>* params)
{
    Q_ASSERT(params);
    d->getQueryParameters(params);
}

QDebug KDbExpression::debug(QDebug dbg, KDb::ExpressionCallStack* callStack) const
{
    if (d)
        d->debug(dbg, callStack);
    return dbg.space();
}

bool KDbExpression::operator==(const KDbExpression& e) const
{
    return d == e.d;
}

bool KDbExpression::operator!=(const KDbExpression& e) const
{
    return !operator==(e);
}

bool KDbExpression::isNArg() const
{
    return d->convertConst<KDbNArgExpressionData>();
}

bool KDbExpression::isUnary() const
{
    return d->convertConst<KDbUnaryExpressionData>();
}

bool KDbExpression::isBinary() const
{
    return d->convertConst<KDbBinaryExpressionData>();
}

bool KDbExpression::isConst() const
{
    return d->convertConst<KDbConstExpressionData>();
}

bool KDbExpression::isVariable() const
{
    return d->convertConst<KDbVariableExpressionData>();
}

bool KDbExpression::isFunction() const
{
    return d->convertConst<KDbFunctionExpressionData>();
}

bool KDbExpression::isQueryParameter() const
{
    return d->convertConst<KDbQueryParameterExpressionData>();
}

#define CAST(T) \
    d->convert<T ## Data>() ? T(d) : T()

KDbNArgExpression KDbExpression::toNArg() const
{
    return CAST(KDbNArgExpression);
}

KDbUnaryExpression KDbExpression::toUnary() const
{
    return CAST(KDbUnaryExpression);
}

KDbBinaryExpression KDbExpression::toBinary() const
{
    return CAST(KDbBinaryExpression);
}

KDbConstExpression KDbExpression::toConst() const
{
    return CAST(KDbConstExpression);
}

KDbQueryParameterExpression KDbExpression::toQueryParameter() const
{
    return CAST(KDbQueryParameterExpression);
}

KDbVariableExpression KDbExpression::toVariable() const
{
    return CAST(KDbVariableExpression);
}

KDbFunctionExpression KDbExpression::toFunction() const
{
    return CAST(KDbFunctionExpression);
}

void KDbExpression::setLeftOrRight(const KDbExpression& e, int index)
{
    if (this == &e) {
        kdbWarning() << "Expression" << *this << "cannot be set as own child";
        return;
    }
    if (d->children.indexOf(e.d) == index) { // cannot set twice
        return;
    }
    if (d->children[index == 0 ? 1 : 0] == e.d) { // this arg was at right, remove
        d->children[index] = e.d;
        d->children[index == 0 ? 1 : 0] = new KDbExpressionData;
    }
    else {
        if (e.d->parent) { // remove from old parent
            e.d->parent->children.removeOne(e.d);
        }
        d->children[index] = e.d;
    }
}

// static
KDb::ExpressionClass KDbExpression::classForToken(KDbToken token)
{
    switch (token.value()) {
    case '+':
    case '-':
    case '*':
    case '/':
    case '&':
    case '|':
    case '%':
    case BITWISE_SHIFT_RIGHT:
    case BITWISE_SHIFT_LEFT:
    case CONCATENATION:
        return KDb::ArithmeticExpression;
    case '=':
    case '<':
    case '>':
    case NOT_EQUAL:
    case NOT_EQUAL2:
    case LESS_OR_EQUAL:
    case GREATER_OR_EQUAL:
    case LIKE:
    case NOT_LIKE:
    case SQL_IN:
    case SIMILAR_TO:
    case NOT_SIMILAR_TO:
        return KDb::RelationalExpression;
    case OR:
    case AND:
    case XOR:
        return KDb::LogicalExpression;
    case AS:
    case AS_EMPTY:
        return KDb::SpecialBinaryExpression;
    default:;
    }
    return KDb::UnknownExpression;
}

#undef CAST

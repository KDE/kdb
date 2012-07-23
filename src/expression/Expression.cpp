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
 * Boston, MA 02110-1301, USA.
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 */

#include <Predicate/Expression>
#include <Predicate/Utils>
#include <Predicate/QuerySchema>
#include "parser/SqlParser.h"
#include "parser/Parser_p.h"
#include <Predicate/Tools/Static>

#include <ctype.h>

using namespace Predicate;

//! Cache
class ExpressionClassNames
{
public:
    ExpressionClassNames()
     : names((QLatin1String[]){
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
            QLatin1String("QueryParameter")})
    {
    }
    const QLatin1String names[13];
};

PREDICATE_EXPORT QString Predicate::expressionClassName(ExpressionClass c)
{
    PREDICATE_GLOBAL_STATIC(ExpressionClassNames, Predicate_expressionClassNames)
    Q_ASSERT(c < sizeof(Predicate_expressionClassNames->names));
    return Predicate_expressionClassNames->names[c];
}

PREDICATE_EXPORT QDebug operator<<(QDebug dbg, const Predicate::Expression& expr)
{
    CallStack callStack;
    return expr.debug(dbg.nospace(), &callStack);
}

//=========================================

ExpressionData::ExpressionData()
: token(0)
, expressionClass(Predicate::UnknownExpressionClass)
{
    //ExpressionDebug << "ExpressionData" << ref;
}

/*Data(const Data& other)
: QSharedData(other)
, token(other.token)
, expressionClass(other.expressionClass)
, parent(other.parent)
, children(other.children)
{
    ExpressionDebug << "ExpressionData" << ref;
}*/

ExpressionData::~ExpressionData()
{
    //ExpressionDebug << "~ExpressionData" << ref;
}

ExpressionData* ExpressionData::clone()
{
    ExpressionDebug << "ExpressionData::clone" << *this;
    return new ExpressionData(*this);
}

Field::Type ExpressionData::typeInternal(CallStack* callStack) const
{
    Q_UNUSED(callStack);
    return Field::InvalidType;
}

Field::Type ExpressionData::type(CallStack* callStack) const
{
    if (!addToCallStack(0, callStack)) {
        return Field::InvalidType;
    }
    const Field::Type t = typeInternal(callStack);
    callStack->removeLast();
    return t;
}

Field::Type ExpressionData::type() const
{
    CallStack callStack;
    return type(&callStack);
}

bool ExpressionData::validate(ParseInfo *parseInfo)
{
    CallStack callStack;
    return validate(parseInfo, &callStack);
}

bool ExpressionData::validate(ParseInfo *parseInfo, CallStack* callStack)
{
    if (!addToCallStack(0, callStack)) {
        return false;
    }
    bool result = validateInternal(parseInfo, callStack);
    callStack->removeLast();
    return result;
}

bool ExpressionData::validateInternal(ParseInfo *parseInfo, CallStack* callStack)
{
    Q_UNUSED(parseInfo);
    Q_UNUSED(callStack);
    return true;
}

QString ExpressionData::tokenToString() const
{
    if (token < 255 && isprint(token))
        return Expression::tokenToDebugString(token);
    return QString();
}

EscapedString ExpressionData::toString(QuerySchemaParameterValueListIterator* params) const
{
    CallStack callStack;
    return toString(params, &callStack);
}

EscapedString ExpressionData::toString(QuerySchemaParameterValueListIterator* params,
                                       CallStack* callStack) const
{
    if (!addToCallStack(0, callStack)) {
        return EscapedString("<CYCLE!>");
    }
    EscapedString s = toStringInternal(params, callStack);
    callStack->removeLast();
    return s;
}

EscapedString ExpressionData::toStringInternal(QuerySchemaParameterValueListIterator* params,
                                               CallStack* callStack) const
{
    Q_UNUSED(params);
    Q_UNUSED(callStack);
    return EscapedString("<NULL!>");
}

void ExpressionData::getQueryParameters(QuerySchemaParameterList& params)
{
    Q_UNUSED(params);
}

bool ExpressionData::addToCallStack(QDebug *dbg, QList<const ExpressionData*>* callStack) const
{
    if (callStack->contains(this)) {
        if (dbg)
            dbg->nospace() << "<CYCLE!>";
        qWarning() << "Cycle detected in"
            << expressionClassName(expressionClass) << Expression::tokenToDebugString(token);
        return false;
    }
    callStack->append(this);
    return true;
}

QDebug ExpressionData::debug(QDebug dbg, CallStack* callStack) const
{
    if (!addToCallStack(&dbg, callStack)) {
        return dbg.nospace();
    }
    debugInternal(dbg, callStack);
    callStack->removeLast();
    return dbg.nospace();
}

QDebug operator<<(QDebug dbg, const ExpressionData& expr)
{
    CallStack callStack;
    return expr.debug(dbg.nospace(), &callStack);
}

void ExpressionData::debugInternal(QDebug dbg, CallStack* callStack) const
{
    Q_UNUSED(callStack);
    dbg.nospace() << QString::fromLatin1("Exp(%1,type=%2)")
                   .arg(token).arg(Driver::defaultSQLTypeName(type()));
}

//=========================================

Expression::Expression()
    : d(new ExpressionData)
{
    ExpressionDebug << "Expression ctor ()" << *this << d->ref;
}

Expression::Expression(ExpressionData* data, ExpressionClass aClass, int token)
    : d(data)
{
    d->expressionClass = aClass;
    d->token = token;
}

Expression::Expression(ExpressionData* data)
    : d(data)
{
    ExpressionDebug << "Expression ctor (ExpressionData*)" << *this;
}

Expression::~Expression()
{
    //if (d->parent)
    //     d->parent->children.removeOne(d);
}

bool Expression::isNull() const
{
    return d->expressionClass == Predicate::UnknownExpressionClass;
}

Expression Expression::clone() const
{
    return Expression(d->clone());
}

int Expression::token() const
{
    return d->token;
}

ExpressionClass Expression::expressionClass() const
{
    return d->expressionClass;
}

bool Expression::validate(ParseInfo *parseInfo)
{
    return d->validate(parseInfo);
}

extern const char* tname(int offset);
#define safe_tname(token) ((token>=255 && token<=__LAST_TOKEN) ? tname(token-255) : "")

// static
QString Expression::tokenToDebugString(int token)
{
    if (token < 254) {
        if (isprint(token))
            return QString(QLatin1Char(uchar(token)));
        else
            return QString::number(token);
    }
    return QLatin1String(safe_tname(token));
}

Field::Type Expression::type() const
{
    return d->type();
}

QString Expression::tokenToString() const
{
    return d->tokenToString();
}

Expression Expression::parent() const
{
    return d->parent.data() ? Expression(d->parent.data()) : Expression();
}

QList<ExplicitlySharedExpressionDataPointer> Expression::children() const
{
    return d->children;
}

void Expression::appendChild(const Expression& child)
{
    appendChild(child.d);
}

void Expression::prependChild(const Expression& child)
{
    if (!checkBeforeInsert(child.d))
        return;
    d->children.prepend(child.d);
    child.d->parent = d;
}

void Expression::insertChild(int i, const Expression& child)
{
    if (!checkBeforeInsert(child.d))
        return;
    if (i < 0 || i > d->children.count())
        return;
    d->children.insert(i, child.d);
    child.d->parent = d;
}

void Expression::insertEmptyChild(int i)
{
    //if (isNull())
    //    return;
    if (i < 0 || i > d->children.count())
        return;
    Expression child;
    //ExplicitlySharedExpressionDataPointer child = new Expression();
    d->children.insert(i, child.d);
    child.d->parent = d;
}

bool Expression::removeChild(const Expression& child)
{
    if (isNull() || child.isNull())
        return false;
    child.d->parent.reset(); // no longer parent
    return d->children.removeOne(child.d);
}

void Expression::removeChild(int i)
{
    if (isNull())
        return;
    if (i < 0 || i >= d->children.count())
        return;
    qDebug() << d->children.count() << d->children.at(i);
    d->children.removeAt(i);
}

Expression Expression::takeChild(int i)
{
    if (isNull())
        return Expression();
    if (i < 0 || i >= d->children.count())
        return Expression();
    ExplicitlySharedExpressionDataPointer child = d->children.takeAt(i);
    if (!child)
        return Expression();
    child->parent.reset();
    return Expression(child.data());
}

int Expression::indexOfChild(const Expression& child, int from) const
{
    return d->children.indexOf(child.d, from);
}

int Expression::lastIndexOfChild(const Expression& child, int from) const
{
    return d->children.lastIndexOf(child.d, from);
}

bool Expression::checkBeforeInsert(const ExplicitlySharedExpressionDataPointer& child)
{
    if (isNull())
        return false;
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

void Expression::appendChild(const ExplicitlySharedExpressionDataPointer& child)
{
    if (!checkBeforeInsert(child))
        return;
    d->children.append(child);
    child->parent = d;
}

EscapedString Expression::toString(QuerySchemaParameterValueListIterator* params) const
{
    if (isNull())
        return EscapedString("<NULL!>");
    return d->toString(params);
}

void Expression::getQueryParameters(QuerySchemaParameterList& params)
{
    d->getQueryParameters(params);
}

QDebug Expression::debug(QDebug dbg, CallStack* callStack) const
{
    if (d)
        d->debug(dbg, callStack);
    return dbg.space();
}

bool Expression::operator==(const Expression& e) const
{
    return d == e.d;
}

bool Expression::operator!=(const Expression& e) const
{
    return !operator==(e);
}

bool Expression::isNArg() const
{
    return d->convertConst<NArgExpressionData>();
}

bool Expression::isUnary() const
{
    return d->convertConst<UnaryExpressionData>();
}

bool Expression::isBinary() const
{
    return d->convertConst<BinaryExpressionData>();
}

bool Expression::isConst() const
{
    return d->convertConst<ConstExpressionData>();
}

bool Expression::isVariable() const
{
    return d->convertConst<VariableExpressionData>();
}

bool Expression::isFunction() const
{
    return d->convertConst<FunctionExpressionData>();
}

bool Expression::isQueryParameter() const
{
    return d->convertConst<QueryParameterExpressionData>();
}

#define CAST(T) \
    d->convert<T ## Data>() ? T(d.data()) : T()

NArgExpression Expression::toNArg() const
{
    return CAST(NArgExpression);
}

UnaryExpression Expression::toUnary() const
{
    return CAST(UnaryExpression);
}

BinaryExpression Expression::toBinary() const
{
    return CAST(BinaryExpression);
}

ConstExpression Expression::toConst() const
{
    return CAST(ConstExpression);
}

QueryParameterExpression Expression::toQueryParameter() const
{
    return CAST(QueryParameterExpression);
}

VariableExpression Expression::toVariable() const
{
    return CAST(VariableExpression);
}

FunctionExpression Expression::toFunction() const
{
    return CAST(FunctionExpression);
}

void Expression::setLeftOrRight(const Expression& e, int index)
{
    if (this == &e) {
        qWarning() << "Expression::setLeftOrRight(): Expression cannot be own child";
        return;
    }
    if (d->children.indexOf(e.d) == index) { // cannot set twice
        return;
    }
    if (d->children[index == 0 ? 1 : 0] == e.d) { // this arg was at right, remove
        d->children[index] = e.d;
        d->children[index == 0 ? 1 : 0] = new ExpressionData;
    }
    else {
        if (e.d->parent) { // remove from old parent
            e.d->parent->children.removeOne(e.d);
        }
        d->children[index] = e.d;
    }
}

// static
ExpressionClass Expression::classForToken(int token)
{
    switch (token) {
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
        return ArithmeticExpressionClass;
    case '=':
    case '<':
    case '>':
    case NOT_EQUAL:
    case NOT_EQUAL2:
    case LESS_OR_EQUAL:
    case GREATER_OR_EQUAL:
    case LIKE:
    case SQL_IN:
    case SIMILAR_TO:
    case NOT_SIMILAR_TO:
        return RelationalExpressionClass;
    case OR:
    case AND:
    case XOR:
        return LogicalExpressionClass;
    case AS:
        return SpecialBinaryExpressionClass;
    default:;
    }
    return UnknownExpressionClass;
}

#undef CAST

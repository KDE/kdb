/* This file is part of the KDE project
   Copyright (C) 2003-2015 Jarosław Staniek <staniek@kde.org>

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
#include "KDbParser.h"

#include <QtDebug>
#include <QSet>

class BuiltInAggregates : public QSet<QString>
{
public:
    BuiltInAggregates()
    {
        insert(QLatin1String("SUM"));
        insert(QLatin1String("MIN"));
        insert(QLatin1String("MAX"));
        insert(QLatin1String("AVG"));
        insert(QLatin1String("COUNT"));
        insert(QLatin1String("STD"));
        insert(QLatin1String("STDDEV"));
        insert(QLatin1String("VARIANCE"));
    }
};

Q_GLOBAL_STATIC(BuiltInAggregates, _builtInAggregates)

//=========================================

KDbFunctionExpressionData::KDbFunctionExpressionData()
 : KDbExpressionData()
{
    ExpressionDebug << "FunctionExpressionData" << ref;
    setArguments(ExplicitlySharedExpressionDataPointer());
}

KDbFunctionExpressionData::KDbFunctionExpressionData(const QString& aName,
                                                     ExplicitlySharedExpressionDataPointer arguments)
        : KDbExpressionData()
        , name(aName)
{
    setArguments(arguments);
    ExpressionDebug << "FunctionExpressionData" << ref << *args;
}

KDbFunctionExpressionData::~KDbFunctionExpressionData()
{
    ExpressionDebug << "~FunctionExpressionData" << ref;
}

KDbFunctionExpressionData* KDbFunctionExpressionData::clone()
{
    ExpressionDebug << "FunctionExpressionData::clone" << *this;
    KDbFunctionExpressionData *cloned = new KDbFunctionExpressionData(*this);
    ExpressionDebug << "FunctionExpressionData::clone" << *cloned;
    cloned->args = args->clone();
    return cloned;
}

void KDbFunctionExpressionData::debugInternal(QDebug dbg, KDb::ExpressionCallStack* callStack) const
{
    dbg.nospace() << "FunctionExp(" << name;
    if (args.data()) {
        dbg.nospace() << ',';
        args.data()->debug(dbg, callStack);
    }
    dbg.nospace() << QString::fromLatin1(",type=%1)").arg(KDbDriver::defaultSQLTypeName(type()));
}

KDbEscapedString KDbFunctionExpressionData::toStringInternal(KDbQuerySchemaParameterValueListIterator* params,
                                                       KDb::ExpressionCallStack* callStack) const
{
    return KDbEscapedString(name + QLatin1Char('('))
           + (args.data() ? args.data()->toString(params, callStack) : KDbEscapedString())
           + KDbEscapedString(')');
}

void KDbFunctionExpressionData::getQueryParameters(QList<KDbQuerySchemaParameter>& params)
{
    args->getQueryParameters(params);
}

KDbField::Type KDbFunctionExpressionData::typeInternal(KDb::ExpressionCallStack* callStack) const
{
    Q_UNUSED(callStack);
    if (name == QLatin1String("SUBSTR")) {
        if (args->convert<KDbNArgExpressionData>()->containsNullArgument()) {
            return KDbField::Null;
        }
        return KDbField::Text;
    }
    //! @todo
    return KDbField::InvalidType;
}

bool KDbFunctionExpressionData::validateInternal(KDbParseInfo *parseInfo, KDb::ExpressionCallStack* callStack)
{
    if (!args->validate(parseInfo, callStack)) {
        return false;
    }
    if (args->token != ',') { // arguments required: NArgExpr with token ','
        return false;
    }
    if (!args->validate(parseInfo)) {
        return false;
    }
    if (name.isEmpty()) {
        return false;
    }
    if (name == QLatin1String("SUBSTR")) {
        /* From https://www.sqlite.org/lang_corefunc.html:
        [1] substr(X,Y,Z)
        The substr(X,Y,Z) function returns a substring of input string X that begins
        with the Y-th character and which is Z characters long. If Z is omitted then

        [2] substr(X,Y)
        substr(X,Y) returns all characters through the end of the string X beginning with
        the Y-th. The left-most character of X is number 1. If Y is negative then the
        first character of the substring is found by counting from the right rather than
        the left. If Z is negative then the abs(Z) characters preceding the Y-th
        character are returned. If X is a string then characters indices refer to actual
        UTF-8 characters. If X is a BLOB then the indices refer to bytes. */
        if (args->convert<KDbNArgExpressionData>()->containsInvalidArgument()) {
            return false;
        }
        const int count = args->children.count();
        if (count != 2 && count != 3) {
            parseInfo->setErrorMessage(QObject::tr("Incorrect number of arguments"));
            parseInfo->setErrorDescription(QObject::tr("%1() function requires 2 or 3 arguments.").arg(name));
            return false;
        }
        ExplicitlySharedExpressionDataPointer textExpr = args->children[0];
        if (!textExpr->isTextType() && textExpr->type() != KDbField::Null) {
            parseInfo->setErrorMessage(QObject::tr("Incorrect type of argument"));
            parseInfo->setErrorDescription(QObject::tr("%1() function's first argument should be of type \"%2\". "
                                                       "Specified argument is of type \"%3\".")
                                           .arg(name)
                                           .arg(KDbField::typeName(KDbField::Text))
                                           .arg(KDbField::typeName(textExpr->type())));
            return false;
        }
        ExplicitlySharedExpressionDataPointer startExpr = args->children[1];
        if (!startExpr->isIntegerType() && startExpr->type() != KDbField::Null) {
            parseInfo->setErrorMessage(QObject::tr("Incorrect type of argument"));
            parseInfo->setErrorDescription(QObject::tr("%1() function's second argument should be of type \"%2\". "
                                                       "Specified argument is of type \"%3\".")
                                           .arg(name)
                                           .arg(KDbField::typeName(KDbField::Integer))
                                           .arg(KDbField::typeName(startExpr->type())));
            return false;
        }
        if (count == 3) {
            ExplicitlySharedExpressionDataPointer lengthExpr = args->children[2];
            if (!lengthExpr->isIntegerType() && lengthExpr->type() != KDbField::Null) {
                parseInfo->setErrorMessage(QObject::tr("Incorrect type of argument"));
                parseInfo->setErrorDescription(QObject::tr("%1() function's third argument should be of type \"%2\". "
                                                           "Specified argument is of type \"%3\".")
                                               .arg(name)
                                               .arg(KDbField::typeName(KDbField::Integer))
                                               .arg(KDbField::typeName(lengthExpr->type())));
                return false;
            }
        }
    }
    else {
        return false;
    }
    return true;
}

void KDbFunctionExpressionData::setArguments(ExplicitlySharedExpressionDataPointer arguments)
{
    args = (arguments && arguments->convert<KDbNArgExpressionData>())
            ? arguments : ExplicitlySharedExpressionDataPointer(new KDbNArgExpressionData);
    children.append(args);
    args->parent = this;
    args->token = ',';
    args->expressionClass = KDb::ArgumentListExpression;
}

//=========================================

inline KDb::ExpressionClass classForFunctionName(const QString& name)
{
    if (KDbFunctionExpression::isBuiltInAggregate(name))
        return KDb::AggregationExpression;
    else
        return KDb::FunctionExpression;
}

KDbFunctionExpression::KDbFunctionExpression()
 : KDbExpression(new KDbFunctionExpressionData)
{
    ExpressionDebug << "KDbFunctionExpression() ctor" << *this;
}

KDbFunctionExpression::KDbFunctionExpression(const QString& name)
        : KDbExpression(new KDbFunctionExpressionData(name),
              classForFunctionName(name), 0/*undefined*/)
{
}

KDbFunctionExpression::KDbFunctionExpression(const QString& name,
                                             const KDbNArgExpression& arguments)
        : KDbExpression(new KDbFunctionExpressionData(name.toUpper(), arguments.d),
              classForFunctionName(name), 0/*undefined*/)
{
}

KDbFunctionExpression::KDbFunctionExpression(const KDbFunctionExpression& expr)
        : KDbExpression(expr)
{
}

KDbFunctionExpression::KDbFunctionExpression(KDbExpressionData* data)
    : KDbExpression(data)
{
    ExpressionDebug << "KDbFunctionExpression ctor (KDbExpressionData*)" << *this;
}

KDbFunctionExpression::KDbFunctionExpression(const ExplicitlySharedExpressionDataPointer &ptr)
    : KDbExpression(ptr)
{
}

KDbFunctionExpression::~KDbFunctionExpression()
{
}

// static
bool KDbFunctionExpression::isBuiltInAggregate(const QString& function)
{
    return _builtInAggregates->contains(function.toUpper());
}

// static
QStringList KDbFunctionExpression::builtInAggregates()
{
    return _builtInAggregates->toList();
}

QString KDbFunctionExpression::name() const
{
    return d->convert<KDbFunctionExpressionData>()->name;
}

void KDbFunctionExpression::setName(const QString &name)
{
    d->convert<KDbFunctionExpressionData>()->name = name;
}

KDbNArgExpression KDbFunctionExpression::arguments()
{
    return KDbNArgExpression(d->convert<KDbFunctionExpressionData>()->args);
}

void KDbFunctionExpression::setArguments(const KDbNArgExpression &arguments)
{
    d->convert<KDbFunctionExpressionData>()->setArguments(arguments.d);
}

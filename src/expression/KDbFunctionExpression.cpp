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

#include "KDbExpression.h"
#include "KDb.h"
#include "KDbQuerySchema.h"
#include "KDbDriver.h"

#include <ctype.h>

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
}

KDbFunctionExpressionData::KDbFunctionExpressionData(const QString& aName)
        : KDbExpressionData()
        , name(aName)
{
    ExpressionDebug << "FunctionExpressionData" << ref;
/*    if (aArgs) {
        args = *aArgs;
        args->setParent(this);
    }*/
}

KDbFunctionExpressionData::~KDbFunctionExpressionData()
{
    ExpressionDebug << "~FunctionExpressionData" << ref;
}

KDbFunctionExpressionData* KDbFunctionExpressionData::clone()
{
    ExpressionDebug << "FunctionExpressionData::clone" << *this;
    return new KDbFunctionExpressionData(*this);
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
//! @todo
    Q_UNUSED(callStack);
    return KDbField::InvalidType;
}

bool KDbFunctionExpressionData::validateInternal(KDbParseInfo *parseInfo, KDb::ExpressionCallStack* callStack)
{
    return args.data() ? args.data()->validate(parseInfo, callStack) : true;
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

KDbFunctionExpression::KDbFunctionExpression(const QString& name, KDbNArgExpression& args)
        : KDbExpression(new KDbFunctionExpressionData(name),//, args.data()),
              classForFunctionName(name), 0/*undefined*/)
{
    if (!args.isNull()) {
        d->convert<KDbFunctionExpressionData>()->args = args.d;
        appendChild(args);
    }
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

KDbNArgExpression KDbFunctionExpression::arguments() const
{
    return KDbNArgExpression(d->convert<KDbFunctionExpressionData>()->args.data());
}

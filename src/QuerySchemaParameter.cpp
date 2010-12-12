/* This file is part of the KDE project
   Copyright (C) 2006-2010 Jarosław Staniek <staniek@kde.org>

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

#include "QuerySchemaParameter.h"
#include "Driver.h"
#include "DriverManager_p.h"

#include <QWeakPointer>

using namespace Predicate;

QuerySchemaParameter::QuerySchemaParameter()
        : type(Field::InvalidType)
{
}

QuerySchemaParameter::~QuerySchemaParameter()
{
}

QDebug operator<<(QDebug dbg, const QuerySchemaParameter& parameter)
{
    dbg.nospace() << "MESSAGE=" << parameter.message << "TYPE=" << Field::typeName(parameter.type);
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const QuerySchemaParameterList& list)
{
    dbg.nospace() << QString::fromLatin1("QUERY PARAMETERS (%1):").arg(list.count());
    foreach(const QuerySchemaParameter& parameter, list) {
        dbg.nospace() << " - " << parameter;
    }
    return dbg.space();
}

//================================================

class QuerySchemaParameterValueListIterator::Private
{
public:
    Private(Driver* driver, const QList<QVariant>& aParams)
            : driverWeakPointer(DriverManagerInternal::self()->driverWeakPointer(driver))
            , params(aParams)
    {
        //move to last item, as the order is reversed due to parser's internals
        paramsIt = params.constEnd(); //fromLast();
        --paramsIt;
        paramsItPosition = params.count();
    }
    QWeakPointer<Driver> driverWeakPointer;
    const QList<QVariant> params;
    QList<QVariant>::ConstIterator paramsIt;
    uint paramsItPosition;
};

QuerySchemaParameterValueListIterator::QuerySchemaParameterValueListIterator(
    Driver* driver, const QList<QVariant>& params)
        : d(new Private(driver, params))
{
}

QuerySchemaParameterValueListIterator::~QuerySchemaParameterValueListIterator()
{
    delete d;
}

QVariant QuerySchemaParameterValueListIterator::getPreviousValue()
{
    if (d->paramsItPosition == 0) { //d->params.constEnd()) {
        PreWarn << "no prev value";
        return QVariant();
    }
    QVariant res(*d->paramsIt);
    --d->paramsItPosition;
    --d->paramsIt;
// ++d->paramsIt;
    return res;
}

EscapedString QuerySchemaParameterValueListIterator::getPreviousValueAsString(Field::Type type)
{
    if (d->paramsItPosition == 0) { //d->params.constEnd()) {
        PreWarn << "no prev value";
        return d->driverWeakPointer.toStrongRef()->valueToSQL(type, QVariant()); //"NULL"
    }
    EscapedString res(d->driverWeakPointer.toStrongRef()->valueToSQL(type, *d->paramsIt));
    --d->paramsItPosition;
    --d->paramsIt;
// ++d->paramsIt;
    return res;
}

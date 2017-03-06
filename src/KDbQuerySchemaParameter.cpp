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

#include "KDbQuerySchemaParameter.h"
#include "KDbDriver.h"
#include "KDbDriverManager_p.h"
#include "kdb_debug.h"

#include <QWeakPointer>

KDbQuerySchemaParameter::KDbQuerySchemaParameter()
        : type(KDbField::InvalidType)
{
}

KDbQuerySchemaParameter::~KDbQuerySchemaParameter()
{
}

QDebug operator<<(QDebug dbg, const KDbQuerySchemaParameter& parameter)
{
    dbg.nospace() << "MESSAGE=" << parameter.message << "TYPE=" << KDbField::typeName(parameter.type);
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const QList<KDbQuerySchemaParameter>& list)
{
    dbg.nospace() << QString::fromLatin1("QUERY PARAMETERS (%1):").arg(list.count());
    foreach(const KDbQuerySchemaParameter& parameter, list) {
        dbg.nospace() << " - " << parameter;
    }
    return dbg.space();
}

//================================================

class Q_DECL_HIDDEN KDbQuerySchemaParameterValueListIterator::Private
{
public:
    Private(/*const KDbDriver &driver, */const QList<QVariant>& aParams)
            : //driverWeakPointer(DriverManagerInternal::self()->driverWeakPointer(driver))
            params(aParams)
    {
        //move to last item, as the order is reversed due to parser's internals
        paramsIt = params.constEnd();
        --paramsIt;
        paramsItPosition = params.count();
    }
    //! @todo ?? QWeakPointer<const KDbDriver> driverWeakPointer;
    const QList<QVariant> params;
    QList<QVariant>::ConstIterator paramsIt;
    int paramsItPosition;
private:
    Q_DISABLE_COPY(Private)
};

KDbQuerySchemaParameterValueListIterator::KDbQuerySchemaParameterValueListIterator(
    const QList<QVariant>& params)
        : d(new Private(params))
{
}

KDbQuerySchemaParameterValueListIterator::~KDbQuerySchemaParameterValueListIterator()
{
    delete d;
}

QVariant KDbQuerySchemaParameterValueListIterator::previousValue() const
{
    if (d->paramsItPosition == 0) { //d->params.constEnd()) {
        kdbWarning() << "no prev value";
        return QVariant();
    }
    QVariant res(*d->paramsIt);
    --d->paramsItPosition;
    --d->paramsIt;
    return res;
}

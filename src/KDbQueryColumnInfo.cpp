/* This file is part of the KDE project
   Copyright (C) 2003-2018 Jarosław Staniek <staniek@kde.org>

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

#include "KDbQueryColumnInfo.h"
#include "KDbQuerySchema_p.h"
#include "KDbTableSchema.h"
#include "KDbField.h"
#include "KDbField_p.h"
#include "kdb_debug.h"

KDbQueryColumnInfo::KDbQueryColumnInfo(KDbField *f, const QString& alias, bool visible,
                                 KDbQueryColumnInfo *foreignColumn)
        : d(new Private(f, alias, visible, foreignColumn))
{
}

KDbQueryColumnInfo::~KDbQueryColumnInfo()
{
    delete d;
}

KDbField* KDbQueryColumnInfo::field()
{
    return d->field;
}

const KDbField* KDbQueryColumnInfo::field() const
{
    return d->field;
}

void KDbQueryColumnInfo::setField(KDbField *field)
{
    d->field = field;
}

QString KDbQueryColumnInfo::alias() const
{
    return d->alias;
}

void KDbQueryColumnInfo::setAlias(const QString &alias)
{
    d->alias = alias;
}

QString KDbQueryColumnInfo::aliasOrName() const
{
    return d->alias.isEmpty() ? d->field->name() : d->alias;
}

QString KDbQueryColumnInfo::captionOrAliasOrName() const
{
    return d->field->caption().isEmpty() ? aliasOrName() : d->field->caption();
}

bool KDbQueryColumnInfo::isVisible() const
{
    return d->visible;
}

void KDbQueryColumnInfo::setVisible(bool set)
{
    d->visible = set;
}

int KDbQueryColumnInfo::indexForVisibleLookupValue() const
{
    return d->indexForVisibleLookupValue;
}

void KDbQueryColumnInfo::setIndexForVisibleLookupValue(int index)
{
    d->indexForVisibleLookupValue = index;
}

KDbQueryColumnInfo *KDbQueryColumnInfo::foreignColumn()
{
    return d->foreignColumn;
}

const KDbQueryColumnInfo *KDbQueryColumnInfo::foreignColumn() const
{
    return d->foreignColumn;
}

const KDbQuerySchema* KDbQueryColumnInfo::querySchema() const
{
    return d->querySchema;
}

KDbConnection* KDbQueryColumnInfo::connection()
{
    return d->connection;
}

const KDbConnection* KDbQueryColumnInfo::connection() const
{
    return d->connection;
}

QDebug operator<<(QDebug dbg, const KDbQueryColumnInfo& info)
{
    QString fieldName;
    if (info.field()->name().isEmpty()) {
        fieldName = QLatin1String("<NONAME>");
    } else {
        fieldName = info.field()->name();
    }
    dbg.nospace()
        << (info.field()->table() ? (info.field()->table()->name() + QLatin1Char('.')) : QString())
           + fieldName;
    debug(dbg, *info.field(), KDbFieldDebugNoOptions);
    dbg.nospace()
        << qPrintable(info.alias().isEmpty() ? QString() : (QLatin1String(" AS ") + info.alias()))
        << qPrintable(QLatin1String(info.isVisible() ? nullptr : " [INVISIBLE]"));
    return dbg.space();
}

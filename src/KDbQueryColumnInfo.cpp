/* This file is part of the KDE project
   Copyright (C) 2003-2014 Jarosław Staniek <staniek@kde.org>

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
#include "KDbTableSchema.h"
#include "KDbField.h"
#include "KDbField_p.h"
#include "kdb_debug.h"

KDbQueryColumnInfo::KDbQueryColumnInfo(KDbField *f, const QString& _alias, bool _visible,
                                 KDbQueryColumnInfo *foreignColumn)
        : field(f), alias(_alias), visible(_visible), m_indexForVisibleLookupValue(-1)
        , m_foreignColumn(foreignColumn)
{
}

KDbQueryColumnInfo::~KDbQueryColumnInfo()
{
}

QString KDbQueryColumnInfo::aliasOrName() const
{
    return alias.isEmpty() ? field->name() : alias;
}

QString KDbQueryColumnInfo::captionOrAliasOrName() const
{
    return field->caption().isEmpty() ? aliasOrName() : field->caption();
}

int KDbQueryColumnInfo::indexForVisibleLookupValue() const
{
    return m_indexForVisibleLookupValue;
}

void KDbQueryColumnInfo::setIndexForVisibleLookupValue(int index)
{
    m_indexForVisibleLookupValue = index;
}

KDbQueryColumnInfo *KDbQueryColumnInfo::foreignColumn() const
{
    return m_foreignColumn;
}

QDebug operator<<(QDebug dbg, const KDbQueryColumnInfo& info)
{
    QString fieldName;
    if (info.field->name().isEmpty()) {
        fieldName = QLatin1String("<NONAME>");
    } else {
        fieldName = info.field->name();
    }
    dbg.nospace()
        << (info.field->table() ? (info.field->table()->name() + QLatin1Char('.')) : QString())
           + fieldName;
    debug(dbg, *info.field, KDbFieldDebugNoOptions);
    dbg.nospace()
        << qPrintable(info.alias.isEmpty() ? QString() : (QLatin1String(" AS ") + info.alias))
        << qPrintable(info.visible ? QLatin1String() : QLatin1String(" [INVISIBLE]"));
    return dbg.space();
}

/* This file is part of the KDE project
   Copyright (C) 2003-2010 Jarosław Staniek <staniek@kde.org>

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

#include "KDbObject.h"
#include "KDbConnection.h"


KDbObject::KDbObject(int type)
    : d(new Data)
{
    d->type = type;
}

KDbObject::~KDbObject()
{
}

void KDbObject::clear()
{
    const int type = d->type;
    d = new Data;
    d->type = type;
}

QDebug operator<<(QDebug dbg, const KDbObject& object)
{
    dbg.nospace() << "KDbObject:";
    QString desc = object.description();
    if (desc.length() > 120) {
        desc.truncate(120);
        desc += QLatin1String("...");
    }
    dbg.nospace() << " ID=" << object.id() << " NAME=" << object.name() << " CAPTION="
                  << object.caption() << " DESC=" << desc;
    return dbg.nospace();
}

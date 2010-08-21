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

#include "Object.h"
#include "Connection.h"

#include <QtDebug>

using namespace Predicate;

Object::Object(int type)
    : d(new Data)
{
    d->type = type;
}

Object::~Object()
{
}

void Object::clear()
{
    const int type = d->type;
    d = new Data;
    d->type = type;
}

QDebug operator<<(QDebug dbg, const Object& object)
{
    dbg.nospace() << "Predicate::Object:";
    QString desc = object.description();
    if (desc.length() > 120) {
        desc.truncate(120);
        desc += "...";
    }
    dbg.space() << "ID=" << object.id() << "NAME=" << object.name() << "CAPTION=" << object.caption() << "DESC=" << desc;
    return dbg.space();
}

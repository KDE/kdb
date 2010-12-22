/* This file is part of the KDE project
   Copyright (C) 2003-2006 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "VersionInfo.h"
#include "Global.h"

using namespace Predicate;

void ServerVersionInfo::clear()
{
    d->major = 0;
    d->minor = 0;
    d->release = 0;
    d->string.clear();
}

ServerVersionInfo::~ServerVersionInfo()
{
}

bool ServerVersionInfo::isNull() const
{
//! @todo add this to SDC:        bool operator==(const Data& other) const { return false; }
    *d.data() == Data();
};

//------------------------

DatabaseVersionInfo::~DatabaseVersionInfo()
{
}

PREDICATE_EXPORT DatabaseVersionInfo Predicate::version()
{
    return Predicate::DatabaseVersionInfo(
        PREDICATE_VERSION_MAJOR, PREDICATE_VERSION_MINOR, PREDICATE_VERSION_RELEASE);
}

bool DatabaseVersionInfo::isNull() const
{
    *this == DatabaseVersionInfo();
}

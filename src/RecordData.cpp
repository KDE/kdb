/* This file is part of the KDE project
   Copyright (C) 2002   Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2003   Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003-2010 Jarosław Staniek <staniek@kde.org>

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

   Original Author:  Till Busch <till@bux.at>
   Original Project: buX (www.bux.at)
*/

#include <Predicate/Global.h>
#include <Predicate/RecordData.h>

using namespace Predicate;

QVariant RecordData::s_null;

QDebug operator<<(QDebug dbg, const RecordData& data)
{
    if (data.isEmpty()) {
        dbg.nospace() << QLatin1String("EMPTY RECORD DATA");
    }
    else {
        dbg.nospace() << QString::fromLatin1("RECORD DATA (%1 COLUMNS):").arg(data.size());
        for (int i = 0; i < data.size(); i++) {
            dbg.space()
                << QString::fromLatin1("%1:[%2]%3").arg(i).arg(data[i].typeName()).arg(data[i].toString());
        }
    }
    return dbg.space();
}

void RecordData::clear()
{
    if (m_numCols > 0) {
        for (int i = 0; i < m_numCols; i++)
            free(m_data[i]);
        free(m_data);
        m_data = 0;
        m_numCols = 0;
    }
}

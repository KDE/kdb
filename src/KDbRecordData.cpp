/* This file is part of the KDE project
   Copyright (C) 2002   Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2003   Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003-2016 Jarosław Staniek <staniek@kde.org>

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

#include "KDbRecordData.h"
#include "KDbGlobal.h"
#include "KDbUtils.h"
#include "kdb_debug.h"

QVariant KDbRecordData::s_null;

QDebug operator<<(QDebug dbg, const KDbRecordData& data)
{
    if (data.isEmpty()) {
        dbg.nospace() << QLatin1String("EMPTY RECORD DATA");
    }
    else {
        dbg.nospace() << "RECORD DATA (" << data.size() << " COLUMNS):";
        for (int i = 0; i < data.size(); i++) {
            dbg.nospace()
                << " " << i << ":" << KDbUtils::squeezedValue(data[i]);
        }
    }
    return dbg.space();
}

void KDbRecordData::clear()
{
    if (m_numCols > 0) {
        for (int i = 0; i < m_numCols; i++)
            free(m_data[i]);
        free(m_data);
        m_data = nullptr;
        m_numCols = 0;
    }
}

void KDbRecordData::resize(int numCols)
{
    if (m_numCols == numCols)
        return;
    else if (m_numCols < numCols) { // grow
        m_data = (QVariant **)realloc(m_data, numCols * sizeof(QVariant *));
        memset(m_data + m_numCols, 0, (numCols - m_numCols) * sizeof(QVariant *));
        m_numCols = numCols;
    } else { // shrink
        for (int i = numCols; i < m_numCols; i++)
            delete m_data[i];
        m_data = (QVariant **)realloc(m_data, numCols * sizeof(QVariant *));
        m_numCols = numCols;
    }
}

void KDbRecordData::clearValues()
{
    for (int i = 0; i < m_numCols; i++) {
        delete m_data[i];
        m_data[i] = nullptr;
    }
}

QList<QVariant> KDbRecordData::toList() const
{
    QList<QVariant> list;
    list.reserve(m_numCols);
    for (int i = 0; i < m_numCols; ++i) {
        list.append(*m_data[i]);
    }
    return list;
}

/* This file is part of the KDE project
   Copyright (C) 2003-2011 Jarosław Staniek <staniek@kde.org>

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

#include "RecordEditBuffer.h"
#include "Utils.h"

#include <QtDebug>

using namespace Predicate;

RecordEditBuffer::RecordEditBuffer(bool dbAwareBuffer)
        : m_simpleBuffer(dbAwareBuffer ? 0 : new SimpleMap())
        , m_simpleBufferIt(dbAwareBuffer ? 0 : new SimpleMap::ConstIterator())
        , m_dbBuffer(dbAwareBuffer ? new DBMap() : 0)
        , m_dbBufferIt(dbAwareBuffer ? new DBMap::Iterator() : 0)
        , m_defaultValuesDbBuffer(dbAwareBuffer ? new QMap<QueryColumnInfo*, bool>() : 0)
        , m_defaultValuesDbBufferIt(dbAwareBuffer ? new QMap<QueryColumnInfo*, bool>::ConstIterator() : 0)
{
}

RecordEditBuffer::~RecordEditBuffer()
{
    delete m_simpleBuffer;
    delete m_simpleBufferIt;
    delete m_dbBuffer;
    delete m_dbBufferIt;
    delete m_defaultValuesDbBuffer;
    delete m_defaultValuesDbBufferIt;
}

const QVariant* RecordEditBuffer::at(QueryColumnInfo& ci, bool useDefaultValueIfPossible) const
{
    if (!m_dbBuffer) {
        PreWarn << "not db-aware buffer!";
        return 0;
    }
    *m_dbBufferIt = m_dbBuffer->find(&ci);
    QVariant* result = 0;
    if (*m_dbBufferIt != m_dbBuffer->end())
        result = &(*m_dbBufferIt).value();
    if (useDefaultValueIfPossible
            && (!result || result->isNull())
            && ci.field && !ci.field->defaultValue().isNull() && Predicate::isDefaultValueAllowed(ci.field)
            && !hasDefaultValueAt(ci)) {
        //no buffered or stored value: try to get a default value declared in a field, so user can modify it
        if (!result)
            m_dbBuffer->insert(&ci, ci.field->defaultValue());
        result = &(*m_dbBuffer)[ &ci ];
        m_defaultValuesDbBuffer->insert(&ci, true);
    }
    return (const QVariant*)result;
}

const QVariant* RecordEditBuffer::at(Field& f) const
{
    if (!m_simpleBuffer) {
        PreWarn << "this is db-aware buffer!";
        return 0;
    }
    *m_simpleBufferIt = m_simpleBuffer->constFind(f.name());
    if (*m_simpleBufferIt == m_simpleBuffer->constEnd())
        return 0;
    return &(*m_simpleBufferIt).value();
}

const QVariant* RecordEditBuffer::at(const QString& fname) const
{
    if (!m_simpleBuffer) {
        PreWarn << "this is db-aware buffer!";
        return 0;
    }
    *m_simpleBufferIt = m_simpleBuffer->constFind(fname);
    if (*m_simpleBufferIt == m_simpleBuffer->constEnd())
        return 0;
    return &(*m_simpleBufferIt).value();
}

void RecordEditBuffer::removeAt(const QueryColumnInfo& ci)
{
    if (!m_dbBuffer) {
        PreWarn << "not db-aware buffer!";
        return;
    }
    m_dbBuffer->remove(const_cast<QueryColumnInfo*>(&ci)); // const_cast ok here, we won't modify ci
}

void RecordEditBuffer::removeAt(const Field& f)
{
    if (!m_simpleBuffer) {
        PreWarn << "this is db-aware buffer!";
        return;
    }
    m_simpleBuffer->remove(f.name());
}

void RecordEditBuffer::removeAt(const QString& fname)
{
    if (!m_simpleBuffer) {
        PreWarn << "this is db-aware buffer!";
        return;
    }
    m_simpleBuffer->remove(fname);
}

void RecordEditBuffer::clear()
{
    if (m_dbBuffer) {
        m_dbBuffer->clear();
        m_defaultValuesDbBuffer->clear();
    }
    if (m_simpleBuffer)
        m_simpleBuffer->clear();
}

bool RecordEditBuffer::isEmpty() const
{
    if (m_dbBuffer)
        return m_dbBuffer->isEmpty();
    if (m_simpleBuffer)
        return m_simpleBuffer->isEmpty();
    return true;
}

void RecordEditBuffer::debug()
{
    if (isDBAware()) {
        PreDbg << "RecordEditBuffer type=DB-AWARE, " << m_dbBuffer->count() << " items";
        for (DBMap::ConstIterator it = m_dbBuffer->constBegin(); it != m_dbBuffer->constEnd(); ++it) {
            PreDbg << "* field name=" << it.key()->field->name() << " val="
            << (it.value().isNull() ? QString("<NULL>") : it.value().toString())
            << (hasDefaultValueAt(*it.key()) ? " DEFAULT" : "");
        }
        return;
    }
    PreDbg << "RecordEditBuffer type=SIMPLE, " << m_simpleBuffer->count() << " items";
    for (SimpleMap::ConstIterator it = m_simpleBuffer->constBegin(); it != m_simpleBuffer->constEnd(); ++it) {
        PreDbg << "* field name=" << it.key() << " val="
        << (it.value().isNull() ? QString("<NULL>") : it.value().toString());
    }
}

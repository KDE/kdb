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

#include "KDbRecordEditBuffer.h"
#include "KDb.h"

#include <QtDebug>

KDbRecordEditBuffer::KDbRecordEditBuffer(bool dbAwareBuffer)
        : m_simpleBuffer(dbAwareBuffer ? 0 : new SimpleMap())
        , m_simpleBufferIt(dbAwareBuffer ? 0 : new SimpleMap::ConstIterator())
        , m_dbBuffer(dbAwareBuffer ? new DBMap() : 0)
        , m_dbBufferIt(dbAwareBuffer ? new DBMap::Iterator() : 0)
        , m_defaultValuesDbBuffer(dbAwareBuffer ? new QMap<KDbQueryColumnInfo*, bool>() : 0)
        , m_defaultValuesDbBufferIt(dbAwareBuffer ? new QMap<KDbQueryColumnInfo*, bool>::ConstIterator() : 0)
{
}

KDbRecordEditBuffer::~KDbRecordEditBuffer()
{
    delete m_simpleBuffer;
    delete m_simpleBufferIt;
    delete m_dbBuffer;
    delete m_dbBufferIt;
    delete m_defaultValuesDbBuffer;
    delete m_defaultValuesDbBufferIt;
}

const QVariant* KDbRecordEditBuffer::at(KDbQueryColumnInfo& ci, bool useDefaultValueIfPossible) const
{
    if (!m_dbBuffer) {
        KDbWarn << "not db-aware buffer!";
        return 0;
    }
    *m_dbBufferIt = m_dbBuffer->find(&ci);
    QVariant* result = 0;
    if (*m_dbBufferIt != m_dbBuffer->end())
        result = &(*m_dbBufferIt).value();
    if (useDefaultValueIfPossible
            && (!result || result->isNull())
            && ci.field && !ci.field->defaultValue().isNull() && KDb::isDefaultValueAllowed(ci.field)
            && !hasDefaultValueAt(ci)) {
        //no buffered or stored value: try to get a default value declared in a field, so user can modify it
        if (!result)
            m_dbBuffer->insert(&ci, ci.field->defaultValue());
        result = &(*m_dbBuffer)[ &ci ];
        m_defaultValuesDbBuffer->insert(&ci, true);
    }
    return (const QVariant*)result;
}

const QVariant* KDbRecordEditBuffer::at(KDbField& f) const
{
    if (!m_simpleBuffer) {
        KDbWarn << "this is db-aware buffer!";
        return 0;
    }
    *m_simpleBufferIt = m_simpleBuffer->constFind(f.name());
    if (*m_simpleBufferIt == m_simpleBuffer->constEnd())
        return 0;
    return &(*m_simpleBufferIt).value();
}

const QVariant* KDbRecordEditBuffer::at(const QString& fname) const
{
    if (!m_simpleBuffer) {
        KDbWarn << "this is db-aware buffer!";
        return 0;
    }
    *m_simpleBufferIt = m_simpleBuffer->constFind(fname);
    if (*m_simpleBufferIt == m_simpleBuffer->constEnd())
        return 0;
    return &(*m_simpleBufferIt).value();
}

void KDbRecordEditBuffer::removeAt(const KDbQueryColumnInfo& ci)
{
    if (!m_dbBuffer) {
        KDbWarn << "not db-aware buffer!";
        return;
    }
    m_dbBuffer->remove(const_cast<KDbQueryColumnInfo*>(&ci)); // const_cast ok here, we won't modify ci
}

void KDbRecordEditBuffer::removeAt(const KDbField& f)
{
    if (!m_simpleBuffer) {
        KDbWarn << "this is db-aware buffer!";
        return;
    }
    m_simpleBuffer->remove(f.name());
}

void KDbRecordEditBuffer::removeAt(const QString& fname)
{
    if (!m_simpleBuffer) {
        KDbWarn << "this is db-aware buffer!";
        return;
    }
    m_simpleBuffer->remove(fname);
}

void KDbRecordEditBuffer::clear()
{
    if (m_dbBuffer) {
        m_dbBuffer->clear();
        m_defaultValuesDbBuffer->clear();
    }
    if (m_simpleBuffer)
        m_simpleBuffer->clear();
}

bool KDbRecordEditBuffer::isEmpty() const
{
    if (m_dbBuffer)
        return m_dbBuffer->isEmpty();
    if (m_simpleBuffer)
        return m_simpleBuffer->isEmpty();
    return true;
}

QDebug operator<<(QDebug dbg, const KDbRecordEditBuffer& buffer)
{
    if (buffer.isDBAware()) {
        dbg.space() << "RecordEditBuffer type=DB-AWARE,";
        dbg.space() << buffer.dbBuffer().count() << " items\n";
        for (KDbRecordEditBuffer::DBMap::ConstIterator it = buffer.dbBuffer().constBegin(); it != buffer.dbBuffer().constEnd(); ++it) {
            dbg.nospace() << "* field name=" << it.key()->field->name() << "val="
            << (it.value().isNull() ? QLatin1String("<NULL>") : it.value().toString())
            << (buffer.hasDefaultValueAt(*it.key()) ? " DEFAULT\n" : "\n");
        }
        return dbg.space();
    }
    dbg.space() << "RecordEditBuffer type=SIMPLE,";
    dbg.space() << buffer.simpleBuffer().count() << " items\n";
    for (KDbRecordEditBuffer::SimpleMap::ConstIterator it = buffer.simpleBuffer().constBegin(); it != buffer.simpleBuffer().constEnd(); ++it) {
        dbg.space() << "* field name=" << it.key() << "val="
        << (it.value().isNull() ? QLatin1String("<NULL>") : it.value().toString()) << "\n";
    }
    return dbg.space();
}

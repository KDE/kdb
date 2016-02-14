/* This file is part of the KDE project
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2003-2007 Jarosław Staniek <staniek@kde.org>

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

#include "KDbTableSchema.h"
#include "KDbDriver.h"
#include "KDbConnection.h"
#include "KDbLookupFieldSchema.h"
#include "kdb_debug.h"

#include <assert.h>

//! @internal
class KDbTableSchema::Private
{
public:
    Private()
            : anyNonPKField(0) {
    }

    ~Private() {
        clearLookupFields();
    }

    void clearLookupFields() {
        qDeleteAll(lookupFields);
        lookupFields.clear();
    }

    KDbField *anyNonPKField;
    QHash<const KDbField*, KDbLookupFieldSchema*> lookupFields;
    QVector<KDbLookupFieldSchema*> lookupFieldsList;
};

//-------------------------------------

KDbTableSchema::KDbTableSchema(const QString& name)
        : KDbFieldList(true)
        , KDbObject(KDb::TableObjectType)
        , d( new Private )
        , m_isKDbSystem(false)
{
    setName(name);
    init(0);
}

KDbTableSchema::KDbTableSchema(const KDbObject& other)
        : KDbFieldList(true)
        , KDbObject(other)
        , d( new Private )
{
    init(0);
}

KDbTableSchema::KDbTableSchema()
        : KDbFieldList(true)
        , KDbObject(KDb::TableObjectType)
        , d( new Private )
{
    init(0);
}

KDbTableSchema::KDbTableSchema(const KDbTableSchema& ts, bool copyId)
        : KDbFieldList(static_cast<const KDbFieldList&>(ts))
        , KDbObject(static_cast<const KDbObject&>(ts))
        , d( new Private )
{
    init(ts, copyId);
}

KDbTableSchema::KDbTableSchema(const KDbTableSchema& ts, int id)
        : KDbFieldList(static_cast<const KDbFieldList&>(ts))
        , KDbObject(static_cast<const KDbObject&>(ts))
        , d( new Private )
{
    init(ts, false);
    setId(id);
}

// used by KDbConnection
KDbTableSchema::KDbTableSchema(KDbConnection *conn, const QString & name)
        : KDbFieldList(true)
        , KDbObject(KDb::TableObjectType)
        , d( new Private )
{
    Q_ASSERT(conn);
    setName(name);
    init(conn);
}

KDbTableSchema::~KDbTableSchema()
{
    if (m_conn)
        m_conn->removeMe(this);
    qDeleteAll(m_indices);
    delete m_query;
    delete d;
}

void KDbTableSchema::init(KDbConnection* conn)
{
    m_conn = conn;
    m_query = 0; //not cached
    m_isKDbSystem = false;
    m_pkey = new KDbIndexSchema(this);
    m_indices.append(m_pkey);
}

void KDbTableSchema::init(const KDbTableSchema& ts, bool copyId)
{
    m_conn = ts.m_conn;
    m_query = 0; //not cached
    m_isKDbSystem = false;
    setName(ts.name());
    m_pkey = 0; //will be copied
    if (!copyId)
        setId(-1);

    //deep copy all members
    foreach(KDbIndexSchema* otherIdx, ts.m_indices) {
        KDbIndexSchema *idx = new KDbIndexSchema(
            *otherIdx, *this /*fields from _this_ table will be assigned to the index*/);
        if (idx->isPrimaryKey()) {//assign pkey
            m_pkey = idx;
        }
        m_indices.append(idx);
    }

    KDbField::ListIterator tsIter(ts.fieldsIterator());
    KDbField::ListIterator iter(fieldsIterator());
    for (; iter != fieldsIteratorConstEnd(); ++tsIter, ++iter) {
        const KDbLookupFieldSchema *lookup = ts.lookupFieldSchema(**tsIter);
        if (lookup) {
            d->lookupFields.insert(*iter, new KDbLookupFieldSchema(*lookup));
        }
    }
}

KDbIndexSchema* KDbTableSchema::primaryKey() const
{
    return m_pkey;
}

const KDbIndexSchema::ListIterator KDbTableSchema::indicesIterator() const
{
    return KDbIndexSchema::ListIterator(m_indices.constBegin());
}

const KDbIndexSchema::List* KDbTableSchema::indices() const
{
    return &m_indices;
}

bool KDbTableSchema::isKDbSystem() const
{
    return m_isKDbSystem;
}

void KDbTableSchema::setPrimaryKey(KDbIndexSchema *pkey)
{
    if (m_pkey && m_pkey != pkey) {
        if (m_pkey->fieldCount() == 0) {//this is empty key, probably default - remove it
            m_indices.removeOne(m_pkey);
            delete m_pkey;
        }
        else {
            m_pkey->setPrimaryKey(false); //there can be only one pkey..
            //thats ok, the old pkey is still on indices list, if not empty
        }
    }

    if (!pkey) {//clearing - set empty pkey
        pkey = new KDbIndexSchema(this);
    }
    m_pkey = pkey; //!< @todo
    m_pkey->setPrimaryKey(true);
    d->anyNonPKField = 0; //for safety
}

bool KDbTableSchema::insertField(int index, KDbField *field)
{
    Q_ASSERT(field);
    KDbFieldList::insertField(index, field);
    if (!field || index > m_fields.count()) {
        return false;
    }
    field->setTable(this);
    field->m_order = index;
    //update order for next next fields
    const int fieldCount = m_fields.count();
    for (int i = index + 1; i < fieldCount; i++)
        m_fields.at(i)->m_order = i;

    //Check for auto-generated indices:
    KDbIndexSchema *idx = 0;
    if (field->isPrimaryKey()) {// this is auto-generated single-field unique index
        idx = new KDbIndexSchema(this);
        idx->setAutoGenerated(true);
        const bool ok = idx->addField(field);
        Q_ASSERT(ok);
        setPrimaryKey(idx);
    }
    if (field->isUniqueKey()) {
        if (!idx) {
            idx = new KDbIndexSchema(this);
            idx->setAutoGenerated(true);
            const bool ok = idx->addField(field);
            Q_ASSERT(ok);
        }
        idx->setUnique(true);
    }
    if (field->isIndexed()) {// this is auto-generated single-field
        if (!idx) {
            idx = new KDbIndexSchema(this);
            idx->setAutoGenerated(true);
            const bool ok = idx->addField(field);
            Q_ASSERT(ok);
        }
    }
    if (idx) {
        m_indices.append(idx);
    }
    return true;
}

bool KDbTableSchema::removeField(KDbField *field)
{
    KDbLookupFieldSchema* lookup = d->lookupFields.take(field);
    if (!KDbFieldList::removeField(field)) {
        return false;
    }
    if (d->anyNonPKField && field == d->anyNonPKField) //d->anyNonPKField will be removed!
        d->anyNonPKField = 0;
    delete lookup;
    return true;
}

void KDbTableSchema::clear()
{
    m_indices.clear();
    d->clearLookupFields();
    KDbFieldList::clear();
    KDbObject::clear();
    m_conn = 0;
}

QDebug KDbTableSchema::debugFields(QDebug dbg) const
{
    dbg.nospace() << static_cast<const KDbFieldList&>(*this);
    foreach(const KDbField *f, m_fields) {
        const KDbLookupFieldSchema *lookupSchema = lookupFieldSchema(*f);
        if (lookupSchema)
            dbg.nospace() << '\n' << f->name() << *lookupSchema;
    }
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const KDbTableSchema& table)
{
    dbg.nospace() << "TABLE";
    dbg.space() << static_cast<const KDbObject&>(table) << '\n';
    table.debugFields(dbg);
    return dbg.space();
}

//! @todo IMPORTANT: replace QPointer<KDbConnection> m_conn
KDbConnection* KDbTableSchema::connection() const
{
    return static_cast<KDbConnection*>(m_conn);
}

void KDbTableSchema::setKDbSystem(bool set)
{
    if (set)
        setNative(true);
    m_isKDbSystem = set;
    if (m_isKDbSystem)
        setNative(true);
}

KDbQuerySchema* KDbTableSchema::query()
{
    if (m_query)
        return m_query;
    m_query = new KDbQuerySchema(this);   //it's owned by me
    return m_query;
}

KDbField* KDbTableSchema::anyNonPKField()
{
    if (!d->anyNonPKField) {
        KDbField *f = 0;
        for (QListIterator<KDbField*> it(m_fields); it.hasPrevious();) {
            f = it.previous();
            if (!f->isPrimaryKey() && (!m_pkey || !m_pkey->hasField(*f)))
                break;
        }
        d->anyNonPKField = f;
    }
    return d->anyNonPKField;
}

bool KDbTableSchema::setLookupFieldSchema(const QString& fieldName, KDbLookupFieldSchema *lookupFieldSchema)
{
    KDbField *f = field(fieldName);
    if (!f) {
        kdbWarning() << "no such field" << fieldName << "in table" << name();
        return false;
    }
    delete d->lookupFields.take(f);
    if (lookupFieldSchema) {
        d->lookupFields.insert(f, lookupFieldSchema);
    }
    d->lookupFieldsList.clear(); //this will force to rebuid the internal cache
    return true;
}

KDbLookupFieldSchema *KDbTableSchema::lookupFieldSchema(const KDbField& field) const
{
    return d->lookupFields.value(&field);
}

KDbLookupFieldSchema *KDbTableSchema::lookupFieldSchema(const QString& fieldName)
{
    KDbField *f = KDbTableSchema::field(fieldName);
    if (!f)
        return 0;
    return lookupFieldSchema(*f);
}

QVector<KDbLookupFieldSchema*> KDbTableSchema::lookupFields() const
{
    if (d->lookupFields.isEmpty())
        return QVector<KDbLookupFieldSchema*>();
    if (!d->lookupFields.isEmpty() && !d->lookupFieldsList.isEmpty())
        return d->lookupFieldsList; //already updated
    //update
    d->lookupFieldsList.clear();
    d->lookupFieldsList.resize(d->lookupFields.count());
    int i = 0;
    foreach(KDbField* f, m_fields) {
        QHash<const KDbField*, KDbLookupFieldSchema*>::ConstIterator itMap = d->lookupFields.constFind(f);
        if (itMap != d->lookupFields.constEnd()) {
            d->lookupFieldsList[i] = itMap.value();
            i++;
        }
    }
    return d->lookupFieldsList;
}

//--------------------------------------

KDbInternalTableSchema::KDbInternalTableSchema(const QString& name)
        : KDbTableSchema(name)
{
}

KDbInternalTableSchema::KDbInternalTableSchema(const KDbTableSchema& ts)
        : KDbTableSchema(ts, false)
{
}

KDbInternalTableSchema::~KDbInternalTableSchema()
{
}

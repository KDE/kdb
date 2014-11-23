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

#include "TableSchema.h"
#include "Driver.h"
#include "Connection.h"
#include "LookupFieldSchema.h"

#include <assert.h>
#include <QtDebug>

namespace Predicate
{

//! @internal
class TableSchema::Private
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

    Field *anyNonPKField;
    QHash<const Field*, LookupFieldSchema*> lookupFields;
    QVector<LookupFieldSchema*> lookupFieldsList;
};
}

//-------------------------------------

using namespace Predicate;

TableSchema::TableSchema(const QString& name)
        : FieldList(true)
        , Object(Predicate::TableObjectType)
        , d( new Private )
        , m_isPredicateSystem(false)
{
    setName(name);
    init(0);
}

TableSchema::TableSchema(const Object& other)
        : FieldList(true)
        , Object(other)
        , d( new Private )
{
    init(0);
}

TableSchema::TableSchema()
        : FieldList(true)
        , Object(Predicate::TableObjectType)
        , d( new Private )
{
    init(0);
}

TableSchema::TableSchema(const TableSchema& ts, bool copyId)
        : FieldList(static_cast<const FieldList&>(ts))
        , Object(static_cast<const Object&>(ts))
        , d( new Private )
{
    init(ts, copyId);
}

TableSchema::TableSchema(const TableSchema& ts, int id)
        : FieldList(static_cast<const FieldList&>(ts))
        , Object(static_cast<const Object&>(ts))
        , d( new Private )
{
    init(ts, false);
    setId(id);
}

// used by Connection
TableSchema::TableSchema(Connection *conn, const QString & name)
        : FieldList(true)
        , Object(Predicate::TableObjectType)
        , d( new Private )
{
    Q_ASSERT(conn);
    setName(name);
    init(conn);
}

TableSchema::~TableSchema()
{
    if (m_conn)
        m_conn->removeMe(this);
    qDeleteAll(m_indices);
    delete m_query;
    delete d;
}

void TableSchema::init(Connection* conn)
{
    m_conn = conn;
    m_query = 0; //not cached
    m_isPredicateSystem = false;
    m_pkey = new IndexSchema(this);
    m_indices.append(m_pkey);
}

void TableSchema::init(const TableSchema& ts, bool copyId)
{
    m_conn = ts.m_conn;
    m_query = 0; //not cached
    m_isPredicateSystem = false;
    setName(ts.name());
    m_pkey = 0; //will be copied
    if (!copyId)
        setId(-1);

    //deep copy all members
    foreach(IndexSchema* otherIdx, ts.m_indices) {
        IndexSchema *idx = new IndexSchema(
            *otherIdx, *this /*fields from _this_ table will be assigned to the index*/);
        if (idx->isPrimaryKey()) {//assign pkey
            m_pkey = idx;
        }
        m_indices.append(idx);
    }

    Field::ListIterator tsIter(ts.fieldsIterator());
    Field::ListIterator iter(fieldsIterator());
    for (; iter != fieldsIteratorConstEnd(); ++tsIter, ++iter) {
        const LookupFieldSchema *lookup = ts.lookupFieldSchema(**tsIter);
        if (lookup) {
            d->lookupFields.insert(*iter, new LookupFieldSchema(*lookup));
        }
    }
}

void TableSchema::setPrimaryKey(IndexSchema *pkey)
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
        pkey = new IndexSchema(this);
    }
    m_pkey = pkey; //!< @todo
    m_pkey->setPrimaryKey(true);
    d->anyNonPKField = 0; //for safety
}

FieldList& TableSchema::insertField(uint index, Field *field)
{
    assert(field);
    FieldList::insertField(index, field);
    if (!field || index > (uint)m_fields.count())
        return *this;
    field->setTable(this);
    field->m_order = index;
    //update order for next next fields
    uint fieldsCount = m_fields.count();
    for (uint i = index + 1; i < fieldsCount; i++)
        m_fields.at(i)->m_order = i;

    //Check for auto-generated indices:
    IndexSchema *idx = 0;
    if (field->isPrimaryKey()) {// this is auto-generated single-field unique index
        idx = new IndexSchema(this);
        idx->setAutoGenerated(true);
        idx->addField(field);
        setPrimaryKey(idx);
    }
    if (field->isUniqueKey()) {
        if (!idx) {
            idx = new IndexSchema(this);
            idx->setAutoGenerated(true);
            idx->addField(field);
        }
        idx->setUnique(true);
    }
    if (field->isIndexed()) {// this is auto-generated single-field
        if (!idx) {
            idx = new IndexSchema(this);
            idx->setAutoGenerated(true);
            idx->addField(field);
        }
    }
    if (idx)
        m_indices.append(idx);
    return *this;
}

bool TableSchema::removeField(Field *field)
{
    LookupFieldSchema* lookup = d->lookupFields.take(field);
    if (!FieldList::removeField(field)) {
        return false;
    }
    if (d->anyNonPKField && field == d->anyNonPKField) //d->anyNonPKField will be removed!
        d->anyNonPKField = 0;
    delete lookup;
    return true;
}

void TableSchema::clear()
{
    m_indices.clear();
    d->clearLookupFields();
    FieldList::clear();
    Object::clear();
    m_conn = 0;
}

QDebug TableSchema::debugFields(QDebug dbg) const
{
    dbg.nospace() << static_cast<const FieldList&>(*this);
    foreach(const Field *f, m_fields) {
        const LookupFieldSchema *lookupSchema = lookupFieldSchema(*f);
        if (lookupSchema)
            dbg.nospace() << '\n' << f->name() << *lookupSchema;
    }
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const TableSchema& table)
{
    dbg.nospace() << QLatin1String("TABLE");
    dbg.space() << static_cast<const Object&>(table) << '\n';
    table.debugFields(dbg);
    return dbg.space();
}

#ifdef __GNUC__
#warning replace QPointer<Connection> m_conn;
#else
#pragma WARNING(replace QPointer<Connection> m_conn;)
#endif

Connection* TableSchema::connection() const
{
    return static_cast<Connection*>(m_conn);
}

void TableSchema::setPredicateSystem(bool set)
{
    if (set)
        setNative(true);
    m_isPredicateSystem = set;
    if (m_isPredicateSystem)
        setNative(true);
}

QuerySchema* TableSchema::query()
{
    if (m_query)
        return m_query;
    m_query = new QuerySchema(this);   //it's owned by me
    return m_query;
}

Field* TableSchema::anyNonPKField()
{
    if (!d->anyNonPKField) {
        Field *f = 0;
        for (QListIterator<Field*> it(m_fields); it.hasPrevious();) {
            f = it.previous();
            if (!f->isPrimaryKey() && (!m_pkey || !m_pkey->hasField(*f)))
                break;
        }
        d->anyNonPKField = f;
    }
    return d->anyNonPKField;
}

bool TableSchema::setLookupFieldSchema(const QString& fieldName, LookupFieldSchema *lookupFieldSchema)
{
    Field *f = field(fieldName);
    if (!f) {
        PreWarn << "no such field" << fieldName << "in table" << name();
        return false;
    }
    delete d->lookupFields.take(f);
    if (lookupFieldSchema) {
        d->lookupFields.insert(f, lookupFieldSchema);
    }
    d->lookupFieldsList.clear(); //this will force to rebuid the internal cache
    return true;
}

LookupFieldSchema *TableSchema::lookupFieldSchema(const Field& field) const
{
    return d->lookupFields.value(&field);
}

LookupFieldSchema *TableSchema::lookupFieldSchema(const QString& fieldName)
{
    Field *f = TableSchema::field(fieldName);
    if (!f)
        return 0;
    return lookupFieldSchema(*f);
}

QVector<LookupFieldSchema*> TableSchema::lookupFields() const
{
    if (d->lookupFields.isEmpty())
        return QVector<LookupFieldSchema*>();
    if (!d->lookupFields.isEmpty() && !d->lookupFieldsList.isEmpty())
        return d->lookupFieldsList; //already updated
    //update
    d->lookupFieldsList.clear();
    d->lookupFieldsList.resize(d->lookupFields.count());
    uint i = 0;
    foreach(Field* f, m_fields) {
        QHash<const Field*, LookupFieldSchema*>::ConstIterator itMap = d->lookupFields.constFind(f);
        if (itMap != d->lookupFields.constEnd()) {
            d->lookupFieldsList[i] = itMap.value();
            i++;
        }
    }
    return d->lookupFieldsList;
}

//--------------------------------------

InternalTableSchema::InternalTableSchema(const QString& name)
        : TableSchema(name)
{
}

InternalTableSchema::InternalTableSchema(const TableSchema& ts)
        : TableSchema(ts, false)
{
}

InternalTableSchema::~InternalTableSchema()
{
}


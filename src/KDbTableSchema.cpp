/* This file is part of the KDE project
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2003-2016 Jarosław Staniek <staniek@kde.org>

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
#include "KDbQuerySchema.h"
#include "kdb_debug.h"

//! @internal
class Q_DECL_HIDDEN KDbTableSchema::Private
{
public:
    Private(KDbTableSchema *t)
            : q(t)
            , anyNonPKField(nullptr)
            , conn(nullptr)
            , pkey(nullptr)
            , query(nullptr)
    {
    }

    ~Private() {
        clearLookupFields();
        qDeleteAll(indices);
        delete query;
    }

    void clearLookupFields() {
        qDeleteAll(lookupFields);
        lookupFields.clear();
    }

    void addIndex(KDbIndexSchema *index) {
        indices.append(index);
        index->setTable(q);
    }

    KDbTableSchema * const q;
    KDbField *anyNonPKField;
    QHash<const KDbField*, KDbLookupFieldSchema*> lookupFields;
    QVector<KDbLookupFieldSchema*> lookupFieldsList;
    QList<KDbIndexSchema*> indices;
    //! @todo IMPORTANT: use something like QPointer<KDbConnection> conn
    KDbConnection *conn;
    KDbIndexSchema *pkey;
    KDbQuerySchema *query; //!< cached query schema that is defined by "select * from <this_table_name>"
private:
    Q_DISABLE_COPY(Private)
};

//-------------------------------------

KDbTableSchema::KDbTableSchema(const QString& name)
        : KDbFieldList(true)
        , KDbObject(KDb::TableObjectType)
        , d(new Private(this))
{
    setName(name);
    init(nullptr);
}

KDbTableSchema::KDbTableSchema(const KDbObject& other)
        : KDbFieldList(true)
        , KDbObject(other)
        , d(new Private(this))
{
    init(nullptr);
}

KDbTableSchema::KDbTableSchema()
        : KDbFieldList(true)
        , KDbObject(KDb::TableObjectType)
        , d(new Private(this))
{
    init(nullptr);
}

KDbTableSchema::KDbTableSchema(const KDbTableSchema& ts, bool copyId)
        : KDbFieldList(static_cast<const KDbFieldList&>(ts))
        , KDbObject(static_cast<const KDbObject&>(ts))
        , d(new Private(this))
{
    init(ts, copyId);
}

KDbTableSchema::KDbTableSchema(const KDbTableSchema& ts, int id)
        : KDbFieldList(static_cast<const KDbFieldList&>(ts))
        , KDbObject(static_cast<const KDbObject&>(ts))
        , d(new Private(this))
{
    init(ts, false);
    setId(id);
}

// used by KDbConnection
KDbTableSchema::KDbTableSchema(KDbConnection *conn, const QString & name)
        : KDbFieldList(true)
        , KDbObject(KDb::TableObjectType)
        , d(new Private(this))
{
    setName(name);
    init(conn);
}

KDbTableSchema::~KDbTableSchema()
{
    if (d->conn) {
        d->conn->removeMe(this);
    }
    delete d;
}

void KDbTableSchema::init(KDbConnection* conn)
{
    d->conn = conn;
    d->pkey = new KDbIndexSchema;
    d->addIndex(d->pkey);
}

void KDbTableSchema::init(const KDbTableSchema& ts, bool copyId)
{
    d->conn = ts.connection();
    setName(ts.name());
    d->pkey = nullptr; //will be copied
    if (!copyId)
        setId(-1);

    //deep copy all members
    foreach(KDbIndexSchema* otherIdx, *ts.indices()) {
        // fields from _this_ table will be assigned to the index
        KDbIndexSchema *idx = copyIndexFrom(*otherIdx);
        if (idx->isPrimaryKey()) {//assign pkey
            d->pkey = idx;
        }
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

KDbIndexSchema* KDbTableSchema::primaryKey()
{
    return d->pkey;
}

const KDbIndexSchema* KDbTableSchema::primaryKey() const
{
    return d->pkey;
}

const QList<KDbIndexSchema*>::ConstIterator KDbTableSchema::indicesIterator() const
{
    return QList<KDbIndexSchema*>::ConstIterator (d->indices.constBegin());
}

const QList<KDbIndexSchema*>* KDbTableSchema::indices() const
{
    return &d->indices;
}

bool KDbTableSchema::addIndex(KDbIndexSchema *index)
{
    if (index && !d->indices.contains(index)) {
        d->addIndex(index);
        return true;
    }
    return false;
}

bool KDbTableSchema::removeIndex(KDbIndexSchema *index)
{
    if (index) {
        d->indices.removeOne(index);
        return true;
    }
    return false;
}

KDbIndexSchema* KDbTableSchema::copyIndexFrom(const KDbIndexSchema& index)
{
    KDbIndexSchema *newIndex = new KDbIndexSchema(index, this);
    addIndex(newIndex);
    return newIndex;
}

bool KDbTableSchema::isInternal() const
{
    return dynamic_cast<const KDbInternalTableSchema*>(this);
}

void KDbTableSchema::setPrimaryKey(KDbIndexSchema *pkey)
{
    if (pkey && !d->indices.contains(pkey)) {
        kdbWarning() << *pkey << "index can't be made primary key because it does not belong "
                                 "to table schema" << name();
        return;
    }
    if (d->pkey && d->pkey != pkey) {
        if (d->pkey->fieldCount() == 0) {//this is empty key, probably default - remove it
            d->indices.removeOne(d->pkey);
            delete d->pkey;
        }
        else {
            d->pkey->setPrimaryKey(false); //there can be only one pkey..
            //that's ok, the old pkey is still on indices list, if not empty
        }
    }

    if (!pkey) {//clearing - set empty pkey
        pkey = new KDbIndexSchema;
        d->addIndex(pkey);
    }
    d->pkey = pkey; //!< @todo
    d->pkey->setPrimaryKey(true);
    d->anyNonPKField = nullptr; //for safety
}

bool KDbTableSchema::insertField(int index, KDbField *field)
{
    if (!field) {
        return false;
    }
    KDbField::List *fieldsList = fields();
    KDbFieldList::insertField(index, field);
    if (!field || index > fieldsList->count()) {
        return false;
    }
    field->setTable(this);
    field->setOrder(index);
    //update order for next next fields
    const int fieldCount = fieldsList->count();
    for (int i = index + 1; i < fieldCount; i++) {
        fieldsList->at(i)->setOrder(i);
    }

    //Check for auto-generated indices:
    KDbIndexSchema *idx = nullptr;
    if (field->isPrimaryKey()) {// this is auto-generated single-field unique index
        idx = new KDbIndexSchema;
        d->addIndex(idx);
        idx->setAutoGenerated(true);
        const bool ok = idx->addField(field);
        Q_ASSERT(ok);
        setPrimaryKey(idx);
    }
    if (field->isUniqueKey()) {
        if (!idx) {
            idx = new KDbIndexSchema;
            d->addIndex(idx);
            idx->setAutoGenerated(true);
            const bool ok = idx->addField(field);
            Q_ASSERT(ok);
        }
        idx->setUnique(true);
    }
    if (field->isIndexed()) {// this is auto-generated single-field
        if (!idx) {
            idx = new KDbIndexSchema;
            d->addIndex(idx);
            idx->setAutoGenerated(true);
            const bool ok = idx->addField(field);
            Q_ASSERT(ok);
        }
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
        d->anyNonPKField = nullptr;
    delete lookup;
    return true;
}

void KDbTableSchema::clear()
{
    d->indices.clear();
    d->clearLookupFields();
    KDbFieldList::clear();
    KDbObject::clear();
    d->conn = nullptr;
}

QDebug KDbTableSchema::debugFields(QDebug dbg) const
{
    dbg.nospace() << static_cast<const KDbFieldList&>(*this);
    for (const KDbField *f : *fields()) {
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

QDebug operator<<(QDebug dbg, const KDbInternalTableSchema& table)
{
    dbg.nospace() << "INTERNAL_TABLE";
    dbg.space() << static_cast<const KDbObject&>(table) << '\n';
    table.debugFields(dbg);
    return dbg.space();
}

KDbConnection* KDbTableSchema::connection() const
{
    return d->conn;
}

void KDbTableSchema::setConnection(KDbConnection* conn)
{
    d->conn = conn;
}

KDbQuerySchema* KDbTableSchema::query()
{
    if (d->query)
        return d->query;
    d->query = new KDbQuerySchema(this);   //it's owned by me
    return d->query;
}

KDbField* KDbTableSchema::anyNonPKField()
{
    if (!d->anyNonPKField) {
        KDbField *f = nullptr;
        for (QListIterator<KDbField*> it(*fields()); it.hasPrevious();) {
            f = it.previous();
            if (!f->isPrimaryKey() && (!d->pkey || !d->pkey->hasField(*f)))
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

KDbLookupFieldSchema *KDbTableSchema::lookupFieldSchema(const KDbField& field)
{
    return d->lookupFields.value(&field);
}

const KDbLookupFieldSchema *KDbTableSchema::lookupFieldSchema(const KDbField& field) const
{
    return d->lookupFields.value(&field);
}

KDbLookupFieldSchema *KDbTableSchema::lookupFieldSchema(const QString& fieldName)
{
    KDbField *f = KDbTableSchema::field(fieldName);
    if (!f)
        return nullptr;
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
    for (KDbField* f : *fields()) {
        QHash<const KDbField*, KDbLookupFieldSchema*>::ConstIterator itMap = d->lookupFields.constFind(f);
        if (itMap != d->lookupFields.constEnd()) {
            d->lookupFieldsList[i] = itMap.value();
            i++;
        }
    }
    return d->lookupFieldsList;
}

//--------------------------------------

class Q_DECL_HIDDEN KDbInternalTableSchema::Private
{
public:
    Private() {}
    bool dummy = false;
};

KDbInternalTableSchema::KDbInternalTableSchema(const QString& name)
        : KDbTableSchema(name)
        , d(new Private)
{
}

KDbInternalTableSchema::KDbInternalTableSchema(const KDbTableSchema& ts)
        : KDbTableSchema(ts, false)
        , d(new Private)
{
}

KDbInternalTableSchema::KDbInternalTableSchema(const KDbInternalTableSchema& ts)
        : KDbTableSchema(ts, false)
        , d(new Private)
{
}

KDbInternalTableSchema::~KDbInternalTableSchema()
{
    delete d;
}

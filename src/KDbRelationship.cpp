/* This file is part of the KDE project
   Copyright (C) 2003-2017 Jarosław Staniek <staniek@kde.org>

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

#include "KDbRelationship.h"
#include "KDbIndexSchema.h"
#include "KDbTableSchema.h"
#include "KDbQuerySchema.h"
#include "KDbDriver.h"
#include "kdb_debug.h"

class Q_DECL_HIDDEN KDbRelationship::Private
{
public:
    Private(KDbRelationship *r)
        : q(r)
    {
    }

    void createIndices(KDbQuerySchema *query, KDbField *field1, KDbField *field2)
    {
        if (!field1 || !field2 || !query) {
            kdbWarning() << "!masterField || !detailsField || !query";
            return;
        }
        if (field1->isQueryAsterisk() || field2->isQueryAsterisk()) {
            kdbWarning() << "relationship's fields cannot be asterisks";
            return;
        }
        if (field1->table() == field2->table()) {
            kdbWarning() << "fields cannot belong to the same table";
            return;
        }
        if (!query->contains(field1->table()) || !query->contains(field2->table())) {
            kdbWarning() << "fields do not belong to this query";
            return;
        }
        //! @todo: check more things: -types
        //! @todo: find existing global db relationships

        KDbField *masterField = nullptr;
        KDbField *detailsField = nullptr;
        bool p1 = field1->isPrimaryKey(), p2 = field2->isPrimaryKey();
        if (p1 && p2) {
            //2 primary keys
            masterField = field1;
            masterIndex = masterField->table()->primaryKey();
            detailsField = field2;
            detailsIndex = detailsField->table()->primaryKey();
        } else if (!p1 && p2) {
            //foreign + primary: swap
            KDbField *tmp = field1;
            field1 = field2;
            field2 = tmp;
            p1 = !p1;
            p2 = !p2;
        }

        if (p1 && !p2) {
            //primary + foreign
            masterField = field1;
            masterIndex = masterField->table()->primaryKey();
            detailsField = field2;
            //create foreign key
            //! @todo: check if it already exists
            detailsIndex = new KDbIndexSchema;
            detailsField->table()->addIndex(detailsIndex);
            detailsIndexOwned = true;
            const bool ok = detailsIndex->addField(detailsField);
            Q_ASSERT(ok);
            detailsIndex->setForeignKey(true);
        } else if (!p1 && !p2) {
            masterField = field1;
            masterIndex = new KDbIndexSchema;
            masterField->table()->addIndex(masterIndex);
            masterIndexOwned = true;
            bool ok = masterIndex->addField(masterField);
            Q_ASSERT(ok);
            masterIndex->setForeignKey(true);

            detailsField = field2;
            detailsIndex = new KDbIndexSchema;
            detailsField->table()->addIndex(detailsIndex);
            detailsIndexOwned = true;
            ok = detailsIndex->addField(detailsField);
            Q_ASSERT(ok);
            detailsIndex->setForeignKey(true);
        }

        if (!masterIndex || !detailsIndex) {
            return; //failed
        }

        (void)setIndices(masterIndex, detailsIndex, false);
    }

    /*! Internal version of setIndices(). @a ownedByMaster parameter is passed
     to KDbIndexSchema::attachRelationship() */
    bool setIndices(KDbIndexSchema *newMasterIndex, KDbIndexSchema *newDetailsIndex,
                    bool ownedByMaster)
    {
        masterIndex = nullptr;
        detailsIndex = nullptr;
        pairs.clear();
        if (!newMasterIndex || !newDetailsIndex || !newMasterIndex->table()
            || !newDetailsIndex->table() || newMasterIndex->table() == newDetailsIndex->table()
            || newMasterIndex->fieldCount() != newDetailsIndex->fieldCount()) {
            return false;
        }
        const KDbField::List *masterIndexFields = newMasterIndex->fields();
        const KDbField::List *detailsIndexFields = newDetailsIndex->fields();
        KDbField::ListIterator masterIt(masterIndexFields->constBegin());
        KDbField::ListIterator detailsIt(detailsIndexFields->constBegin());
        for (; masterIt != masterIndexFields->constEnd()
             && detailsIt != detailsIndexFields->constEnd();
             ++masterIt, ++detailsIt) {
            KDbField *masterField = *masterIt;
            KDbField *detailsField = *detailsIt;
            const KDbField::Type masterType
                = masterField->type(); // cache: evaluating type of expressions can be expensive
            const KDbField::Type detailsType = detailsField->type();
            if (masterType != detailsType
                && KDbField::isIntegerType(masterType) != KDbField::isIntegerType(detailsType)
                && KDbField::isTextType(masterType) != KDbField::isTextType(detailsType)) {
                kdbWarning() << "INDEX on" << newMasterIndex->table()->name() << ", INDEX on"
                             << newDetailsIndex->table()->name()
                             << ": !equal field types:" << KDbDriver::defaultSqlTypeName(masterType)
                             << masterField->name() << ","
                             << KDbDriver::defaultSqlTypeName(detailsType) << detailsField->name();
                pairs.clear();
                return false;
            }
#if 0 // too STRICT!
            if ((masterField->isUnsigned() && !detailsField->isUnsigned())
                    || (!masterField->isUnsigned() && detailsField->isUnsigned())) {
                kdbWarning() << "KDbRelationship::setIndices(INDEX on '" << masterIndex->table()->name()
                << "',INDEX on " << detailsIndex->table()->name() << "): !equal signedness of field types: "
                << KDbDriver::defaultSqlTypeName(masterField->type()) << " " << masterField->name() << ", "
                << KDbDriver::defaultSqlTypeName(detailsField->type()) << " " << detailsField->name();
                pairs.clear();
                return;
            }
#endif
            pairs.append(KDbField::Pair(masterField, detailsField));
        }
        // ok: update information
        if (masterIndex) { // detach yourself
            masterIndex->detachRelationship(q);
        }
        if (detailsIndex) { // detach yourself
            detailsIndex->detachRelationship(q);
        }
        masterIndex = newMasterIndex;
        detailsIndex = newDetailsIndex;
        masterIndex->attachRelationship(q, ownedByMaster);
        detailsIndex->attachRelationship(q, ownedByMaster);
        return true;
    }

    KDbIndexSchema *masterIndex = nullptr;
    KDbIndexSchema *detailsIndex = nullptr;
    KDbField::PairList pairs;
    bool masterIndexOwned = false;
    bool detailsIndexOwned = false;

private:
    KDbRelationship * const q;
};

KDbRelationship::KDbRelationship()
    : d(new Private(this))
{
}

KDbRelationship::KDbRelationship(KDbIndexSchema* masterIndex, KDbIndexSchema* detailsIndex)
    : KDbRelationship()
{
    (void)setIndices(masterIndex, detailsIndex);
}

KDbRelationship::KDbRelationship(KDbQuerySchema *query, KDbField *field1, KDbField *field2)
    : KDbRelationship()
{
    d->createIndices(query, field1, field2);
}

KDbRelationship::~KDbRelationship()
{
    if (d->masterIndexOwned) {
        delete d->masterIndex;
    }
    if (d->detailsIndexOwned) {
        delete d->detailsIndex;
    }
    delete d;
}

KDbRelationship& KDbRelationship::operator=(KDbRelationship &other)
{
    (void)setIndices(other.masterIndex(), other.detailsIndex());
    return *this;
}

bool KDbRelationship::operator==(const KDbRelationship& other) const
{
    return d->masterIndex == other.masterIndex() && d->detailsIndex == other.detailsIndex();
}

KDbIndexSchema *KDbRelationship::masterIndex()
{
    return d->masterIndex;
}

const KDbIndexSchema *KDbRelationship::masterIndex() const
{
    return d->masterIndex;
}

KDbIndexSchema *KDbRelationship::detailsIndex()
{
    return d->detailsIndex;
}

const KDbIndexSchema *KDbRelationship::detailsIndex() const
{
    return d->detailsIndex;
}

KDbField::PairList *KDbRelationship::fieldPairs()
{
    return &d->pairs;
}

const KDbField::PairList *KDbRelationship::fieldPairs() const
{
    return &d->pairs;
}

bool KDbRelationship::isEmpty() const
{
    return d->pairs.isEmpty();
}

KDbTableSchema* KDbRelationship::masterTable()
{
    return d->masterIndex ? d->masterIndex->table() : nullptr;
}

const KDbTableSchema* KDbRelationship::masterTable() const
{
    return d->masterIndex ? d->masterIndex->table() : nullptr;
}

KDbTableSchema* KDbRelationship::detailsTable()
{
    return d->detailsIndex ? d->detailsIndex->table() : nullptr;
}

const KDbTableSchema* KDbRelationship::detailsTable() const
{
    return d->detailsIndex ? d->detailsIndex->table() : nullptr;
}

bool KDbRelationship::setIndices(KDbIndexSchema* masterIndex, KDbIndexSchema* detailsIndex)
{
    return d->setIndices(masterIndex, detailsIndex, true);
}

/* This file is part of the KDE project
   Copyright (C) 2003-2004 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_RELATIONSHIP_H
#define KDB_RELATIONSHIP_H

#include "KDbField.h"

/*! KDbRelationship provides information about one-to-many relationship between two tables.
 Relationship is defined by a pair of (potentially multi-field) indices:
 - "one" or "master" side: unique key
 - "many" or "details" side: referenced foreign key
 <pre>
 [unique key, master] ----< [foreign key, details]
 </pre>

 In this documentation, we will call table that owns fields of "one" side as
 "master side of the relationship", and the table that owns foreign key fields of
 as "details side of the relationship".
 Use masterTable(), and detailsTable() to get one-side table and many-side table, respectively.

 Note: some engines (e.g. MySQL with InnoDB) requires that indices at both sides
 have to be explicitly created.

 @todo (js) It is planned that this will be handled by KDb internally and transparently.

 Each (of the two) key can be defined (just like index) as list of fields owned by one table.
 Indeed, relationship info can retrieved from KDbRelationship object in two ways:
 -# pair of indices; use masterIndex(), detailsIndex() for that
 -# ordered list of field pairs (<master-side-field, details-side-field>); use fieldPairs() for that

 No assigned objects (like fields, indices) are owned by KDbRelationship object. The exception is that
 list of field-pairs is internally created (on demand) and owned.

 KDbRelationship object is owned by KDbIndexSchema object (the one that is defined at master-side of the
 relationship).
 Note also that KDbIndexSchema objects are owned by appropriate tables, so thus
 there is implicit ownership between KDbTableSchema and KDbRelationship.

 If KDbRelationship object is not attached to KDbIndexSchema object,
 you should care about destroying it by hand.

  Example:
  <pre>
            ----------
   ---r1--<|          |
           | Table A [uk]----r3---<
   ---r2--<|          |
            ----------
  </pre>
  Table A has two relationships (r1, r2) at details side and one (r3) at master side.
  [uk] stands for unique key.
*/

class KDbIndexSchema;
class KDbTableSchema;
class KDbQuerySchema;

class KDB_EXPORT KDbRelationship
{
public:
    typedef QList<KDbRelationship*> List;
    typedef QList<KDbRelationship*>::ConstIterator ListIterator;

    /*! Creates uninitialized KDbRelationship object.
      setIndices() will be required to call.
    */
    KDbRelationship();

    /*! Creates KDbRelationship object and initialises it just by
     calling setIndices(). If setIndices() failed, object is still uninitialised.
    */
    KDbRelationship(KDbIndexSchema* masterIndex, KDbIndexSchema* detailsIndex);

    virtual ~KDbRelationship();

    /*! @return index defining master side of this relationship
     or null if there is no information defined. */
    KDbIndexSchema* masterIndex() const {
        return m_masterIndex;
    }

    /*! @return index defining referenced side of this relationship.
     or null if there is no information defined. */
    KDbIndexSchema* detailsIndex() const {
        return m_detailsIndex;
    }

    /*! @return ordered list of field pairs -- alternative form
     for representation of relationship or null if there is no information defined.
     Each pair has a form of <master-side-field, details-side-field>. */
    KDbField::PairList* fieldPairs() {
        return &m_pairs;
    }

    bool isEmpty() const {
        return m_pairs.isEmpty();
    }

    /*! @return table assigned at "master / one" side of this relationship.
     or null if there is no information defined. */
    KDbTableSchema* masterTable() const;

    /*! @return table assigned at "details / many / foreign" side of this relationship.
     or null if there is no information defined. */
    KDbTableSchema* detailsTable() const;

    /*! Sets @a masterIndex and @a detailsIndex indices for this relationship.
     This also sets information about tables for master- and details- sides.
     Notes:
     - both indices must contain the same number of fields
     - both indices must not be owned by the same table, and table (owner) must be not null.
     - corresponding field types must be the same
     - corresponding field types' signedness must be the same
     If above rules are not fulfilled, information about this relationship is cleared.
     On success, this KDbRelationship object is detached from previous KDbIndexSchema objects that were
     assigned before, and new are attached.
     */
    void setIndices(KDbIndexSchema* masterIndex, KDbIndexSchema* detailsIndex);

protected:
    KDbRelationship(KDbQuerySchema *query, KDbField *field1, KDbField *field2);

    void createIndices(KDbQuerySchema *query, KDbField *field1, KDbField *field2);

    /*! Internal version of setIndices(). @a ownedByMaster parameter is passed
     to KDbIndexSchema::attachRelationship() */
    void setIndices(KDbIndexSchema* masterIndex, KDbIndexSchema* detailsIndex, bool ownedByMaster);

    KDbIndexSchema *m_masterIndex;
    KDbIndexSchema *m_detailsIndex;

    KDbField::PairList m_pairs;

    bool m_masterIndexOwned;
    bool m_detailsIndexOwned;

    friend class KDbConnection;
    friend class KDbTableSchema;
    friend class KDbQuerySchema;
    friend class KDbIndexSchema;
};

#endif

/* This file is part of the KDE project
   Copyright (C) 2003-2017 Jarosław Staniek <staniek@kde.org>

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

    /**
     * @brief Creates a new uninitialized KDbRelationship object
     */
    KDbRelationship();

    /**
     * @brief Creates a new KDbRelationship object and initialises it using setIndices()
     *
     * If setIndices() failed, object is still uninitialised. Check this using isEmpty().
     */
    KDbRelationship(KDbIndexSchema* masterIndex, KDbIndexSchema* detailsIndex);

    virtual ~KDbRelationship();

    //! Assigns @a other to this object returns a reference to this object.
    //! @since 3.1
    KDbRelationship& operator=(KDbRelationship &other);

    //! @return true if this relationship is the same as @a other
    //! Relationships are equal if they have the same details and master indices are equal
    //! @since 3.1
    bool operator==(const KDbRelationship& other) const;

    //! @return @c true if this object is not equal to @a other; otherwise returns @c false.
    //! @since 3.1
    inline bool operator!=(const KDbRelationship &other) const { return !operator==(other); }

    /*! @return index defining master side of this relationship
     or @c nullptr if there is no information defined. */
    KDbIndexSchema* masterIndex();

    //! @overload
    const KDbIndexSchema* masterIndex() const;

    /*! @return index defining referenced side of this relationship.
     or @c nullptr if there is no information defined. */
    KDbIndexSchema* detailsIndex();

    //! @overload
    const KDbIndexSchema* detailsIndex() const;

    /**
     * Returns ordered list of field pairs, alternative representation of relationship
     *
     * @c nullptr is returned if there is no information defined.
     * Each pair has a form of <master-side-field, details-side-field>.
     */
    KDbField::PairList* fieldPairs();

    //! @overload
    const KDbField::PairList* fieldPairs() const;

    //! @return true if there are no master-details pairs in this relationship object
    //! @see fieldPairs()
    bool isEmpty() const;

    /*! @return table assigned at "master / one" side of this relationship.
     or @c nullptr if there is no information defined. */
    KDbTableSchema* masterTable();

    //! @overload
    const KDbTableSchema* masterTable() const;

    /*! @return table assigned at "details / many / foreign" side of this relationship.
     or @c nullptr if there is no information defined. */
    KDbTableSchema* detailsTable();

    //! @overload
    const KDbTableSchema* detailsTable() const;

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
    bool setIndices(KDbIndexSchema* masterIndex, KDbIndexSchema* detailsIndex);

protected:
    KDbRelationship(KDbQuerySchema *query, KDbField *field1, KDbField *field2);

    friend class KDbConnection;
    friend class KDbTableSchema;
    friend class KDbQuerySchema;
    friend class KDbIndexSchema;

private:
    class Private;
    Private * const d;
};

#endif

/* This file is part of the KDE project
   Copyright (C) 2003-2010 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_INDEXSCHEMA_H
#define KDB_INDEXSCHEMA_H

#include <QList>
#include <QSet>

#include "KDbObject.h"
#include "KDbFieldList.h"

class KDbConnection;
class KDbTableSchema;
class KDbQuerySchema;
class KDbRelationship;

/*! @short Provides information about database index that can be created for a database table.

  KDbIndexSchema object stores information about table fields that
  defines this index and additional properties like: whether index is unique
  or primary key (requires unique). Single-field index can be also auto generated.
*/
class KDB_EXPORT KDbIndexSchema : public KDbFieldList, public KDbObject
{
public:
    typedef QList<KDbIndexSchema*> List;
    typedef QList<KDbIndexSchema*>::ConstIterator ListIterator;

    /*! Constructs empty index schema object
     that is assigned to @a table, and will be owned by this table.
     Any fields added with addField() won't be owned by index,
     but by its table. Do not forget to add these fields to table,
     because adding these to KDbIndexSchema is not enough.
     */
    explicit KDbIndexSchema(KDbTableSchema *tableSchema);

    /*! Copy constructor. Copies all attributes from index @a idx, and
     fields assigned with it but the fields are taken (by name) from
     @a parentTable, not from @a idx itself, so it's possible to copy of index
     for a copy of table.

     To copy an index within the same table it's enough to call:
     @code
     new KDbIndexSchema(idx, idx.table());
     @endcode
     @todo All relationships should be also copied
     */
    KDbIndexSchema(const KDbIndexSchema& idx, KDbTableSchema* parentTable);

    /*! Destroys the index. KDbField objects are not deleted.
     All KDbRelationship objects listed in masterRelationships() list
     are destroyed (these are also detached from
     detail-side indices before destruction).
     KDbRelationship objects listed in detailsRelationships() are not touched. */
    virtual ~KDbIndexSchema();

    /*! Adds field at the end of field list.
     KDbField will not be owned by index. KDbField must belong to a table
     the index is bulit on, otherwise field couldn't be added. */
    virtual bool addField(KDbField *field);

    /*! @return table that index is defined for. */
    KDbTableSchema* table() const;

    /*! @return list of relationships from the table (of this index),
     i.e. any such relationship in which this table is at 'master' side.
     See KDbRelationship class documentation for more information.
     All objects listed here will be automatically destroyed on this KDbIndexSchema object destruction. */
    inline QList<KDbRelationship*>* masterRelationships() {
        return &m_master_rels;
    }

    /*! @return list of relationships to the table (of this index),
     i.e. any such relationship in which this table is at 'details' side.
     See KDbRelationship class documentation for more information. */
    inline QList<KDbRelationship*>* detailsRelationships() {
        return &m_details_rels;
    }

    /*! Attaches relationship definition @a rel to this KDbIndexSchema object.
     If @a rel relationship has this KDbIndexSchema defined at the master-side,
     @a rel is added to the list of master relationships (available with masterRelationships()).
     If @a rel relationship has this KDbIndexSchema defined at the details-side,
     @a rel is added to the list of details relationships (available with detailsRelationships()).
     For the former case, attached @a rel object is now owned by this KDbIndexSchema object.

     Note: call detachRelationship() for KDbIndexSchema object that @a rel
     was previously attached to, if any. */
    void attachRelationship(KDbRelationship *rel);

    /*! Detaches relationship definition @a rel for this KDbIndexSchema object
     from the list of master relationships (available with masterRelationships()),
     or from details relationships list, depending for which side of the relationship
     is this IndexSchem object assigned.

     Note: If @a rel was detached from masterRelationships() list,
     this object now has no parent, so you need to attach it to somewhere or destruct it.
    */
    void detachRelationship(KDbRelationship *rel);

    /*! @return true if index is auto-generated.
      Auto-generated index is one-field index
      that was automatically generated
      for CREATE TABLE statement when the field has
      UNIQUE or PRIMARY KEY constraint enabled.

      Any newly created KDbIndexSchema object
      has this flag set to false.

      This flag is handled internally by KDbTableSchema.
      It can be usable for GUI application if we do not
      want display implicity/auto generated indices
      on the indices list or we if want to show these
      indices to the user in a special way.
    */
    bool isAutoGenerated() const;

    /*! @return true if this index is primary key of its table.
      This can be one or multifield. */
    bool isPrimaryKey() const;

    /*! Sets PRIMARY KEY flag. @see isPrimary().
     Note: Setting PRIMARY KEY on (true),
     UNIQUE flag will be also implicity set. */
    void setPrimaryKey(bool set);

    /*! @return true if this is unique index.
     This can be one or multifield. */
    bool isUnique() const;

    /*! Sets UNIQUE flag. @see isUnique().
     Note: Setting UNIQUE off (false), PRIMARY KEY flag will
     be also implicity set off, because this UNIQUE
     is the requirement for PRIMARY KEYS. */
    void setUnique(bool set);

    /*! @return true if the index defines a foreign key,
     Created implicity for KDbRelationship object.*/
    bool isForeignKey() const;

protected:
    /*! Sets auto-generated flag. This method should be called only
     from KDbTableSchema code
    @see isAutoGenerated(). */
    void setAutoGenerated(bool set);

    /*! If @a set is true, declares that the index defines a foreign key,
     created implicity for KDbRelationship object. Setting this to true, implies
     clearing 'primary key', 'unique' and 'auto generated' flags.
     If this index contains just single field, it's 'foreign field'
     flag will be set to true as well. */
    void setForeignKey(bool set);

    /*! Internal version of attachRelationship(). If @a ownedByMaster is true,
     attached @a rel object will be owned by this index. */
    void attachRelationship(KDbRelationship *rel, bool ownedByMaster);

    KDbTableSchema *m_tableSchema; //! table on that index is built

    /*! A set of master relationships for the table (of this index),
     this index is a master key for these relationships
     and therefore - owner of these */
    QSet<KDbRelationship*> m_master_owned_rels;

    /*! a list of master relationships that are not owned by this schema */
    QList<KDbRelationship*> m_master_rels;

    /*! a list of relationships to table (of this index) */
    QList<KDbRelationship*> m_details_rels;

    bool m_primary;
    bool m_unique;
    bool m_isAutoGenerated;
    bool m_isForeignKey;

    friend class KDbConnection;
    friend class KDbTableSchema;
    friend class KDbQuerySchema;
    friend class KDbRelationship;
private:
    Q_DISABLE_COPY(KDbIndexSchema)
};

//! Sends information about index schema @a index to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbIndexSchema& index);

#endif

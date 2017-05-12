/* This file is part of the KDE project
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

#ifndef KDB_INDEXSCHEMA_H
#define KDB_INDEXSCHEMA_H

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
    /*! Constructs empty index schema object that not assigned to any @a table.
     KDbTableSchema::addIndex() should be called afterwards, before adding any fields
     or attaching relationships.
     Any fields added with addField() will not be owned by index but by their table.
     */
    KDbIndexSchema();

    /*! Deletes the index. Referenced KDbField objects are not deleted.
     All KDbRelationship objects listed by masterRelationships() are detached from
     detail-side indices and then deleted.
     KDbRelationship objects listed by detailsRelationships() are not deleted. */
    ~KDbIndexSchema() override;

    /*! Adds field at the end of field list.
     KDbField will not be owned by index. KDbField must belong to a table
     specified by a KDbTableSchema::addIndex() call, otherwise field couldn't be added.
     @note Do not forget to add the field to a table, because adding it only to
     the KDbIndexSchema is not enough. */
    virtual bool addField(KDbField *field);

    /*! @return table that index belongs to
     Index should be assigned to a table using KDbTableSchema::addIndex().
     If it is not, table() returns @c nullptr. */
    KDbTableSchema* table();

    /*! @return table that index is defined for, const version. */
    const KDbTableSchema* table() const;

    /*! @return list of relationships from the table (of this index),
     i.e. any such relationship in which this table is at 'master' side.
     See KDbRelationship class documentation for more information.
     All objects on this list will be automatically deleted when this KDbIndexSchema
     object is deleted. */
    QList<const KDbRelationship*> masterRelationships() const;

    /*! @return list of relationships to the table (of this index),
     i.e. any such relationship in which this table is at 'details' side.
     See KDbRelationship class documentation for more information. */
    QList<const KDbRelationship*> detailsRelationships() const;

    /*! Attaches relationship definition @a rel to this KDbIndexSchema object.
     If @a rel relationship has this KDbIndexSchema defined at the master-side,
     @a rel is added to the list of master relationships (available with masterRelationships()).
     If @a rel relationship has this KDbIndexSchema defined at the details-side,
     @a rel is added to the list of details relationships (available with detailsRelationships()).
     For the former case, attached @a rel object is now owned by this KDbIndexSchema object.

     Note: call detachRelationship() for KDbIndexSchema object that @a rel
     was previously attached to, if any.
     @note Before using attachRelationship() the index KDbField must already belong to a table
     specified by a KDbTableSchema::addIndex() call. */
    void attachRelationship(KDbRelationship *rel);

    /*! Detaches relationship definition @a rel for this KDbIndexSchema object
     from the list of master relationships (available with masterRelationships()),
     or from details relationships list, depending for which side of the relationship
     is this IndexSchem object assigned.

     Note: If @a rel was detached from masterRelationships() list,
     this object now has no parent, so it must be attached to other index or deleted.
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
    //! Used by KDbTableSchema::copyIndex(const KDbIndexSchema&)
    KDbIndexSchema(const KDbIndexSchema& index, KDbTableSchema* parentTable);

    //! Assigns this index to @a table
    //! table() must be @c nullptr and @a table must be not @a nullptr
    //! @since 3.1
    void setTable(KDbTableSchema *table);

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

    friend class KDbConnection;
    friend class KDbTableSchema;
    friend class KDbQuerySchema;
    friend class KDbRelationship;
private:
    class Private;
    Private * const d;
    Q_DISABLE_COPY(KDbIndexSchema)
};

//! Sends information about index schema @a index to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbIndexSchema& index);

#endif

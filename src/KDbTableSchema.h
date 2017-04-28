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

#ifndef KDB_TABLESCHEMA_H
#define KDB_TABLESCHEMA_H

#include <QString>
#include <QVector>

#include "KDbFieldList.h"
#include "KDbIndexSchema.h"

class KDbConnection;
class KDbLookupFieldSchema;

/*! Provides information about native database table
  that can be stored using KDb database engine.
*/
class KDB_EXPORT KDbTableSchema : public KDbFieldList, public KDbObject
{
public:
    explicit KDbTableSchema(const QString& name);
    explicit KDbTableSchema(const KDbObject& object);
    KDbTableSchema();

    /*! Copy constructor.
     If @a copyId is true, it's copied as well, otherwise the table id becomes -1,
     what is usable when we want to store the copy as an independent table. */
    explicit KDbTableSchema(const KDbTableSchema& ts, bool copyId);

    /*! Copy constructor like @ref KDbTableSchema(const KDbTableSchema&, bool).
     @a id is set as the table identifier. This is rarely usable, e.g.
     in project and data migration routines when we need to need deal with unique identifiers;
     @see KexiMigrate::performImport(). */
    KDbTableSchema(const KDbTableSchema& ts, int id);

    ~KDbTableSchema() override;

    /*! Inserts @a field into a specified position (@a index).
     'order' property of @a field is set automatically. */
    bool insertField(int index, KDbField *field) override;

    /*! Reimplemented for internal reasons. */
    bool removeField(KDbField *field) override;

    /*! @return list of fields that are primary key of this table.
     This method never returns @c nullptr value,
     if there is no primary key, empty KDbIndexSchema object is returned.
     KDbIndexSchema object is owned by the table schema. */
    KDbIndexSchema* primaryKey();

    //! @overload KDbIndexSchema* primaryKey()
    const KDbIndexSchema* primaryKey() const;

    /*! Sets table's primary key index to @a pkey.
     Pass pkey as @c nullptr to unassign existing primary key. In this case "primary"
     property of previous primary key KDbIndexSchema object that will be cleared,
     making it an ordinary index.

     If this table already has primary key assigned, it is unassigned using setPrimaryKey(nullptr).

     Before assigning as primary key, you should add the index to indices list
     with addIndex() (this is not done automatically!).
    */
    void setPrimaryKey(KDbIndexSchema *pkey);

    const QList<KDbIndexSchema*>::ConstIterator indicesIterator() const;

    const QList<KDbIndexSchema*>* indices() const;

    //! Adds index @a index to this table schema
    //! Ownership of the index is transferred to the table schema.
    //! @return true on success
    //! @since 3.1
    bool addIndex(KDbIndexSchema *index);

    //! Removes index @a index from this table schema
    //! Ownership of the index is transferred to the table schema.
    //! @return true on success
    //! @since 3.1
    bool removeIndex(KDbIndexSchema *index);

    /*! Creates a copy of index @a index with references moved to fields of this table.
     The new index is added to this table schema.
     Table fields are taken by name from this table. This way it's possible to copy index
     owned by other table and add it to another table, e.g. a copied one.

     To copy an index from another table, call:
     @code
     KDbIndexSchema *originalIndex = anotherTable->indices()->at(...);
     KDbIndexSchema *newIndex = table->copyIndex(*originalIndex);
     // newIndex is now created and is added to the table 'table'
     @endcode

     To copy an index within the same table, call:
     @code
     KDbIndexSchema *originalIndex = table->indices()->at(...);
     KDbIndexSchema *newIndex = table->copyIndex(*originalIndex);
     // newIndex is now created and is added to the table
     @endcode
     @since 3.1
     @todo All relationships should be also copied
     */
    KDbIndexSchema* copyIndexFrom(const KDbIndexSchema& index);

    /*! Removes all fields from the list, clears name and all other properties.
      @see KDbFieldList::clear() */
    void clear() override;

    /*! Sends information about fields of this table schema to debug output @a dbg. */
    QDebug debugFields(QDebug dbg) const;

    /*! @return connection object if table was created/retrieved using a connection,
      otherwise 0. */
    KDbConnection* connection() const;

    /*! @return true if this is internal KDb's table.
     Internal tables are hidden in applications (if desired) but are available
     for schema export/import functionality.

     Any internal KDb system table's schema (kexi__*) has
     cleared its KDbObject part, e.g. id=-1 for such table,
     and no description, caption and so on. This is because
     it represents a native database table rather that extended Kexi table.

     KDbTableSchema object has this property set to false, KDbInternalTableSchema has it
     set to true. */
    bool isInternal() const;

    /*! @return query schema object that is defined by "select * from <this_table_name>"
     This query schema object is owned by the table schema object.
     It is convenient way to get such a query when it is not available otherwise.
     Always returns non-0. */
    KDbQuerySchema* query();

    /*! @return any field not being a part of primary key of this table.
     If there is no such field, returns @c nullptr. */
    KDbField* anyNonPKField();

    /*! Sets lookup field schema @a lookupFieldSchema for @a fieldName.
     Passing null @a lookupFieldSchema will remove the previously set lookup field.
     @return true if @a lookupFieldSchema has been added,
     or false if there is no such field @a fieldName. */
    bool setLookupFieldSchema(const QString& fieldName, KDbLookupFieldSchema *lookupFieldSchema);

    /*! @return lookup field schema for @a field.
     0 is returned if there is no such field in the table or this field has no lookup schema.
     Note that even id non-zero is returned here, you may want to check whether lookup field's
     recordSource().name() is empty (if so, the field should behave as there was no lookup field
     defined at all). */
    KDbLookupFieldSchema *lookupFieldSchema(const KDbField& field) const;

    /*! @overload KDbLookupFieldSchema *KDbTableSchema::lookupFieldSchema( KDbField& field ) const */
    KDbLookupFieldSchema *lookupFieldSchema(const QString& fieldName);

    /*! @return list of lookup field schemas for this table.
     The order is the same as the order of fields within the table. */
    QVector<KDbLookupFieldSchema*> lookupFields() const;

protected:
    /*! Automatically retrieves table schema via connection. */
    explicit KDbTableSchema(KDbConnection *conn, const QString & name = QString());

    /*! For KDbConnection. */
    void setConnection(KDbConnection* conn);

private:
    //! Used by some ctors.
    void init(KDbConnection* conn);

    //! Used by some ctors.
    void init(const KDbTableSchema& ts, bool copyId);

    class Private;
    Private * const d;

    friend class KDbConnection;
    friend class KDbNativeStatementBuilder;
    Q_DISABLE_COPY(KDbTableSchema)
};

/*! Internal table with a name @a name. Rarely used.
 Use KDbConnection::createTable() to create a table using this schema.
 The table will not be visible as user table.
 For example, 'kexi__blobs' table is created this way by Kexi application. */
class KDB_EXPORT KDbInternalTableSchema : public KDbTableSchema
{
public:
    explicit KDbInternalTableSchema(const QString& name);
    explicit KDbInternalTableSchema(const KDbTableSchema& ts);
    KDbInternalTableSchema(const KDbInternalTableSchema& ts);
    ~KDbInternalTableSchema() override;

private:
    class Private;
    Private * const d;
};

//! Sends information about table schema @a table to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbTableSchema& table);

//! Sends information about internal table schema @a table to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbInternalTableSchema& table);

#endif

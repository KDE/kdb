/* This file is part of the KDE project
   Copyright (C) 2003-2018 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_ORDERBYCOLUMN_H
#define KDB_ORDERBYCOLUMN_H

#include "KDbEscapedString.h"
#include "KDbGlobal.h"

class KDbConnection;
class KDbField;
class KDbQueryColumnInfo;
class KDbQuerySchema;

//! @short KDbOrderByColumn provides information about a single query column used for sorting
/*! The column can be expression or table field. */
class KDB_EXPORT KDbOrderByColumn
{
public:
    //! Column sort order
    //! @since 3.1
    enum class SortOrder {
        Ascending = Qt::AscendingOrder,
        Descending = Qt::DescendingOrder
    };

    //! Creates an empty information about a single query column.
    KDbOrderByColumn();

    //! Creates a copy of the @a other
    //! @since 3.1
    KDbOrderByColumn(const KDbOrderByColumn &other);

    //! Creates information about a single query column @a column used for sorting.
    explicit KDbOrderByColumn(KDbQueryColumnInfo* column,
                              SortOrder order = SortOrder::Ascending, int pos = -1);

    //! Like above but used when the field @a field is not present on the list of columns.
    //! (e.g. SELECT a FROM t ORDER BY b; where T is a table with fields (a,b)).
    explicit KDbOrderByColumn(KDbField* field, SortOrder order = SortOrder::Ascending);

    ~KDbOrderByColumn();

    /*! @return copy of this KDbOrderByColumn object.
     @a fromQuery and @a toQuery is needed if column() is assigned to this info.
     Then, column info within @a toQuery will be assigned to the new KDbOrderByColumn object,
     corresponding to column() from "this" KDbOrderByColumn object. */
    Q_REQUIRED_RESULT KDbOrderByColumn *copy(KDbConnection *conn, KDbQuerySchema *fromQuery,
                                             KDbQuerySchema *toQuery) const;

    //! A column to sort.
    KDbQueryColumnInfo* column() const;

    /*! A helper for column() that allows you to know that sorting column
     was defined by providing its position. -1 by default.
     Example query: SELECT a, b FROM T ORDER BY 2 */
    int position() const;

    //! A field to sort, used only in case when the second constructor was used.
    KDbField *field() const;

    //! @return sort order for the column
    KDbOrderByColumn::SortOrder sortOrder() const;

    //! Assigns @a other to this object returns a reference to this object.
    //! @since 3.1
    KDbOrderByColumn& operator=(const KDbOrderByColumn &other);

    //! @return true if this column is the same as @a col
    bool operator==(const KDbOrderByColumn& col) const;

    //! @return @c true if this object is not equal to @a other; otherwise returns @c false.
    //! @since 3.1
    inline bool operator!=(const KDbOrderByColumn &other) const { return !operator==(other); }

    /** Return an SQL string like "name ASC" or "2 DESC" usable for building an SQL statement
     *
     * If @a includeTableNames is @c true fields that are related to a table are
     * printed as "tablename.fieldname".
     *
     * @a escapingType can be used to alter default escaping type.
     * If @a conn is not provided for DriverEscaping, no escaping is performed.
     * If @a query is provided, it can be used to obtain alias information.
     *
     * @since 3.2
     */
    KDbEscapedString toSqlString(bool includeTableName,
                                 KDbConnection *conn, KDbQuerySchema *query,
                                 KDb::IdentifierEscapingType escapingType = KDb::DriverEscaping) const;

    /*! @overload

     @deprecated since 3.2, use overload that also takes query schema
    */
    KDB_DEPRECATED KDbEscapedString toSqlString(bool includeTableName = true,
                                 KDbConnection *conn = nullptr,
                                 KDb::IdentifierEscapingType escapingType = KDb::DriverEscaping) const;

    //! Converts @a order to Qt::SortOrder type
    inline static Qt::SortOrder toQt(SortOrder order) { return static_cast<Qt::SortOrder>(order); }

    //! Converts @a order to SortOrder type
    inline static SortOrder fromQt(Qt::SortOrder order) { return static_cast<SortOrder>(order); }

private:
    class Private;
    Private * const d;
};

//! @short KDbOrderByColumnList provides list of sorted columns for a query schema
class KDB_EXPORT KDbOrderByColumnList
{
public:
    /*! Constructs empty list of ordered columns. */
    KDbOrderByColumnList();

    /*! A copy constructor. */
    KDbOrderByColumnList(const KDbOrderByColumnList& other, KDbConnection *conn,
                         KDbQuerySchema* fromQuery, KDbQuerySchema* toQuery);

    ~KDbOrderByColumnList();

    //! @return @c true if this object is equal to @a other; otherwise returns @c false.
    //! @since 3.1
    bool operator==(const KDbOrderByColumnList &other) const;

    //! @return @c true if this object is not equal to @a other; otherwise returns @c false.
    //! @since 3.1
    inline bool operator!=(const KDbOrderByColumnList &other) const { return !operator==(other); }

    //! Returns column with given index.
    //! @since 3.1
    const KDbOrderByColumn* value(int index) const;

    //! @overload
    KDbOrderByColumn* value(int index);

    /*! Appends multiple fields for sorting. @a querySchema
     is used to find appropriate field or alias name.
     @return false if there is at least one name for which a field or alias name does not exist
     (all the newly appended fields are removed in this case)
     Returns @c false if @a querySchema is @c nullptr. */
    bool appendFields(KDbConnection *conn, KDbQuerySchema* querySchema,
                      const QString& field1, KDbOrderByColumn::SortOrder order1 = KDbOrderByColumn::SortOrder::Ascending,
                      const QString& field2 = QString(), KDbOrderByColumn::SortOrder order2 = KDbOrderByColumn::SortOrder::Ascending,
                      const QString& field3 = QString(), KDbOrderByColumn::SortOrder order3 = KDbOrderByColumn::SortOrder::Ascending,
                      const QString& field4 = QString(), KDbOrderByColumn::SortOrder order4 = KDbOrderByColumn::SortOrder::Ascending,
                      const QString& field5 = QString(), KDbOrderByColumn::SortOrder order5 = KDbOrderByColumn::SortOrder::Ascending);

    /*! Appends column @a columnInfo.
     Does nothing if @a columnInfo is @c nullptr. */
    void appendColumn(KDbQueryColumnInfo* columnInfo,
                      KDbOrderByColumn::SortOrder order = KDbOrderByColumn::SortOrder::Ascending);

    /*! Appends a field @a field.
     Read documentation of @ref KDbOrderByColumn(KDbField* field, SortOrder order)
     for more info.
     Does nothing if @a field is @c nullptr. */
    void appendField(KDbField* field,
                     KDbOrderByColumn::SortOrder order = KDbOrderByColumn::SortOrder::Ascending);

    /*! Appends field with a name @a field.
     @return @c true on successful appending, and @c false if there is no such field or alias
     name in the @a querySchema.
     Returns @c false if @a querySchema is @c nullptr. */
    bool appendField(KDbConnection *conn, KDbQuerySchema* querySchema, const QString& fieldName,
                     KDbOrderByColumn::SortOrder order = KDbOrderByColumn::SortOrder::Ascending);

    /*! Appends a column that is at position @a pos (counted from 0).
     @return true on successful adding and false if there is no such position @a pos.
     Returns @c false if @a querySchema is @c nullptr. */
    bool appendColumn(KDbConnection *conn, KDbQuerySchema* querySchema,
                      KDbOrderByColumn::SortOrder order = KDbOrderByColumn::SortOrder::Ascending,
                      int pos = -1);

    /*! @return true if the list is empty. */
    bool isEmpty() const;

    /*! @return number of elements of the list. */
    int count() const;

    /*! Removes all elements from the list (deletes them). */
    void clear();

    /*! Returns an STL-style iterator pointing to the first column in the list. */
    QList<KDbOrderByColumn*>::Iterator begin();

    /*! Returns an STL-style iterator pointing to the imaginary item after the last column
     * in the list.
     */
    QList<KDbOrderByColumn*>::Iterator end();

    /*! Returns an const STL-style iterator pointing to the first column in the list. */
    QList<KDbOrderByColumn*>::ConstIterator constBegin() const;

    /*! Returns a const STL-style iterator pointing to the imaginary item after the last column
     * in the list.
     */
    QList<KDbOrderByColumn*>::ConstIterator constEnd() const;

    /** Return an SQL string like "name ASC, 2 DESC" usable for building an SQL statement
     *
     * If @a includeTableNames is @c true (the default) fields that are related to a table are
     * printed as "tablename.fieldname".
     *
     * @a escapingType can be used to alter default escaping type.
     * If @a conn is not provided for DriverEscaping, no escaping is performed.
     * If @a query is provided, it can be used to obtain alias information.
     *
     * @since 3.2
     */
    KDbEscapedString toSqlString(bool includeTableNames,
                                 KDbConnection *conn, KDbQuerySchema *query,
                                 KDb::IdentifierEscapingType escapingType = KDb::DriverEscaping) const;

    /*! @overload

     @deprecated since 3.2, use overload that also takes query schema
    */
    KDB_DEPRECATED KDbEscapedString toSqlString(bool includeTableNames = true,
                                 KDbConnection *conn = nullptr,
                                 KDb::IdentifierEscapingType escapingType = KDb::DriverEscaping) const;

private:
    class Private;
    Private * const d;
    Q_DISABLE_COPY(KDbOrderByColumnList)
};

//! Sends order-by-column information @a order to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbOrderByColumn& order);

//! Sends order-by-column-list information @a list to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbOrderByColumnList& list);

#endif

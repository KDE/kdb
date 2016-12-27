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
    //! @a column must not be 0.
    explicit KDbOrderByColumn(KDbQueryColumnInfo* column,
                              SortOrder order = SortOrder::Ascending, int pos = -1);

    //! Like above but used when the field @a field is not present on the list of columns.
    //! (e.g. SELECT a FROM t ORDER BY b; where T is a table with fields (a,b)).
    //! @a field must not be 0.
    explicit KDbOrderByColumn(KDbField* field, SortOrder order = SortOrder::Ascending);

    ~KDbOrderByColumn();

    /*! @return copy of this KDbOrderByColumn object.
     In @a fromQuery and @a toQuery is needed if column() is assigned to this info.
     Then, column info within @a toQuery will be assigned to the new KDbOrderByColumn object,
     corresponding to column() from "this" KDbOrderByColumn object. */
    KDbOrderByColumn* copy(KDbQuerySchema* fromQuery, KDbQuerySchema* toQuery) const;

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

    //! @return true if this column is thesame as @a col
    bool operator==(const KDbOrderByColumn& col) const;

    //! @return @c true if this object is not equal to @a other; otherwise returns @c false.
    //! @since 3.1
    inline bool operator!=(const KDbOrderByColumn &other) const { return !operator==(other); }

    /*! @return a string like "name ASC" usable for building an SQL statement.
     If @a includeTableNames is true (the default) field is output in a form
     of "tablename.fieldname" (but only if fieldname is not a name of alias).

     @a escapingType can be used to alter default escaping type.
     If @a conn is not provided for DriverEscaping, no escaping is performed. */
    KDbEscapedString toSQLString(bool includeTableName = true,
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
class KDB_EXPORT KDbOrderByColumnList : protected QList<KDbOrderByColumn*>
{
public:
    /*! Constructs empty list of ordered columns. */
    KDbOrderByColumnList();

    /*! A copy constructor. */
    KDbOrderByColumnList(const KDbOrderByColumnList& other,
                      KDbQuerySchema* fromQuery, KDbQuerySchema* toQuery);

    ~KDbOrderByColumnList();

    class KDB_EXPORT const_iterator : public QList<KDbOrderByColumn*>::const_iterator
    {
    public:
        inline const_iterator()
                : QList<KDbOrderByColumn*>::const_iterator() {}
        inline const_iterator(const QList<KDbOrderByColumn*>::const_iterator &o)
                : QList<KDbOrderByColumn*>::const_iterator(o) {}
    };

    class KDB_EXPORT iterator : public QList<KDbOrderByColumn*>::iterator
    {
    public:
        inline iterator()
                : QList<KDbOrderByColumn*>::iterator() {}
        inline iterator(const QList<KDbOrderByColumn*>::iterator &o)
                : QList<KDbOrderByColumn*>::iterator(o) {}
    };

    /*! Appends multiple fields for sorting. @a querySchema
     is used to find appropriate field or alias name.
     @return false if there is at least one name for which a field or alias name does not exist
     (all the newly appended fields are removed in this case) */
    //! @note @a querySchema must not be 0.
    bool appendFields(KDbQuerySchema* querySchema,
                      const QString& field1, KDbOrderByColumn::SortOrder order1 = KDbOrderByColumn::SortOrder::Ascending,
                      const QString& field2 = QString(), KDbOrderByColumn::SortOrder order2 = KDbOrderByColumn::SortOrder::Ascending,
                      const QString& field3 = QString(), KDbOrderByColumn::SortOrder order3 = KDbOrderByColumn::SortOrder::Ascending,
                      const QString& field4 = QString(), KDbOrderByColumn::SortOrder order4 = KDbOrderByColumn::SortOrder::Ascending,
                      const QString& field5 = QString(), KDbOrderByColumn::SortOrder order5 = KDbOrderByColumn::SortOrder::Ascending);

    /*! Appends column @a columnInfo. */
    //! @note @a columnInfo must not be @c nullptr.
    void appendColumn(KDbQueryColumnInfo* columnInfo,
                      KDbOrderByColumn::SortOrder order = KDbOrderByColumn::SortOrder::Ascending);

    /*! Appends a field @a field.
     Read documentation of @ref KDbOrderByColumn(KDbField* field, SortOrder order)
     for more info. */
    //! @note @a field must not be 0.
    void appendField(KDbField* field,
                     KDbOrderByColumn::SortOrder order = KDbOrderByColumn::SortOrder::Ascending);

    /*! Appends field with a name @a field.
     @return @c true on successful appending, and @c false if there is no such field or alias
     name in the @a querySchema. */
    //! @note @a querySchema must not be 0.
    bool appendField(KDbQuerySchema* querySchema, const QString& fieldName,
                     KDbOrderByColumn::SortOrder order = KDbOrderByColumn::SortOrder::Ascending);

    /*! Appends a column that is at position @a pos (counted from 0).
     @return true on successful adding and false if there is no such position @a pos. */
    //! @note @a querySchema must not be @c nullptr.
    bool appendColumn(KDbQuerySchema* querySchema,
                      KDbOrderByColumn::SortOrder order = KDbOrderByColumn::SortOrder::Ascending,
                      int pos = -1);

    /*! @return true if the list is empty. */
    bool isEmpty() const;

    /*! @return number of elements of the list. */
    int count() const;

    /*! Removes all elements from the list (deletes them). */
    void clear();

    iterator begin();

    iterator end();

    const_iterator constBegin() const;

    const_iterator constEnd() const;

    /*! @return a string like "name ASC, 2 DESC" usable for building an SQL statement.
     If @a includeTableNames is true (the default) fields are output in a form
     of "tablename.fieldname".

     @a escapingType can be used to alter default escaping type.
     If @a conn is not provided for DriverEscaping, no escaping is performed. */
    KDbEscapedString toSQLString(bool includeTableNames = true,
                                 KDbConnection *conn = nullptr,
                                 KDb::IdentifierEscapingType escapingType = KDb::DriverEscaping) const;
private:
    Q_DISABLE_COPY(KDbOrderByColumnList)
};

//! Sends order-by-column information @a order to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbOrderByColumn& order);

//! Sends order-by-column-list information @a list to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbOrderByColumnList& list);

#endif

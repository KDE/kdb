/* This file is part of the KDE project
   Copyright (C) 2004-2015 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDBTABLEORQUERYSCHEMA_H
#define KDBTABLEORQUERYSCHEMA_H

#include <QByteArray>

#include "KDbQueryColumnInfo.h"

class KDbConnection;
class KDbFieldList;
class KDbTableSchema;
class KDbQuerySchema;

/*! Variant class providing a pointer to table or query. */
class KDB_EXPORT KDbTableOrQuerySchema
{
public:
    //! Type of object: table or query
    //! @since 3.1
    enum class Type {
        Table,
        Query
    };

    /*! Creates a new KDbTableOrQuerySchema variant object, retrieving table or query schema
     using @a conn connection and @a name. If both table and query exists for @a name,
     table has priority over query.
     Check whether a query or table has been found by testing (query() || table()) expression. */
    KDbTableOrQuerySchema(KDbConnection *conn, const QByteArray& name);

    /*! Creates a new KDbTableOrQuerySchema variant object, retrieving table or query schema
     using @a conn connection and @a name. If @a type is Table, @a name is assumed
     to be a table name, otherwise @a name is assumed to be a query name.
     Check whether a query or table has been found by testing (query() || table()) expression.
     @since 3.1 */
    KDbTableOrQuerySchema(KDbConnection *conn, const QByteArray &name, Type type);

    /*! Creates a new KDbTableOrQuerySchema variant object. @a tableOrQuery must be of
     class KDbTableSchema or KDbQuerySchema.
     Check whether a query or table has been found by testing (query() || table()) expression. */
    explicit KDbTableOrQuerySchema(KDbFieldList *tableOrQuery);

    /*! Creates a new KDbTableOrQuerySchema variant object, retrieving table or query schema
     using @a conn connection and @a id.
     Check whether a query or table has been found by testing (query() || table()) expression. */
    KDbTableOrQuerySchema(KDbConnection *conn, int id);

    /*! Creates a new KDbTableOrQuerySchema variant object, keeping a pointer to @a table
     object. */
    explicit KDbTableOrQuerySchema(KDbTableSchema* table);

    /*! Creates a new KDbTableOrQuerySchema variant object, keeping a pointer to @a query
     object. */
    explicit KDbTableOrQuerySchema(KDbQuerySchema* query);

    ~KDbTableOrQuerySchema();

    //! @return a pointer to the query if it's provided
    KDbQuerySchema* query() const;

    //! @return a pointer to the table if it's provided
    KDbTableSchema* table() const;

    //! @return name of a query or table
    QByteArray name() const;

    //! @return caption (if present) or name of the table/query
    QString captionOrName() const;

    //! @return number of fields
    int fieldCount() const;

    //! @return all columns for the table or the query
    const KDbQueryColumnInfo::Vector columns(bool unique = false);

    /*! @return a field of the table or the query schema for name @a name
     or 0 if there is no such field. */
    KDbField* field(const QString& name);

    /*! Like KDbField* field(const QString& name);
     but returns all information associated with field/column @a name. */
    KDbQueryColumnInfo* columnInfo(const QString& name);

    /*! @return connection object, for table or query or 0 if there's no table or query defined. */
    KDbConnection* connection() const;

private:
    class Private;
    Private * const d;

    Q_DISABLE_COPY(KDbTableOrQuerySchema)
};

namespace KDb {
//! Sends information about table or query schema @a schema to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbTableOrQuerySchema& schema);
}

#endif

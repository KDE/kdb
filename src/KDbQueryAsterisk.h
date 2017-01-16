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

#ifndef KDB_QUERYASTERISK_H
#define KDB_QUERYASTERISK_H

#include "KDbField.h"

class KDbQuerySchema;

//! @short KDbQueryAsterisk class encapsulates information about single asterisk in query definition
/*! There are two types of query asterisks:

 1. "Single-table" asterisk, that references all fields of given table used
 in the query.
 Example SQL statement:
 @code
 SELECT staff.*, cars.model from staff, cars WHERE staff.car = cars.number;
 @endcode
 The "staff.*" element is our "single-table" asterisk;
 this tells us that we want to get all fields of table "staff".

 2. "All-tables" asterisk, that references all fields of all tables used in the query.
 Example SQL statement:
 @code
 SELECT * from staff, cars WHERE staff.car = cars.number;
 @endcode
 The "*" is our "all-tables" asterisk;
 this tells us that we want to get all fields of all used tables (here: "staff" and "cars").

 There can be many asterisks of 1st type defined for given single query.
 There can be one asterisk of 2nd type defined for given single query.
*/
class KDB_EXPORT KDbQueryAsterisk : public KDbField
{
public:
    /*! Constructs an "all-tables" query asterisk definition object ("*" in SQL notation).

     KDbQueryAsterisk objects are owned by KDbQuerySchema object
     (not by KDbTableSchema object like for ordinary KDbField objects)
     for that the KDbQueryAsterisk object was added (using KDbQuerySchema::addField()). */
    explicit KDbQueryAsterisk(KDbQuerySchema *query);

    /*! Constructs a "single-table" query asterisk definition object ("T.*" in SQL notation).
     @a table schema is the single table for the asterisk.

     KDbQueryAsterisk objects are owned by KDbQuerySchema object
     (not by KDbTableSchema object like for ordinary KDbField objects)
     for that the KDbQueryAsterisk object was added (using KDbQuerySchema::addField()). */
    KDbQueryAsterisk(KDbQuerySchema *query, const KDbTableSchema &table);

    /*! Constructs a deep copy of query asterisk definition object @a asterisk. */
    KDbQueryAsterisk(const KDbQueryAsterisk &asterisk);

    virtual ~KDbQueryAsterisk();

    /*! @return Query object for that this asterisk object is defined */
    KDbQuerySchema *query();

    /*! @overload KDbQuerySchema *query() */
    const KDbQuerySchema *query() const;

    /*! @return table schema object for that this asterisk object is defined.
    If this is a "all-tables" asterisk, @c nullptr is returned. */
    const KDbTableSchema* table();

    /*! @overload const KDbTableSchema* table() */
    const KDbTableSchema* table() const;

    /*! Sets table schema for this asterisk.
     If table is supplied, the asterisk become a "single-table" asterisk.
     If @a table is @c nullptr the asterisk becames "all-tables" asterisk. */
    void setTable(const KDbTableSchema *table);

    /*! This is convenience method that returns @c true
     if the asterisk has "all-tables" type (2nd type).*/
    bool isSingleTableAsterisk() const;

    /*! This is convenience method that returns @c true
     if the asterisk has "single-table" type (2nd type).*/
    bool isAllTableAsterisk() const;

protected:
    //! @return a deep copy of this object. Used in KDbFieldList(const KDbFieldList& fl).
    KDbField* copy() override;

    KDbQueryAsterisk(KDbQuerySchema *query, const KDbTableSchema *table);

private:
    class Private;
    Private * const d;
    KDbQueryAsterisk& operator=(const KDbQueryAsterisk &) = delete;
    void setTable(KDbTableSchema *table); // protect
};

//! Sends query asterisk information @a asterisk to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbQueryAsterisk& asterisk);

#endif

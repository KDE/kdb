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

#ifndef KDB_KDBSELECTSTATEMENTOPTIONS_H
#define KDB_KDBSELECTSTATEMENTOPTIONS_H

#include "kdb_export.h"

class KDbConnection;
class KDbQuerySchema;
class KDbTableSchema;
class KDbEscapedString;

//! Options used in KDbNativeStatementBuilder::generateSelectStatement()
class KDB_EXPORT KDbSelectStatementOptions //SDC: operator==
{
public:
    /*!
    @getter
    @return @c true if record IDs should be also retrieved. @c false by default.
    @setter
    Specifies whether record IDs should be also retrieved.
    */
    bool alsoRetrieveRecordId; //SDC: default=false

    /*!
    @getter
    @return @c true if relations (LEFT OUTER JOIN) for visible lookup columns should be added.
    @c false is used only when user-visible statement is generated e.g. the one used in Kexi's
    Query Designer. @c true by default.
    @setter
    Specifies whether if relations (LEFT OUTER JOIN) for visible lookup columns should be added.
    */
    bool addVisibleLookupColumns; //SDC: default=true
};

#endif

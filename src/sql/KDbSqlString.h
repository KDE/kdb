/* This file is part of the KDE project
   Copyright (C) 2016 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_SQLSTRING_H
#define KDB_SQLSTRING_H

#include "kdb_export.h"
#include <QByteArray>

//! The KDbSqlString class abstracts low-level information about a single string value returned by KDbSqlRecord
/**
 * KDbSqlString exists for optimization purposes. KDbSqlRecord can return KDbSqlString
 * objects to avoid premature converting to QString or QByteArray. This way memory allocations
 * are not required.
 */
class KDbSqlString
{
public:
    //! Creates an empty string object
    inline KDbSqlString() : string(nullptr), length(0) {}

    //! Creates string object from raw string @a s, of specified length
    inline KDbSqlString(const char *s, quint64 len) : string(s), length(len) {}

    //! @return true if this string value is empty. Here, NULL values are considered empty too.
    inline bool isEmpty() const { return !string || length == 0; }

    //! @return string value converted to bytea array
    //! For optimization, raw string data is used via, see QByteArray::fromRawData() for details.
    //! The caller must not delete data or modify the parent KDbSqlRecord object
    //! directly as long as the returned QByteArray exists.
    inline QByteArray rawDataToByteArray() const { return QByteArray::fromRawData(string, length); }

    const char *string;
    quint64 length;
};

#endif

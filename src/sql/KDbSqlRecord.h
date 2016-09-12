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

#ifndef KDB_SQLRECORD_H
#define KDB_SQLRECORD_H

#include "kdb_export.h"
#include <QtGlobal>

class QByteArray;
class QString;
class KDbSqlString;

//! The KDbSqlRecord class abstracts a single record obtained from a KDbSqlResult object
/**
 * KDbSqlRecord provides value of each column in a form of optimized KDbSqlString value,
 * QString or QByteArray.
 */
class KDB_EXPORT KDbSqlRecord
{
public:
    KDbSqlRecord();

    virtual ~KDbSqlRecord();

    virtual QString stringValue(int index) = 0;

    virtual QByteArray toByteArray(int index) = 0;

    virtual KDbSqlString cstringValue(int index) = 0;
private:
    Q_DISABLE_COPY(KDbSqlRecord)
};

#endif

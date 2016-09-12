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

#ifndef KDB_SQLFIELD_H
#define KDB_SQLFIELD_H

#include "kdb_export.h"
#include <QString>

//! The KDbSqlField class abstracts low-level information about a single field for KDbSqlResult
class KDB_EXPORT KDbSqlField
{
public:
    KDbSqlField();

    virtual ~KDbSqlField();

    //! @return column name
    virtual QString name() = 0;

    //! @return column type
    virtual int type() = 0;

    //! @return length limit
    virtual int length() = 0;
private:
    Q_DISABLE_COPY(KDbSqlField)
};

#endif

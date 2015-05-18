/* This file is part of the KDE project
   Copyright (C) 2005 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_CONNECTION_P_H
#define KDB_CONNECTION_P_H

#include "kdb_export.h"

class KDbConnection;

//! Interface for accessing connection's internal result, for use by drivers.
class KDB_EXPORT KDbConnectionInternal
{
public:
    explicit KDbConnectionInternal(KDbConnection *conn);

protected:
    KDbConnection* const connection;
};

#endif

/* This file is part of the KDE project
   Copyright (C) 2006 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_ADMIN_H
#define KDB_ADMIN_H

#include "KDbResult.h"

class KDbConnectionData;

//! @short An interface containing a set of tools for database administration
/*! Can be implemented in database drivers. @see KDbDriver::adminTools
*/
class KDB_EXPORT KDbAdminTools : public KDbResultable
{
public:
    KDbAdminTools();
    ~KDbAdminTools() override;

    /*! Performs vacuum (compacting) for connection @a data.
     Can be implemented for your driver.
     Note: in most cases the database should not be opened.

     Currently it is implemented for SQLite drivers.

     @return true on success, false on failure
     (then you can get error status from the KDbAdminTools object). */
    virtual bool vacuum(const KDbConnectionData& data, const QString& databaseName);

private:
    Q_DISABLE_COPY(KDbAdminTools)
    class Private;
    Private * const d;
};

#endif

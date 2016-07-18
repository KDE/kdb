/* This file is part of the KDE project
   Copyright (C) 2003-2016 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_DRIVERMETADATA_H
#define KDB_DRIVERMETADATA_H

#include <KPluginMetaData>

#include "kdb_export.h"

//! Provides information about a single driver plugin
class KDB_EXPORT KDbDriverMetaData : public KPluginMetaData
{
public:
    ~KDbDriverMetaData();

    //! @return internal name of the plugin, a shortcut of pluginId()
    QString id() const;


    //! @return true if the driver is for file-based databases like SQLite.
    /*! Defined by a "X-KDb-FileBased" field in "kdb_*.desktop" information files. */
    bool isFileBased() const;

    //! @return true if the driver is for a backend that allows importing.
    /*! Defined by X-KDb-ImportingEnabled field in "kdb_*.desktop" information files.
        Used for migration. */
    bool isImportingEnabled() const;

protected:
    explicit KDbDriverMetaData(const QPluginLoader &loader);
    friend class DriverManagerInternal;

private:
    Q_DISABLE_COPY(KDbDriverMetaData)
    class Private;
    Private * const d;
};

#endif

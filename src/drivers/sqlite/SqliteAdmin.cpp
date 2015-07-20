/* This file is part of the KDE project
   Copyright (C) 2006-2012 Jarosław Staniek <staniek@kde.org>

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

#include "SqliteAdmin.h"
#include "SqliteVacuum.h"

#include "KDbDriverManager.h"
#include "KDbDriver_p.h"

#include <QDir>

SqliteAdminTools::SqliteAdminTools()
        : KDbAdminTools()
{
}

SqliteAdminTools::~SqliteAdminTools()
{
}

#ifdef KDB_SQLITE_VACUUM
bool SqliteAdminTools::vacuum(const KDbConnectionData& data, const QString& databaseName)
{
    clearResult();
    KDbDriverManager manager;
    KDbDriver *drv = manager.driver(data.driverId());
    QString title(SqliteVacuum::tr("Could not compact database \"%1\".").arg(QDir::fromNativeSeparators(databaseName)));
    if (!drv) {
        m_result = manager.result();
        m_result.prependMessage(title);
        return false;
    }
    QFileInfo file(databaseName);
    SqliteVacuum vacuum(QDir::fromNativeSeparators(file.absoluteFilePath()));
    tristate result = vacuum.run();
    if (false == result) {
        m_result = KDbResult(title);
        return false;
    } else { //success or cancelled
        return true;
    }
}
#endif

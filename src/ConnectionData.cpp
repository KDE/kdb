/* This file is part of the KDE project
   Copyright (C) 2003-2010 Jarosław Staniek <staniek@kde.org>

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

#include "ConnectionData.h"
#include "DriverManager.h"

#include <QFileInfo>
#include <QDir>

using namespace Predicate;

ConnectionData::~ConnectionData()
{
}

void ConnectionData::setFileName(const QString& fileName)
{
    QFileInfo file(fileName);
    if (!fileName.isEmpty() && d->fileName != file.absoluteFilePath()) {
        d->fileName = QDir::convertSeparators(file.absoluteFilePath());
        d->databasePath = QDir::convertSeparators(file.absolutePath());
        d->databaseFileName = file.fileName();
    }
}

QString ConnectionData::serverInfoString(ServerInfoStringOptions options) const
{
    if (!d->databaseFileName.isEmpty())
        return QObject::tr("file: %1")
               .arg(d->databasePath.isEmpty() ? QString() : d->databasePath + QDir::separator() + d->databaseFileName);

    if (!d->driverName.isEmpty()) {
        DriverManager mananager;
        const DriverInfo info = mananager.driverInfo(d->driverName);
        if (info.isValid() && info.isFileBased())
            return QObject::tr("<file>");
    }

    return ((d->userName.isEmpty() || !(options & AddUserToServerInfoString)) ? QLatin1String("") : (d->userName + '@'))
           + (d->hostName.isEmpty() ? QLatin1String("localhost") : d->hostName)
           + (d->port != 0 ? (QLatin1String(":") + QString::number(d->port)) : QString());
}

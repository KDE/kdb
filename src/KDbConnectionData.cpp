/* This file is part of the KDE project
   Copyright (C) 2003-2015 Jarosław Staniek <staniek@kde.org>

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

#include "KDbConnectionData.h"
#include "KDbDriverManager.h"
#include "KDbDriverMetaData.h"
#include "KDbConnection.h"

#if 0 // needed by lupdate to avoid "Qualifying with unknown namespace/class"
class KDbConnectionData { Q_DECLARE_TR_FUNCTIONS(KDbConnectionData) };
#endif

KDbConnectionData::~KDbConnectionData()
{
}

QString KDbConnectionData::toUserVisibleString(UserVisibleStringOptions options) const
{
    KDbDriverManager mananager;
    const KDbDriverMetaData *metaData = mananager.driverMetaData(d->driverId);
    if (!metaData) {
        return d->databaseName;
    } else if (metaData->isValid() && metaData->isFileBased()) {
        if (d->databaseName.isEmpty()) {
            return tr("<file>");
        }
        return tr("file: %1").arg(d->databaseName);
    }
    return ((d->userName.isEmpty() || !(options & UserVisibleStringOption::AddUser))
                ? QString()
                : (d->userName + QLatin1Char('@')))
        + (d->hostName.isEmpty() ? QLatin1String("localhost") : d->hostName)
        + (d->port != 0 ? (QLatin1Char(':') + QString::number(d->port)) : QString());
}

bool KDbConnectionData::isPasswordNeeded() const
{
    KDbDriverManager mananager;
    const KDbDriverMetaData *metaData = mananager.driverMetaData(d->driverId);
    if (!metaData) {
        return false;
    }
    const bool fileBased = metaData->isValid() && metaData->isFileBased();

    return !d->savePassword && !fileBased; //!< @todo temp.: change this if there are
                                           //!< file-based drivers requiring a password
}

KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbConnectionData& data)
{
    dbg.nospace() << "CONNDATA";
    KDbDriverManager mananager;
    const KDbDriverMetaData *metaData = mananager.driverMetaData(data.driverId());
    dbg.nospace()
        << " databaseName=" << data.databaseName()
        << " caption=" << data.caption()
        << " description=" << data.description()
        << " driverId=" << data.driverId()
        << " userName=" << data.userName()
        << " hostName=" << data.hostName()
        << " port=" << data.port()
        << " useLocalSocketFile=" << data.useLocalSocketFile()
        << " localSocketFileName=" << data.localSocketFileName()
        << " password=" << QString::fromLatin1("*").repeated(data.password().length()) /* security */
        << " savePassword=" << data.savePassword()
        << " isPasswordNeeded=" <<
           qPrintable(metaData ? QString::number(data.isPasswordNeeded())
                     : QString::fromLatin1("[don't know, no valid driverId]"))
        << " userVisibleString=" << data.toUserVisibleString();
    return dbg.nospace();
}

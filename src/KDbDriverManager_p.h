/* This file is part of the KDE project
   Copyright (C) 2003-2015 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_DRIVER_MANAGER_P_H
#define KDB_DRIVER_MANAGER_P_H

#include <QObject>

#include "KDbDriver.h"
#include "KDbDriverMetaData.h"

//! Internal class of the driver manager.
class DriverManagerInternal : public QObject, public KDbResultable
{
    Q_OBJECT
public:
    /*! Used by self() */
    DriverManagerInternal();

    ~DriverManagerInternal();

    QStringList driverIds();

    /*! Tries to load a driver with identifier @a id.
      @return db driver, or @c nullptr on error (then error result is also set) */
    KDbDriver* driver(const QString& id);

    const KDbDriverMetaData* driverMetaData(const QString &id);

    QStringList driverIdsForMimeType(const QString& mimeType);

    QStringList possibleProblems() const;

    static DriverManagerInternal *self();

protected Q_SLOTS:
    /*! Used to destroy all drivers on QApplication quit, so even if there are
     KDbDriverManager's static instances that are destroyed on program
     "static destruction", drivers are not kept after QApplication death.
    */
    void slotAppQuits();

private:
    bool lookupDrivers();

    QMap<QString, KDbDriverMetaData*> m_metadata_by_mimetype;
    QMap<QString, KDbDriverMetaData*> m_driversMetaData; //!< used to store driver metadata
    QMap<QString, KDbDriver*> m_drivers; //!< for owning drivers
    QString m_pluginsDir;
    QStringList m_possibleProblems;
    bool m_lookupDriversNeeded;
};

#endif

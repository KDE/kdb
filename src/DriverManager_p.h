/* This file is part of the KDE project
   Copyright (C) 2003-2010 Jarosław Staniek <staniek@kde.org>

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

#ifndef PREDICATE_DRIVER_MNGR_P_H
#define PREDICATE_DRIVER_MNGR_P_H

#include <QObject>
#include <QWeakPointer>
#include <Predicate/Driver.h>

namespace Predicate
{

//! Internal class of the driver manager.
class DriverManagerInternal : public QObject, public Resultable
{
    Q_OBJECT
public:
    /*! Used by self() */
    DriverManagerInternal();

    ~DriverManagerInternal();

    /*! Tries to load db driver \a name.
      \return db driver, or 0 if error (then error message is also set) */
    Driver* driver(const QString& name);

    DriverInfo driverInfo(const QString &name);

    static DriverManagerInternal *self();

    /*! increments the refcount for the manager */
//void incRefCount();

    /*! decrements the refcount for the manager
      if the refcount reaches a value less than 1 the manager is freed */
//void decRefCount();

//2.0    /*! Called from Driver dtor (because sometimes KLibrary (used by Driver)
//2.0     is destroyed before DriverManagerInternal) */
//2.0    void aboutDelete(Driver* drv);

protected Q_SLOTS:
    /*! Used to destroy all drivers on QApplication quit, so even if there are
     DriverManager's static instances that are destroyed on program
     "static destruction", drivers are not kept after QApplication death.
    */
    void slotAppQuits();

private:
    bool lookupDrivers();
    void lookupDriversForDirectory(const QString& pluginsDir);

    QMap<QString, DriverInfo> m_infos_by_mimetype;
    QMap<QString, DriverInfo> m_driversInfo; //!< used to store drivers information
    QMap<QString, Driver* > m_drivers; //!< for owning drivers
    QString m_pluginsDir;

//pred    QString m_serverErrMsg;
//pred    int m_serverResultNum;
//pred    QString m_serverResultName;
//    //! result names for KParts::ComponentFactory::ComponentLoadingError
//    QHash<int, QString> m_componentLoadingErrors;

    QStringList possibleProblems;

//pred    ulong refCount() const { return m_refCount; }

//ulong m_refCount;
    bool lookupDriversNeeded;

    friend class DriverManager;
};
}

#endif

/* This file is part of the KDE project
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
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

#ifndef PREDICATE_DRIVER_MNGR_H
#define PREDICATE_DRIVER_MNGR_H

//#include <klibloader.h>
//#include <kservice.h>

#include <Predicate/Driver.h>

namespace Predicate
{

class DriverManagerInternal;
class Connection;

//! Database driver manager, provided for finding and loading drivers.
class PREDICATE_EXPORT DriverManager
{
public:
    DriverManager();
    virtual ~DriverManager();

    //! @return result of the recent operation.
    Result result() const;

    //! @return Resultable object for the recent operation.
    //! It adds serverResultName() in addition to the result().
    const Resultable& resultable() const;

    /*! Tries to load db driver with named name @a name.
      The name is case insensitive.
      @return driver object, or 0 if error (then result is also set, see result()). */
    Driver* driver(const QString& name);

    /*! returns list of available drivers names.
      That drivers can be loaded by first use of driver() method. */
    QStringList driverNames();

    /*! @return information about driver's named with @a name.
      The name is case insensitive.
      You can check if driver information is not found calling
      Info::name.isEmpty() (then error message is also set). */
    DriverInfo driverInfo(const QString &name);

    /*! @return list of driver names for @a mimeType mime type
     or empty list if no driver has been found.
     Works only with drivers for file-based databases like SQLite.
     The lookup is case insensitive. */
    QStringList driversForMimeType(const QString& mimeType);

    /*! HTML information about possible problems encountered.
     It's displayed in 'details' section, if an error encountered.
     Currently it contains a list of incompatible db drivers.
     Used in KexiStartupHandler::detectDriverForFile(). */
//! @todo make just QStringList
    QString possibleProblemsInfoMsg() const;

protected:
//???    virtual void drv_clearServerResult();
};

} //namespace Predicate

#endif

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

#ifndef KDB_VERSIONINFO_H
#define KDB_VERSIONINFO_H

#include "kdb_export.h"
#include <QString>
#ifdef __GNUC__
# include <sys/types.h> // We use minor/major identifiers, force this include
                        // to have "#define minor gnu_dev_minor" from sys/sysmacros.h
                        // and immediately undefine that; same for major.
# undef minor
# undef major
#endif

/*! Provides KDb-specific information about version of the database.

 The version is stored as internal database properties. */
shared class export=KDB_EXPORT operator== KDbVersionInfo
{
public:
    /*!
    @getter
    @return major version number, e.g. 1 for 1.8.9
    @setter
    Sets the major version number.
    */
    data_member int major default=0;

    /*!
    @getter
    @return minor version number, e.g. 8 for 1.8.9
    @setter
    Sets the minor version number.
    */
    data_member int minor default=0;

    /*!
    @getter
    @return release version number, e.g. 9 for 1.8.9
    @setter
    Sets the release version number.
    */
    data_member int release default=0;

    inline KDbVersionInfo(int majorVersion, int minorVersion, int releaseVersion)
     : d(new Data)
    {
        d->major = majorVersion;
        d->minor = minorVersion;
        d->release = releaseVersion;
    }

    //! @return true if @a major and @a minor exatcly matches major and minor version of this info, respectively.
    inline bool matches(int major, int minor) const { return major == d->major && minor == d->minor; }

    //! @return true if this version info is null, i.e. all the version numbers are zero.
    bool isNull() const;
};

/*! Provides information about version of given database backend.
*/
shared class export=KDB_EXPORT operator== KDbServerVersionInfo
{
public:
    /*!
    @getter
    @return major version number, e.g. 1 for 1.8.9
    @setter
    Sets the major version number.
    */
    data_member int major default=0;

    /*!
    @getter
    @return minor version number, e.g. 8 for 1.8.9
    @setter
    Sets the minor version number.
    */
    data_member int minor default=0;

    /*!
    @getter
    @return release version number, e.g. 9 for 1.8.9
    @setter
    Sets the release version number.
    */
    data_member int release default=0;

    /*!
    @getter
    @return version string, as returned by the server.
    @setter
    Sets the version string, as returned by the server.
    */
    data_member QString string;

    //! Clears the information - integers will be set to 0 and string to null
    void clear();

    //! @return true if this version info is null, i.e. all the version numbers are zero.
    bool isNull() const;
};

#endif

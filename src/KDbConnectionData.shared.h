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

#ifndef KDB_CONNECTION_DATA_H
#define KDB_CONNECTION_DATA_H

#include "kdb_export.h"

#include <QCoreApplication>
#include <QDebug>
#include <QMap>
#include <QSharedData>
#include <QString>

/*! @brief Database specific connection data, e.g. host, port.

 KDbConnection data, once configured, can be later stored for reuse.
*/
class KDB_EXPORT KDbConnectionData //SDC: with_from_to_map operator==
{
    Q_DECLARE_TR_FUNCTIONS(KDbConnectionData)
public:
    /*!
    @getter
    @return database name

    Optional attribute explicitly providing database name.
    If not empty, the database driver will attempt to use the database
    (e.g. with "USE DATABASE" SQL statement).

    For file-based database engines like SQLite, the database name is equal to filename
    (absolute or relative) that should be open. In this case hostName and port is unused.

    Can be empty, in which case if database name is required by the connection,
    after connecting KDbConnection::useDatabase() should be called.
    @setter
    Explicitly sets database name.
    */
    QString databaseName; //SDC:

    /*!
    @getter
    @return caption of the connection
    Captions are optional for identyfying given connection by name eg. for users.
    @setter
    Sets caption of the connection
    */
    QString caption; //SDC:

    /*!
    @getter
    @return additional description for the connection
    @setter
    Sets additional description for the connection
    */
    QString description; //SDC:

    /*!
    @getter
    @return identifier of the driver.
    ID (unique, not i18n-ed) of driver that is used (or should be used) to
    create a connection. If you pass this KDbConnectionData object to
    KDbDriver::createConnection() to create connection, the @a driverId member
    will be updated with a valid KDb driver ID.
    In other situations the @a driverId member may be used to store information what
    driver should be used to perform connection, before we get an appropriate
    driver object from KDbDriverManager.
    @setter
    Sets identifier of the driver to use
    */
    QString driverId; //SDC:

    /*!
    @getter
    @return username used for creating connections
    Can be empty.
    @setter
    Sets username used for creating connections
    */
    QString userName; //SDC:

    /*!
    @getter
    @return host name used for creating remote connections
    Can be IP number. Can be empty if the connection is not remote.
    If empty, "localhost" is used.
    @setter
    Sets host name used for creating remote connections
    */
    QString hostName; //SDC:

    /*!
    @getter port used for creating remote connections
    The default is 0, what means using the database engine's default port.
    @setter
    Sets port used for creating remote connections
    */
    int port; //SDC: default=0

    /*!
    @getter
    @return true if local socket file should be used instead of TCP/IP port.

    Only meaningful for connections with localhost as server.
    True by default, so local communication can be optimized, and users can avoid problems
    with TCP/IP connections disabled by firewalls.

    If true, @a hostName and @a port will be ignored and @a localSocketFileName() will be used.
    On MS Windows this option is usually ignored and TCP/IP connection to the localhost is performed.
    @setter
    Sets flag for usage of local socket file. @see useLocalSocketFile()
    */
    bool useLocalSocketFile; //SDC: default=true

    /*!
    @getter
    @return name of local (named) socket file.

    For local connections only. If empty, it's driver will try to locate existing local socket
    file. Empty by default.
    @setter
    Sets name  of local (named) socket file
    */
    QString localSocketFileName; //SDC:

    /*!
    @getter
    @return password used for creating connections

    Can be empty string (QString("")) or null (QString()). If it is empty, empty password is passed
    to the connection. If it is null, no password is saved and thereform no password is passed to the connection.
    In the latter case, applications that KDb should ask for the password before performing connection
    if password is required by the connection.
    @setter
    Sets password used for creating connections
    */
    QString password; //SDC:

    /*!
    @getter
    @return true if the connection's password should be stored in a configuration file for later use.
    False by default, in most cases can be set to true when non-null password is known.
    For instance, this flag can be then shown for a user as a checkbox in the graphical interface.
    @setter
    Sets password-saving flag used to decide if the connection's password should be stored
    in a configuration file for later use
    */
    bool savePassword; //SDC: default=false

    //! Used in toUserVisibleString()
    enum class UserVisibleStringOption {
        None = 0,
        AddUser = 1
    };
    Q_DECLARE_FLAGS(UserVisibleStringOptions, UserVisibleStringOption)

    /*! @return A user-visible string for the connection data
    driverId() is used to know if driver handles server connections. If it's not possible
    to check the driver, defaults to "file" connection.

    Example strings:
     - "myhost.org:12345" if a host and port is specified;
     - "localhost:12345" if only a port and server-based driver is specified;
     - "user@myhost.org:12345" if user is specified too
     - "<file>" if a file-based driver is specified but no filename in the databaseName attribute
     - "file: pathto/mydb.kexi" if a file-based driver is specified and filename
       is specified in the databaseName attribute
     - "<file>" if the driver is unknown or not specified and no databaseName is specified

     User name is added if (@a options & UserVisibleStringOption::AddUser) is true (the default).
    */
    QString toUserVisibleString(UserVisibleStringOptions options = UserVisibleStringOption::AddUser) const;

    /*! @return true if password is needed for performing connection.
    The password has to be provided by the user.
    @note This method needs information about driver ID; it returns false if driverId()
          does not return a valid ID.
    */
    bool isPasswordNeeded() const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KDbConnectionData::UserVisibleStringOptions)

//! Sends information about connection data @a data to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbConnectionData& data);

#endif

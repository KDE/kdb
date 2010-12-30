/* This file is part of the KDE project
   Copyright (C) 2003-2008 Jarosław Staniek <staniek@kde.org>

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

#ifndef PREDICATE_GLOBAL_H
#define PREDICATE_GLOBAL_H

#include <Predicate/predicate_export.h>
#include <QtCore/QString>

//global public definitions

/*! Predicate implementation version.
 It is altered after every API change:
 - major number is increased after Predicate storage format change,
 - minor is increased after adding binary-incompatible change.
 In external code: do not use this to get library version information.
 See Predicate::version() if you need the Predicate version used at runtime.
*/
#cmakedefine PREDICATE_VERSION_MAJOR @PREDICATE_VERSION_MAJOR@
#cmakedefine PREDICATE_VERSION_MINOR @PREDICATE_VERSION_MINOR@
#cmakedefine PREDICATE_VERSION_RELEASE @PREDICATE_VERSION_RELEASE@

#define PREDICATE_VERSION_MAJOR_STRING "@PREDICATE_VERSION_MAJOR@"
#define PREDICATE_VERSION_MINOR_STRING "@PREDICATE_VERSION_MINOR@"
#define PREDICATE_VERSION_RELEASE_STRING "@PREDICATE_VERSION_MINOR@"
#define PREDICATE_VERSION_STRING "@PREDICATE_VERSION_STRING@"

/**
 * @brief Make a number from the major, minor and release number of a Predicate version
 *
 * This function can be used for preprocessing when PREDICATE_IS_VERSION is not
 * appropriate.
 */
#define PREDICATE_MAKE_VERSION( a,b,c ) (((a) << 16) | ((b) << 8) | (c))

/**
 * @brief Version of Predicate as number, at compile time
 *
 * This macro contains the Predicate version in number form. As it is a macro,
 * it contains the version at compile time. See versionString() if you need
 * the Predicate version used at runtime.
 */
#define PREDICATE_VERSION \
  PREDICATE_MAKE_VERSION(PREDICATE_VERSION_MAJOR, PREDICATE_VERSION_MINOR, PREDICATE_VERSION_RELEASE)

/**
 * @brief Check if the Predicate version matches a certain version or is higher
 *
 * This macro is typically used to compile conditionally a part of code:
 * @code
 * #if PREDICATE_IS_VERSION(2,0,90)
 * // Code for Predicate 2.1
 * #else
 * // Code for Predicate 2.0
 * #endif
 * @endcode
 *
 * @warning Especially during development phases of Predicate, be careful
 * when choosing the version number that you are checking against.
 * Otherwise you might risk to break the next Predicate release.
 * Therefore be careful that development version have a
 * version number lower than the released version, so do not check 
 * e.g. for Predicate 2.1 with PREDICATE_IS_VERSION(2,1,0)
 * but with the actual version number at a time a needed feature was introduced.
 */
#define PREDICATE_IS_VERSION(a,b,c) ( PREDICATE_VERSION >= PREDICATE_MAKE_VERSION(a,b,c) )


/*! \namespace Predicate
\brief High-level database connectivity library with database backend drivers

\section Framework
DriverManager

Database access
 - Connection
 - ConnectionData

Database structure
 - Schema
  - tableschema
  - queryschema
  - indexschema

Stored in the database.


Data representation
 - Record
 - Field


\section Drivers

Drivers are loaded using DriverManager::driver(const QString& name).  The names
of drivers are given in their drivers .desktop file in the DriverName field.

Predicate supports two kinds of databases: file-based and network-based databases.
The type of a driver is available from several places. The DriverType
field in the driver's .desktop file, is read by the DriverManager and
available by calling DriverManager::driverInfo(const QString &name) and using
the DriverInfo#fileBased member from the result. Given a reference to a
Driver, its type can also be found directly using Driver::isFileBased().

Each database backend driver consists of three main classes: a driver,
a connection and a cursor class, e.g SQLiteDriver, SQLiteConnection,
SQLiteCursor.

The driver classes subclass the Driver class.  They set Driver#m_typeNames,
which maps Predicate Field::Type on to the types supported by the database.  They also
provide functions for escaping strings and checking table names.  These may be
used, for example, on a database backend that uses the database name as a
filename.  In this case, it should be ensured that all the characters in the
database name are valid characters in a filename.

The connection classes subclass the Connection class, and include most of the
calls to the native database API.

The cursor classes subclass Cursor, and implement cursor functionality specific
to the database backend.

*/
namespace Predicate
{

#if !defined(_DEBUG) && !defined(DEBUG)
# define PREDICATE_DEBUG if (true); else
#else
# define PREDICATE_DEBUG
#endif

//! Debug command for the core Predicate code
# define PreDbg PREDICATE_DEBUG qDebug() << "Predicate:"
//! Debug command for Predicate driver's code
# define PreDrvDbg PREDICATE_DEBUG qDebug() << "Predicate-drv(" PREDICATE_DRIVER_NAME "):"
//! Warning command for the core Predicate code
# define PreWarn PREDICATE_DEBUG qWarning() << "Predicate:"
//! Warning command for Predicate driver's code
# define PreDrvWarn PREDICATE_DEBUG qWarning() << "Predicate-drv(" PREDICATE_DRIVER_NAME "):"
//! Fatal command for the core Predicate code
# define PreFatal PREDICATE_DEBUG qCritical() << "Predicate:"
//! Fatal command for Predicate driver's code
# define PreDrvFatal PREDICATE_DEBUG qCritical() << "Predicate-drv(" PREDICATE_DRIVER_NAME "):"

/*! Object types set like table or query. */
enum ObjectType {
    UnknownObjectType = -1, //!< helper
    AnyObjectType = 0,      //!< helper
    TableObjectType = 1,
    QueryObjectType = 2,
    LastObjectType = 2, //ALWAYS UPDATE THIS

    PredicateSystemTableObjectType = 128,//!< helper, not used in storage
    //!< (allows to select kexidb system tables
    //!< may be or'd with TableObjectType)
    IndexObjectType = 256 //!< special
};

}

#ifndef futureI18n
# define futureI18n QString
# define futureI18n2(a,b) QString(b)
#endif

#ifndef FUTURE_I18N_NOOP
# define FUTURE_I18N_NOOP(x) (x)
#endif

#endif

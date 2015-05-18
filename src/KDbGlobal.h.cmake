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

#ifndef KDBGLOBAL_H
#define KDBGLOBAL_H

#include "kdb_export.h"
#include "kdb_version.h"

#include <QString>

/*! @file KDbGlobal.h
    Global public definitions
*/

//! Indicates the git sha1 commit which was used for compilation of KDb.
#cmakedefine KDB_GIT_SHA1_STRING "@KDB_GIT_SHA1_STRING@"

//! The subdirectory relative to the install prefix for executables.
#define BIN_INSTALL_DIR "${BIN_INSTALL_DIR}"

/**
 * @brief Make a number from the major, minor and release number of a KDb version
 *
 * This function can be used for preprocessing when KDB_IS_VERSION is not
 * appropriate.
 */
#define KDB_MAKE_VERSION( a,b,c ) (((a) << 16) | ((b) << 8) | (c))

/**
 * @brief Check if the KDb version matches a certain version or is higher
 *
 * This macro is typically used to compile conditionally a part of code:
 * @code
 * #if KDB_IS_VERSION(3, 1, 0)
 * // Code for KDb 3.1
 * #else
 * // Code for KDb older than 3.1
 * #endif
 * @endcode
 *
 * @warning Especially during development phases of KDb, be careful
 * when choosing the version number that you are checking against.
 * Otherwise you might risk to break the next KDb release.
 * Therefore be careful that development version have a
 * version number lower than the released version, so do not check 
 * e.g. for KDb 3.1 with KDB_IS_VERSION(3, 1, 0)
 * but with the actual version number at a time a needed feature was introduced,
 * e.g. KDB_IS_VERSION(3, 0, 90) for beta 3.1
 */
#define KDB_IS_VERSION(a,b,c) ( KDB_VERSION >= KDB_MAKE_VERSION(a,b,c) )


/*! @namespace KDb
@brief High-level database connectivity library with database backend drivers

@section Framework
KDbDriverManager

Database access
 - KDbConnection
 - KDbConnectionData

Database structure
 - Schema
  - KDbTableSchema
  - KDbQuerySchema
  - KDbIndexSchema

Stored in the database.

Data representation
 - KDbRecord
 - KDbField

@section Drivers

Drivers are loaded using KDbDriverManager::driver(const QString& name).  The names
of drivers are given in their drivers .desktop file in the DriverName field.

KDb supports two kinds of databases: file-based and network-based databases.
The type of a driver is available from several places. The DriverType
field in the driver's .desktop file, is read by the KDbDriverManager and
available by calling KDbDriverManager::KDbDriverInfo(const QString &name) and using
the KDbDriverInfo#fileBased member from the result. Given a reference to a
driver, its type can also be found directly using KDbDriver::isFileBased().

Each database backend driver consists of three main classes: a driver,
a connection and a cursor class, e.g SQLiteDriver, SQLiteConnection,
SQLiteCursor.

The driver classes subclass the KDbDriver class.  They set KDbDriver#m_typeNames,
which maps KDb KDbField::Type on to the types supported by the database.  They also
provide functions for escaping strings and checking table names.  These may be
used, for example, on a database backend that uses the database name as a
filename.  In this case, it should be ensured that all the characters in the
database name are valid characters in a filename.

The connection classes subclass the KDbConnection class, and include most of the
calls to the native database API.

The cursor classes subclass KDbCursor, and implement cursor functionality specific
to the database backend.

*/
namespace KDb
{

#if !defined(_DEBUG) && !defined(DEBUG)
# define KDB_DEBUG if (true); else
#else
# define KDB_DEBUG
#endif

//! Debug command for the core KDb code
# define KDbDbg KDB_DEBUG qDebug() << "KDb:"
//! Debug command for KDb driver's code
# define KDbDrvDbg KDB_DEBUG qDebug() << "KDb-drv(" KDB_DRIVER_NAME "):"
//! Warning command for the core KDb code
# define KDbWarn KDB_DEBUG qWarning() << "KDb:"
//! Warning command for KDb driver's code
# define KDbDrvWarn KDB_DEBUG qWarning() << "KDb-drv(" KDB_DRIVER_NAME "):"
//! Fatal command for the core KDb code
# define KDbFatal KDB_DEBUG qCritical() << "KDb:"
//! Fatal command for KDb driver's code
# define KDbDrvFatal KDB_DEBUG qCritical() << "KDb-drv(" KDB_DRIVER_NAME "):"

/*! Object types set like table or query. */
enum ObjectType {
    UnknownObjectType = -1, //!< helper
    AnyObjectType = 0,      //!< helper
    TableObjectType = 1,
    QueryObjectType = 2,
    LastObjectType = 2, //ALWAYS UPDATE THIS

    KDbSystemTableObjectType = 128, //!< helper, not used in storage
                                    //!< (allows to select kexidb system tables
                                    //!< may be or'd with TableObjectType)
    IndexObjectType = 256 //!< special
};

//! Escaping type for identifiers.
enum IdentifierEscapingType {
    DriverEscaping, //!< Identifiers are escaped by driver
    KDbEscaping     //!< Identifiers are escaped using KDb's generic convention
};

}

//! Macros for marking future QObject::tr() translations.
#ifndef futureTr
# define futureTr QString
# define futureTr2(a,b) QString(b)
#endif

//! Macros for marking future QT_TR_NOOP translations.
#ifndef FUTURE_TR_NOOP
# define FUTURE_TR_NOOP(x) (x)
#endif

//! Macro to use in drivers to avoid redundant translations.
#define KDbTr QObject::tr

//! Debugging options for expressions
#cmakedefine KDB_EXPRESSION_DEBUG
#ifdef KDB_EXPRESSION_DEBUG
# define ExpressionDebug qDebug()
#else
# define ExpressionDebug if (1) {} else qDebug()
#endif

//! GUI for debugging
#cmakedefine KDB_DEBUG_GUI

//! Include unfinished features
#cmakedefine KDB_UNFINISHED

#ifndef WARNING
#ifdef _MSC_VER
/* WARNING preprocessor directive
 Reserved: preprocessor needs two indirections to replace __LINE__ with actual
 string
*/
#define _MSG0(msg)     #msg
/* Preprocessor needs two indirections to replace __LINE__ or __FILE__
 with actual string. */
#define _MSG1(msg)    _MSG0(msg)

/*! Creates message prolog with the name of the source file and the line
   number where a preprocessor message has been inserted.

  Example:
     #pragma KMESSAGE(Your message)
  Output:
     C:\MyCode.cpp(111) : Your message
*/
# define _MSGLINENO __FILE__ "(" _MSG1(__LINE__) ") : warning: "
# define WARNING(msg) message(_MSGLINENO #msg)
#endif /*_MSC_VER*/
#endif /*WARNING*/

#endif

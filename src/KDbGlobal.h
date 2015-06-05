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
@brief A database connectivity and creation framework

KDb is consisted of a general-purpose C++ Qt library and set of plugins delivering support
for various database vendors.

@section Framework
KDbDriverManager
KDbDriverMetaData
KDbDriver

Database access
 - KDbConnection
 - KDbConnectionData
 - KDbTransaction
 - KDbRecordEditBuffer
 - KDbPreparedStatement

Database structure
 - Schema
  - KDbTableSchema
  - KDbQuerySchema
  - KDbQuerySchemaParameter
  - KDbQueryColumnInfo
  - KDbTableOrQuerySchema
  - KDbIndexSchema
  - KDbFieldList
  - KDbLookupFieldSchema
  - KDbRelationship
  - KDbParser
  - KDbExpression

Data representation
 - KDbField
 - KDbRecordData
 - KDbTableViewData
 - KDbTableViewColumn

Tools
 - KDbObject
 - KDbEscapedString
 - KDbMessageHandler
 - KDbProperties
 - KDbAdmin
 - KDbAlter
 - KDbValidator
 - KDbUtils

@section Drivers

Drivers are loaded as plugins on demand by KDbDriverManager. The IDs, descriptions
and other details about drivers are given in their metadata by KDbDriverManager::driverMetaData().
The metadata is accessible without actually loading any driver.

KDb supports two families of databases, file and network-based while providing a single
uniform API.

Each database driver implements of three main classes KDbDriver, KDbConnection, KDbCursor.
The driver classes handle database-related specifics such as data types, naming and hide
them behind a general purpose API. The connection classes act as a proxy between the KDb API
and the native database. The cursor classes implement cursor functionality specific to
the native database at record level.
*/
namespace KDb
{

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

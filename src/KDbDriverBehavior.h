/* This file is part of the KDE project
   Copyright (C) 2003-2017 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_DRIVERBEHAVIOR_H
#define KDB_DRIVERBEHAVIOR_H

#include "KDbDriver.h"
#include "KDbUtils.h"

/**
 * @brief Detailed definition of driver's default behavior
 *
 * This class is mostly interesting for KDb driver developers.
 * Defaults can be changed by KDbDriver subclass in constructors.
 */
class KDB_EXPORT KDbDriverBehavior
{
public:
    explicit KDbDriverBehavior(KDbDriver *driver);

    ~KDbDriverBehavior();

    /*! Features (like transactions, etc.) supported by this driver
     (sum of selected  Features enum items).
     This member should be filled in driver implementation's constructor
     (by default m_features==NoFeatures). */
    int features;

    //! real type names for this engine
    QVector<QString> typeNames;

    /*! Driver properties (indexed by name), useful for presenting properties to the user.
     Contains i18n-ed captions.
     In driver implementations available properties can be initialized, for example:
     @code
        beh->properties.insert("maximum_performance", 1000, SomeClass::tr("Maximum performance"));
     @endcode
     You do not need to set captions of properties predefined by the KDbDriver superclass,
     they will be reused. Predefined properties are set in KDbDriver. */
    KDbUtils::PropertySet properties;

    //! "UNSIGNED" by default
    QString UNSIGNED_TYPE_KEYWORD;

    //! "AUTO_INCREMENT" by default, used as add-in word to field definition
    //! May be also used as full definition if SPECIAL_AUTO_INCREMENT_DEF is true.
    QString AUTO_INCREMENT_FIELD_OPTION;

    //! "AUTO_INCREMENT PRIMARY KEY" by default, used as add-in word to field definition
    //! May be also used as full definition if SPECIAL_AUTO_INCREMENT_DEF is true.
    QString AUTO_INCREMENT_PK_FIELD_OPTION;

    //! "" by default, used as type string for autoinc. field definition
    //! pgsql defines it as "SERIAL", sqlite defines it as "INTEGER"
    QString AUTO_INCREMENT_TYPE;

    /*! True if autoincrement field has special definition
     e.g. like "INTEGER PRIMARY KEY" for SQLite.
     Special definition string should be stored in AUTO_INCREMENT_FIELD_OPTION.
     False by default. */
    bool SPECIAL_AUTO_INCREMENT_DEF;

    /*! True if autoincrement requires field to be declared as primary key.
     This is true for SQLite. False by default. */
    bool AUTO_INCREMENT_REQUIRES_PK;

    /*! Name of a field (or built-in function) with autoincremented unique value,
     typically returned by KDbSqlResult::lastInsertRecordId().

     Examples:
     - PostgreSQL and SQLite engines use 'OID' field
     - MySQL uses LAST_INSERT_ID() built-in function
    */
    QString ROW_ID_FIELD_NAME;

    /*! True if the value (fetched from field or function,
     defined by ROW_ID_FIELD_NAME member) is EXACTLY the value of autoincremented field,
     not an implicit (internal) row number. Default value is false.

     Examples:
     - PostgreSQL and SQLite engines have this flag set to false ('OID' field has
        it's own implicit value)
     - MySQL engine has this flag set to true (LAST_INSERT_ID() returns real value
     of last autoincremented field).

     Notes:
     If it's false, we have a convenient way for identifying row even when there's
     no primary key defined. So, as '_ROWID' column in MySQL is really
     just a synonym for the primary key, this engine needs to have primary keys always
     defined if we want to use interactive editing features like row updating and deleting.
    */
    bool ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE;

    /*! Name of any (e.g. first found) database for this connection that
     typically always exists. This can be not set if we want to do some magic checking
     what database name is availabe by reimplementing
     KDbConnection::anyAvailableDatabaseName().
     Example: for PostgreSQL this is "template1".

     @see KDbConnection::SetAvailableDatabaseName()
    */
    QString ALWAYS_AVAILABLE_DATABASE_NAME;

    /*! Opening quotation mark used for escaping identifier (see KDbDriver::escapeIdentifier()).
     Default value is '"'. Change it for your driver.
    */
    char OPENING_QUOTATION_MARK_BEGIN_FOR_IDENTIFIER;

    /*! Opening quotation mark used for escaping identifier (see KDbDriver::escapeIdentifier()).
     Default value is '"'. Change it for your driver.
    */
    char CLOSING_QUOTATION_MARK_BEGIN_FOR_IDENTIFIER;

    /*! True if using database is required to perform real connection.
     This is true for may engines, e.g. for PostgreSQL, where connection
     strings should contain a database name.
     If the flag is false, then developer has to call KDbConnection::useDatabase()
     after creating the KDbConnection object.
     This flag can be also used for file-based db drivers, e.g. SQLite sets it to true.
     MySQL sets it to false.
     By default this flag is true.
    */
    bool USING_DATABASE_REQUIRED_TO_CONNECT;

    /*! True if connection should be established (KDbConnection::connect()) in order
     to check database existence (KDbConnection::databaseExists()).
     This is true for most engines, but not for SQLite (and probably most
     file-based databases) where existence of database is checked at filesystem level.
     By default this flag is true.
    */
    bool CONNECTION_REQUIRED_TO_CHECK_DB_EXISTENCE;

    /*! True if connection should be established (KDbConnection::connect()) in order
     to create new database (KDbConnection::createDatabase()).
     This is true for most engines, but not for SQLite (and probably most
     file-based databases) where opening physical connection for nonexisting
     file creates new file.
     By default this flag is true.
    */
    bool CONNECTION_REQUIRED_TO_CREATE_DB;

    /*! True if connection should be established (KDbConnection::connect()) in order
     to drop database (KDbConnection::dropDatabase()).
     This is true for most engines, but not for SQLite (and probably most
     file-based databases) where dropping database is performed at filesystem level.
     By default this flag is true.
    */
    bool CONNECTION_REQUIRED_TO_DROP_DB;

    /*! Because some engines need to have opened any database before
     executing administrative sql statements like "create database" or "drop database",
     using appropriate, existing temporary database for this connection.
     This is the case for PostgreSQL.
     For file-based db drivers this flag is ignored.
     By default this flag is false.

     Note: This method has nothing to do with creating or using temporary databases
     in such meaning that these database are not persistent.

     @see KDbConnection::useTemporaryDatabaseIfNeeded()
    */
    bool USE_TEMPORARY_DATABASE_FOR_CONNECTION_IF_NEEDED;

    /*! Set to @c true in a subclass if after successful drv_createDatabase() the database
     is in opened state (as after useDatabase()).
     @c false for most engines. */
    bool IS_DB_OPEN_AFTER_CREATE;

    /*! True if before we know whether the fetched result of executed query
     is empty or not, we need to fetch first record. Particularly, it's true for SQLite.
     The flag is used in KDbCursor::open(). By default this flag is false. */
    bool _1ST_ROW_READ_AHEAD_REQUIRED_TO_KNOW_IF_THE_RESULT_IS_EMPTY;

    /*! True if "SELECT 1 from (subquery)" is supported. False by default.
     Used in KDbConnection::resultExists() for optimization. It's set to true for SQLite driver. */
    bool SELECT_1_SUBQUERY_SUPPORTED;

    /*! Literal for boolean true. "1" by default
        which is typically expected by backends even while the standard says "TRUE":
        http://troels.arvin.dk/db/rdbms/#data_types-boolean
    */
    QString BOOLEAN_TRUE_LITERAL;

    /*! Literal for boolean false. "0" by default
        which is typically expected by backends even while the standard says "TRUE":
        http://troels.arvin.dk/db/rdbms/#data_types-boolean
    */
    QString BOOLEAN_FALSE_LITERAL;

    /*! Specifies maximum length for Text type. If 0, there is are limits and length argument is not needed,
     e.g. VARCHAR works for the driver this is the case for SQLite and PostgreSQL.
     If greater than 0, this value should be used to express maximum value, e.g. VARCHAR(...).
     This is the case for MySQL.
     The default is 0. */
    int TEXT_TYPE_MAX_LENGTH;

    /*! "LIKE" by default, used to construct native expressions "x LIKE y" and "x NOT LIKE y".
     LIKE is case-insensitive for MySQL, SQLite, and often on Sybase/MSSQL
     while for PostgreSQL it's case-sensitive. So for PostgreSQL LIKE_OPERATOR == "ILIKE". */
    QString LIKE_OPERATOR;

    /*! "RANDOM()" function name, used in Driver::randomFunctionToString() to construct native
     expressions. */
    QString RANDOM_FUNCTION;

private:
    void initInternalProperties();
    friend class KDbDriver;

    Q_DISABLE_COPY(KDbDriverBehavior)
    class Private;
    Private * const d;
};

#endif

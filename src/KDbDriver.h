/* This file is part of the KDE project
   Copyright (C) 2003-2018 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_DRIVER_H
#define KDB_DRIVER_H

#include <QDateTime>

#include "KDb.h"
#include "KDbResult.h"
#include "KDbEscapedString.h"
#include "KDbExpressionData.h"

class KDbAdminTools;
class KDbConnection;
class KDbConnectionData;
class KDbConnectionOptions;
class KDbDriverBehavior;
class KDbDriverMetaData;
class KDbBinaryExpression;
class KDbNArgExpression;
class KDbQuerySchemaParameterValueListIterator;
class KDbDriverPrivate;

#define KDB_DRIVER_PLUGIN_FACTORY(class_name, name) \
    K_PLUGIN_FACTORY_WITH_JSON(class_name ## Factory, name, registerPlugin<class_name>();)

//! Database driver's abstraction.
/*! This class is a prototype of the database driver.
 KDbDriver allows new connections to be created, and groups as their parent.
 Before destruction, all owned connections are destructed.
*/
class KDB_EXPORT KDbDriver : public QObject, public KDbResultable
{
    Q_OBJECT

public:
    /*! Features supported by driver (sum of few Features enum items). */
    enum Features {
        NoFeatures = 0,
        //! single trasactions are only supported
        SingleTransactions = 1,
        //! multiple concurrent trasactions are supported
        //! (this implies !SingleTransactions)
        MultipleTransactions = 2,
//(js) NOT YET IN USE:
        /*! nested trasactions are supported
         (this should imply !SingleTransactions and MultipleTransactions) */
        NestedTransactions = 4,
        /*! forward moving is supported for cursors
         (if not available, no cursors available at all) */
        CursorForward = 8,
        /*! backward moving is supported for cursors (this implies CursorForward) */
        CursorBackward = (CursorForward + 16),
        /*! compacting database supported (aka VACUUM) */
        CompactingDatabaseSupported = 32,
        //-- temporary options: can be removed later, use at your own risk --
        /*! If set, actions related to transactions will be silently bypassed
         with success. Set this if your driver does not support transactions at all
         Currently, this is only way to get it working with KDb.
         Keep in mind that this hack do not provide data integrity!
         This flag is currently used for MySQL driver. */
        IgnoreTransactions = 1024
    };

    /*! Creates connection using @a connData as parameters.
     @return @c nullptr and sets error message on error.
     driverId member of @a connData will be updated with the driver's ID.
     @a options can be set for the new connection. */
    KDbConnection *createConnection(const KDbConnectionData& connData,
                                    const KDbConnectionOptions &options);

    //! @overload createConnection(const KDbConnectionData&, const KDbConnectionOptions&)
    KDbConnection *createConnection(const KDbConnectionData& connData);

    /*! @return Set of created connections. */
    const QSet<KDbConnection*> connections() const;

    /*! Info about the driver. */
    const KDbDriverMetaData* metaData() const;

    /*! @return true if @a n is a database type-specific system object's name,
     e.g. name of a built-in system table that cannot be created by the user,
     and in most cases a name that user shouldn't even see.
     @see isSystemDatabaseName() isKDbSystemObjectName() isSystemFieldName()
    */
    virtual bool isSystemObjectName(const QString& name) const = 0;

    /*! @return true if @a name is a related to KDb's 'system' object's
     name, i.e. when @a name starts with "kexi__" prefix.
     @see isSystemDatabaseName() isSystemObjectName() isSystemFieldName()
    */
    static bool isKDbSystemObjectName(const QString& name);

    /*! @return true if @a name is a database type-specific system database's name,
     e.g. name of a built-in system database that cannot be created by a user,
     and in most cases user a name that user shouldn't even see.
     @see isKDbSystemObjectName() isSystemObjectName() isSystemFieldName()
    */
    virtual bool isSystemDatabaseName(const QString& name) const = 0;

    /*! @return true if @a name is a system field's name, build-in system
     field that cannot be used or created by a user,
     and in most cases user even shouldn't see this. The list is specific for
     a given driver implementation.
     @see isSystemDatabaseName() isKDbSystemObjectName() isSystemObjectName()
    */
    bool isSystemFieldName(const QString& name) const;

    /*! @return true if @a word is a driver-specific keyword.
     @see KDb::isKDbSqlKeyword(const QByteArray&) */
    bool isDriverSpecificKeyword(const QByteArray& word) const;

    /*! @return driver's features that are combination of KDbDriver::Features enum.
    @todo change int to Features */
    int features() const;

    /*! @return true if transaction are supported (single or multiple). */
    bool transactionsSupported() const;

    /*! @return admin tools object providing a number of database administration
     tools for the driver. Tools availablility varies from driver to driver.
     You can check it using features().  */
    KDbAdminTools& adminTools() const;

    /*! SQL-implementation-dependent name of given type */
    virtual QString sqlTypeName(KDbField::Type type, const KDbField &field) const;

    /*! used when we do not have KDbDriver instance yet */
    static QString defaultSqlTypeName(KDbField::Type type);

    /*! Escapes and converts value @a v (for type @a ftype)
     to string representation required by SQL commands.
     Reimplement this if you need other behavior (eg. for 'date' type handling)
     This implementation return date, datetime and time values in ISO format,
     what seems to be accepted by SQL servers.
     @see Qt::DateFormat */
    virtual KDbEscapedString valueToSql(KDbField::Type ftype, const QVariant& v) const;

    //! Like above but with the fildtype as string.
    inline KDbEscapedString valueToSql(const QString& ftype, const QVariant& v) const {
        return valueToSql(KDbField::typeForString(ftype), v);
    }

    //! Like above method, for @a field.
    inline KDbEscapedString valueToSql(const KDbField *field, const QVariant& v) const {
        return valueToSql((field ? field->type() : KDbField::InvalidType), v);
    }

    /**
     * Converts date value to string
     *
     * Default implementation uses KDb::dateToSql().
     *
     * Not compatible with all drivers - reimplement.
     *
     * @since 3.2.0
     */
    virtual KDbEscapedString dateToSql(const QVariant &v) const;

    /**
     * Converts time value to string
     *
     * Default implementation uses KDb::timeToIsoString().
     *
     * Not compatible with all drivers - reimplement.
     *
     * @since 3.2.0
     */
    virtual KDbEscapedString timeToSql(const QVariant &v) const;

    /**
     * Converts date/time value to string
     *
     * Default implementation uses KDb::dateTimeToIsoString().
     *
     * Not compatible with all drivers - reimplement.
     *
     * @since 3.2.0
     */
    virtual KDbEscapedString dateTimeToSql(const QVariant &v) const;

    /**
     * Converts date/time value to string
     *
     * Default implementation uses dateTimeToSql(QVariant).
     * Deprecated, use dateTimeToSql(QVariant).
     *
     * Not compatible with all drivers - reimplement.
     */
    KDB_DEPRECATED virtual KDbEscapedString dateTimeToSql(const QDateTime& v) const;

    /*! Driver-specific SQL string escaping.
     Implement escaping for any character like " or ' as your
     database engine requires. Prepend and append quotation marks.
    */
    virtual KDbEscapedString escapeString(const QString& str) const = 0;

    /*! This is overloaded version of escapeString( const QString& str )
     to be implemented in the same way.
    */
    virtual KDbEscapedString escapeString(const QByteArray& str) const = 0;

    /*! Driver-specific SQL BLOB value escaping.
     Implement escaping for any character like " or ' and \\0 as your
     database engine requires. Prepend and append quotation marks.
    */
    virtual KDbEscapedString escapeBLOB(const QByteArray& array) const = 0;

    /*! @return SQL clause to add for unicode text collation sequence
     used in ORDER BY clauses of SQL statements generated by KDb.
     Later other clauses may use this statement.
     One space character should be be prepended.
     Can be reimplemented for other drivers, e.g. the SQLite3 driver returns " COLLATE ''".
     Default implementation returns empty string. */
    virtual KDbEscapedString collationSql() const {
        return KDbEscapedString();
    }

    //! @return @a str string with applied driver-specific identifier escaping
    /*! This escaping can be used for field, table, database names, etc.
        @see KDb::escapeIdentifier */
    QString escapeIdentifier(const QString& str) const;

    //! @overload QString escapeIdentifier(const QString&) const
    QByteArray escapeIdentifier(const QByteArray& str) const;

    //! @return internal property with a name @a name for this driver.
    //! If there's no such property defined for driver, a null property is returned.
    KDbUtils::Property internalProperty(const QByteArray& name) const;

    //! @return a list of internal property names for this driver.
    QList<QByteArray> internalPropertyNames() const;

    //! @internal
    ~KDbDriver() override;

    //! Generates native (driver-specific) HEX() function call.
    //! Default implementation uses HEX(val).
    virtual KDbEscapedString hexFunctionToString(
                                        const KDbNArgExpression &args,
                                        KDbQuerySchemaParameterValueListIterator* params,
                                        KDb::ExpressionCallStack* callStack) const;

    //! Generates native (driver-specific) IFNULL() function call.
    //! Default implementation uses IFNULL().
    virtual KDbEscapedString ifnullFunctionToString(
                                           const KDbNArgExpression &args,
                                           KDbQuerySchemaParameterValueListIterator* params,
                                           KDb::ExpressionCallStack* callStack) const;

    //! Generates native (driver-specific) LENGTH() function call.
    //! Default implementation uses LENGTH().
    virtual KDbEscapedString lengthFunctionToString(
                                           const KDbNArgExpression &args,
                                           KDbQuerySchemaParameterValueListIterator* params,
                                           KDb::ExpressionCallStack* callStack) const;

    //! Generates native (driver-specific) GREATEST() and LEAST() function calls.
    //! Default implementation just uses GREATEST() and LEAST(), respectively.
    //! (this works only with MySQL >= 5.0.13).
    //! For backends workarounds are added.
    virtual KDbEscapedString greatestOrLeastFunctionToString(
                                                    const QString &name,
                                                    const KDbNArgExpression &args,
                                                    KDbQuerySchemaParameterValueListIterator* params,
                                                    KDb::ExpressionCallStack* callStack) const;

    //! Generates native (driver-specific) RANDOM() and RANDOM(X,Y) function calls.
    //! Accepted @a args can contain zero or two positive integer arguments X, Y; X < Y.
    //! In case of numeric arguments, RANDOM(X, Y) returns a random integer that is equal
    //! or greater than X and less than Y.
    //! Default implementation for RANDOM() returns F() where F is behavior()->RANDOM_FUNCTION.
    //! This works with PostgreSQL.
    //! Default implementation for RANDOM(X,Y) returns (X + FLOOR(F()*(Y-X+1))) where
    //! F is behavior()->RANDOM_FUNCTION. This works with PostgreSQL.
    //! If @a args has neither zero nor two arguments, empty string is returned.
    virtual KDbEscapedString randomFunctionToString(
                                           const KDbNArgExpression &args,
                                           KDbQuerySchemaParameterValueListIterator* params,
                                           KDb::ExpressionCallStack* callStack) const;

    //! Generates native (driver-specific) CEILING() and FLOOR() function calls.
    //! Default implementation USES CEILING() and FLOOR(), respectively.
    //! Special case is for SQLite.
    virtual KDbEscapedString ceilingOrFloorFunctionToString(
                                            const QString &name,
                                            const KDbNArgExpression &args,
                                            KDbQuerySchemaParameterValueListIterator* params,
                                            KDb::ExpressionCallStack* callStack) const;

    //! Generates native (driver-specific) UNICODE() function call.
    //! Default implementation USES UNICODE().
    //! Special case is for MYSQL and PostgreSQL.
    virtual KDbEscapedString unicodeFunctionToString(
                                            const KDbNArgExpression &args,
                                            KDbQuerySchemaParameterValueListIterator* params,
                                            KDb::ExpressionCallStack* callStack) const;

    //! Generates native (driver-specific) function call for concatenation of two strings.
    //! Default implementation USES infix "||" operator.
    //! Special case is for MYSQL (CONCAT()).
    //! @todo API supporting KDbNArgExpression would be useful so instead of a||b||c can be
    //!       expressed as CONCAT(a,b,c) instead of CONCAT(CONCAT(a,b),c).
    //!       This requires changes to the KDbSQL parser.
    KDbEscapedString concatenateFunctionToString(const KDbBinaryExpression &args,
                                                 KDbQuerySchemaParameterValueListIterator* params,
                                                 KDb::ExpressionCallStack* callStack) const;

protected:
    /**
     * @brief Returns structure that provides detailed information about driver's default behavior
     *
     * @since 3.1
     */
    KDbDriverBehavior *behavior();

    /**
     * @overload
     */
    const KDbDriverBehavior *behavior() const;

    /*! Used by KDbDriverManager.
     Note for driver developers: Reimplement this.
     In your reimplementation you should initialize:
     - beh->typeNames - to types accepted by your engine
     - beh->features - to combination of selected values from Features enum

     You may also want to change options in KDbDriverBehavior *beh member.
     See drivers/mySQL/mysqldriver.cpp for usage example.
     */
    KDbDriver(QObject *parent, const QVariantList &args);

    /*! For reimplementation: creates and returns connection object
     with additional structures specific for a given driver.
     KDbConnection object should inherit KDbConnection and have a destructor
     that descructs all allocated driver-dependent connection structures. */
    virtual KDbConnection *drv_createConnection(const KDbConnectionData& connData,
                                                const KDbConnectionOptions &options) = 0;

    /*! Driver-specific SQL string escaping.
     This method is used by escapeIdentifier().
     Implement escaping for any character like " or ' as your
     database engine requires. Do not append or prepend any quotation
     marks characters - it is automatically done by escapeIdentifier() using
     KDbDriverBehavior::OPENING_QUOTATION_MARK_BEGIN_FOR_IDENTIFIER
     and KDbDriverBehavior::CLOSING_QUOTATION_MARK_BEGIN_FOR_IDENTIFIER.
    */
    virtual QString drv_escapeIdentifier(const QString& str) const = 0;

    /*! This is overloaded version of drv_escapeIdentifier( const QString& str )
     to be implemented in the same way.
    */
    virtual QByteArray drv_escapeIdentifier(const QByteArray& str) const = 0;

    /*! @return true if @a name is a system field's name, build-in system
     field that cannot be used or created by a user,
     and in most cases user even shouldn't see this. The list is specific for
     a given driver implementation. For implementation.*/
    virtual bool drv_isSystemFieldName(const QString& name) const = 0;

    /*! Creates admin tools object providing a number of database administration
     tools for the driver. This is called once per driver.

     Note for driver developers: Reimplement this method by returning
     a KDbAdminTools-derived object. Default implementation creates
     anmd returns an empty admin tools KDbAdminTools object.
     @see adminTools() */
    virtual KDbAdminTools* drv_createAdminTools() const;

    /*! @return connection @a conn, does not delete it nor affect.
     Returns @c nullptr if @a conn is not owned by this driver.
     After this, you are owner of @a conn object, so you should
     eventually delete it. Better use KDbConnection destructor. */
    KDbConnection* removeConnection(KDbConnection *conn);

    /*! Used to initialise the dictionary of driver-specific keywords.
      Should be called by the driver's constructor.
      @a keywords should be 0-terminated array of null-terminated strings. */
    void initDriverSpecificKeywords(const char* const* keywords);

    /*! @return SQL statement @a sql modified by appending a "LIMIT 1" clause,
     (if possible and if @a add is @c true). Used for optimization for the server side.
     Can be reimplemented for other drivers. */
    virtual KDbEscapedString addLimitTo1(const KDbEscapedString& sql, bool add = true);

    /*! @return true if the database supports specifying default values for field @a field.
     @c true by default.
     For example MySQL does not support default values for BLOB, TEXT, GEOMETRY, and JSON types.
     (https://dev.mysql.com/doc/refman/5.7/en/data-type-defaults.html). */
    virtual bool supportsDefaultValue(const KDbField &field) const { Q_UNUSED(field); return true; }

    /*! Used by the driver manager to set metaData for just loaded driver. */
    void setMetaData(const KDbDriverMetaData *metaData);

    /*! @return true if this driver's implementation is valid.
     Just a few constraints are checked to ensure that driver developer didn't forget something.
     This method is called automatically on createConnection(), and proper error message
     is set properly on error.
     Drivers can reimpement this method but should call KDbDriver::isValid() first. */
    virtual bool isValid();

    friend class KDbConnection;
    friend class KDbCursor;
    friend class KDbDriverBehavior;
    friend class KDbNativeStatementBuilder;
    friend class DriverManagerInternal;
    friend class KDbDriverPrivate;

    KDbDriverPrivate * const d;
private:
    Q_DISABLE_COPY(KDbDriver)
};

namespace KDb {

//! @return string @a string with applied driver-specific identifier escaping if @a driver
//!         is not KDbSQL general identifier escaping when @a driver is 0.
/*! This escaping can be used for field, table, database names, etc.
    @see KDb::escapeIdentifier */
KDB_EXPORT QString escapeIdentifier(const KDbDriver* driver,
                                    const QString& string);

//! @overload QString escapeIdentifier(const KDbDriver*, const QString&)
KDB_EXPORT QByteArray escapeIdentifier(const KDbDriver* driver,
                                       const QByteArray& str);

inline KDbEscapedString valueToSql(const KDbDriver *driver, KDbField::Type ftype, const QVariant& v)
{
    return driver ? driver->valueToSql(ftype, v) : KDb::valueToSql(ftype, v);
}

}

#endif

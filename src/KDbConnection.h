/* This file is part of the KDE project
   Copyright (C) 2003-2013 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_CONNECTION_H
#define KDB_CONNECTION_H

#include <QObject>
#include <QStringList>
#include <QHash>
#include <QVector>
#include <QVariant>
#include <QPointer>

#include "KDbConnectionData.h"
#include "KDbTableSchema.h"
#include "KDbQuerySchema.h"
#include "KDbQuerySchemaParameter.h"
#include "KDbTransaction.h"
#include "KDbDriver.h"
#include "KDbPreparedStatement.h"
#include "KDbRecordData.h"
#include "KDb.h"
#include "KDbTristate.h"

class KDbCursor;
class ConnectionPrivate;
class KDbRecordEditBuffer;
class KDbProperties;

class KDB_EXPORT ConnectionSqlInterface : public KDbResultable
{
    protected:
        inline void setSql(const KDbEscapedString& sql) {
            m_result.setSql(sql);
        }
/*        inline QString sql() const {
            return m_result.sql();
        }*/
        inline void setServerMessage(const QString& serverMessage) {
            m_result.setServerMessage(serverMessage);
        }
        inline void setServerResultCode(int code) {
            m_result.setServerResultCode(code);
        }
        inline void setResult(const KDbResult& result) {
            m_result = result;
        }
    friend class KDbCursor;
    friend class ConnectionInternal;
};

/*! @short Provides database connection, allowing queries and data modification.

 This class represents a database connection established within a data source.
 It supports data queries and modification by creating client-side database cursors.
 Database transactions are supported.
*/
class KDB_EXPORT KDbConnection : public ConnectionSqlInterface
{
public:

    /*! Opened connection is automatically disconnected and removed
     from driver's connections list.
     Note for driver developers: you should call destroy()
     from you KDbConnection's subclass destructor. */
    virtual ~KDbConnection();

    /*! @return parameters that were used to create this connection. */
    KDbConnectionData data() const;

    /*! @return the driver used for this connection. */
    inline KDbDriver* driver() const {
        return m_driver;
    }

    /*!
    @brief Connects to driver with given parameters.
    @return true if successful.

     Note: many database drivers may require connData.databaseName() to be specified
     because explicit database name is needed to perform connection (e.g. SQLite, PostgreSQL).
     MySQL does not require database name; KDbConnection::useDatabase() can be called later.
    */
    bool connect();

    /*! @return true, if connection is properly established. */
    bool isConnected() const;

    /*! @return true, both if connection is properly established
     and any database within this connection is properly used
     with useDatabase(). */
    bool isDatabaseUsed() const;

    /*! @return true for read only connection. Used especially for file-based drivers.
     Can be reimplemented in a driver to provide real read-only flag of the connection
     (sqlite driver does this). */
    virtual bool isReadOnly() const;

    /*! Reimplemented, also clears sql string.
     @sa recentSQLString() */
    void clearResult();

    /*! @brief Disconnects from driver with given parameters.

     The database (if used) is closed, and any active transactions
     (if supported) are rolled back, so commit these before disconnecting,
     if you'd like to save your changes. */
    bool disconnect();

    /*! @return list of database names for opened connection.
     If @a also_system_db is true, the system database names are also returned. */
    QStringList databaseNames(bool also_system_db = false);

    /*! @return true if database @a dbName exists.
     If @a ignoreErrors is true, error flag of connection
      won't be modified for any errors (it will quietly return),
      else (ignoreErrors == false) we can check why the database does
      not exist using error(), errorNum() and/or errorMsg(). */
    bool databaseExists(const QString &dbName, bool ignoreErrors = true);

    /*! @brief Creates new database with name @a dbName, using this connection.

     If database with @a dbName already exists, or other error occurred,
     false is returned.
     For file-based drivers, @a dbName should be equal to the database
     filename (the same as specified for KDbConnectionData).

     See docs/kdb_issues.txt document, chapter "Table schema, query schema, etc. storage"
     for database schema documentation (detailed description of kexi__* 'system' tables).

     @see useDatabase() */
    bool createDatabase(const QString &dbName);

    /*!
    @brief Opens an existing database specified by @a dbName.

     If @a kexiCompatible is true (the default) initial checks will be performed
     to recognize database Kexi-specific format. Set @a kexiCompatible to false
     if you're using native database (one that have no Kexi System tables).
     For file-based drivers, @a dbName can be skipped, so the same as specified for KDbConnectionData is used.
     @return true on success, false on failure.
     If user has cancelled this action and @a cancelled is not 0, *cancelled is set to true. */
    bool useDatabase(const QString &dbName = QString(), bool kexiCompatible = true, bool *cancelled = 0,
                     KDbMessageHandler* msgHandler = 0);

    /*!
    @brief Closes currently used database for this connection.

     Any active transactions (if supported) are rolled back,
     so commit these before closing, if you'd like to save your changes. */
    bool closeDatabase();

    /*! @brief Get the name of the current database

    @return name of currently used database for this connection or empty string
      if there is no used database */
    QString currentDatabase() const;

    /*! @brief Drops database with name @a dbName.

     if dbName is not specified, currently used database name is used
     (it is closed before dropping).
    */
    bool dropDatabase(const QString &dbName = QString());

    /*! @return names of all the @a objectType (see @a ObjectType in KDbGlobal.h)
    schemas stored in currently used database. KDb::AnyObjectType can be passed
    as @a objectType to get names of objects of any type.
    If @a ok is not null then variable pointed by it will be set to the result.
    On error, the functions can return incomplete list. */
    QStringList objectNames(int objectType = KDb::AnyObjectType, bool* ok = 0);

    /*! @return names of all table schemas stored in currently
     used database. If @a also_system_tables is true,
     internal KDb system table names (kexi__*) are also returned.
     @see kdbSystemTableNames() */
    QStringList tableNames(bool also_system_tables = false);

    /*! @return list of internal KDb system table names
     (kexi__*). This does not mean that these tables can be found
     in currently opened database. Just static list of table
     names is returned.

     The list contents may depend on KDb library version;
     opened database can contain fewer 'system' tables than in current
     KDb implementation, if the current one is newer than the one used
     to build the database.
     @todo this will depend on KDb lib version */
    static QStringList kdbSystemTableNames();

    /*! @return server version information for this connection.
     If database is not connected (i.e. isConnected() is false) null KDbServerVersionInfo is returned. */
    KDbServerVersionInfo serverVersion() const;

    /*! @return version information for this connection.
     If database is not used (i.e. isDatabaseUsed() is false) null KDbVersionInfo is returned.
     It can be compared to drivers' and KDb library version to maintain
     backward/upward compatiblility. */
    KDbVersionInfo databaseVersion() const;

    /*! @return KDbProperties object allowing to read and write global database properties
     for this connection. */
    KDbProperties databaseProperties() const;

    /*! @return ids of all table schema names stored in currently
     used database. These ids can be later used as argument for tableSchema().
     This is a shortcut for objectIds(KDb::TableObjectType).
     Internal KDb system tables (kexi__*) are not available here
     because these have no identifiers assigned (more formally: id=-1).

     Note: the fact that given id is on the returned list does not mean
     that tableSchema( id ) returns anything. The table definition can be broken,
     so you have to double check this. */
    QList<int> tableIds();

    /*! @return ids of all database query schemas stored in currently
     used database. These ids can be later used as argument for querySchema().
     This is a shortcut for objectIds(KDb::QueryObjectType).

     Note: the fact that given id is on the returned list does not mean
     that querySchema( id ) returns anything. The query definition can be broken,
     so you have to double check this.

     @see tableIds()
     */
    QList<int> queryIds();

    /*! @return names of all schemas of object with @a objectType type
     that are stored in currently used database.

     Note: the fact that given id is on the returned list does not mean
     that the definition of the object is valid,
     so you have to double check this.

     @see queryIds() */
    QList<int> objectIds(int objectType);

    /*! @brief Creates new KDbTransaction handle and starts a new transaction.
     @return KDbTransaction object if transaction has been started
     successfully, otherwise null transaction.
     For drivers that allow single transaction per connection
     (KDbDriver::features() && SingleTransactions) this method can be called one time,
     and then this single transaction will be default ( setDefaultTransaction() will
     be called).
     For drivers that allow multiple transactions per connection, no default transaction is
     set automatically in beginTransaction() method, you could do this by hand.
     @see setDefaultTransaction(), defaultTransaction().
    */
    KDbTransaction beginTransaction();

    /*! @todo for nested transactions:
        Tansaction* beginTransaction(transaction *parent_transaction);
    */
    /*! Commits transaction @a trans.
     If there is not @a trans argument passed, and there is default transaction
     (obtained from defaultTransaction()) defined, this one will be committed.
     If default is not present, false is returned (when ignore_inactive is
     false, the default), or true is returned (when ignore_inactive is true).

     On successful commit, @a trans object will be destroyed.
     If this was default transaction, there is no default transaction for now.
    */
    bool commitTransaction(KDbTransaction trans = KDbTransaction(),
                           bool ignore_inactive = false);

    /*! Rollbacks transaction @a trans.
     If there is not @a trans argument passed, and there is default transaction
     (obtained from defaultTransaction()) defined, this one will be rolled back.
     If default is not present, false is returned (when ignore_inactive is
     false, the default), or true is returned (when ignore_inactive is true).

     or any error occurred, false is returned.

     On successful rollback, @a trans object will be destroyed.
     If this was default transaction, there is no default transaction for now.
    */
    bool rollbackTransaction(KDbTransaction trans = KDbTransaction(),
                             bool ignore_inactive = false);

    /*! @return handle for default transaction for this connection
     or null transaction if there is no such a transaction defined.
     If transactions are supported: Any operation on database (e.g. inserts)
     that is started without specifying transaction context, will be performed
     in the context of this transaction.

     Returned null transaction doesn't mean that there is no transactions
     started at all.
     Default transaction can be defined automatically for some drivers --
     see beginTransaction().
     @see KDbDriver::transactionsSupported()
    */
    KDbTransaction defaultTransaction() const;

    /*! Sets default transaction that will be used as context for operations
     on data in opened database for this connection. */
    void setDefaultTransaction(const KDbTransaction& trans);

    /*! @return set of handles of currently active transactions.
     Note that in multithreading environment some of these
     transactions can be already inactive after calling this method.
     Use KDbTransaction::active() to check that. Inactive transaction
     handle is useless and can be safely dropped.
    */
    QList<KDbTransaction> transactions();

    /*! @return true if "auto commit" option is on.

     When auto commit is on (the default on for any new KDbConnection object),
     every sql functional statement (statement that changes
     data in the database implicitly starts a new transaction.
     This transaction is automatically committed
     after successful statement execution or rolled back on error.

     For drivers that do not support transactions (see KDbDriver::features())
     this method shouldn't be called because it does nothing ans always returns false.

     No internal KDb object should changes this option, although auto commit's
     behaviour depends on database engine's specifics. Engines that support only single
     transaction per connection (see KDbDriver::SingleTransactions),
     use this single connection for autocommiting, so if there is already transaction
     started by the KDb user program (with beginTransaction()), this transaction
     is committed before any sql functional statement execution. In this situation
     default transaction is also affected (see defaultTransaction()).

     Only for drivers that support nested transactions (KDbDriver::NestedTransactions),
     autocommiting works independently from previously started transaction,

     For other drivers set this option off if you need use transaction
     for grouping more statements together.

     NOTE: nested transactions are not yet implemented in KDb API.
    */
    bool autoCommit() const;

    /*! Changes auto commit option. This does not affect currently started transactions.
     This option can be changed even when connection is not established.
     @see autoCommit() */
    bool setAutoCommit(bool on);

    /*! Connection-specific string escaping. Default implementation uses driver's escaping.
     Use KDbEscapedString::isValid() to check if escaping has been performed successfully.
     Invalid strings are set to null in addition, that is KDbEscapedString::isNull() is true,
     not just isEmpty().
    */
    virtual KDbEscapedString escapeString(const QString& str) const;

    /*! Prepares SELECT query described by raw @a statement.
     @return opened cursor created for results of this query
     or NULL if there was any error. KDbCursor can have optionally applied @a cursor_options
     (one of more selected from KDbCursor::Options).
     Preparation means that returned cursor is created but not opened.
     Open this when you would like to do it with KDbCursor::open().

     Note for driver developers: you should initialize cursor engine-specific
     resources and return KDbCursor subclass' object
     (passing @a statement and @a cursor_options to it's constructor).
    */
    virtual KDbCursor* prepareQuery(const KDbEscapedString& statement, uint cursor_options = 0) = 0;

    /*! @overload prepareQuery(const KDbEscapedString&, uint)
     Prepares query described by @a query schema. @a params are values of parameters that
     will be inserted into places marked with [] before execution of the query.

     Note for driver developers: you should initialize cursor engine-specific
     resources and return KDbCursor subclass' object
     (passing @a query and @a cursor_options to it's constructor).
     Kexi SQL and driver-specific escaping is performed on table names.
    */
    KDbCursor* prepareQuery(KDbQuerySchema* query, const QList<QVariant>& params,
                         uint cursor_options = 0);

    /*! @overload prepareQuery(KDbQuerySchema* query, const QList<QVariant>& params,
      uint cursor_options = 0 )
     Prepares query described by @a query schema without parameters.
    */
    virtual KDbCursor* prepareQuery(KDbQuerySchema* query, uint cursor_options = 0) = 0;

    /*! @overload prepareQuery(const KDbEscapedString&, uint)
     Statement is build from data provided by @a table schema,
     it is like "select * from table_name".*/
    KDbCursor* prepareQuery(KDbTableSchema* table, uint cursor_options = 0);

    /*! Executes SELECT query described by @a statement.
     @return opened cursor created for results of this query
     or NULL if there was any error on the cursor creation or opening.
     KDbCursor can have optionally applied @a cursor_options
     (one of more selected from KDbCursor::Options).
     Identifiers in @a statement that are the same as keywords
     in KDbSQL dialect or the backend's SQL need to have been escaped.
     */
    KDbCursor* executeQuery(const KDbEscapedString& statement, uint cursor_options = 0);

    /*! @overload executeQuery(const KDbEscapedString&, uint)
     @a params are values of parameters that
     will be inserted into places marked with [] before execution of the query.

     Statement is build from data provided by @a query schema.
     Kexi SQL and driver-specific escaping is performed on table names. */
    KDbCursor* executeQuery(KDbQuerySchema* query, const QList<QVariant>& params,
                         uint cursor_options = 0);

    /*! @overload executeQuery( KDbQuerySchema* query, const QList<QVariant>& params,
      uint cursor_options = 0 ) */
    KDbCursor* executeQuery(KDbQuerySchema* query, uint cursor_options = 0);

    /*! @overload executeQuery(const KDbEscapedString&, uint)
     Executes query described by @a query schema without parameters.
     Statement is build from data provided by @a table schema,
     it is like "select * from table_name".*/
    KDbCursor* executeQuery(KDbTableSchema* table, uint cursor_options = 0);

    /*! Deletes cursor @a cursor previously created by functions like executeQuery()
     for this connection.
     There is an attempt to close the cursor with KDbCursor::close() if it was opened.
     Anyway, at last cursor is deleted.
     @return true if cursor is properly closed before deletion. */
    bool deleteCursor(KDbCursor *cursor);

    /*! @return schema of a table pointed by @a tableId, retrieved from currently
     used database. The schema is cached inside connection,
     so retrieval is performed only once, on demand. */
    KDbTableSchema* tableSchema(int tableId);

    /*! @return schema of a table pointed by @a tableName, retrieved from currently
     used database. KDb system table schema can be also retrieved.
     @see tableSchema( int tableId ) */
    KDbTableSchema* tableSchema(const QString& tableName);

    /*! @return schema of a query pointed by @a queryId, retrieved from currently
     used database. The schema is cached inside connection,
     so retrieval is performed only once, on demand. */
    KDbQuerySchema* querySchema(int queryId);

    /*! @return schema of a query pointed by @a queryName, retrieved from currently
     used database.  @see querySchema( int queryId ) */
    KDbQuerySchema* querySchema(const QString& queryName);

    /*! Sets @a queryName query obsolete by moving it out of the query sets, so it will not be
     accessible by querySchema( const QString& queryName ). The existing query object is not
     destroyed, to avoid problems when it's referenced. In this case,
     a new query schema will be retrieved directly from the backend.

     For now it's used in KexiQueryDesignerGuiEditor::storeLayout().
     This solves the problem when user has changed a query schema but already form still uses
     previously instantiated query schema.
     @return true if there is such query. Otherwise the method does nothing. */
    bool setQuerySchemaObsolete(const QString& queryName);

    /*! Executes @a statement query and stores first record's data inside @a data.
     This is convenient method when we need only first record from query result,
     or when we know that query result has only one record.
     If @a addLimitTo1 is true (the default), adds a LIMIT clause to the query,
     so @a statement should not include one already.
     @return true if query was successfully executed and first record has been found,
     false on data retrieving failure, and cancelled if there's no single record available. */
    tristate querySingleRecord(const KDbEscapedString& statement, KDbRecordData* data, bool addLimitTo1 = true);

    /*! Like tristate @ref querySingleRecord(const QString&, KDbRecordData*, bool)
     but uses KDbQuerySchema object.
     If @a addLimitTo1 is true (the default), adds a LIMIT clause to the query. */
    tristate querySingleRecord(KDbQuerySchema* query, KDbRecordData* data, bool addLimitTo1 = true);

    /*! Executes @a statement query and stores first record's field's (number @a column) string value
     inside @a value. For efficiency it's recommended that a query defined by @a statement
     should have just one field (SELECT one_field FROM ....).
     If @a addLimitTo1 is true (the default), adds a LIMIT clause to the query,
     so @a statement should not include one already.
     @return true if query was successfully executed and first record has been found,
     false on data retrieving failure, and cancelled if there's no single record available.
     @see queryStringList() */
    tristate querySingleString(const KDbEscapedString& statement, QString* value, uint column = 0,
                               bool addLimitTo1 = true);

    /*! Convenience function: executes @a statement query and stores first
     record's field's (number @a column) value inside @a number. @see querySingleString().
     Note: "LIMIT 1" is appended to @a sql statement if @a addLimitTo1 is true (the default).
     @return true if query was successfully executed and first record has been found,
     false on data retrieving failure, and cancelled if there's no single record available. */
    tristate querySingleNumber(const KDbEscapedString& statement, int* number, uint column = 0,
                               bool addLimitTo1 = true);

    /*! Executes @a statement query and stores Nth field's string value of every record
     inside @a list, where N is equal to @a column. The list is initially cleared.
     For efficiency it's recommended that a query defined by @a statement
     should have just one field (SELECT one_field FROM ....).
     @return true if all values were fetched successfuly,
     false on data retrieving failure. Returning empty list can be still a valid result.
     On errors, the list is not cleared, it may contain a few retrieved values. */
    bool queryStringList(const KDbEscapedString& statement, QStringList* list, uint column = 0);

    /*! @return true if there is at least one record returned in @a statement query.
     Does not fetch any records. @a success will be set to false
     on query execution errors (true otherwise), so you can see a difference between
     "no results" and "query execution error" states.
     Note: real executed query is: "SELECT 1 FROM (@a statement) LIMIT 1"
     if @a addLimitTo1 is true (the default). */
    bool resultExists(const KDbEscapedString& statement, bool* success, bool addLimitTo1 = true);

    /*! @return true if there is at least one record in @a table. */
    bool isEmpty(KDbTableSchema* table, bool *success);

//! @todo perhaps use quint64 here?
    /*! @return number of records in @a statement query.
     Does not fetch any records. -1 is returned on query execution errors (>0 otherwise).
     Note: real executed query is: "SELECT COUNT() FROM (@a statement) LIMIT 1"
     (using querySingleNumber()) */
    int resultCount(const KDbEscapedString& statement);

    virtual KDbEscapedString recentSQLString() const;

    //PROTOTYPE:
#define A , const QVariant&
#define H_INS_REC(args) bool insertRecord(KDbTableSchema* tableSchema args)
#define H_INS_REC_ALL \
    H_INS_REC(A); \
    H_INS_REC(A A); \
    H_INS_REC(A A A); \
    H_INS_REC(A A A A); \
    H_INS_REC(A A A A A); \
    H_INS_REC(A A A A A A); \
    H_INS_REC(A A A A A A A); \
    H_INS_REC(A A A A A A A A)
    H_INS_REC_ALL;

#undef H_INS_REC
#define H_INS_REC(args) bool insertRecord(KDbFieldList* fields args)

    H_INS_REC_ALL;
#undef H_INS_REC_ALL
#undef H_INS_REC
#undef A

    bool insertRecord(KDbTableSchema* tableSchema, const QList<QVariant>& values);

    bool insertRecord(KDbFieldList* fields, const QList<QVariant>& values);

    /*! Creates table defined by @a tableSchema.
     Schema information is also added into kexi system tables, for later reuse.
     @return true on success - @a tableSchema object is then
     inserted to KDbConnection structures - it is owned by KDbConnection object now,
     so you shouldn't destroy the tableSchema object by hand
     (or declare it as local-scope variable).

     If @a replaceExisting is false (the default) and table with the same name
     (as tableSchema->name()) exists, false is returned.
     If @a replaceExisting is true, a table schema with the same name (if exists)
     is overwritten, then a new table schema gets the same identifier
     as existing table schema's identifier.

     Table and column definitions are added to to kexi__* "system schema" tables.
     Checks that a database is in use, and that the schema defines at least one column.

     Note that on error:
     - @a tableSchema is not inserted into KDbConnection's structures,
       so you are still owner of this object
     - existing table schema object is not destroyed (i.e. it is still available
       e.g. using KDbConnection::tableSchema(const QString&), even if the table
       was physically dropped.
    */
    bool createTable(KDbTableSchema* tableSchema, bool replaceExisting = false);

    /*! Creates a copy of table schema defined by @a tableSchema with data.
     Name, caption and description will be copied from @a newData.
     @return a table schema object. It is inserted into the KDbConnection structures
     and is owned by the KDbConnection object. The created table schema object should not
     be destroyed by hand afterwards.
     0 is returned on failure. Table with destination name must not exist.
     @see createTable() */
    KDbTableSchema *copyTable(const KDbTableSchema &tableSchema, const KDbObject &newData);

    /*! It is a convenience function, does exactly the same as
     KDbTableSchema *copyTable(const KDbTableSchema&, const KDbObject&). */
    KDbTableSchema *copyTable(const QString& tableName, const KDbObject &newData);

    /*! Drops a table defined by @a tableSchema (both table object as well as physically).
     If true is returned, schema information @a tableSchema is destoyed
     (because it's owned), so don't keep this anymore!
     No error is raised if the table does not exist physically
     - its schema is removed even in this case.

     Removes the table and column definitions in kexi__* "system schema" tables.
     First checks that the table is not a system table.

     @todo Check that a database is currently in use? (c.f. createTable)
     @todo Update any structure (e.g. query) that depends on this table */
    tristate dropTable(KDbTableSchema* tableSchema);

    /*! It is a convenience function, does exactly the same as
     bool dropTable( KDbTableSchema* tableSchema ) */
    tristate dropTable(const QString& tableName);

    /*! Alters @a tableSchema using @a newTableSchema in memory and on the db backend.
     @return true on success, cancelled if altering was cancelled. */
//! @todo (js): implement real altering
//! @todo (js): update any structure (e.g. query) that depend on this table!
    tristate alterTable(KDbTableSchema* tableSchema, KDbTableSchema* newTableSchema);

    /*! Alters name of table described by @a tableSchema to @a newName.
     If @a replace is true, destination table is completely dropped and replaced
     by @a tableSchema, if present. In this case, identifier of
     @a tableSchema becomes equal to the dropped table's id, what can be useful
     if @a tableSchema was created with a temporary name and ID (used in KDbAlterTableHandler).

     If @a replace is false (the default) and destination table is present
     -- false is returned and ERR_OBJECT_EXISTS error is set.
     The schema of @a tableSchema is updated on success.
     @return true on success. */
    bool alterTableName(KDbTableSchema* tableSchema, const QString& newName, bool replace = false);

    /*! Drops a query defined by @a querySchema.
     If true is returned, schema information @a querySchema is destoyed
     (because it's owned), so don't keep this anymore!
    */
    bool dropQuery(KDbQuerySchema* querySchema);

    /*! It is a convenience function, does exactly the same as
     bool dropQuery( KDbQuerySchema* querySchema ) */
    bool dropQuery(const QString& queryName);

    /*! Removes information about object with @a objId
     from internal "kexi__object" and "kexi__objectdata" tables.
     @return true on success. */
    bool removeObject(uint objId);

    /*! @return first field from @a fieldlist that has system name,
     null if there are no such field.
     For checking, KDbDriver::isSystemFieldName() is used, so this check can
     be driver-dependent. */
    KDbField* findSystemFieldName(const KDbFieldList& fieldlist);

    /*! @return name of any (e.g. first found) database for this connection.
     This method does not close or open this connection. The method can be used
     (it is also internally used, e.g. for database dropping) when we need
     a database name before we can connect and execute any SQL statement
     (e.g. DROP DATABASE).

     The method can return nul lstring, but in this situation no automatic (implicit)
     connections could be made, what is useful by e.g. dropDatabase().

     Note for driver developers: return here a name of database which you are sure
     is existing.
     Default implementation returns:
     - value that previously had been set using setAvailableDatabaseName() for
       this connection, if it is not empty
     - else (2nd priority): value of KDbDriverBehaviour::ALWAYS_AVAILABLE_DATABASE_NAME
     if it is not empty.

     See decription of KDbDriverBehaviour::ALWAYS_AVAILABLE_DATABASE_NAME member.
     You may want to reimplement this method only when you need to depend on
     this connection specifics
     (e.g. you need to check something remotely).
    */
    virtual QString anyAvailableDatabaseName();

    /*! Sets @a dbName as name of a database that can be accessible.
     This is option that e.g. application that make use of KDb library can set
     to tune connection's behaviour when it needs to temporary connect to any database
     in the server to do some work.
     You can pass empty dbName - then anyAvailableDatabaseName() will try return
     KDbDriverBehaviour::ALWAYS_AVAILABLE_DATABASE_NAME (the default) value
     instead of the one previously set with setAvailableDatabaseName().

     @see anyAvailableDatabaseName()
    */
    void setAvailableDatabaseName(const QString& dbName);

    /*! Because some engines need to have opened any database before
     executing administrative SQL statements like "create database" or "drop database",
     this method is used to use appropriate, existing database for this connection.
     For file-based db drivers this always return true and does not set @a name
     to any value. For other db drivers: this sets @a name to db name computed
     using anyAvailableDatabaseName(), and if the name computed is empty, false
     is returned; if it is not empty, useDatabase() is called.
     False is returned also when useDatabase() fails.
     You can call this method from your application's level if you really want to perform
     tasks that require any used database. In such a case don't forget
     to closeDatabase() if returned @a name is not empty.

     Note: This method has nothing to do with creating or using temporary databases
     in such meaning that these database are not persistent
    */
    bool useTemporaryDatabaseIfNeeded(QString* name);

    /*! @return autoincrement field's @a aiFieldName value
     of last inserted record. This refers @a tableName table.

     Simply, method internally fetches last inserted record and returns selected
     field's value. Requirements: field must be of integer type, there must be a
     record inserted in current database session (whatever this means).
     On error (quint64)-1 is returned.
     Last inserted record is identified by magical record identifier, usually called
     ROWID (PostgreSQL has it as well as SQLite;
     see KDbDriverBehaviour::ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE).
     ROWID's value will be assigned back to @a recordId if this pointer is not null.
    */
    quint64 lastInsertedAutoIncValue(const QString& aiFieldName, const QString& tableName,
                                     quint64* recordId = 0);

    /*! @overload int lastInsertedAutoIncValue(const QString&, const QString&, quint64*)
    */
    quint64 lastInsertedAutoIncValue(const QString& aiFieldName,
                                     const KDbTableSchema& table, quint64* recordId = 0);

    /*! Executes query @a statement, but without returning resulting
     records (used mostly for functional queries).
     Only use this method if you really need. */
    bool executeSQL(const KDbEscapedString& statement);

    //! @short options used in selectStatement()
    class KDB_EXPORT SelectStatementOptions
    {
    public:
        SelectStatementOptions();
        ~SelectStatementOptions();

        //! True if record ID should be also retrieved. False by default.
        bool alsoRetrieveRecordId;

        /*! True if relations (LEFT OUTER JOIN) for visible lookup columns should be added.
         True by default. This is set to false when user-visible statement is generated
         e.g. for the Query Designer. */
        bool addVisibleLookupColumns;
    };

    /*! @return "SELECT ..." statement's string needed for executing query
     defined by @a querySchema, @a params and @a options. */
    KDbEscapedString selectStatement(KDbQuerySchema* querySchema,
                                  const QList<QVariant>& params,
                                  const SelectStatementOptions& options = SelectStatementOptions());

    /*! @overload QString selectStatement( KDbQuerySchema* querySchema,
      QList<QVariant> params = QList<QVariant>(),
      const SelectStatementOptions& options = SelectStatementOptions() ) const;
     @return "SELECT ..." statement's string needed for executing query
     defined by @a querySchema. */
    inline KDbEscapedString selectStatement(KDbQuerySchema* querySchema,
                                         const SelectStatementOptions& options = SelectStatementOptions())
    {
        return selectStatement(querySchema, QList<QVariant>(), options);
    }

    /*! Stores object (id, name, caption, description)
    described by @a object on the backend. It is expected that entry on the
    backend already exists, so it's updated. Changes to identifier attribute are not allowed.
    @return true on success. */
    bool storeObjectData(KDbObject* object);

    /*! Stores new entry for object (id, name, caption, description)
    described by @a object on the backend. If object.id() was less than 0,
    new, unique object identifier is obtained and assigned to @a object (see KDbObject::id()).
    @return true on success. */
    bool storeNewObjectData(KDbObject* object);

    /*! Added for convenience.
     @see setupObjectData(const KDbRecordData*, KDbObject*).
     @return true on success, false on failure and cancelled when such object couldn't be found. */
    tristate loadObjectData(int id, KDbObject* object);

    /*! Finds object schema data for object of type @a type and name @a name.
     If the object is found, resulted schema is stored in @a object and true is returned,
     otherwise false is returned. */
    tristate loadObjectData(int type, const QString& name, KDbObject* object);

    /*! Loads (potentially large) data block (e.g. xml form's representation), referenced by objectID
     and puts it to @a dataString. The can be block indexed with optional @a dataID.
     @return true on success, false on failure and cancelled when there is no such data block
     @see storeDataBlock(). */
    tristate loadDataBlock(int objectID, QString* dataString, const QString& dataID = QString());

    /*! Stores (potentially large) data block @a dataString (e.g. xml form's representation),
     referenced by objectID. Block will be stored in "kexi__objectdata" table and
     an optional @a dataID identifier.
     If there is already such record in the table, it's simply overwritten.
     @return true on success
     @see loadDataBlock() removeDataBlock() copyDataBlock(). */
    bool storeDataBlock(int objectID, const QString &dataString,
                        const QString& dataID = QString());

    /*! Copies (potentially large) data, e.g. form's XML representation,
     referenced by @a sourceObjectID pointed by optional @a dataID.
     @return true on success. Does not fail if blocks do not exist.
     Prior to copying, existing data blocks are removed even if there are no new blocks to copy.
     Copied data blocks will have @a destObjectID object identifier assigned.
     Note that if @a dataID is not specified, all data blocks found for the @a sourceObjectID
     will be copied.
     @see loadDataBlock() storeDataBlock() removeDataBlock(). */
    bool copyDataBlock(int sourceObjectID, int destObjectID, const QString& dataID = QString());

    /*! Removes (potentially large) string data (e.g. xml form's representation),
     referenced by @a objectID, and pointed by optional @a dataID.
     @return true on success. Does not fail if the block does not exist.
     Note that if @a dataID is not specified, all data blocks for the @a objectID will be removed.
     @see loadDataBlock() storeDataBlock() copyDataBlock(). */
    bool removeDataBlock(int objectID, const QString& dataID = QString());

    class KDB_EXPORT TableSchemaChangeListenerInterface
    {
    public:
        TableSchemaChangeListenerInterface() {}
        virtual ~TableSchemaChangeListenerInterface() {}

        /*! Closes listening object so it will be deleted and thus no longer use
         a conflicting table schema. */
        virtual tristate closeListener() = 0;

        /*! i18n-ed string that can be displayed for user to inform about
         e.g. conflicting listeners. */
        QString listenerInfoString;
    };
//TMP// TODO: will be more generic
    /** Register @a listener for receiving (listening) information about changes
     in KDbTableSchema object. Changes could be: altering and removing. */
    void registerForTableSchemaChanges(TableSchemaChangeListenerInterface* listener,
                                       KDbTableSchema* schema);

    void unregisterForTableSchemaChanges(TableSchemaChangeListenerInterface* listener,
                                         KDbTableSchema* schema);

    void unregisterForTablesSchemaChanges(TableSchemaChangeListenerInterface* listener);

    QSet<KDbConnection::TableSchemaChangeListenerInterface*>*
    tableSchemaChangeListeners(KDbTableSchema* tableSchema) const;

    tristate closeAllTableSchemaChangeListeners(KDbTableSchema* tableSchema);

//! @todo move this somewhere to low level class (MIGRATION?)
    /*! LOW LEVEL METHOD. For implementation: returns true if table
     with name @a tableName exists in the database.
     @return false if it does not exist or error occurred.
     The lookup is case insensitive. */
    virtual bool drv_containsTable(const QString &tableName) = 0;

    /*! Creates table using @a tableSchema information.
     @return true on success. Default implementation
     builds a statement using createTableStatement() and calls drv_executeSQL()
     Note for driver developers: reimplement this only if you want do to
     this in other way.

     Moved to public for KexiMigrate.
     @todo fix this after refactoring
     */
    virtual bool drv_createTable(const KDbTableSchema& tableSchema);

    /*! Alters table's described @a tableSchema name to @a newName.
     This is the default implementation, using "ALTER TABLE <oldname> RENAME TO <newname>",
     what's supported by SQLite >= 3.2, PostgreSQL, MySQL.
     Backends lacking ALTER TABLE, for example SQLite2, reimplement this with by an inefficient
     data copying to a new table. In any case, renaming is performed at the backend.
     It's good idea to keep the operation within a transaction.
     @return true on success.

     Moved to public for KexiProject.
     @todo fix this after refactoring
    */
    virtual bool drv_alterTableName(KDbTableSchema* tableSchema, const QString& newName);

    /*! Copies table data from @a tableSchema to @a destinationTableSchema
     Default implementation executes "INSERT INTO .. SELECT * FROM .."
     @return true on success. */
    virtual bool drv_copyTableData(const KDbTableSchema &tableSchema,
                                   const KDbTableSchema &destinationTableSchema);

    /*! Physically drops table named with @a name.
     Default impelmentation executes "DROP TABLE.." command,
     so you rarely want to change this.

      Moved to public for KexiMigrate
      @todo fix this after refactoring
    */
    virtual bool drv_dropTable(const QString& tableName);

    /*! Prepare a SQL statement and return a @a KDbPreparedStatement instance. */
    KDbPreparedStatement prepareStatement(KDbPreparedStatement::Type type,
        KDbFieldList* fields, const QStringList& whereFieldNames = QStringList());

    bool isInternalTableSchema(const QString& tableName);

    /*! Setups data for object that owns @a object (e.g. table, query)
      opened on 'kexi__objects' table, pointing to a record
      corresponding to given object.

      Moved to public for KexiMigrate
    */
    bool setupObjectData(const KDbRecordData& data, KDbObject* object);

    /*! @return a new field table schema for a table retrieved from @a data.
     Used internally by tableSchema().

      Moved to public for KexiMigrate
      @todo fix this after refatoring
    */
    KDbField* setupField(const KDbRecordData& data);

    /*! @internal. Inserts internal table to KDbConnection's structures, so it can be found by name.
     This method is used for example in KexiProject to insert information about "kexi__blobs"
     table schema. Use createTable() to physically create table. After createTable()
     calling insertInternalTable() is not required.
     Also used internally by KDbConnection::newKDbSystemTableSchema(const QString&) */
    void insertInternalTable(KDbTableSchema* tableSchema);

    //! Identifier escaping function in the associated KDbDriver.
    /*! Calls the identifier escaping function in this connection to
     escape table and column names.  This should be used when explicitly
     constructing SQL strings (e.g. "FROM " + escapeIdentifier(tablename)).
     It should not be used for other functions (e.g. don't do
     useDatabase(escapeIdentifier(database))), because the identifier will
     be escaped when the called function generates, for example, "USE " +
     escapeIdentifier(database).

     For efficiency, KDb "system" tables (prefixed with kexi__)
     and columns therein are not escaped - we assume these are valid identifiers for all drivers.

     Use KDbEscapedString::isValid() to check if escaping has been performed successfully.
     Invalid strings are set to null in addition, that is KDbEscapedString::isNull() is true,
     not just isEmpty().
    */
    virtual QString escapeIdentifier(const QString& id) const {
        return m_driver->escapeIdentifier(id);
    }

protected:
    /*! Used by KDbDriver */
    KDbConnection(KDbDriver *driver, const KDbConnectionData& connData);

    /*! Method to be called form KDbConnection's subclass destructor.
     @see ~KDbConnection() */
    void destroy();

    /*! @internal drops table @a tableSchema physically, but destroys
     @a tableSchema object only if @a alsoRemoveSchema is true.
     Used (alsoRemoveSchema==false) on table altering:
     if recreating table can fail we're giving up and keeping
     the original table schema (even if it is no longer points to any real data). */
    tristate dropTable(KDbTableSchema* tableSchema, bool alsoRemoveSchema);

    /*! For implementation: connects to database.
      @return true on success. */
    virtual bool drv_connect() = 0;

    /*! For implementation: Sets @a version to real server's version.
     Depending on backend type this method is called after
     (if KDbDriverBehaviour::USING_DATABASE_REQUIRED_TO_CONNECT is true)
     or before database is used
     (if KDbDriverBehaviour::USING_DATABASE_REQUIRED_TO_CONNECT is false),
     i.e. for PostgreSQL it is called after.
     In any case it is called after successful drv_connect().
     @return true on success. */
    virtual bool drv_getServerVersion(KDbServerVersionInfo* version) = 0;

    /*! For implementation: disconnects database
      @return true on success. */
    virtual bool drv_disconnect() = 0;

    /*! Executes query @a statement, but without returning resulting
     records (used mostly for functional queries).
     It is already verified that @a statement is valid (properly escaped).
     Only use this method if you really need. */
    virtual bool drv_executeSQL(const KDbEscapedString& statement) = 0;

    /*! For reimplementation: loads list of databases' names available for this connection
     and adds these names to @a list. If your server is not able to offer such a list,
     consider reimplementing drv_databaseExists() instead.
     The method should return true only if there was no error on getting database names
     list from the server.
     Default implementation puts empty list into @a list and returns true. */
    virtual bool drv_getDatabasesList(QStringList* list);

//! @todo move this somewhere to low level class (MIGRATION?)
    /*! LOW LEVEL METHOD. For implementation: loads low-level list of table names
     available for this connection. The names are in lower case.
     The method should return true only if there was no error on getting database names
     list from the server. */
    virtual bool drv_getTablesList(QStringList* list) = 0;

    /*! For optional reimplementation: asks server if database @a dbName exists.
     This method is used internally in databaseExists(). The default  implementation
     calls databaseNames and checks if that list contains @a dbName. If you need to
     ask the server specifically if a database exists, eg. if you can't retrieve a list
     of all available database names, please reimplement this method and do all
     needed checks.

     See databaseExists() description for details about ignoreErrors argument.
     You should use it properly in your implementation.

     Note: This method should also work if there is already database used (with useDatabase());
     in this situation no changes should be made in current database selection. */
    virtual bool drv_databaseExists(const QString &dbName, bool ignoreErrors = true);

    /*! For implementation: creates new database using connection */
    virtual bool drv_createDatabase(const QString &dbName = QString()) = 0;

    /*! For implementation: opens existing database using connection
     @return true on success, false on failure; sets @a cancelled to true if this action
     has been cancelled. */
    virtual bool drv_useDatabase(const QString &dbName = QString(), bool *cancelled = 0,
                                 KDbMessageHandler* msgHandler = 0) = 0;

    /*! For implementation: closes previously opened database
      using connection. */
    virtual bool drv_closeDatabase() = 0;

    /*! @return true if internal driver's structure is still in opened/connected
     state and database is used.
     Note for driver developers: Put here every test that you can do using your
     internal engine's database API,
     eg (a bit schematic):  my_connection_struct->isConnected()==true.
     Do not check things like KDbConnection::isDatabaseUsed() here or other things
     that "KDb already knows" at its level.
     If you cannot test anything, just leave default implementation (that returns true).

     Result of this method is used as an additional chance to check for isDatabaseUsed().
     Do not call this method from your driver's code, it should be used at KDb level only.
    */
    virtual bool drv_isDatabaseUsed() const {
        return true;
    }

    /*! For implementation: drops database from the server
      using connection. After drop, database shouldn't be accessible
      anymore. */
    virtual bool drv_dropDatabase(const QString &dbName = QString()) = 0;

    /*! @return "CREATE TABLE ..." statement string needed for @a tableSchema
     creation in the database.

     Note: The statement string can be specific for this connection's driver database,
     and thus not reusable in general.
    */
    KDbEscapedString createTableStatement(const KDbTableSchema& tableSchema) const;

    /*! @return "SELECT ..." statement's string needed for executing query
     defined by "select * from table_name" where <i>table_name</i> is @a tableSchema's name.
     This method's variant can be useful when there is no appropriate KDbQuerySchema defined.

     Note: The statement string can be specific for this connection's driver database,
     and thus not reusable in general.
    */
    KDbEscapedString selectStatement(KDbTableSchema* tableSchema,
                                  const SelectStatementOptions& options = SelectStatementOptions());

    /*!
     Creates table named by @a tableName. Schema object must be on
     schema tables' list before calling this method (otherwise false if returned).
     Just uses drv_createTable( const KDbTableSchema& tableSchema ).
     Used internally, e.g. in createDatabase().
     @return true on success
    */
    virtual bool drv_createTable(const QString& tableName);

    /*! @return unique identifier of the most recently inserted record.
     Typically this is just primary key value.
     This identifier could be reused when we want to reference
     just inserted record.
     If there was no insertion recently performed for this connection, 0 is returned.
     Note for driver developers: contact staniek (at) kde.org
     if your engine does not offer this information. */
    virtual quint64 drv_lastInsertRecordId() = 0;

    /*! Note for driver developers: begins new transaction
     and returns handle to it. Default implementation just
     executes "BEGIN" sql statement and returns just empty data (KDbTransactionData object).

     Drivers that do not support transactions (see KDbDriver::features())
     do never call this method.
     Reimplement this method if you need to do something more
     (e.g. if you driver will support multiple transactions per connection).
     Make subclass of KDbTransactionData (declared in KDbTransaction.h)
     and return object of this subclass.
     You should return NULL if any error occurred.
     Do not check anything in connection (isConnected(), etc.) - all is already done.
    */
    virtual KDbTransactionData* drv_beginTransaction();

    /*! Note for driver developers: begins new transaction
     and returns handle to it. Default implementation just
     executes "COMMIT" sql statement and returns true on success.

     @see drv_beginTransaction()
    */
    virtual bool drv_commitTransaction(KDbTransactionData* trans);

    /*! Note for driver developers: begins new transaction
     and returns handle to it. Default implementation just
     executes "ROLLBACK" sql statement and returns true on success.

     @see drv_beginTransaction()
    */
    virtual bool drv_rollbackTransaction(KDbTransactionData* trans);


    /*! Preprocessing (if any) required by drivers before execution of an
        Insert statement.
        Reimplement this method in your driver if there are any special processing steps to be
        executed before an Insert statement.
      @see drv_afterInsert()
    */
    virtual bool drv_beforeInsert(const QString& tableName, KDbFieldList* fields) {
        Q_UNUSED(tableName);
        Q_UNUSED(fields);
        return true;
    }

    /*! Postprocessing (if any) required by drivers before execution of an
        Insert statement.
        Reimplement this method in your driver if there are any special processing steps to be
        executed after an Insert statement.
      @see drv_beforeInsert()
    */
    virtual bool drv_afterInsert(const QString& tableName, KDbFieldList* fields) {
        Q_UNUSED(tableName);
        Q_UNUSED(fields);
        return true;
    }

    /*! Preprocessing required by drivers before execution of an
        Update statement.
        Reimplement this method in your driver if there are any special processing steps to be
        executed before an Update statement.
    @see drv_afterUpdate()
    */
    virtual bool drv_beforeUpdate(const QString& tableName, KDbFieldList* fields) {
        Q_UNUSED(tableName);
        Q_UNUSED(fields);
        return true;
    }

    /*! Postprocessing required by drivers before execution of an
        Insert statement.
        Reimplement this method in your driver if there are any special processing steps to be
        executed after an Update statement.
      @see drv_beforeUpdate()
    */
    virtual bool drv_afterUpdate(const QString& tableName, KDbFieldList* fields) {
        Q_UNUSED(tableName);
        Q_UNUSED(fields);
        return true;
    }

    /*! Changes autocommiting option for established connection.
      @return true on success.

      Note for driver developers: reimplement this only if your engine
      allows to set special auto commit option (like "SET AUTOCOMMIT=.." in MySQL).
      If not, auto commit behaviour will be simulated if at least single
      transactions per connection are supported by the engine.
      Do not set any internal flags for autocommiting -- it is already done inside
      setAutoCommit().

      Default implementation does nothing with connection, just returns true.

      @see drv_beginTransaction(), autoCommit(), setAutoCommit()
     */
    virtual bool drv_setAutoCommit(bool on);

    /*! Prepare a SQL statement and return a @a KDbPreparedStatementInterface-derived object. */
    virtual KDbPreparedStatementInterface* prepareStatementInternal() = 0;

    /*! Internal, for handling autocommited transactions:
     begins transaction if one is supported.
     @return true if new transaction started
     successfully or no transactions are supported at all by the driver
     or if autocommit option is turned off.
     A handle to a newly created transaction (or null on error) is passed
     to @a tg parameter.

     Special case when used database driver has only single transaction support
     (KDbDriver::SingleTransactions):
     and there is already transaction started, it is committed before
     starting a new one, but only if this transaction has been started inside KDbConnection object.
     (i.e. by beginAutoCommitTransaction()). Otherwise, a new transaction will not be started,
     but true will be returned immediately.
    */
    bool beginAutoCommitTransaction(KDbTransactionGuard* tg);

    /*! Internal, for handling autocommited transactions:
     Commits transaction prevoiusly started with beginAutoCommitTransaction().
     @return true on success or when no transactions are supported
     at all by the driver.

     Special case when used database driver has only single transaction support
     (KDbDriver::SingleTransactions): if @a trans has been started outside KDbConnection object
     (i.e. not by beginAutoCommitTransaction()), the transaction will not be committed.
    */
    bool commitAutoCommitTransaction(const KDbTransaction& trans);

    /*! Internal, for handling autocommited transactions:
     Rollbacks transaction prevoiusly started with beginAutoCommitTransaction().
     @return true on success or when no transactions are supported
     at all by the driver.

     Special case when used database driver has only single transaction support
     (KDbDriver::SingleTransactions): @a trans will not be rolled back
     if it has been started outside this KDbConnection object.
    */
    bool rollbackAutoCommitTransaction(const KDbTransaction& trans);

    /*! Helper: checks if connection is established;
      if not: error message is set up and false returned */
    bool checkConnected();

    /*! Helper: checks both if connection is established and database any is used;
      if not: error message is set up and false returned */
    bool checkIsDatabaseUsed();

    /*! @return a full table schema for a table retrieved using 'kexi__*' system tables.
     Used internally by tableSchema() methods. */
    KDbTableSchema* setupTableSchema(const KDbRecordData& data);

    /*! @return a full query schema for a query using 'kexi__*' system tables.
     Used internally by querySchema() methods. */
    KDbQuerySchema* setupQuerySchema(const KDbRecordData& data);

    /*! Update a record. */
    bool updateRecord(KDbQuerySchema* query, KDbRecordData* data, KDbRecordEditBuffer* buf, bool useRecordId = false);
    /*! Insert a new record. */
    bool insertRecord(KDbQuerySchema* query, KDbRecordData* data, KDbRecordEditBuffer* buf, bool getRecordId = false);
    /*! Delete an existing record. */
    bool deleteRecord(KDbQuerySchema* query, KDbRecordData* data, bool useRecordId = false);
    /*! Delete all existing records. */
    bool deleteAllRecords(KDbQuerySchema* query);

    /*! Allocates all needed table KDb system objects for kexi__* KDb library's
     system tables schema.
     These objects are used internally in this connection
     and are added to list of tables (by name,
     not by id because these have no ids).
    */
    bool setupKDbSystemSchema();

    /*! used internally by setupKDbSystemSchema():
     Allocates single table KDb system object named @a tableName
     and adds this to list of such objects (for later removal on closeDatabase()).
    */
    KDbTableSchema* newKDbSystemTableSchema(const QString& tableName);

    /*! Called by KDbTableSchema -- signals destruction to KDbConnection object
     To avoid having deleted table object on its list. */
    void removeMe(KDbTableSchema *ts);

    /*! @internal
     @return true if the cursor @a cursor contains column @a column,
     else, sets appropriate error with a message and returns false. */
    bool checkIfColumnExists(KDbCursor *cursor, uint column);

    /*! @internal used by querySingleRecord() methods.
     Note: "LIMIT 1" is appended to @a sql statement if @a addLimitTo1 is true (the default). */
    tristate querySingleRecordInternal(KDbRecordData* data, const KDbEscapedString* sql,
                                       KDbQuerySchema* query, bool addLimitTo1 = true);

    /*! @internal used by KDbDriver::createConnection().
     Only works if connection is not yet established. */
    void setReadOnly(bool set);

    /*! Loads extended schema information for table @a tableSchema,
     if present (see ExtendedTableSchemaInformation in Kexi Wiki).
     @return true on success */
    bool loadExtendedTableSchemaData(KDbTableSchema* tableSchema);

    /*! Stores extended schema information for table @a tableSchema,
     (see ExtendedTableSchemaInformation in Kexi Wiki).
     The action is performed within the current transaction,
     so it's up to you to commit.
     Used, e.g. by createTable(), within its transaction.
     @return true on success */
    bool storeExtendedTableSchemaData(KDbTableSchema* tableSchema);

    /*! @internal
     Stores main field's schema information for field @a field.
     Used in table altering code when information in kexi__fields has to be updated.
     @return true on success and false on failure. */
    bool storeMainFieldSchema(KDbField *field);

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /*! This is a part of alter table interface implementing lower-level operations
     used to perform table schema altering. Used by KDbAlterTableHandler.

     Changes value of field property.
     @return true on success, false on failure, cancelled if the action has been cancelled.

     Note for driver developers: implement this if the driver has to supprot the altering. */
    virtual tristate drv_changeFieldProperty(KDbTableSchema* table, KDbField* field,
            const QString& propertyName, const QVariant& value) {
        Q_UNUSED(table); Q_UNUSED(field); Q_UNUSED(propertyName); Q_UNUSED(value);
        return cancelled;
    }

    //! Used by KDbCursor class
    void addCursor(KDbCursor* cursor);

    //! Used by KDbCursor class
    void takeCursor(KDbCursor* cursor);

private:
    //! Internal, used by storeObjectData(KDbObject*) and storeNewObjectData(KDbObject* object).
    bool storeObjectDataInternal(KDbObject* object, bool newObject);

    //! @internal
    //! @return identifier escaped by driver (if @a escapingType is KDb::DriverEscaping)
    //! or by the KDb's built-in escape routine.
    inline QString escapeIdentifier(const QString& id, KDb::IdentifierEscapingType escapingType) const {
        return escapingType == KDb::KDbEscaping
            ? KDb::escapeIdentifier(id) : escapeIdentifier(id);
    }

    ConnectionPrivate* d; //!< @internal d-pointer class.
    KDbDriver* const m_driver; //!< The driver this @a KDbConnection instance uses.
    bool m_destructor_started; //!< helper: true if destructor is started.
    bool m_insideCloseDatabase; //!< helper: true while closeDatabase() is executed

    friend class KDbDriver;
    friend class KDbCursor;
    friend class KDbTableSchema; //!< for removeMe()
    friend class KDbProperties; //!< for setError()
    friend class ConnectionPrivate;
    friend class KDbAlterTableHandler;
};

namespace KDb
{
/*! @return "SELECT ..." statement's string needed for executing query
    defined by @a querySchema, @a params and @a options. 
    @a driver is used to generate driver-dependent statement. */
KDB_EXPORT KDbEscapedString selectStatement(const KDbDriver &driver,
                                               KDbQuerySchema *querySchema,
                                               const QList<QVariant>& params,
                                               const KDbConnection::SelectStatementOptions& options
                                                = KDbConnection::SelectStatementOptions());

/*! @overload QString selectStatement(const KDbDriver&,
    KDbQuerySchema*, const QList<QVariant>&, const KDbConnection::SelectStatementOptions&); */
KDB_EXPORT inline KDbEscapedString selectStatement(const KDbDriver &driver,
                                                      KDbQuerySchema *querySchema,
                                                      const KDbConnection::SelectStatementOptions& options
                                                        = KDbConnection::SelectStatementOptions())
{
    return selectStatement(driver, querySchema, QList<QVariant>(), options);
}

/*! @return "SELECT ..." KDbSQL statement's string needed for executing query
    defined by @a querySchema, @a params and @a options. */
KDB_EXPORT KDbEscapedString selectStatement(KDbQuerySchema *querySchema,
                                               const QList<QVariant>& params,
                                               const KDbConnection::SelectStatementOptions& options
                                                 = KDbConnection::SelectStatementOptions());


/*! @overload QString selectStatement(KDbQuerySchema*, const QList<QVariant>&,
    const KDbConnection::SelectStatementOptions&); */
KDB_EXPORT inline KDbEscapedString selectStatement(KDbQuerySchema *querySchema,
                                                      const KDbConnection::SelectStatementOptions& options
                                                       = KDbConnection::SelectStatementOptions())
{
    return selectStatement(querySchema, QList<QVariant>(), options);
}

} // namespace KDb

#endif
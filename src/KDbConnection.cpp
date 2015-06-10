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

#include "KDbConnection.h"
#include "KDbConnection_p.h"
#include "KDbError.h"
#include "KDbExpression.h"
#include "KDbConnectionData.h"
#include "KDbDriver.h"
#include "KDbDriver_p.h"
#include "KDbDriverMetaData.h"
#include "KDbObject.h"
#include "KDbTableSchema.h"
#include "KDbRelationship.h"
#include "KDbTransaction.h"
#include "KDbCursor.h"
#include "KDbRecordEditBuffer.h"
#include "KDb.h"
#include "KDbProperties.h"
#include "KDbLookupFieldSchema.h"
#include "KDbPreparedStatementInterface.h"
#include "KDbParser.h"
#include "kdb_debug.h"

#include <QDir>
#include <QFileInfo>
#include <QDomDocument>

/*! Version number of extended table schema.

  List of changes:
  * 2: (Kexi 2.5.0) Added maxLengthIsDefault property (type: bool, if true, KDbField::maxLengthStrategy() == KDbField::DefaultMaxLength)
  * 1: (Kexi 1.x) Initial version
*/
#define KDB_EXTENDED_TABLE_SCHEMA_VERSION 2

KDbConnection::SelectStatementOptions::SelectStatementOptions()
        : alsoRetrieveRecordId(false)
        , addVisibleLookupColumns(true)
{
}

KDbConnection::SelectStatementOptions::~SelectStatementOptions()
{
}

//================================================

KDbConnectionInternal::KDbConnectionInternal(KDbConnection *conn)
        : connection(conn)
{
}

//================================================

KDbConnectionOptions::KDbConnectionOptions()
 : m_connection(0)
{
    KDbUtils::PropertySet::insert("readOnly", false, QObject::tr("Read only", "Read only connection"));
}

bool KDbConnectionOptions::isReadOnly() const
{
    return property("readOnly").value.toBool();
}

void KDbConnectionOptions::insert(const QByteArray &name, const QVariant &value,
                                  const QString &caption)
{
    if (name == "readOnly") {
        setReadOnly(value.toBool());
        return;
    }
    QString realCaption;
    if (property(name).caption.isEmpty()) { // don't allow to change the caption
        realCaption = caption;
    }
    KDbUtils::PropertySet::insert(name, value, realCaption);
}

void KDbConnectionOptions::setCaption(const QByteArray &name, const QString &caption)
{
    KDbUtils::PropertySet::insert(name, property(name).value, caption);
}

void KDbConnectionOptions::remove(const QByteArray &name)
{
    if (name == "readOnly") {
        return;
    }
    KDbUtils::PropertySet::remove(name);
}

void KDbConnectionOptions::setReadOnly(bool set)
{
    if (m_connection && m_connection->isConnected()) {
        return; //sanity
    }
    KDbUtils::PropertySet::insert("readOnly", set);
}

//================================================
//! @internal
class ConnectionPrivate
{
public:
    ConnectionPrivate(KDbConnection* const conn, const KDbConnectionData& _connData,
                      const KDbConnectionOptions &_options)
            : conn(conn)
            , connData(_connData)
            , options(_options)
            , m_parser(0)
            , dbProperties(conn)
            , dont_remove_transactions(false)
            , skip_databaseExists_check_in_useDatabase(false)
            , default_trans_started_inside(false)
            , isConnected(false)
            , autoCommit(true)
            , takeTableEnabled(true)
    {
        options.m_connection = conn;
    }

    ~ConnectionPrivate() {
        options.m_connection = 0;
        qDeleteAll(cursors);
        delete m_parser;
        qDeleteAll(tableSchemaChangeListeners);
        qDeleteAll(obsoleteQueries);
    }

    void errorInvalidDBContents(const QString& details) {
        conn->m_result = KDbResult(ERR_INVALID_DATABASE_CONTENTS, QObject::tr("Invalid database contents. %1").arg(details));
    }

    QString strItIsASystemObject() const {
        return QObject::tr("It is a system object.");
    }

    inline KDbParser *parser() {
        return m_parser ? m_parser : (m_parser = new KDbParser(conn));
    }

    inline KDbTableSchema* table(const QString& name) const {
        return tables_byname.value(name);
    }

    inline KDbTableSchema* table(int id) const {
        return tables.value(id);
    }

    //! used just for removing system KDbTableSchema objects on db close.
    inline QSet<KDbTableSchema*> kdbSystemTables() const {
        return _kdbSystemTables;
    }

    inline void insertTable(KDbTableSchema* tableSchema) {
        tables.insert(tableSchema->id(), tableSchema);
        tables_byname.insert(tableSchema->name(), tableSchema);
    }

    /*! @internal. Inserts internal table to KDbConnection's structures, so it can be found by name.
     Used by KDbConnection::insertInternalTable(KDbTableSchema*) */
    void insertInternalTable(KDbTableSchema* tableSchema) {
        tableSchema->setKDbSystem(true);
        _kdbSystemTables.insert(tableSchema);
        tables_byname.insert(tableSchema->name(), tableSchema);
    }

    /*! @internal Removes table schema pointed by tableSchema.id() and tableSchema.name()
     from internal structures and destroys it. Does not make any change at the backend.
     Note that the table schema being removed may be not the same as @a tableSchema. */
    void removeTable(const KDbTableSchema& tableSchema) {
        tables_byname.remove(tableSchema.name());
        KDbTableSchema *toDelete = tables.take(tableSchema.id());
        delete toDelete;
    }

    void takeTable(KDbTableSchema* tableSchema) {
        if (!takeTableEnabled)
            return;
        tables.take(tableSchema->id());
        tables_byname.take(tableSchema->name());
    }

    void renameTable(KDbTableSchema* tableSchema, const QString& newName) {
        tables_byname.take(tableSchema->name());
        tableSchema->setName(newName);
        tables_byname.insert(tableSchema->name(), tableSchema);
    }

    void changeTableId(KDbTableSchema* tableSchema, int newId) {
        tables.take(tableSchema->id());
        tables.insert(newId, tableSchema);
    }

    void clearTables() {
        tables_byname.clear();
        qDeleteAll(_kdbSystemTables);
        _kdbSystemTables.clear();
        takeTableEnabled = false; //!< needed because otherwise 'tables' hash will
        //!< be touched by takeTable() what's not allowed during qDeleteAll()
        qDeleteAll(tables);
        takeTableEnabled = true;
        tables.clear();
    }

    inline KDbQuerySchema* query(const QString& name) const {
        return queries_byname.value(name);
    }

    inline KDbQuerySchema* query(int id) const {
        return queries.value(id);
    }

    void insertQuery(KDbQuerySchema* query) {
        queries.insert(query->id(), query);
        queries_byname.insert(query->name(), query);
    }

    /*! @internal Removes @a querySchema from internal structures and
     destroys it. Does not make any change at the backend. */
    void removeQuery(KDbQuerySchema* querySchema) {
        queries_byname.remove(querySchema->name());
        queries.remove(querySchema->id());
        delete querySchema;
    }

    void setQueryObsolete(KDbQuerySchema* query) {
        obsoleteQueries.insert(query);
        queries_byname.take(query->name());
        queries.take(query->id());
    }

    void clearQueries() {
        qDeleteAll(queries);
        queries.clear();
    }

    KDbConnection* const conn; //!< The @a KDbConnection instance this @a ConnectionPrivate belongs to.
    KDbConnectionData connData; //!< the @a KDbConnectionData used within that connection.

    //! True for read only connection. Used especially for file-based drivers.
    KDbConnectionOptions options;

    /*! Default transaction handle.
    If transactions are supported: Any operation on database (e.g. inserts)
    that is started without specifying transaction context, will be performed
    in the context of this transaction. */
    KDbTransaction default_trans;
    QList<KDbTransaction> transactions;

    QHash<KDbTableSchema*, QSet<KDbConnection::TableSchemaChangeListenerInterface*>* > tableSchemaChangeListeners;

    //! Used in KDbConnection::setQuerySchemaObsolete( const QString& queryName )
    //! to collect obsolete queries. THese are deleted on connection deleting.
    QSet<KDbQuerySchema*> obsoleteQueries;

    //! server version information for this connection.
    KDbServerVersionInfo serverVersion;

    //! Database version information for this connection.
    KDbVersionInfo databaseVersion;

    KDbParser *m_parser;

    //! cursors created for this connection
    QSet<KDbCursor*> cursors;

    //! Database properties
    KDbProperties dbProperties;

    QString availableDatabaseName; //!< used by anyAvailableDatabaseName()
    QString usedDatabase; //!< database name that is opened now (the currentDatabase() name)

    //! true if rollbackTransaction() and commitTransaction() shouldn't remove
    //! the transaction object from 'transactions' list; used by closeDatabase()
    bool dont_remove_transactions;

    //! used to avoid endless recursion between useDatabase() and databaseExists()
    //! when useTemporaryDatabaseIfNeeded() works
    bool skip_databaseExists_check_in_useDatabase;

    /*! Used when single transactions are only supported (KDbDriver::SingleTransactions).
     True value means default KDbTransaction has been started inside connection object
     (by beginAutoCommitTransaction()), otherwise default transaction has been started outside
     of the object (e.g. before createTable()), so we shouldn't autocommit the transaction
     in commitAutoCommitTransaction(). Also, beginAutoCommitTransaction() doesn't restarts
     transaction if default_trans_started_inside is false. Such behaviour allows user to
     execute a sequence of actions like CREATE TABLE...; INSERT DATA...; within a single transaction
     and commit it or rollback by hand. */
    bool default_trans_started_inside;

    bool isConnected;

    bool autoCommit;

private:
    //! Table schemas retrieved on demand with tableSchema()
    QHash<int, KDbTableSchema*> tables;
    QHash<QString, KDbTableSchema*> tables_byname;
    //! used just for removing system KDbTableSchema objects on db close.
    QSet<KDbTableSchema*> _kdbSystemTables;
    //! Query schemas retrieved on demand with querySchema()
    QHash<int, KDbQuerySchema*> queries;
    QHash<QString, KDbQuerySchema*> queries_byname;
    bool takeTableEnabled; //!< used by takeTable() needed because otherwise 'tables' hash will
    //!< be touched by takeTable() what's not allowed during qDeleteAll()
};

//================================================

//! static: list of internal KDb system table names
QStringList KDb_kdbSystemTableNames;

KDbConnection::KDbConnection(KDbDriver *driver, const KDbConnectionData& connData,
                             const KDbConnectionOptions &options)
        : d(new ConnectionPrivate(this, connData, options))
        , m_driver(driver)
        , m_destructor_started(false)
        , m_insideCloseDatabase(false)
{
}

void KDbConnection::destroy()
{
    disconnect();
    //do not allow the driver to touch me: I will kill myself.
    m_driver->d->connections.remove(this);
}

KDbConnection::~KDbConnection()
{
    m_destructor_started = true;
    delete d;
    d = 0;
}

KDbConnectionData KDbConnection::data() const
{
    return d->connData;
}

bool KDbConnection::connect()
{
    clearResult();
    if (d->isConnected) {
        m_result = KDbResult(ERR_ALREADY_CONNECTED, QObject::tr("Connection already established."));
        return false;
    }
    if (m_driver->beh->USING_DATABASE_REQUIRED_TO_CONNECT && d->connData.databaseName().isEmpty()) {
        m_result = KDbResult(ERR_MISSING_DB_LOCATION,
                          QObject::tr("Database name required to create connection."));
        return 0;
    }

    d->serverVersion.clear();
    if (!(d->isConnected = drv_connect())) {
        m_result = KDbResult(m_driver->metaData()->isFileBased() ?
                    QObject::tr("Could not open \"%1\" project file.")
                    .arg(QDir::fromNativeSeparators(QFileInfo(d->connData.databaseName()).fileName()))
                 :  QObject::tr("Could not connect to \"%1\" database server.")
                    .arg(d->connData.toUserVisibleString()));
    }
    if (d->isConnected && !m_driver->beh->USING_DATABASE_REQUIRED_TO_CONNECT) {
        if (!drv_getServerVersion(&d->serverVersion))
            return false;
    }
    return d->isConnected;
}

bool KDbConnection::isDatabaseUsed() const
{
    return !d->usedDatabase.isEmpty() && d->isConnected && drv_isDatabaseUsed();
}

void KDbConnection::clearResult()
{
    KDbResultable::clearResult();
}

bool KDbConnection::disconnect()
{
    clearResult();
    if (!d->isConnected)
        return true;

    if (!closeDatabase())
        return false;

    bool ok = drv_disconnect();
    if (ok)
        d->isConnected = false;
    return ok;
}

bool KDbConnection::isConnected() const
{
    return d->isConnected;
}

bool KDbConnection::checkConnected()
{
    if (d->isConnected) {
        clearResult();
        return true;
    }
    m_result = KDbResult(ERR_NO_CONNECTION, QObject::tr("Not connected to the database server."));
    return false;
}

bool KDbConnection::checkIsDatabaseUsed()
{
    if (isDatabaseUsed()) {
        clearResult();
        return true;
    }
    m_result = KDbResult(ERR_NO_DB_USED, QObject::tr("Currently no database is used."));
    return false;
}

QStringList KDbConnection::databaseNames(bool also_system_db)
{
    //kdbDebug() << also_system_db;
    if (!checkConnected())
        return QStringList();

    QString tmpdbName;
    //some engines need to have opened any database before executing "create database"
    if (!useTemporaryDatabaseIfNeeded(&tmpdbName))
        return QStringList();

    QStringList list, non_system_list;

    bool ret = drv_getDatabasesList(&list);

    if (!tmpdbName.isEmpty()) {
        //whatever result is - now we have to close temporary opened database:
        if (!closeDatabase())
            return QStringList();
    }

    if (!ret)
        return QStringList();

    if (also_system_db)
        return list;
    //filter system databases:
    for (QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
        //kdbDebug() << *it;
        if (!m_driver->isSystemDatabaseName(*it)) {
            //kdbDebug() << "add " << *it;
            non_system_list << (*it);
        }
    }
    return non_system_list;
}

bool KDbConnection::drv_getDatabasesList(QStringList* list)
{
    list->clear();
    return true;
}

bool KDbConnection::drv_databaseExists(const QString &dbName, bool ignoreErrors)
{
    QStringList list = databaseNames(true);//also system
    if (m_result.isError()) {
        return false;
    }

    if (list.indexOf(dbName) == -1) {
        if (!ignoreErrors)
            m_result = KDbResult(ERR_OBJECT_NOT_FOUND, QObject::tr("The database \"%1\" does not exist.").arg(dbName));
        return false;
    }

    return true;
}

bool KDbConnection::databaseExists(const QString &dbName, bool ignoreErrors)
{
// kdbDebug() << dbName << ignoreErrors;
    if (m_driver->beh->CONNECTION_REQUIRED_TO_CHECK_DB_EXISTENCE && !checkConnected())
        return false;
    clearResult();

    if (m_driver->metaData()->isFileBased()) {
        //for file-based db: file must exists and be accessible
        QFileInfo file(d->connData.databaseName());
        if (!file.exists() || (!file.isFile() && !file.isSymLink())) {
            if (!ignoreErrors)
                m_result = KDbResult(ERR_OBJECT_NOT_FOUND, QObject::tr("The database file \"%1\" does not exist.")
                                                        .arg(QDir::fromNativeSeparators(QFileInfo(d->connData.databaseName()).fileName())));
            return false;
        }
        if (!file.isReadable()) {
            if (!ignoreErrors)
                m_result = KDbResult(ERR_ACCESS_RIGHTS, QObject::tr("Database file \"%1\" is not readable.")
                                                     .arg(QDir::fromNativeSeparators(QFileInfo(d->connData.databaseName()).fileName())));
            return false;
        }
        if (!d->options.isReadOnly() && !file.isWritable()) {
            if (!ignoreErrors)
                m_result = KDbResult(ERR_ACCESS_RIGHTS, QObject::tr("Database file \"%1\" is not writable.")
                                                     .arg(QDir::fromNativeSeparators(QFileInfo(d->connData.databaseName()).fileName())));
            return false;
        }
        return true;
    }

    QString tmpdbName;
    //some engines need to have opened any database before executing "create database"
    const bool orig_skip_databaseExists_check_in_useDatabase = d->skip_databaseExists_check_in_useDatabase;
    d->skip_databaseExists_check_in_useDatabase = true;
    bool ret = useTemporaryDatabaseIfNeeded(&tmpdbName);
    d->skip_databaseExists_check_in_useDatabase = orig_skip_databaseExists_check_in_useDatabase;
    if (!ret)
        return false;

    ret = drv_databaseExists(dbName, ignoreErrors);

    if (!tmpdbName.isEmpty()) {
        //whatever result is - now we have to close temporary opened database:
        if (!closeDatabase())
            return false;
    }

    return ret;
}

#define createDatabase_CLOSE \
    { if (!closeDatabase()) { \
            m_result = KDbResult(QObject::tr("Database \"%1\" created but could not be closed after creation.").arg(dbName)); \
            return false; \
        } }

#define createDatabase_ERROR \
    { createDatabase_CLOSE; return false; }


bool KDbConnection::createDatabase(const QString &dbName)
{
    if (m_driver->beh->CONNECTION_REQUIRED_TO_CREATE_DB && !checkConnected())
        return false;

    if (databaseExists(dbName)) {
        m_result = KDbResult(ERR_OBJECT_EXISTS, QObject::tr("Database \"%1\" already exists.").arg(dbName));
        return false;
    }
    if (m_driver->isSystemDatabaseName(dbName)) {
        m_result = KDbResult(ERR_SYSTEM_NAME_RESERVED,
                 QObject::tr("Cannot create database \"%1\". This name is reserved for system database.").arg(dbName));
        return false;
    }
    if (m_driver->metaData()->isFileBased()) {
        //update connection data if filename differs
        if (QFileInfo(dbName).isAbsolute()) {
            d->connData.setDatabaseName(dbName);
        }
        else {
            d->connData.setDatabaseName(
                QFileInfo(d->connData.databaseName()).absolutePath()
                + QDir::separator() +  QFileInfo(dbName).fileName());
        }
    }

    QString tmpdbName;
    //some engines need to have opened any database before executing "create database"
    if (!useTemporaryDatabaseIfNeeded(&tmpdbName))
        return false;

    //low-level create
    if (!drv_createDatabase(dbName)) {
        m_result.prependMessage(QObject::tr("Error creating database \"%1\" on the server.").arg(dbName));
        closeDatabase();//sanity
        return false;
    }

    if (!tmpdbName.isEmpty()) {
        //whatever result is - now we have to close temporary opened database:
        if (!closeDatabase())
            return false;
    }

    if (!tmpdbName.isEmpty() || !m_driver->d->isDBOpenedAfterCreate) {
        //db need to be opened
        if (!useDatabase(dbName, false/*not yet kexi compatible!*/)) {
            m_result = KDbResult(QObject::tr("Database \"%1\" created but could not be opened.").arg(dbName));
            return false;
        }
    } else {
        //just for the rule
        d->usedDatabase = dbName;
        d->isConnected = true;
    }

    KDbTransaction trans;
    if (m_driver->transactionsSupported()) {
        trans = beginTransaction();
        if (!trans.active())
            return false;
    }

    //-create system tables schema objects
    if (!setupKDbSystemSchema())
        return false;

    //-physically create system tables
    foreach(KDbTableSchema* t, d->kdbSystemTables()) {
        if (!drv_createTable(t->name()))
            createDatabase_ERROR;
    }

    //-insert KDb version info:
    KDbTableSchema *table = d->table(QLatin1String("kexi__db"));
    if (!table)
        createDatabase_ERROR;
    if (!insertRecord(table, QLatin1String("kdb_major_ver"), KDb::version().major())
            || !insertRecord(table, QLatin1String("kdb_minor_ver"), KDb::version().minor()))
        createDatabase_ERROR;

    if (trans.active() && !commitTransaction(trans))
        createDatabase_ERROR;

    createDatabase_CLOSE;
    return true;
}

#undef createDatabase_CLOSE
#undef createDatabase_ERROR

bool KDbConnection::useDatabase(const QString &dbName, bool kexiCompatible, bool *cancelled, KDbMessageHandler* msgHandler)
{
    if (cancelled)
        *cancelled = false;
    //kdbDebug() << dbName << kexiCompatible;
    if (!checkConnected())
        return false;

    QString my_dbName;
    if (dbName.isEmpty())
        my_dbName = d->connData.databaseName();
    else
        my_dbName = dbName;
    if (my_dbName.isEmpty())
        return false;

    if (d->usedDatabase == my_dbName)
        return true; //already used

    if (!d->skip_databaseExists_check_in_useDatabase) {
        if (!databaseExists(my_dbName, false /*don't ignore errors*/))
            return false; //database must exist
    }

    if (!d->usedDatabase.isEmpty() && !closeDatabase()) //close db if already used
        return false;

    d->usedDatabase.clear();

    if (!drv_useDatabase(my_dbName, cancelled, msgHandler)) {
        if (cancelled && *cancelled)
            return false;
        QString msg(QObject::tr("Opening database \"%1\" failed.").arg(my_dbName));
        m_result.prependMessage(msg);
        return false;
    }
    if (d->serverVersion.isNull() && m_driver->beh->USING_DATABASE_REQUIRED_TO_CONNECT) {
        // get version just now, it was not possible earlier
        if (!drv_getServerVersion(&d->serverVersion))
            return false;
    }

    //-create system tables schema objects
    if (!setupKDbSystemSchema())
        return false;

    if (kexiCompatible && my_dbName.compare(anyAvailableDatabaseName(), Qt::CaseInsensitive) != 0) {
        //-get global database information
        bool ok;
        const int major = d->dbProperties.value(QLatin1String("kdb_major_ver")).toInt(&ok);
        if (!ok) {
            m_result = d->dbProperties.result();
            return false;
        }
        const int minor = d->dbProperties.value(QLatin1String("kdb_minor_ver")).toInt(&ok);
        if (!ok) {
            m_result = d->dbProperties.result();
            return false;
        }
        d->databaseVersion.setMajor(major);
        d->databaseVersion.setMinor(minor);
    }
    d->usedDatabase = my_dbName;
    return true;
}

bool KDbConnection::closeDatabase()
{
    if (d->usedDatabase.isEmpty())
        return true; //no db used
    if (!checkConnected())
        return true;

    bool ret = true;

    /*! @todo (js) add CLEVER algorithm here for nested transactions */
    if (m_driver->transactionsSupported()) {
        //rollback all transactions
        d->dont_remove_transactions = true; //lock!
        foreach(const KDbTransaction& tr, d->transactions) {
            if (!rollbackTransaction(tr)) {//rollback as much as you can, don't stop on prev. errors
                ret = false;
            } else {
                kdbDebug() << "transaction rolled back!";
                kdbDebug() << "trans.refcount==" << (tr.m_data ? QString::number(tr.m_data->refcount) : QLatin1String("(null)"));
            }
        }
        d->dont_remove_transactions = false; //unlock!
        d->transactions.clear(); //free trans. data
    }

    m_insideCloseDatabase = true;

    //delete own cursors:
    qDeleteAll(d->cursors);
    d->cursors.clear();
    //delete own schemas
    d->clearTables();
    d->clearQueries();

    m_insideCloseDatabase = false;

    if (!drv_closeDatabase())
        return false;

    d->usedDatabase.clear();
    return ret;
}

QString KDbConnection::currentDatabase() const
{
    return d->usedDatabase;
}

bool KDbConnection::useTemporaryDatabaseIfNeeded(QString* name)
{
    if (m_driver->beh->USE_TEMPORARY_DATABASE_FOR_CONNECTION_IF_NEEDED && !isDatabaseUsed()) {
        //we have no db used, but it is required by engine to have used any!
        *name = anyAvailableDatabaseName();
        if (name->isEmpty()) {
            m_result = KDbResult(ERR_NO_DB_USED, QObject::tr("Cannot find any database for temporary connection."));
            return false;
        }
        const bool orig_skip_databaseExists_check_in_useDatabase = d->skip_databaseExists_check_in_useDatabase;
        d->skip_databaseExists_check_in_useDatabase = true;
        bool ret = useDatabase(*name, false);
        d->skip_databaseExists_check_in_useDatabase = orig_skip_databaseExists_check_in_useDatabase;
        if (!ret) {
            m_result = KDbResult(m_result.code(),
                              QObject::tr("Error during starting temporary connection using \"%1\" database name.")
                              .arg(*name));
            return false;
        }
    }
    return true;
}

bool KDbConnection::dropDatabase(const QString &dbName)
{
    if (m_driver->beh->CONNECTION_REQUIRED_TO_DROP_DB && !checkConnected())
        return false;

    QString dbToDrop;
    if (dbName.isEmpty() && d->usedDatabase.isEmpty()) {
        if (!m_driver->metaData()->isFileBased()
                || (m_driver->metaData()->isFileBased() && d->connData.databaseName().isEmpty()))
        {
            m_result = KDbResult(ERR_NO_NAME_SPECIFIED,
                              QObject::tr("Cannot delete database - name not specified."));
            return false;
        }
        //this is a file driver so reuse previously passed filename
        dbToDrop = d->connData.databaseName();
    } else {
        if (dbName.isEmpty()) {
            dbToDrop = d->usedDatabase;
        } else {
            if (m_driver->metaData()->isFileBased()) //lets get full path
                dbToDrop = QFileInfo(dbName).absoluteFilePath();
            else
                dbToDrop = dbName;
        }
    }

    if (dbToDrop.isEmpty()) {
        m_result = KDbResult(ERR_NO_NAME_SPECIFIED, QObject::tr("Cannot delete database - name not specified."));
        return false;
    }

    if (m_driver->isSystemDatabaseName(dbToDrop)) {
        m_result = KDbResult(ERR_SYSTEM_NAME_RESERVED, QObject::tr("Cannot delete system database \"%1\".").arg(dbToDrop));
        return false;
    }

    if (isDatabaseUsed() && d->usedDatabase == dbToDrop) {
        //we need to close database because cannot drop used this database
        if (!closeDatabase())
            return false;
    }

    QString tmpdbName;
    //some engines need to have opened any database before executing "drop database"
    if (!useTemporaryDatabaseIfNeeded(&tmpdbName))
        return false;

    //ok, now we have access to dropping
    bool ret = drv_dropDatabase(dbToDrop);

    if (!tmpdbName.isEmpty()) {
        //whatever result is - now we have to close temporary opened database:
        if (!closeDatabase())
            return false;
    }
    return ret;
}

QStringList KDbConnection::objectNames(int objectType, bool* ok)
{
    QStringList list;

    if (!checkIsDatabaseUsed()) {
        if (ok)
            *ok = false;
        return list;
    }

    KDbEscapedString sql;
    if (objectType == KDb::AnyObjectType)
        sql = "SELECT o_name FROM kexi__objects ORDER BY o_id";
    else
        sql = "SELECT o_name FROM kexi__objects WHERE o_type=" + QByteArray::number(objectType)
              + " ORDER BY o_id";

    KDbCursor *c = executeQuery(sql);
    if (!c) {
        if (ok)
            *ok = false;
        return list;
    }

    for (c->moveFirst(); !c->eof(); c->moveNext()) {
        QString name = c->value(0).toString();
        if (KDb::isIdentifier(name)) {
            list.append(name);
        }
    }

    if (!deleteCursor(c)) {
        if (ok)
            *ok = false;
        return list;
    }

    if (ok)
        *ok = true;
    return list;
}

QStringList KDbConnection::tableNames(bool also_system_tables)
{
    bool ok = true;
    QStringList list = objectNames(KDb::TableObjectType, &ok);
    if (also_system_tables && ok) {
        list += kdbSystemTableNames();
    }
    return list;
}

QStringList KDbConnection::kdbSystemTableNames()
{
    if (KDb_kdbSystemTableNames.isEmpty()) {
        KDb_kdbSystemTableNames
        << QLatin1String("kexi__objects")
        << QLatin1String("kexi__objectdata")
        << QLatin1String("kexi__fields")
        << QLatin1String("kexi__db")
        ;
    }
    return KDb_kdbSystemTableNames;
}

KDbServerVersionInfo KDbConnection::serverVersion() const
{
    return isConnected() ? d->serverVersion : KDbServerVersionInfo();
}

KDbVersionInfo KDbConnection::databaseVersion() const
{
    return isDatabaseUsed() ? d->databaseVersion : KDbVersionInfo();
}

KDbProperties KDbConnection::databaseProperties() const
{
    return d->dbProperties;
}

QList<int> KDbConnection::tableIds()
{
    return objectIds(KDb::TableObjectType);
}

QList<int> KDbConnection::queryIds()
{
    return objectIds(KDb::QueryObjectType);
}

QList<int> KDbConnection::objectIds(int objectType)
{
    QList<int> list;

    if (!checkIsDatabaseUsed())
        return list;

    KDbEscapedString sql;
    if (objectType == KDb::AnyObjectType)
        sql = "SELECT o_id, o_name FROM kexi__objects ORDER BY o_id";
    else
        sql = "SELECT o_id, o_name FROM kexi__objects WHERE o_type=" + QByteArray::number(objectType)
              + " ORDER BY o_id";

    KDbCursor *c = executeQuery(sql);
    if (!c)
        return list;
    for (c->moveFirst(); !c->eof(); c->moveNext()) {
        QString tname = c->value(1).toString(); //kexi__objects.o_name
        if (KDb::isIdentifier(tname)) {
            list.append(c->value(0).toInt()); //kexi__objects.o_id
        }
    }
    deleteCursor(c);
    return list;
}

KDbEscapedString KDbConnection::createTableStatement(const KDbTableSchema& tableSchema) const
{
// Each SQL identifier needs to be escaped in the generated query.
    KDbEscapedString sql;
    sql.reserve(4096);
    sql = KDbEscapedString("CREATE TABLE ") + escapeIdentifier(tableSchema.name()) + " (";
    bool first = true;
    foreach(KDbField *field, tableSchema.m_fields) {
        if (first)
            first = false;
        else
            sql += ", ";
        KDbEscapedString v = KDbEscapedString(escapeIdentifier(field->name())) + ' ';
        const bool autoinc = field->isAutoIncrement();
        const bool pk = field->isPrimaryKey() || (autoinc && m_driver->beh->AUTO_INCREMENT_REQUIRES_PK);
//! @todo warning: ^^^^^ this allows only one autonumber per table when AUTO_INCREMENT_REQUIRES_PK==true!
        if (autoinc && m_driver->beh->SPECIAL_AUTO_INCREMENT_DEF) {
            if (pk)
                v.append(m_driver->beh->AUTO_INCREMENT_TYPE).append(' ').append(m_driver->beh->AUTO_INCREMENT_PK_FIELD_OPTION);
            else
                v.append(m_driver->beh->AUTO_INCREMENT_TYPE).append(' ').append(m_driver->beh->AUTO_INCREMENT_FIELD_OPTION);
        } else {
            if (autoinc && !m_driver->beh->AUTO_INCREMENT_TYPE.isEmpty())
                v += m_driver->beh->AUTO_INCREMENT_TYPE;
            else
                v += m_driver->sqlTypeName(field->type(), field->precision());

            if (field->isUnsigned())
                v.append(' ').append(m_driver->beh->UNSIGNED_TYPE_KEYWORD);

            if (field->isFPNumericType() && field->precision() > 0) {
                if (field->scale() > 0)
                    v += QString::fromLatin1("(%1,%2)").arg(field->precision()).arg(field->scale());
                else
                    v += QString::fromLatin1("(%1)").arg(field->precision());
            }
            else if (field->type() == KDbField::Text) {
                uint realMaxLen;
                if (m_driver->beh->TEXT_TYPE_MAX_LENGTH == 0) {
                    realMaxLen = field->maxLength(); // allow to skip (N)
                }
                else { // max length specified by driver
                    if (field->maxLength() == 0) { // as long as possible
                        realMaxLen = m_driver->beh->TEXT_TYPE_MAX_LENGTH;
                    }
                    else { // not longer than specified by driver
                        realMaxLen = qMin(m_driver->beh->TEXT_TYPE_MAX_LENGTH, field->maxLength());
                    }
                }
                if (realMaxLen > 0) {
                    v += QString::fromLatin1("(%1)").arg(realMaxLen);
                }
            }

            if (autoinc) {
                v.append(' ').append(pk ? m_driver->beh->AUTO_INCREMENT_PK_FIELD_OPTION : m_driver->beh->AUTO_INCREMENT_FIELD_OPTION);
            }
            else {
                //! @todo here is automatically a single-field key created
                if (pk)
                    v += " PRIMARY KEY";
            }
            if (!pk && field->isUniqueKey())
                v += " UNIQUE";
///@todo IS this ok for all engines?: if (!autoinc && !field->isPrimaryKey() && field->isNotNull())
            if (!autoinc && !pk && field->isNotNull())
                v += " NOT NULL"; //only add not null option if no autocommit is set
            if (field->defaultValue().isValid()) {
                KDbEscapedString valToSQL(m_driver->valueToSQL(field, field->defaultValue()));
                if (!valToSQL.isEmpty()) //for sanity
                    v += " DEFAULT " + valToSQL;
            }
        }
        sql += v;
    }
    sql += ')';
    return sql;
}

//yeah, it is very efficient:
#define C_A(a) , const QVariant& c ## a

#define V_A0 m_driver->valueToSQL( tableSchema->field(0), c0 )
#define V_A(a) + ',' + m_driver->valueToSQL( \
        tableSchema->field(a) ? tableSchema->field(a)->type() : KDbField::Text, c ## a )

//  kdbDebug() << "******** " << QString("INSERT INTO ") +
//   escapeIdentifier(tableSchema->name()) +
//   " VALUES (" + vals + ")";

#define C_INS_REC(args, vals) \
    bool KDbConnection::insertRecord(KDbTableSchema* tableSchema args) {\
        if ( !drv_beforeInsert( tableSchema->name(), tableSchema ) )  \
            return false;                                      \
        \
        bool res = executeSQL(                                      \
                   KDbEscapedString("INSERT INTO ") + escapeIdentifier(tableSchema->name()) \
                       + " (" \
                       + tableSchema->sqlFieldsList(this) \
                       + ") VALUES (" + vals + ")" \
                   ); \
        \
        if ( !drv_afterInsert( tableSchema->name(), tableSchema ) ) \
            return false;                                      \
        \
        return res;                                             \
    }

#define C_INS_REC_ALL \
    C_INS_REC( C_A(0), V_A0 ) \
    C_INS_REC( C_A(0) C_A(1), V_A0 V_A(1) ) \
    C_INS_REC( C_A(0) C_A(1) C_A(2), V_A0 V_A(1) V_A(2) ) \
    C_INS_REC( C_A(0) C_A(1) C_A(2) C_A(3), V_A0 V_A(1) V_A(2) V_A(3) ) \
    C_INS_REC( C_A(0) C_A(1) C_A(2) C_A(3) C_A(4), V_A0 V_A(1) V_A(2) V_A(3) V_A(4) ) \
    C_INS_REC( C_A(0) C_A(1) C_A(2) C_A(3) C_A(4) C_A(5), V_A0 V_A(1) V_A(2) V_A(3) V_A(4) V_A(5) ) \
    C_INS_REC( C_A(0) C_A(1) C_A(2) C_A(3) C_A(4) C_A(5) C_A(6), V_A0 V_A(1) V_A(2) V_A(3) V_A(4) V_A(5) V_A(6) ) \
    C_INS_REC( C_A(0) C_A(1) C_A(2) C_A(3) C_A(4) C_A(5) C_A(6) C_A(7), V_A0 V_A(1) V_A(2) V_A(3) V_A(4) V_A(5) V_A(6) V_A(7) )

C_INS_REC_ALL

#undef V_A0
#undef V_A
#undef C_INS_REC

#define V_A0 value += m_driver->valueToSQL( it.next(), c0 );
#define V_A( a ) value += (',' + m_driver->valueToSQL( it.next(), c ## a ));

#define C_INS_REC(args, vals) \
    bool KDbConnection::insertRecord(KDbFieldList* fields args) \
    { \
        KDbEscapedString value; \
        const KDbField::List *flist = fields->fields(); \
        QListIterator<KDbField*> it(*flist); \
        vals \
        it.toFront(); \
        QString tableName((it.hasNext() && it.peekNext()->table()) ? it.next()->table()->name() : QLatin1String("??")); \
        if ( !drv_beforeInsert( tableName, fields ) )            \
            return false;                                       \
        bool res = executeSQL(                                  \
                       KDbEscapedString(QLatin1String("INSERT INTO ") + escapeIdentifier(tableName)) \
                       + " (" + fields->sqlFieldsList(this) \
                       + ") VALUES (" + value + ')' \
                   ); \
        if ( !drv_afterInsert( tableName, fields ) )    \
            return false;                               \
        return res;                             \
    }

C_INS_REC_ALL

#undef C_A
#undef V_A
#undef V_ALAST
#undef C_INS_REC
#undef C_INS_REC_ALL

bool KDbConnection::insertRecord(KDbTableSchema* tableSchema, const QList<QVariant>& values)
{
// Each SQL identifier needs to be escaped in the generated query.
    const KDbField::List *flist = tableSchema->fields();
    if (flist->isEmpty()) {
        return false;
    }
    KDbField::ListIterator fieldsIt(flist->constBegin());
    QList<QVariant>::ConstIterator it = values.constBegin();
    KDbEscapedString sql;
    sql.reserve(4096);
    while (fieldsIt != flist->constEnd() && (it != values.end())) {
        KDbField *f = *fieldsIt;
        if (sql.isEmpty()) {
            sql = KDbEscapedString("INSERT INTO ") + escapeIdentifier(tableSchema->name())
                  + " VALUES (";
        }
        else {
            sql += ',';
        }
        sql += m_driver->valueToSQL(f, *it);
//  kdbDebug() << "val" << i++ << ": " << m_driver->valueToSQL( f, *it );
        ++it;
        ++fieldsIt;
    }
    sql += ')';
    m_result.setSql(sql);

    if (!drv_beforeInsert(tableSchema->name(), tableSchema))
        return false;
    bool res = executeSQL(sql);
    if (!drv_afterInsert(tableSchema->name(), tableSchema))
        return false;

    return res;
}

bool KDbConnection::insertRecord(KDbFieldList* fields, const QList<QVariant>& values)
{
// Each SQL identifier needs to be escaped in the generated query.
    const KDbField::List *flist = fields->fields();
    if (flist->isEmpty()) {
        return false;
    }
    KDbField::ListIterator fieldsIt(flist->constBegin());
    KDbEscapedString sql;
    sql.reserve(4096);
    QList<QVariant>::ConstIterator it = values.constBegin();
    const QString tableName(flist->first()->table()->name());
    while (fieldsIt != flist->constEnd() && it != values.constEnd()) {
        KDbField *f = *fieldsIt;
        if (sql.isEmpty()) {
            sql = KDbEscapedString("INSERT INTO ") + escapeIdentifier(tableName) + '(' +
                  fields->sqlFieldsList(this) + ") VALUES (";
        }
        else {
            sql += ',';
        }
        sql += m_driver->valueToSQL(f, *it);
//  kdbDebug() << "val" << i++ << ": " << m_driver->valueToSQL( f, *it );
        ++it;
        ++fieldsIt;
        if (fieldsIt == flist->constEnd())
            break;
    }
    sql += ')';
    m_result.setSql(sql);

    if (!drv_beforeInsert(tableName, fields))
        return false;
    bool res = executeSQL(sql);
    if (!drv_afterInsert(tableName, fields))
        return false;

    return res;
}

bool KDbConnection::executeSQL(const KDbEscapedString& sql)
{
    if (!sql.isValid()) {
        m_result = KDbResult(ERR_SQL_EXECUTION_ERROR,
                          QObject::tr("SQL statement for execution is invalid (empty)."));
        return false;
    }
    m_result.setSql(sql); //remember for Error.handling
    if (!drv_executeSQL(sql)) {
        m_result.setMessage(QString()); //clear as this could be most probably just "Unknown error" string.
        m_result.setErrorSql(sql);
        m_result.prependMessage(ERR_SQL_EXECUTION_ERROR, QObject::tr("Error while executing SQL statement."));
        return false;
    }
    return true;
}

static KDbEscapedString selectStatementInternal(const KDbDriver *driver,
                                             KDbConnection *connection,
                                             KDbQuerySchema* querySchema,
                                             const QList<QVariant>& params,
                                             const KDbConnection::SelectStatementOptions& options)
{
//"SELECT FROM ..." is theoretically allowed "
//if (querySchema.fieldCount()<1)
//  return QString();
// Each SQL identifier needs to be escaped in the generated query.

    if (!querySchema->statement().isEmpty())
        return querySchema->statement();

//! @todo looking at singleTable is visually nice but a field name can conflict
//!   with function or variable name...
    uint number = 0;
    bool singleTable = querySchema->tables()->count() <= 1;
    if (singleTable) {
        //make sure we will have single table:
        foreach(KDbField *f, *querySchema->fields()) {
            if (querySchema->isColumnVisible(number) && f->table() && f->table()->lookupFieldSchema(*f)) {
                //uups, no, there's at least one left join
                singleTable = false;
                break;
            }
            number++;
        }
    }

    KDbEscapedString sql; //final sql string
    sql.reserve(4096);
    KDbEscapedString s_additional_joins; //additional joins needed for lookup fields
    KDbEscapedString s_additional_fields; //additional fields to append to the fields list
    uint internalUniqueTableAliasNumber = 0; //used to build internalUniqueTableAliases
    uint internalUniqueQueryAliasNumber = 0; //used to build internalUniqueQueryAliases
    number = 0;
    QList<KDbQuerySchema*> subqueries_for_lookup_data; // subqueries will be added to FROM section
    KDbEscapedString kdb_subquery_prefix("__kdb_subquery_");
    foreach(KDbField *f, *querySchema->fields()) {
        if (querySchema->isColumnVisible(number)) {
            if (!sql.isEmpty())
                sql += ", ";

            if (f->isQueryAsterisk()) {
                if (!singleTable && static_cast<KDbQueryAsterisk*>(f)->isSingleTableAsterisk()) { //single-table *
                    sql.append(KDb::escapeIdentifier(driver, f->table()->name())).append(".*");
                }
                else { //all-tables * (or simplified table.* when there's only one table)
                    sql += '*';
                }
            } else {
                if (f->isExpression()) {
                    sql += f->expression().toString();
                } else {
                    if (!f->table()) //sanity check
                        return KDbEscapedString();

                    QString tableName;
                    int tablePosition = querySchema->tableBoundToColumn(number);
                    if (tablePosition >= 0) {
                        tableName = KDb::iifNotEmpty(querySchema->tableAlias(tablePosition),
                                                           f->table()->name());
                    }
                    if (options.addVisibleLookupColumns) { // try to find table/alias name harder
                        if (tableName.isEmpty()) {
                            tableName = querySchema->tableAlias(f->table()->name());
                        }
                        if (tableName.isEmpty()) {
                            tableName = f->table()->name();
                        }
                    }
                    if (!singleTable && !tableName.isEmpty()) {
                        sql.append(KDb::escapeIdentifier(driver, tableName)).append('.');
                    }
                    sql += KDb::escapeIdentifier(driver, f->name());
                }
                const QString aliasString(querySchema->columnAlias(number));
                if (!aliasString.isEmpty()) {
                    sql.append(" AS ").append(aliasString);
                }
//! @todo add option that allows to omit "AS" keyword
            }
            KDbLookupFieldSchema *lookupFieldSchema = (options.addVisibleLookupColumns && f->table())
                                                   ? f->table()->lookupFieldSchema(*f) : 0;
            if (lookupFieldSchema && lookupFieldSchema->boundColumn() >= 0) {
                // Lookup field schema found
                // Now we also need to fetch "visible" value from the lookup table, not only the value of binding.
                // -> build LEFT OUTER JOIN clause for this purpose (LEFT, not INNER because the binding can be broken)
                // "LEFT OUTER JOIN lookupTable ON thisTable.thisField=lookupTable.boundField"
                KDbLookupFieldSchema::RecordSource recordSource = lookupFieldSchema->recordSource();
                if (recordSource.type() == KDbLookupFieldSchema::RecordSource::Table) {
                    KDbTableSchema *lookupTable = querySchema->connection()->tableSchema(recordSource.name());
                    KDbFieldList* visibleColumns = 0;
                    KDbField *boundField = 0;
                    if (lookupTable
                            && (uint)lookupFieldSchema->boundColumn() < lookupTable->fieldCount()
                            && (visibleColumns = lookupTable->subList(lookupFieldSchema->visibleColumns()))
                            && (boundField = lookupTable->field(lookupFieldSchema->boundColumn()))) {
                        //add LEFT OUTER JOIN
                        if (!s_additional_joins.isEmpty())
                            s_additional_joins += ' ';
                        const QString internalUniqueTableAlias(
                            QLatin1String("__kdb_") + lookupTable->name() + QLatin1Char('_')
                            + QString::number(internalUniqueTableAliasNumber++));
                        s_additional_joins += KDbEscapedString("LEFT OUTER JOIN %1 AS %2 ON %3.%4=%5.%6")
                            .arg(KDb::escapeIdentifier(driver, lookupTable->name()))
                            .arg(internalUniqueTableAlias)
                            .arg(KDb::escapeIdentifier(driver, querySchema->tableAliasOrName(f->table()->name())))
                            .arg(KDb::escapeIdentifier(driver, f->name()))
                            .arg(internalUniqueTableAlias)
                            .arg(KDb::escapeIdentifier(driver, boundField->name()));

                        //add visibleField to the list of SELECTed fields //if it is not yet present there
                        if (!s_additional_fields.isEmpty())
                            s_additional_fields += ", ";
//! @todo Add lookup schema option for separator other than ' ' or even option for placeholders like "Name ? ?"
//! @todo Add possibility for joining the values at client side.
                        s_additional_fields += visibleColumns->sqlFieldsList(
                                                   connection, QLatin1String(" || ' ' || "), internalUniqueTableAlias,
                                                   driver ? KDb::DriverEscaping : KDb::KDbEscaping);
                    }
                    delete visibleColumns;
                } else if (recordSource.type() == KDbLookupFieldSchema::RecordSource::Query) {
                    KDbQuerySchema *lookupQuery = querySchema->connection()->querySchema(recordSource.name());
                    if (!lookupQuery) {
                        kdbWarning() << "!lookupQuery";
                        return KDbEscapedString();
                    }
                    const KDbQueryColumnInfo::Vector fieldsExpanded(lookupQuery->fieldsExpanded());
                    if (lookupFieldSchema->boundColumn() >= fieldsExpanded.count()) {
                        kdbWarning() << "(uint)lookupFieldSchema->boundColumn() >= fieldsExpanded.count()";
                        return KDbEscapedString();
                    }
                    KDbQueryColumnInfo *boundColumnInfo = fieldsExpanded.at(lookupFieldSchema->boundColumn());
                    if (!boundColumnInfo) {
                        kdbWarning() << "!boundColumnInfo";
                        return KDbEscapedString();
                    }
                    KDbField *boundField = boundColumnInfo->field;
                    if (!boundField) {
                        kdbWarning() << "!boundField";
                        return KDbEscapedString();
                    }
                    //add LEFT OUTER JOIN
                    if (!s_additional_joins.isEmpty())
                        s_additional_joins += ' ';
                    KDbEscapedString internalUniqueQueryAlias
                        = kdb_subquery_prefix + connection->escapeString(lookupQuery->name()) + '_'
                        + QString::number(internalUniqueQueryAliasNumber++);
                    s_additional_joins += KDbEscapedString("LEFT OUTER JOIN (%1) AS %2 ON %3.%4=%5.%6")
                        .arg(KDb::selectStatement(lookupQuery, params, options))
                        .arg((internalUniqueQueryAlias))
                        .arg(KDb::escapeIdentifier(driver, f->table()->name()))
                        .arg(KDb::escapeIdentifier(driver, f->name()))
                        .arg(internalUniqueQueryAlias)
                        .arg(KDb::escapeIdentifier(driver, boundColumnInfo->aliasOrName()));

                    if (!s_additional_fields.isEmpty())
                        s_additional_fields += ", ";
                    const QList<uint> visibleColumns(lookupFieldSchema->visibleColumns());
                    KDbEscapedString expression;
                    foreach(uint visibleColumnIndex, visibleColumns) {
//! @todo Add lookup schema option for separator other than ' ' or even option for placeholders like "Name ? ?"
//! @todo Add possibility for joining the values at client side.
                        if ((uint)fieldsExpanded.count() <= visibleColumnIndex) {
                            kdbWarning() << "fieldsExpanded.count() <= (*visibleColumnsIt) : "
                            << fieldsExpanded.count() << " <= " << visibleColumnIndex;
                            return KDbEscapedString();
                        }
                        if (!expression.isEmpty())
                            expression += " || ' ' || ";
                        expression += (
                            internalUniqueQueryAlias + '.'
                            + KDb::escapeIdentifier(driver, fieldsExpanded.value(visibleColumnIndex)->aliasOrName())
                        );
                    }
                    s_additional_fields += expression;
                }
                else {
                    kdbWarning() << "unsupported record source type" << recordSource.typeName();
                    return KDbEscapedString();
                }
            }
        }
        number++;
    }

    //add lookup fields
    if (!s_additional_fields.isEmpty())
        sql += (", " + s_additional_fields);

    if (driver && options.alsoRetrieveRecordId) { //append rowid column
        KDbEscapedString s;
        if (!sql.isEmpty())
            s = ", ";
        if (querySchema->masterTable())
            s += KDbEscapedString(querySchema->tableAliasOrName(querySchema->masterTable()->name())) + '.';
        s += driver->behaviour()->ROW_ID_FIELD_NAME;
        sql += s;
    }

    sql.prepend("SELECT ");
    QList<KDbTableSchema*>* tables = querySchema->tables();
    if ((tables && !tables->isEmpty()) || !subqueries_for_lookup_data.isEmpty()) {
        sql += " FROM ";
        KDbEscapedString s_from;
        if (tables) {
            number = 0;
            foreach(KDbTableSchema *table, *tables) {
                if (!s_from.isEmpty())
                    s_from += ", ";
                s_from += KDb::escapeIdentifier(driver, table->name());
                const QString aliasString(querySchema->tableAlias(number));
                if (!aliasString.isEmpty())
                    s_from.append(" AS ").append(aliasString);
                number++;
            }
        }
        // add subqueries for lookup data
        uint subqueries_for_lookup_data_counter = 0;
        foreach(KDbQuerySchema* subQuery, subqueries_for_lookup_data) {
            if (!s_from.isEmpty())
                s_from += ", ";
            s_from += '('
                      + selectStatementInternal(driver, connection, subQuery, params, options);
                      + ") AS " + kdb_subquery_prefix
                      + KDbEscapedString::number(subqueries_for_lookup_data_counter++);
        }
        sql += s_from;
    }
    KDbEscapedString s_where;
    s_where.reserve(4096);

    //JOINS
    if (!s_additional_joins.isEmpty()) {
        sql += ' ' + s_additional_joins + ' ';
    }

//! @todo: we're using WHERE for joins now; use INNER/LEFT/RIGHT JOIN later

    //WHERE
    bool wasWhere = false; //for later use
    foreach(KDbRelationship *rel, *querySchema->relationships()) {
        if (s_where.isEmpty()) {
            wasWhere = true;
        } else
            s_where += " AND ";
        KDbEscapedString s_where_sub;
        foreach(const KDbField::Pair &pair, *rel->fieldPairs()) {
            if (!s_where_sub.isEmpty())
                s_where_sub += " AND ";
            s_where_sub +=
               KDbEscapedString(KDb::escapeIdentifier(driver, pair.first->table()->name())) + '.' +
               KDb::escapeIdentifier(driver, pair.first->name()) + " = " +
               KDb::escapeIdentifier(driver, pair.second->table()->name()) + '.' +
               KDb::escapeIdentifier(driver, pair.second->name());
        }
        if (rel->fieldPairs()->count() > 1) {
            s_where_sub.prepend('(');
            s_where_sub += ')';
        }
        s_where += s_where_sub;
    }
    //EXPLICITLY SPECIFIED WHERE EXPRESSION
    if (driver && !querySchema->whereExpression().isNull()) {
        KDbQuerySchemaParameterValueListIterator paramValuesIt(*driver, params);
        KDbQuerySchemaParameterValueListIterator *paramValuesItPtr = params.isEmpty() ? 0 : &paramValuesIt;
        if (wasWhere) {
//! @todo () are not always needed
            s_where = '(' + s_where + ") AND ("
                + querySchema->whereExpression().toString(paramValuesItPtr) + ')';
        } else {
            s_where = querySchema->whereExpression().toString(paramValuesItPtr);
        }
    }
    if (!s_where.isEmpty())
        sql += " WHERE " + s_where;
//! @todo (js) add other sql parts
    //(use wasWhere here)

    // ORDER BY
    KDbEscapedString orderByString(
        querySchema->orderByColumnList()->toSQLString(
            !singleTable/*includeTableName*/, connection, driver ? KDb::DriverEscaping : KDb::KDbEscaping)
    );
    const QVector<int> pkeyFieldsOrder(querySchema->pkeyFieldsOrder());
    if (orderByString.isEmpty() && !pkeyFieldsOrder.isEmpty()) {
        //add automatic ORDER BY if there is no explicitly defined (especially helps when there are complex JOINs)
        KDbOrderByColumnList automaticPKOrderBy;
        const KDbQueryColumnInfo::Vector fieldsExpanded(querySchema->fieldsExpanded());
        foreach(int pkeyFieldsIndex, pkeyFieldsOrder) {
            if (pkeyFieldsIndex < 0) // no field mentioned in this query
                continue;
            if (pkeyFieldsIndex >= (int)fieldsExpanded.count()) {
                kdbWarning() << "ORDER BY: (*it) >= fieldsExpanded.count() - "
                        << pkeyFieldsIndex << " >= " << fieldsExpanded.count();
                continue;
            }
            KDbQueryColumnInfo *ci = fieldsExpanded[ pkeyFieldsIndex ];
            automaticPKOrderBy.appendColumn(*ci);
        }
        orderByString = automaticPKOrderBy.toSQLString(!singleTable/*includeTableName*/,
                        connection, driver ? KDb::DriverEscaping : KDb::KDbEscaping);
    }
    if (!orderByString.isEmpty())
        sql += (" ORDER BY " + orderByString);

    //kdbDebug() << sql;
    return sql;
}

KDbEscapedString KDb::selectStatement(const KDbDriver &driver,
                                         KDbQuerySchema* querySchema,
                                         const QList<QVariant>& params,
                                         const KDbConnection::SelectStatementOptions& options)
{
    return selectStatementInternal(&driver, 0, querySchema, params, options);
}

KDbEscapedString KDb::selectStatement(KDbQuerySchema* querySchema,
                                         const QList<QVariant>& params,
                                         const KDbConnection::SelectStatementOptions& options)
{
    return selectStatementInternal(0, 0, querySchema, params, options);
}

KDbEscapedString KDbConnection::selectStatement(KDbQuerySchema* querySchema,
                                    const QList<QVariant>& params,
                                    const SelectStatementOptions& options)
{
    return selectStatementInternal(driver(), this, querySchema, params, options);
}

KDbEscapedString KDbConnection::selectStatement(KDbTableSchema* tableSchema,
                                          const SelectStatementOptions& options)
{
    return selectStatement(tableSchema->query(), options);
}

KDbField* KDbConnection::findSystemFieldName(const KDbFieldList& fieldlist)
{
    for (KDbField::ListIterator it(fieldlist.fieldsIterator()); it != fieldlist.fieldsIteratorConstEnd(); ++it) {
        if (m_driver->isSystemFieldName((*it)->name()))
            return *it;
    }
    return 0;
}

quint64 KDbConnection::lastInsertedAutoIncValue(const QString& aiFieldName, const QString& tableName,
        quint64* recordId)
{
    const quint64 foundRecordId = drv_lastInsertRecordId();
    if (recordId)
        *recordId = foundRecordId;
    if (m_driver->beh->ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE) {
        return foundRecordId;
    }
    KDbRecordData rdata;
    if (foundRecordId <= 0 || true != querySingleRecord(
                  KDbEscapedString("SELECT ") + tableName + '.' + aiFieldName
                + " FROM " + tableName
                + " WHERE " + m_driver->beh->ROW_ID_FIELD_NAME
                + '=' + KDbEscapedString::number(foundRecordId), &rdata))
    {
//  kdbDebug() << "foundRecordId<=0 || true!=querySingleRecord()";
        return (quint64) - 1; //ULL;
    }
    return rdata[0].toULongLong();
}

quint64 KDbConnection::lastInsertedAutoIncValue(const QString& aiFieldName,
        const KDbTableSchema& table, quint64* recordId)
{
    return lastInsertedAutoIncValue(aiFieldName, table.name(), recordId);
}

//! Creates a KDbField list for kexi__fields, for sanity. Used by createTable()
static KDbFieldList* createFieldListForKexi__Fields(KDbTableSchema *kexi__fieldsSchema)
{
    if (!kexi__fieldsSchema)
        return 0;
    return kexi__fieldsSchema->subList(
               QList<QByteArray>()
               << "t_id"
               << "f_type"
               << "f_name"
               << "f_length"
               << "f_precision"
               << "f_constraints"
               << "f_options"
               << "f_default"
               << "f_order"
               << "f_caption"
               << "f_help"
           );
}

static QVariant buildLengthValue(const KDbField &f)
{
    if (f.isFPNumericType()) {
        return f.scale();
    }
    return f.maxLength();
}

//! builds a list of values for field's @a f properties. Used by createTable().
void buildValuesForKexi__Fields(QList<QVariant>& vals, KDbField* f)
{
    vals.clear();
    vals
    << QVariant(f->table()->id())
    << QVariant(f->type())
    << QVariant(f->name())
    << buildLengthValue(*f)
    << QVariant(f->isFPNumericType() ? f->precision() : 0)
    << QVariant(f->constraints())
    << QVariant(f->options())
    // KDb::variantToString() is needed here because the value can be of any QVariant type,
    // depending on f->type()
    << (f->defaultValue().isNull()
        ? QVariant() : QVariant(KDb::variantToString(f->defaultValue())))
    << QVariant(f->order())
    << QVariant(f->caption())
    << QVariant(f->description());
}

bool KDbConnection::storeMainFieldSchema(KDbField *field)
{
    if (!field || !field->table())
        return false;
    KDbFieldList *fl = createFieldListForKexi__Fields(d->table(QLatin1String("kexi__fields")));
    if (!fl)
        return false;

    QList<QVariant> vals;
    buildValuesForKexi__Fields(vals, field);
    QList<QVariant>::ConstIterator valsIt = vals.constBegin();
    bool first = true;
    KDbEscapedString sql("UPDATE kexi__fields SET ");
    foreach(KDbField *f, *fl->fields()) {
        sql.append((first ? QString() : QLatin1String(", ")) +
                   f->name() + QLatin1Char('=') + m_driver->valueToSQL(f, *valsIt));
        if (first)
            first = false;
        ++valsIt;
    }
    delete fl;

    sql.append(KDbEscapedString(" WHERE t_id=%1 AND f_name=%2")
                .arg(KDbEscapedString::number(field->table()->id()),
                     m_driver->valueToSQL(KDbField::Text, field->name())));
    return executeSQL(sql);
}

#define createTable_ERR \
    { kdbDebug() << "ERROR!"; \
        m_result.prependMessage(QObject::tr("Creating table failed.")); \
        rollbackAutoCommitTransaction(tg.transaction()); \
        return false; }

bool KDbConnection::createTable(KDbTableSchema* tableSchema, bool replaceExisting)
{
    if (!tableSchema || !checkIsDatabaseUsed())
        return false;

    //check if there are any fields
    if (tableSchema->fieldCount() < 1) {
        clearResult();
        m_result = KDbResult(ERR_CANNOT_CREATE_EMPTY_OBJECT, QObject::tr("Cannot create table without fields."));
        return false;
    }
    const bool internalTable = dynamic_cast<KDbInternalTableSchema*>(tableSchema);

    const QString tableName(tableSchema->name());

    if (!internalTable) {
        if (m_driver->isSystemObjectName(tableName)) {
            clearResult();
            m_result = KDbResult(ERR_SYSTEM_NAME_RESERVED,
                              QObject::tr("System name \"%1\" cannot be used as table name.")
                              .arg(tableSchema->name()));
            return false;
        }

        KDbField *sys_field = findSystemFieldName(*tableSchema);
        if (sys_field) {
            clearResult();
            m_result = KDbResult(ERR_SYSTEM_NAME_RESERVED,
                              QObject::tr("System name \"%1\" cannot be used as one of fields in \"%2\" table.")
                              .arg(sys_field->name(), tableName));
            return false;
        }
    }

    bool previousSchemaStillKept = false;

    KDbTableSchema *existingTable = 0;
    if (replaceExisting) {
        //get previous table (do not retrieve, though)
        existingTable = this->tableSchema(tableName);
        if (existingTable) {
            if (existingTable == tableSchema) {
                clearResult();
                m_result = KDbResult(ERR_OBJECT_EXISTS,
                                  QObject::tr("Could not create the same table \"%1\" twice.").arg(tableSchema->name()));
                return false;
            }
//! @todo (js) update any structure (e.g. queries) that depend on this table!
            if (existingTable->id() > 0)
                tableSchema->setId(existingTable->id()); //copy id from existing table
            previousSchemaStillKept = true;
            if (!dropTable(existingTable, false /*alsoRemoveSchema*/))
                return false;
        }
    } else {
        if (this->tableSchema(tableSchema->name()) != 0) {
            clearResult();
            m_result = KDbResult(ERR_OBJECT_EXISTS, QObject::tr("Table \"%1\" already exists.").arg(tableSchema->name()));
            return false;
        }
    }
    KDbTransactionGuard tg;
    if (!beginAutoCommitTransaction(&tg))
        return false;

    if (!drv_createTable(*tableSchema))
        createTable_ERR;

    //add schema data to kexi__* tables
    if (!internalTable) {
        //update kexi__objects
        if (!storeNewObjectData(tableSchema))
            createTable_ERR;

        KDbTableSchema *ts = d->table(QLatin1String("kexi__fields"));
        if (!ts)
            return false;
        //for sanity: remove field info (if any) for this table id
        if (!KDb::deleteRecord(this, ts, QLatin1String("t_id"), tableSchema->id()))
            return false;

        KDbFieldList *fl = createFieldListForKexi__Fields(ts);
        if (!fl)
            return false;

        foreach(KDbField *f, *tableSchema->fields()) {
            QList<QVariant> vals;
            buildValuesForKexi__Fields(vals, f);
            if (!insertRecord(fl, vals))
                createTable_ERR;
        }
        delete fl;

        if (!storeExtendedTableSchemaData(tableSchema))
            createTable_ERR;
    }
    bool res = commitAutoCommitTransaction(tg.transaction());
    if (res) {
        if (internalTable) {
            //insert the internal table into structures
            insertInternalTable(tableSchema);
        } else {
            if (previousSchemaStillKept) {
                //remove previous table schema
                d->removeTable(*tableSchema);
            }
            //store one schema object locally:
            d->insertTable(tableSchema);
        }
        //ok, this table is not created by the connection
        tableSchema->m_conn = this;
    }
    return res;
}

KDbTableSchema *KDbConnection::copyTable(const KDbTableSchema &tableSchema, const KDbObject &newData)
{
    clearResult();
    if (this->tableSchema(tableSchema.name()) != &tableSchema) {
        m_result = KDbResult(ERR_OBJECT_NOT_FOUND,
                          QObject::tr("Table \"%1\" does not exist.").arg(tableSchema.name()));
        return 0;
    }
    KDbTableSchema *copiedTable = new KDbTableSchema(tableSchema, false /* !copyId*/);
    // copy name, caption, description
    copiedTable->setName(newData.name());
    copiedTable->setCaption(newData.caption());
    copiedTable->setDescription(newData.description());
    // copy the structure and data
    if (!createTable(copiedTable, false /* !replaceExisting */)) {
        delete copiedTable;
        return 0;
    }
    if (!drv_copyTableData(tableSchema, *copiedTable)) {
        dropTable(copiedTable);
        delete copiedTable;
        return 0;
    }
    return copiedTable;
}

KDbTableSchema *KDbConnection::copyTable(const QString &tableName, const KDbObject &newData)
{
    clearResult();
    KDbTableSchema* ts = tableSchema(tableName);
    if (!ts) {
        m_result = KDbResult(ERR_OBJECT_NOT_FOUND,
                          QObject::tr("Table \"%1\" does not exist.").arg(tableName));
        return 0;
    }
    return copyTable(*ts, newData);
}

bool KDbConnection::drv_copyTableData(const KDbTableSchema &tableSchema,
                                   const KDbTableSchema &destinationTableSchema)
{
    KDbEscapedString sql = KDbEscapedString("INSERT INTO %1 SELECT * FROM %2")
                .arg(escapeIdentifier(destinationTableSchema.name()))
                .arg(escapeIdentifier(tableSchema.name()));
    return executeSQL(sql);
}

bool KDbConnection::removeObject(uint objId)
{
    clearResult();
    //remove table schema from kexi__* tables
    if (   !KDb::deleteRecord(this, d->table(QLatin1String("kexi__objects")), QLatin1String("o_id"), objId) //schema entry
        || !KDb::deleteRecord(this, d->table(QLatin1String("kexi__objectdata")), QLatin1String("o_id"), objId)) //data blocks
    {
        m_result = KDbResult(ERR_DELETE_SERVER_ERROR, QObject::tr("Could not remove object's data."));
        return false;
    }
    return true;
}

bool KDbConnection::drv_dropTable(const QString& tableName)
{
    return executeSQL(KDbEscapedString("DROP TABLE ") + escapeIdentifier(tableName));
}

tristate KDbConnection::dropTable(KDbTableSchema* tableSchema)
{
    return dropTable(tableSchema, true);
}

tristate KDbConnection::dropTable(KDbTableSchema* tableSchema, bool alsoRemoveSchema)
{
    // Each SQL identifier needs to be escaped in the generated query.
    clearResult();
    if (!tableSchema)
        return false;

    QString errmsg = QObject::tr("Table \"%1\" cannot be removed.\n");
    //be sure that we handle the correct KDbTableSchema object:
    if (tableSchema->id() < 0
            || this->tableSchema(tableSchema->name()) != tableSchema
            || this->tableSchema(tableSchema->id()) != tableSchema) {
        m_result = KDbResult(ERR_OBJECT_NOT_FOUND, errmsg.arg(tableSchema->name())
                          + QObject::tr("Unexpected name or identifier."));
        return false;
    }

    tristate res = closeAllTableSchemaChangeListeners(tableSchema);
    if (true != res)
        return res;

    //sanity checks:
    if (m_driver->isSystemObjectName(tableSchema->name())) {
        m_result = KDbResult(ERR_SYSTEM_NAME_RESERVED, errmsg.arg(tableSchema->name())
                          + d->strItIsASystemObject());
        return false;
    }

    KDbTransactionGuard tg;
    if (!beginAutoCommitTransaction(&tg))
        return false;

    //for sanity we're checking if this table exists physically
    if (drv_containsTable(tableSchema->name())) {
        if (!drv_dropTable(tableSchema->name()))
            return false;
    }

    KDbTableSchema *ts = d->table(QLatin1String("kexi__fields"));
    if (!KDb::deleteRecord(this, ts, QLatin1String("t_id"), tableSchema->id())) //field entries
        return false;

    //remove table schema from kexi__objects table
    if (!removeObject(tableSchema->id())) {
        return false;
    }

    if (alsoRemoveSchema) {
//! @todo js: update any structure (e.g. queries) that depend on this table!
        tristate res = removeDataBlock(tableSchema->id(), QLatin1String("extended_schema"));
        if (!res)
            return false;
        d->removeTable(*tableSchema);
    }
    return commitAutoCommitTransaction(tg.transaction());
}

tristate KDbConnection::dropTable(const QString& tableName)
{
    clearResult();
    KDbTableSchema* ts = tableSchema(tableName);
    if (!ts) {
        m_result = KDbResult(ERR_OBJECT_NOT_FOUND, QObject::tr("Table \"%1\" does not exist.")
                                                .arg(tableName));
        return false;
    }
    return dropTable(ts);
}

tristate KDbConnection::alterTable(KDbTableSchema* tableSchema, KDbTableSchema* newTableSchema)
{
    clearResult();
    tristate res = closeAllTableSchemaChangeListeners(tableSchema);
    if (true != res)
        return res;

    if (tableSchema == newTableSchema) {
        m_result = KDbResult(ERR_OBJECT_THE_SAME, QObject::tr("Could not alter table \"%1\" using the same table as destination.")
                                               .arg(tableSchema->name()));
        return false;
    }
//! @todo (js) implement real altering
//! @todo (js) update any structure (e.g. query) that depend on this table!
    bool ok, empty;
#if 0 //! @todo uncomment:
    empty = isEmpty(tableSchema, ok) && ok;
#else
    empty = true;
#endif
    if (empty) {
        ok = createTable(newTableSchema, true/*replace*/);
    }
    return ok;
}

bool KDbConnection::alterTableName(KDbTableSchema* tableSchema, const QString& newName, bool replace)
{
    clearResult();
    if (tableSchema != this->tableSchema(tableSchema->id())) {
        m_result = KDbResult(ERR_OBJECT_NOT_FOUND, QObject::tr("Unknown table \"%1\".").arg(tableSchema->name()));
        return false;
    }
    if (newName.isEmpty() || !KDb::isIdentifier(newName)) {
        m_result = KDbResult(ERR_INVALID_IDENTIFIER, QObject::tr("Invalid table name \"%1\".").arg(newName));
        return false;
    }
    const QString oldTableName = tableSchema->name();
    const QString newTableName = newName.trimmed();
    if (oldTableName.trimmed() == newTableName) {
        m_result = KDbResult(ERR_OBJECT_THE_SAME, QObject::tr("Could not rename table \"%1\" using the same name.")
                                               .arg(newTableName));
        return false;
    }
//! @todo alter table name for server DB backends!
//! @todo what about objects (queries/forms) that use old name?
    KDbTableSchema *tableToReplace = this->tableSchema(newName);
    const bool destTableExists = tableToReplace != 0;
    const int origID = destTableExists ? tableToReplace->id() : -1; //will be reused in the new table
    if (!replace && destTableExists) {
        m_result = KDbResult(ERR_OBJECT_EXISTS,
                          QObject::tr("Could not rename table \"%1\" to \"%2\". Table \"%3\" already exists.")
                          .arg(tableSchema->name(), newName, newName));
        return false;
    }

//helper:
#define alterTableName_ERR \
    tableSchema->setName(oldTableName) //restore old name

    KDbTransactionGuard tg;
    if (!beginAutoCommitTransaction(&tg))
        return false;

    // drop the table replaced (with schema)
    if (destTableExists) {
        if (!replace) {
            return false;
        }
        if (!dropTable(newName)) {
            return false;
        }

        // the new table owns the previous table's id:
        if (!executeSQL(
                    KDbEscapedString("UPDATE kexi__objects SET o_id=%1 WHERE o_id=%2 AND o_type=%3")
                    .arg(origID).arg(tableSchema->id()).arg((int)KDb::TableObjectType)))
        {
            return false;
        }
        if (!executeSQL(KDbEscapedString("UPDATE kexi__fields SET t_id=%1 WHERE t_id=%2")
                        .arg(origID, tableSchema->id()))) {
            return false;
        }

        //maintain table ID
        d->changeTableId(tableSchema, origID);
        tableSchema->setId(origID);
    }

    if (!drv_alterTableName(tableSchema, newTableName)) {
        alterTableName_ERR;
        return false;
    }

    // Update kexi__objects
    //! @todo
    if (!executeSQL(KDbEscapedString("UPDATE kexi__objects SET o_name=%1 WHERE o_id=%2")
                    .arg(escapeString(tableSchema->name()), tableSchema->id()))) {
        alterTableName_ERR;
        return false;
    }
//! @todo what about caption?

    //restore old name: it will be changed soon!
    tableSchema->setName(oldTableName);

    if (!commitAutoCommitTransaction(tg.transaction())) {
        alterTableName_ERR;
        return false;
    }

    //update tableSchema:
    d->renameTable(tableSchema, newTableName);
    return true;
}

bool KDbConnection::drv_alterTableName(KDbTableSchema* tableSchema, const QString& newName)
{
    const QString oldTableName = tableSchema->name();
    tableSchema->setName(newName);

    if (!executeSQL(KDbEscapedString("ALTER TABLE %1 RENAME TO %2")
                    .arg(KDbEscapedString(escapeIdentifier(oldTableName)),
                         KDbEscapedString(escapeIdentifier(newName)))))
    {
        tableSchema->setName(oldTableName); //restore old name
        return false;
    }
    return true;
}

bool KDbConnection::dropQuery(KDbQuerySchema* querySchema)
{
    clearResult();
    if (!querySchema)
        return false;

    KDbTransactionGuard tg;
    if (!beginAutoCommitTransaction(&tg))
        return false;

    //remove query schema from kexi__objects table
    if (!removeObject(querySchema->id())) {
        return false;
    }

//! @todo update any structure that depend on this table!
    d->removeQuery(querySchema);
    return commitAutoCommitTransaction(tg.transaction());
}

bool KDbConnection::dropQuery(const QString& queryName)
{
    clearResult();
    KDbQuerySchema* qs = querySchema(queryName);
    if (!qs) {
        m_result = KDbResult(ERR_OBJECT_NOT_FOUND, QObject::tr("Query \"%1\" does not exist.")
                                                .arg(queryName));
        return false;
    }
    return dropQuery(qs);
}

bool KDbConnection::drv_createTable(const KDbTableSchema& tableSchema)
{
    const KDbEscapedString sql( createTableStatement(tableSchema) );
    //kdbDebug() << "******** " << sql;
    return executeSQL(sql);
}

bool KDbConnection::drv_createTable(const QString& tableName)
{
    KDbTableSchema *ts = tableSchema(tableName);
    if (!ts)
        return false;
    return drv_createTable(*ts);
}

bool KDbConnection::beginAutoCommitTransaction(KDbTransactionGuard* tg)
{
    if ((m_driver->d->features & KDbDriver::IgnoreTransactions)
            || !d->autoCommit) {
        tg->setTransaction(KDbTransaction());
        return true;
    }

    // commit current transaction (if present) for drivers
    // that allow single transaction per connection
    if (m_driver->d->features & KDbDriver::SingleTransactions) {
        if (d->default_trans_started_inside) //only commit internally started transaction
            if (!commitTransaction(d->default_trans, true)) {
                tg->setTransaction(KDbTransaction());
                return false; //we have a real error
            }

        d->default_trans_started_inside = d->default_trans.isNull();
        if (!d->default_trans_started_inside) {
            tg->setTransaction(d->default_trans);
            tg->doNothing();
            return true; //reuse externally started transaction
        }
    } else if (!(m_driver->d->features & KDbDriver::MultipleTransactions)) {
        tg->setTransaction(KDbTransaction());
        return true; //no trans. supported at all - just return
    }
    tg->setTransaction(beginTransaction());
    return !m_result.isError();
}

bool KDbConnection::commitAutoCommitTransaction(const KDbTransaction& trans)
{
    if (m_driver->d->features & KDbDriver::IgnoreTransactions)
        return true;
    if (trans.isNull() || !m_driver->transactionsSupported())
        return true;
    if (m_driver->d->features & KDbDriver::SingleTransactions) {
        if (!d->default_trans_started_inside) //only commit internally started transaction
            return true; //give up
    }
    return commitTransaction(trans, true);
}

bool KDbConnection::rollbackAutoCommitTransaction(const KDbTransaction& trans)
{
    if (trans.isNull() || !m_driver->transactionsSupported())
        return true;
    return rollbackTransaction(trans);
}

#define SET_ERR_TRANS_NOT_SUPP \
    { m_result = KDbResult(ERR_UNSUPPORTED_DRV_FEATURE, \
                        QObject::tr("Transactions are not supported for \"%1\" driver.").arg( m_driver->metaData()->name() )); }

#define SET_BEGIN_TR_ERROR \
    { if (!m_result.isError()) \
            m_result = KDbResult(ERR_ROLLBACK_OR_COMMIT_TRANSACTION, QObject::tr("Begin transaction failed.")); }

KDbTransaction KDbConnection::beginTransaction()
{
    if (!checkIsDatabaseUsed())
        return KDbTransaction();
    KDbTransaction trans;
    if (m_driver->d->features & KDbDriver::IgnoreTransactions) {
        //we're creating dummy transaction data here,
        //so it will look like active
        trans.m_data = new KDbTransactionData(this);
        d->transactions.append(trans);
        return trans;
    }
    if (m_driver->d->features & KDbDriver::SingleTransactions) {
        if (d->default_trans.active()) {
            m_result = KDbResult(ERR_TRANSACTION_ACTIVE, QObject::tr("Transaction already started."));
            return KDbTransaction();
        }
        if (!(trans.m_data = drv_beginTransaction())) {
            SET_BEGIN_TR_ERROR;
            return KDbTransaction();
        }
        d->default_trans = trans;
        d->transactions.append(trans);
        return d->default_trans;
    }
    if (m_driver->d->features & KDbDriver::MultipleTransactions) {
        if (!(trans.m_data = drv_beginTransaction())) {
            SET_BEGIN_TR_ERROR;
            return KDbTransaction();
        }
        d->transactions.append(trans);
        return trans;
    }

    SET_ERR_TRANS_NOT_SUPP;
    return KDbTransaction();
}

bool KDbConnection::commitTransaction(const KDbTransaction trans, bool ignore_inactive)
{
    if (!isDatabaseUsed())
        return false;
    if (!m_driver->transactionsSupported()
            && !(m_driver->d->features & KDbDriver::IgnoreTransactions)) {
        SET_ERR_TRANS_NOT_SUPP;
        return false;
    }
    KDbTransaction t = trans;
    if (!t.active()) { //try default tr.
        if (!d->default_trans.active()) {
            if (ignore_inactive)
                return true;
            clearResult();
            m_result = KDbResult(ERR_NO_TRANSACTION_ACTIVE, QObject::tr("Transaction not started."));
            return false;
        }
        t = d->default_trans;
        d->default_trans = KDbTransaction(); //now: no default tr.
    }
    bool ret = true;
    if (!(m_driver->d->features & KDbDriver::IgnoreTransactions))
        ret = drv_commitTransaction(t.m_data);
    if (t.m_data)
        t.m_data->m_active = false; //now this transaction if inactive
    if (!d->dont_remove_transactions) //true=transaction obj will be later removed from list
        d->transactions.removeAt(d->transactions.indexOf(t));
    if (!ret && !m_result.isError())
        m_result = KDbResult(ERR_ROLLBACK_OR_COMMIT_TRANSACTION, QObject::tr("Error on commit transaction."));
    return ret;
}

bool KDbConnection::rollbackTransaction(const KDbTransaction trans, bool ignore_inactive)
{
    if (!isDatabaseUsed())
        return false;
    if (!m_driver->transactionsSupported()
            && !(m_driver->d->features & KDbDriver::IgnoreTransactions)) {
        SET_ERR_TRANS_NOT_SUPP;
        return false;
    }
    KDbTransaction t = trans;
    if (!t.active()) { //try default tr.
        if (!d->default_trans.active()) {
            if (ignore_inactive)
                return true;
            clearResult();
            m_result = KDbResult(ERR_NO_TRANSACTION_ACTIVE, QObject::tr("Transaction not started."));
            return false;
        }
        t = d->default_trans;
        d->default_trans = KDbTransaction(); //now: no default tr.
    }
    bool ret = true;
    if (!(m_driver->d->features & KDbDriver::IgnoreTransactions))
        ret = drv_rollbackTransaction(t.m_data);
    if (t.m_data)
        t.m_data->m_active = false; //now this transaction if inactive
    if (!d->dont_remove_transactions) //true=transaction obj will be later removed from list
        d->transactions.removeAt(d->transactions.indexOf(t));
    if (!ret && !m_result.isError())
        m_result = KDbResult(ERR_ROLLBACK_OR_COMMIT_TRANSACTION, QObject::tr("Error on rollback transaction."));
    return ret;
}

#undef SET_ERR_TRANS_NOT_SUPP
#undef SET_BEGIN_TR_ERROR

/*bool KDbConnection::duringTransaction()
{
  return drv_duringTransaction();
}*/

KDbTransaction KDbConnection::defaultTransaction() const
{
    return d->default_trans;
}

void KDbConnection::setDefaultTransaction(const KDbTransaction& trans)
{
    if (!isDatabaseUsed())
        return;
    if (!(m_driver->d->features & KDbDriver::IgnoreTransactions)
            && (!trans.active() || !m_driver->transactionsSupported())) {
        return;
    }
    d->default_trans = trans;
}

QList<KDbTransaction> KDbConnection::transactions()
{
    return d->transactions;
}

bool KDbConnection::autoCommit() const
{
    return d->autoCommit;
}

bool KDbConnection::setAutoCommit(bool on)
{
    if (d->autoCommit == on || m_driver->d->features & KDbDriver::IgnoreTransactions)
        return true;
    if (!drv_setAutoCommit(on))
        return false;
    d->autoCommit = on;
    return true;
}

KDbTransactionData* KDbConnection::drv_beginTransaction()
{
    if (!executeSQL(KDbEscapedString("BEGIN")))
        return 0;
    return new KDbTransactionData(this);
}

bool KDbConnection::drv_commitTransaction(KDbTransactionData *)
{
    return executeSQL(KDbEscapedString("COMMIT"));
}

bool KDbConnection::drv_rollbackTransaction(KDbTransactionData *)
{
    return executeSQL(KDbEscapedString("ROLLBACK"));
}

bool KDbConnection::drv_setAutoCommit(bool /*on*/)
{
    return true;
}

KDbCursor* KDbConnection::executeQuery(const KDbEscapedString& sql, uint cursor_options)
{
    if (sql.isEmpty())
        return 0;
    KDbCursor *c = prepareQuery(sql, cursor_options);
    if (!c)
        return 0;
    if (!c->open()) {//err - kill that
        m_result = c->result();
        delete c;
        return 0;
    }
    return c;
}

KDbCursor* KDbConnection::executeQuery(KDbQuerySchema* query, const QList<QVariant>& params,
                                 uint cursor_options)
{
    KDbCursor *c = prepareQuery(query, params, cursor_options);
    if (!c)
        return 0;
    if (!c->open()) {//err - kill that
        m_result = c->result();
        delete c;
        return 0;
    }
    return c;
}

KDbCursor* KDbConnection::executeQuery(KDbQuerySchema* query, uint cursor_options)
{
    return executeQuery(query, QList<QVariant>(), cursor_options);
}

KDbCursor* KDbConnection::executeQuery(KDbTableSchema* table, uint cursor_options)
{
    return executeQuery(table->query(), cursor_options);
}

KDbCursor* KDbConnection::prepareQuery(KDbTableSchema* table, uint cursor_options)
{
    return prepareQuery(table->query(), cursor_options);
}

KDbCursor* KDbConnection::prepareQuery(KDbQuerySchema* query, const QList<QVariant>& params,
                                 uint cursor_options)
{
    KDbCursor* cursor = prepareQuery(query, cursor_options);
    if (cursor)
        cursor->setQueryParameters(params);
    return cursor;
}

bool KDbConnection::deleteCursor(KDbCursor *cursor)
{
    if (!cursor)
        return false;
    if (cursor->connection() != this) {//illegal call
        kdbWarning() << "Cannot delete the cursor not owned by the same connection!";
        return false;
    }
    const bool ret = cursor->close();
    delete cursor;
    return ret;
}

//! @todo IMPORTANT: fix KDbConnection::setupObjectData() after refactoring
bool KDbConnection::setupObjectData(const KDbRecordData &data, KDbObject *object)
{
    if (data.count() < 5) {
        kdbWarning() << "Aborting, schema data should have 5 elements, found" << data.count();
        return false;
    }
    bool ok;
    const int id = data[0].toInt(&ok);
    if (!ok)
        return false;
    object->setId(id);
    const QString name(data[2].toString());
    if (!KDb::isIdentifier(name)) {
        m_result = KDbResult(ERR_INVALID_IDENTIFIER, QObject::tr("Invalid object name \"%1\".").arg(name));
        return false;
    }
    object->setName(name);
    object->setCaption(data[3].toString());
    object->setDescription(data[4].toString());

// kdbDebug()<<"@@@ KDbConnection::setupObjectData() == " << sdata.schemaDataDebugString();
    return true;
}

tristate KDbConnection::loadObjectData(int id, KDbObject* object)
{
    KDbRecordData data;
    if (true != querySingleRecord(
            KDbEscapedString("SELECT o_id, o_type, o_name, o_caption, o_desc FROM kexi__objects WHERE o_id=%1")
                          .arg(id),
            &data))
    {
        return cancelled;
    }
    return setupObjectData(data, object);
}

tristate KDbConnection::loadObjectData(int type, const QString& name, KDbObject* object)
{
    KDbRecordData data;
    if (true != querySingleRecord(
            KDbEscapedString("SELECT o_id, o_type, o_name, o_caption, o_desc "
                          "FROM kexi__objects WHERE o_type=%1 AND o_name=%2")
                          .arg(KDbEscapedString::number(type), m_driver->valueToSQL(KDbField::Text, name)),
            &data))
    {
        return cancelled;
    }
    return setupObjectData(data, object);
}

bool KDbConnection::storeObjectDataInternal(KDbObject* object, bool newObject)
{
    KDbTableSchema *ts = d->table(QLatin1String("kexi__objects"));
    if (!ts)
        return false;
    if (newObject) {
        int existingID;
        if (true == querySingleNumber(
                KDbEscapedString("SELECT o_id FROM kexi__objects WHERE o_type=%1 AND o_name=%2")
                              .arg(KDbEscapedString::number(object->type()),
                                   m_driver->valueToSQL(KDbField::Text, object->name())), &existingID))
        {
            //we already have stored a schema data with the same name and type:
            //just update it's properties as it would be existing object
            object->setId(existingID);
            newObject = false;
        }
    }
    if (newObject) {
        KDbFieldList *fl;
        bool ok;
        if (object->id() <= 0) {//get new ID
            fl = ts->subList(
                QList<QByteArray>() << "o_type" << "o_name" << "o_caption" << "o_desc");
            ok = fl != 0;
            if (ok && !insertRecord(fl, QVariant(object->type()), QVariant(object->name()),
                                    QVariant(object->caption()), QVariant(object->description())))
            {
                ok = false;
            }
            delete fl;
            if (!ok)
                return false;
            //fetch newly assigned ID
//! @todo safe to cast it?
            int obj_id = (int)lastInsertedAutoIncValue(QLatin1String("o_id"), *ts);
            //kdbDebug() << "NEW obj_id == " << obj_id;
            if (obj_id <= 0)
                return false;
            object->setId(obj_id);
            return true;
        } else {
            fl = ts->subList(
                QList<QByteArray>() << "o_id" << "o_type" << "o_name" << "o_caption" << "o_desc");
            ok = fl != 0;
            if (ok && !insertRecord(fl, QVariant(object->id()), QVariant(object->type()), QVariant(object->name()),
                                    QVariant(object->caption()), QVariant(object->description())))
            {
                ok = false;
            }
            delete fl;
            return ok;
        }
    }
    //existing object:
    return executeSQL(
               KDbEscapedString("UPDATE kexi__objects SET o_type=%2, o_caption=%3, o_desc=%4 WHERE o_id=%1")
               .arg(KDbEscapedString::number(object->id()), KDbEscapedString::number(object->type()),
                    m_driver->valueToSQL(KDbField::Text, object->caption()),
                    m_driver->valueToSQL(KDbField::Text, object->description())));
}

bool KDbConnection::storeObjectData(KDbObject* object)
{
    return storeObjectDataInternal(object, false);
}

bool KDbConnection::storeNewObjectData(KDbObject* object)
{
    return storeObjectDataInternal(object, true);
}

KDbCursor* KDbConnection::executeQueryInternal(const KDbEscapedString& sql,
                                               KDbQuerySchema* query,
                                               const QList<QVariant>* params)
{
    Q_ASSERT(!sql.isEmpty() || query);
    clearResult();
    if (!sql.isEmpty()) {
        return executeQuery(sql);
    }
    if (!query) {
        return 0;
    }
    if (params) {
        return executeQuery(query, *params);
    }
    return executeQuery(query);
}

tristate KDbConnection::querySingleRecordInternal(KDbRecordData* data,
                                                  const KDbEscapedString* sql,
                                                  KDbQuerySchema* query,
                                                  const QList<QVariant>* params,
                                                  bool addLimitTo1)
{
    Q_ASSERT(sql || query);
    if (sql) {
        //! @todo does not work with non-SQL data sources
        m_result.setSql(m_driver->addLimitTo1(*sql, addLimitTo1));
    }
    KDbCursor *cursor = executeQueryInternal(m_result.sql(), query, params);
    if (!cursor) {
        kdbWarning() << "!querySingleRecordInternal() " << m_result.sql();
        return false;
    }
    if (!cursor->moveFirst()
            || cursor->eof()
            || !cursor->storeCurrentRecord(data))
    {
        const tristate result = cursor->result().isError() ? tristate(false) : tristate(cancelled);
        //kdbDebug() << "!cursor->moveFirst() || cursor->eof() || cursor->storeCurrentRecord(data) "
        //          "m_result.sql()=" << m_result.sql();
        m_result = cursor->result();
        deleteCursor(cursor);
        return result;
    }
    return deleteCursor(cursor);
}

tristate KDbConnection::querySingleRecord(const KDbEscapedString& sql, KDbRecordData* data, bool addLimitTo1)
{
    return querySingleRecordInternal(data, &sql, 0, 0, addLimitTo1);
}

tristate KDbConnection::querySingleRecord(KDbQuerySchema* query, KDbRecordData* data, bool addLimitTo1)
{
    return querySingleRecordInternal(data, 0, query, 0, addLimitTo1);
}

tristate KDbConnection::querySingleRecord(KDbQuerySchema* query, KDbRecordData* data,
                                          const QList<QVariant>& params, bool addLimitTo1)
{
    return querySingleRecordInternal(data, 0, query, &params, addLimitTo1);
}

bool KDbConnection::checkIfColumnExists(KDbCursor *cursor, uint column)
{
    if (column >= cursor->fieldCount()) {
        m_result = KDbResult(ERR_CURSOR_RECORD_FETCHING, QObject::tr("Column \"%1\" does not exist in the query.").arg(column));
        return false;
    }
    return true;
}

tristate KDbConnection::querySingleStringInternal(const KDbEscapedString* sql,
                                                  QString* value, KDbQuerySchema* query,
                                                  const QList<QVariant>* params,
                                                  uint column, bool addLimitTo1)
{
    Q_ASSERT(sql || query);
    if (sql) {
        //! @todo does not work with non-SQL data sources
        m_result.setSql(m_driver->addLimitTo1(*sql, addLimitTo1));
    }
    KDbCursor *cursor = executeQueryInternal(m_result.sql(), query, params);
    if (!cursor) {
        kdbWarning() << "!querySingleStringInternal()" << m_result.sql();
        return false;
    }
    if (!cursor->moveFirst() || cursor->eof()) {
        const tristate result = cursor->result().isError() ? tristate(false) : tristate(cancelled);
        //kdbDebug() << "!cursor->moveFirst() || cursor->eof()" << m_result.sql();
        deleteCursor(cursor);
        return result;
    }
    if (!checkIfColumnExists(cursor, column)) {
        deleteCursor(cursor);
        return false;
    }
    *value = cursor->value(column).toString();
    return deleteCursor(cursor);
}

tristate KDbConnection::querySingleString(const KDbEscapedString& sql, QString* value,
                                          uint column, bool addLimitTo1)
{
    return querySingleStringInternal(&sql, value, 0, 0, column, addLimitTo1);
}

tristate KDbConnection::querySingleString(KDbQuerySchema* query, QString* value, uint column,
                                          bool addLimitTo1)
{
    return querySingleStringInternal(0, value, query, 0, column, addLimitTo1);
}

tristate KDbConnection::querySingleString(KDbQuerySchema* query, QString* value,
                                          const QList<QVariant>& params, uint column,
                                          bool addLimitTo1)
{
    return querySingleStringInternal(0, value, query, &params, column, addLimitTo1);
}

tristate KDbConnection::querySingleNumberInternal(const KDbEscapedString* sql,
                                                  int* number, KDbQuerySchema* query,
                                                  const QList<QVariant>* params,
                                                  uint column, bool addLimitTo1)
{
    QString str;
    const tristate result = querySingleStringInternal(sql, &str, query, params, column,
                                                      addLimitTo1);
    if (result != true)
        return result;
    bool ok;
    const int _number = str.toInt(&ok);
    if (!ok)
        return false;
    *number = _number;
    return true;
}

tristate KDbConnection::querySingleNumber(const KDbEscapedString& sql, int* number,
                                          uint column, bool addLimitTo1)
{
    return querySingleNumberInternal(&sql, number, 0, 0, column, addLimitTo1);
}

tristate KDbConnection::querySingleNumber(KDbQuerySchema* query, int* number, uint column,
                                          bool addLimitTo1)
{
    return querySingleNumberInternal(0, number, query, 0, column, addLimitTo1);
}

tristate KDbConnection::querySingleNumber(KDbQuerySchema* query, int* number,
                                          const QList<QVariant>& params, uint column,
                                          bool addLimitTo1)
{
    return querySingleNumberInternal(0, number, query, &params, column, addLimitTo1);
}

tristate KDbConnection::queryStringListInternal(const KDbEscapedString* sql,
                                                QStringList* list, KDbQuerySchema* query,
                                                const QList<QVariant>* params,
                                                uint column)
{
    if (sql) {
        m_result.setSql(*sql);
    }
    KDbCursor *cursor = executeQueryInternal(m_result.sql(), query, params);
    if (!cursor) {
        kdbWarning() << "!queryStringListInternal() " << m_result.sql();
        return false;
    }
    cursor->moveFirst();
    if (cursor->result().isError()) {
        m_result = cursor->result();
        deleteCursor(cursor);
        return false;
    }
    if (!cursor->eof() && !checkIfColumnExists(cursor, column)) {
        deleteCursor(cursor);
        return false;
    }
    list->clear();
    while (!cursor->eof()) {
        list->append(cursor->value(column).toString());
        if (!cursor->moveNext() && cursor->result().isError()) {
            m_result = cursor->result();
            const tristate result = m_result.isError() ? tristate(false) : tristate(cancelled);
            deleteCursor(cursor);
            return result;
        }
    }
    return deleteCursor(cursor);
}

tristate KDbConnection::queryStringList(const KDbEscapedString& sql, QStringList* list,
                                        uint column)
{
    return queryStringListInternal(&sql, list, 0, 0, column);
}

tristate KDbConnection::queryStringList(KDbQuerySchema* query, QStringList* list, uint column)
{
    return queryStringListInternal(0, list, query, 0, column);
}

tristate KDbConnection::queryStringList(KDbQuerySchema* query, QStringList* list,
                                    const QList<QVariant>& params, uint column)
{
    return queryStringListInternal(0, list, query, &params, column);
}

bool KDbConnection::resultExists(const KDbEscapedString& sql, bool* success, bool addLimitTo1)
{
    //optimization
    if (m_driver->beh->SELECT_1_SUBQUERY_SUPPORTED) {
        //this is at least for sqlite
        if (addLimitTo1 && sql.left(6).toUpper() == "SELECT") {
            m_result.setSql(
                m_driver->addLimitTo1("SELECT 1 FROM (" + sql + ')', addLimitTo1));
        }
        else {
            m_result.setSql(sql);
        }
    } else {
        if (addLimitTo1 && sql.startsWith("SELECT")) {
            m_result.setSql(m_driver->addLimitTo1(sql, addLimitTo1));
        }
        else {
            m_result.setSql(sql);
        }
    }
    KDbCursor *cursor = executeQuery(m_result.sql());
    if (!cursor) {
        kdbWarning() << "!executeQuery()" << m_result.sql();
        *success = false;
        return false;
    }
    if (!cursor->moveFirst() || cursor->eof()) {
        *success = !cursor->result().isError();
        kdbWarning() << "!cursor->moveFirst() || cursor->eof()" << m_result.sql();
        m_result = cursor->result();
        deleteCursor(cursor);
        return false;
    }
    *success = deleteCursor(cursor);
    return true;
}

bool KDbConnection::isEmpty(KDbTableSchema* table, bool* success)
{
    return !resultExists(selectStatement(table->query()), success);
}

//! Used by addFieldPropertyToExtendedTableSchemaData()
static void createExtendedTableSchemaMainElementIfNeeded(
    QDomDocument* doc, QDomElement* extendedTableSchemaMainEl,
    bool* extendedTableSchemaStringIsEmpty)
{
    if (!extendedTableSchemaStringIsEmpty)
        return;
    //init document
    *extendedTableSchemaMainEl = doc->createElement(QLatin1String("EXTENDED_TABLE_SCHEMA"));
    doc->appendChild(*extendedTableSchemaMainEl);
    extendedTableSchemaMainEl->setAttribute(QLatin1String("version"),
                                            QString::number(KDB_EXTENDED_TABLE_SCHEMA_VERSION));
    *extendedTableSchemaStringIsEmpty = false;
}

//! Used by addFieldPropertyToExtendedTableSchemaData()
static void createExtendedTableSchemaFieldElementIfNeeded(QDomDocument* doc,
        QDomElement* extendedTableSchemaMainEl, const QString& fieldName, QDomElement* extendedTableSchemaFieldEl,
        bool append = true)
{
    if (!extendedTableSchemaFieldEl->isNull())
        return;
    *extendedTableSchemaFieldEl = doc->createElement(QLatin1String("field"));
    if (append)
        extendedTableSchemaMainEl->appendChild(*extendedTableSchemaFieldEl);
    extendedTableSchemaFieldEl->setAttribute(QLatin1String("name"), fieldName);
}

/*! @internal used by storeExtendedTableSchemaData()
 Creates DOM node for @a propertyName and @a propertyValue.
 Creates enclosing EXTENDED_TABLE_SCHEMA element if EXTENDED_TABLE_SCHEMA is true.
 Updates extendedTableSchemaStringIsEmpty and extendedTableSchemaMainEl afterwards.
 If extendedTableSchemaFieldEl is null, creates <field> element (with optional
 "custom" attribute is @a custom is false). */
static void addFieldPropertyToExtendedTableSchemaData(
    const KDbField& f, const QByteArray &propertyName, const QVariant& propertyValue,
    QDomDocument* doc, QDomElement* extendedTableSchemaMainEl,
    QDomElement* extendedTableSchemaFieldEl,
    bool* extendedTableSchemaStringIsEmpty,
    bool custom = false)
{
    createExtendedTableSchemaMainElementIfNeeded(doc,
            extendedTableSchemaMainEl, extendedTableSchemaStringIsEmpty);
    createExtendedTableSchemaFieldElementIfNeeded(
        doc, extendedTableSchemaMainEl, f.name(), extendedTableSchemaFieldEl);

    //create <property>
    QDomElement extendedTableSchemaFieldPropertyEl = doc->createElement(QLatin1String("property"));
    extendedTableSchemaFieldEl->appendChild(extendedTableSchemaFieldPropertyEl);
    if (custom)
        extendedTableSchemaFieldPropertyEl.setAttribute(QLatin1String("custom"), QLatin1String("true"));
    extendedTableSchemaFieldPropertyEl.setAttribute(QLatin1String("name"), QLatin1String(propertyName));
    QDomElement extendedTableSchemaFieldPropertyValueEl;
    switch (propertyValue.type()) {
    case QVariant::String:
        extendedTableSchemaFieldPropertyValueEl = doc->createElement(QLatin1String("string"));
        break;
    case QVariant::ByteArray:
        extendedTableSchemaFieldPropertyValueEl = doc->createElement(QLatin1String("cstring"));
        break;
    case QVariant::Int:
    case QVariant::Double:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
        extendedTableSchemaFieldPropertyValueEl = doc->createElement(QLatin1String("number"));
        break;
    case QVariant::Bool:
        extendedTableSchemaFieldPropertyValueEl = doc->createElement(QLatin1String("bool"));
        break;
    default:
//! @todo add more QVariant types
        kdbCritical() << "addFieldPropertyToExtendedTableSchemaData(): impl. error";
    }
    extendedTableSchemaFieldPropertyEl.appendChild(extendedTableSchemaFieldPropertyValueEl);
    extendedTableSchemaFieldPropertyValueEl.appendChild(
        doc->createTextNode(propertyValue.toString()));
}

bool KDbConnection::storeExtendedTableSchemaData(KDbTableSchema* tableSchema)
{
//! @todo future: save in older versions if neeed
    QDomDocument doc(QLatin1String("EXTENDED_TABLE_SCHEMA"));
    QDomElement extendedTableSchemaMainEl;
    bool extendedTableSchemaStringIsEmpty = true;

    //for each field:
    foreach(KDbField* f, *tableSchema->fields()) {
        QDomElement extendedTableSchemaFieldEl;
        if (f->visibleDecimalPlaces() >= 0/*nondefault*/ && KDb::supportsVisibleDecimalPlacesProperty(f->type())) {
            addFieldPropertyToExtendedTableSchemaData(
                *f, "visibleDecimalPlaces", f->visibleDecimalPlaces(), &doc,
                &extendedTableSchemaMainEl, &extendedTableSchemaFieldEl,
                &extendedTableSchemaStringIsEmpty);
        }
        if (f->type() == KDbField::Text) {
            if (f->maxLengthStrategy() == KDbField::DefaultMaxLength) {
                addFieldPropertyToExtendedTableSchemaData(
                    *f, "maxLengthIsDefault", true, &doc,
                    &extendedTableSchemaMainEl, &extendedTableSchemaFieldEl,
                    &extendedTableSchemaStringIsEmpty);
            }
        }

        // boolean field with "not null"

        // add custom properties
        const KDbField::CustomPropertiesMap customProperties(f->customProperties());
        for (KDbField::CustomPropertiesMap::ConstIterator itCustom = customProperties.constBegin();
                itCustom != customProperties.constEnd(); ++itCustom) {
            addFieldPropertyToExtendedTableSchemaData(
                *f, itCustom.key(), itCustom.value(), &doc,
                &extendedTableSchemaMainEl, &extendedTableSchemaFieldEl, &extendedTableSchemaStringIsEmpty,
                /*custom*/true);
        }
        // save lookup table specification, if present
        KDbLookupFieldSchema *lookupFieldSchema = tableSchema->lookupFieldSchema(*f);
        if (lookupFieldSchema) {
            createExtendedTableSchemaFieldElementIfNeeded(
                &doc, &extendedTableSchemaMainEl, f->name(), &extendedTableSchemaFieldEl, false/* !append */);
            lookupFieldSchema->saveToDom(&doc, &extendedTableSchemaFieldEl);

            if (extendedTableSchemaFieldEl.hasChildNodes()) {
                // this element provides the definition, so let's append it now
                createExtendedTableSchemaMainElementIfNeeded(&doc, &extendedTableSchemaMainEl,
                        &extendedTableSchemaStringIsEmpty);
                extendedTableSchemaMainEl.appendChild(extendedTableSchemaFieldEl);
            }
        }
    }

    // Store extended schema information (see ExtendedTableSchemaInformation in Kexi Wiki)
    if (extendedTableSchemaStringIsEmpty) {
#ifdef KDB_DEBUG_GUI
        KDb::alterTableActionDebugGUI(QLatin1String("** Extended table schema REMOVED."));
#endif
        if (!removeDataBlock(tableSchema->id(), QLatin1String("extended_schema")))
            return false;
    } else {
#ifdef KDB_DEBUG_GUI
        KDb::alterTableActionDebugGUI(
                    QLatin1String("** Extended table schema set to:\n") + doc.toString(4));
#endif
        if (!storeDataBlock(tableSchema->id(), doc.toString(1), QLatin1String("extended_schema")))
            return false;
    }
    return true;
}

bool KDbConnection::loadExtendedTableSchemaData(KDbTableSchema* tableSchema)
{
#define loadExtendedTableSchemaData_ERR \
    { m_result = KDbResult(QObject::tr("Error while loading extended table schema.", \
                                    "Extended schema for a table: loading error")); \
      return false; }
#define loadExtendedTableSchemaData_ERR2(details) \
    { m_result = KDbResult(details); \
      m_result.setMessageTitle(QObject::tr("Error while loading extended table schema.", \
                                           "Extended schema for a table: loading error")); \
      return false; }
#define loadExtendedTableSchemaData_ERR3(data) \
    { m_result = KDbResult(QObject::tr("Invalid XML data: %1").arg(data.left(1024))); \
      m_result.setMessageTitle(QObject::tr("Error while loading extended table schema.", \
                                           "Extended schema for a table: loading error")); \
      return false; }

    // Load extended schema information, if present (see ExtendedTableSchemaInformation in Kexi Wiki)
    QString extendedTableSchemaString;
    tristate res = loadDataBlock(tableSchema->id(),
                                 &extendedTableSchemaString, QLatin1String("extended_schema"));
    if (!res)
        loadExtendedTableSchemaData_ERR;
    // extendedTableSchemaString will be just empty if there is no such data block

    if (extendedTableSchemaString.isEmpty())
        return true;

    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;
    if (!doc.setContent(extendedTableSchemaString, &errorMsg, &errorLine, &errorColumn)) {
        loadExtendedTableSchemaData_ERR2(
            QObject::tr("Error in XML data: \"%1\" in line %2, column %3.\nXML data: %4")
            .arg(errorMsg).arg(errorLine).arg(errorColumn).arg(extendedTableSchemaString.left(1024)));
    }

//! @todo look at the current format version (KDB_EXTENDED_TABLE_SCHEMA_VERSION)

    if (doc.doctype().name() != QLatin1String("EXTENDED_TABLE_SCHEMA"))
        loadExtendedTableSchemaData_ERR3(extendedTableSchemaString);

    QDomElement docEl = doc.documentElement();
    if (docEl.tagName() != QLatin1String("EXTENDED_TABLE_SCHEMA"))
        loadExtendedTableSchemaData_ERR3(extendedTableSchemaString);

    for (QDomNode n = docEl.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement fieldEl = n.toElement();
        if (fieldEl.tagName() == QLatin1String("field")) {
            KDbField *f = tableSchema->field(fieldEl.attribute(QLatin1String("name")));
            if (f) {
                //set properties of the field:
//! @todo more properties
                for (QDomNode propNode = fieldEl.firstChild();
                     !propNode.isNull(); propNode = propNode.nextSibling())
                {
                    const QDomElement propEl = propNode.toElement();
                    bool ok;
                    int intValue;
                    if (propEl.tagName() == QLatin1String("property")) {
                        QByteArray propertyName = propEl.attribute(QLatin1String("name")).toLatin1();
                        if (propEl.attribute(QLatin1String("custom")) == QLatin1String("true")) {
                            //custom property
                            const QVariant v(KDb::loadPropertyValueFromDom(propEl.firstChild(), &ok));
                            if (ok) {
                                f->setCustomProperty(propertyName, v);
                            }
                        }
                        else if (propertyName == "visibleDecimalPlaces") {
                            if (KDb::supportsVisibleDecimalPlacesProperty(f->type())) {
                                intValue = KDb::loadIntPropertyValueFromDom(propEl.firstChild(), &ok);
                                if (ok)
                                    f->setVisibleDecimalPlaces(intValue);
                            }
                        }
                        else if (propertyName == "maxLengthIsDefault") {
                            if (f->type() == KDbField::Text) {
                                const bool maxLengthIsDefault
                                    = KDb::loadPropertyValueFromDom(propEl.firstChild(), &ok).toBool();
                                if (ok) {
                                    f->setMaxLengthStrategy(
                                        maxLengthIsDefault ? KDbField::DefaultMaxLength : KDbField::DefinedMaxLength);
                                }
                            }
                        }
//! @todo more properties...
                    } else if (propEl.tagName() == QLatin1String("lookup-column")) {
                        KDbLookupFieldSchema *lookupFieldSchema = KDbLookupFieldSchema::loadFromDom(propEl);
                        if (lookupFieldSchema) {
                            kdbDebug() << f->name() << *lookupFieldSchema;
                            tableSchema->setLookupFieldSchema(f->name(), lookupFieldSchema);
                        }
                    }
                }
            } else {
                kdbWarning() << "no such field:" << fieldEl.attribute(QLatin1String("name"))
                        << "in table:" << tableSchema->name();
            }
        }
    }

    return true;
}

KDbField* KDbConnection::setupField(const KDbRecordData &data)
{
    bool ok = true;
    int f_int_type = data.at(1).toInt(&ok);
    if (f_int_type <= KDbField::InvalidType || f_int_type > KDbField::LastType)
        ok = false;
    if (!ok)
        return 0;
    KDbField::Type f_type = (KDbField::Type)f_int_type;
    int f_len = qMax(0, data.at(3).toInt(&ok)); // defined limit
    if (!ok) {
        return 0;
    }
    if (f_len < 0) {
        f_len = 0;
    }
//! @todo load maxLengthStrategy info to see if the maxLength is the default

    int f_prec = data.at(4).toInt(&ok);
    if (!ok)
        return 0;
    KDbField::Constraints f_constr = (KDbField::Constraints)data.at(5).toInt(&ok);
    if (!ok)
        return 0;
    KDbField::Options f_opts = (KDbField::Options)data.at(6).toInt(&ok);
    if (!ok)
        return 0;

    QString name(data.at(2).toString().toLower());
    if (!KDb::isIdentifier(name)) {
        m_result = KDbResult(ERR_INVALID_IDENTIFIER, QObject::tr("Invalid object name \"%1\".")
                                                  .arg(data.at(2).toString()));
        ok = false;
        return 0;
    }

    KDbField *f = new KDbField(
        data.at(2).toString(), f_type, f_constr, f_opts, f_len, f_prec);

    f->setDefaultValue(KDb::stringToVariant(data.at(7).toString(), KDbField::variantType(f_type), &ok));
    if (!ok) {
        kdbWarning() << "problem with KDb::stringToVariant(" << data.at(7).toString() << ')';
    }
    ok = true; //problem with defaultValue is not critical

    f->setCaption(data.at(9).toString());
    f->setDescription(data.at(10).toString());
    return f;
}

KDbTableSchema* KDbConnection::setupTableSchema(const KDbRecordData &data)
{
    KDbTableSchema *t = new KDbTableSchema(this);
    if (!setupObjectData(data, t)) {
        delete t;
        return 0;
    }

    KDbCursor *cursor;
    if (!(cursor = executeQuery(
            KDbEscapedString("SELECT t_id, f_type, f_name, f_length, f_precision, f_constraints, "
                          "f_options, f_default, f_order, f_caption, f_help "
                          "FROM kexi__fields WHERE t_id=%1 ORDER BY f_order").arg(t->id()))))
    {
        delete t;
        return 0;
    }
    if (!cursor->moveFirst()) {
        if (!cursor->result().isError() && cursor->eof()) {
            m_result = KDbResult(QObject::tr("Table has no fields defined."));
        }
        deleteCursor(cursor);
        delete t;
        return 0;
    }

    // For each field: load its schema
    KDbRecordData fieldData;
    bool ok = true;
    while (!cursor->eof()) {
//  kdbDebug()<<"@@@ f_name=="<<cursor->value(2).asCString();
        if (!cursor->storeCurrentRecord(&fieldData)) {
            ok = false;
            break;
        }
        KDbField *f = setupField(fieldData);
        if (!f) {
            ok = false;
            break;
        }
        t->addField(f);
        cursor->moveNext();
    }

    if (!ok) {//error:
        deleteCursor(cursor);
        delete t;
        return 0;
    }

    if (!deleteCursor(cursor)) {
        delete t;
        return 0;
    }

    if (!loadExtendedTableSchemaData(t)) {
        delete t;
        return 0;
    }
    //store locally:
    d->insertTable(t);
    return t;
}

KDbTableSchema* KDbConnection::tableSchema(const QString& tableName)
{
    KDbTableSchema *t = d->table(tableName);
    if (t)
        return t;
    //not found: retrieve schema
    KDbRecordData data;
    if (true != querySingleRecord(
            KDbEscapedString("SELECT o_id, o_type, o_name, o_caption, o_desc FROM kexi__objects "
                          "WHERE o_name='%1' AND o_type=%2")
                          .arg(KDbEscapedString(tableName), KDbEscapedString::number(KDb::TableObjectType)), &data))
    {
        return 0;
    }
    return setupTableSchema(data);
}

KDbTableSchema* KDbConnection::tableSchema(int tableId)
{
    KDbTableSchema *t = d->table(tableId);
    if (t)
        return t;
    //not found: retrieve schema
    KDbRecordData data;
    if (true != querySingleRecord(
            KDbEscapedString("SELECT o_id, o_type, o_name, o_caption, o_desc FROM kexi__objects WHERE o_id=%1")
                          .arg(tableId), &data))
    {
        return 0;
    }
    return setupTableSchema(data);
}

tristate KDbConnection::loadDataBlock(int objectID, QString* dataString, const QString& dataID)
{
    if (objectID <= 0)
        return false;
    return querySingleString(
               KDbEscapedString("SELECT o_data FROM kexi__objectdata WHERE o_id=%1 AND %2")
                             .arg(KDbEscapedString::number(objectID),
                                  KDbEscapedString(KDb::sqlWhere(m_driver, KDbField::Text,
                                                QLatin1String("o_sub_id"),
                                                dataID.isEmpty() ? QVariant() : QVariant(dataID)))),
               dataString);
}

bool KDbConnection::storeDataBlock(int objectID, const QString &dataString, const QString& dataID)
{
    if (objectID <= 0)
        return false;
    KDbEscapedString sql(
        KDbEscapedString("SELECT kexi__objectdata.o_id FROM kexi__objectdata WHERE o_id=%1").arg(objectID));
    KDbEscapedString sql_sub(KDb::sqlWhere(m_driver, KDbField::Text, QLatin1String("o_sub_id"),
                                              dataID.isEmpty() ? QVariant() : QVariant(dataID)));

    bool ok, exists;
    exists = resultExists(sql + " AND " + sql_sub, &ok);
    if (!ok)
        return false;
    if (exists) {
        return executeSQL("UPDATE kexi__objectdata SET o_data="
                          + m_driver->valueToSQL(KDbField::LongText, dataString)
                          + " WHERE o_id=" + QString::number(objectID) + " AND " + sql_sub);
    }
    return executeSQL(
               KDbEscapedString("INSERT INTO kexi__objectdata (o_id, o_data, o_sub_id) VALUES (")
               + KDbEscapedString::number(objectID) + ',' + m_driver->valueToSQL(KDbField::LongText, dataString)
               + ',' + m_driver->valueToSQL(KDbField::Text, dataID) + ')');
}

bool KDbConnection::copyDataBlock(int sourceObjectID, int destObjectID, const QString &dataID)
{
    if (sourceObjectID <= 0 || destObjectID <= 0)
        return false;
    if (sourceObjectID == destObjectID)
        return true;
    if (!removeDataBlock(destObjectID, dataID)) // remove before copying
        return false;
    KDbEscapedString sql = KDbEscapedString(
         "INSERT INTO kexi__objectdata SELECT %1, t.o_data, t.o_sub_id "
         "FROM kexi__objectdata AS t WHERE o_id=%2")
         .arg(destObjectID).arg(sourceObjectID);
    if (!dataID.isEmpty()) {
        sql += KDbEscapedString(" AND ") + KDb::sqlWhere(m_driver, KDbField::Text,
                                                            QLatin1String("o_sub_id"), dataID);
    }
    return executeSQL(sql);
}

bool KDbConnection::removeDataBlock(int objectID, const QString& dataID)
{
    if (objectID <= 0)
        return false;
    if (dataID.isEmpty())
        return KDb::deleteRecord(this, QLatin1String("kexi__objectdata"),
                                       QLatin1String("o_id"), QString::number(objectID));
    else
        return KDb::deleteRecord(this, QLatin1String("kexi__objectdata"),
                                       QLatin1String("o_id"), KDbField::Integer, objectID,
                                       QLatin1String("o_sub_id"), KDbField::Text, dataID);
}

KDbQuerySchema* KDbConnection::setupQuerySchema(const KDbRecordData &data)
{
    bool ok = true;
    const int objID = data[0].toInt(&ok);
    if (!ok)
        return 0;
    QString sql;
    if (!loadDataBlock(objID, &sql, QLatin1String("sql"))) {
        m_result = KDbResult(ERR_OBJECT_NOT_FOUND,
                          QObject::tr("Could not find definition for query \"%1\". Removing this query is recommended.")
                          .arg(data[2].toString()));
        return 0;
    }
    d->parser()->parse(KDbEscapedString(sql));
    KDbQuerySchema *query = d->parser()->query();
    //error?
    if (!query) {
        m_result = KDbResult(ERR_SQL_PARSE_ERROR,
                          QObject::tr("<p>Could not load definition for query \"%1\". "
                             "SQL statement for this query is invalid:<br><tt>%2</tt></p>\n"
                             "<p>You can open this query in Text View and correct it.</p>")
                             .arg(data[2].toString(), sql));
        return 0;
    }
    if (!setupObjectData(data, query)) {
        delete query;
        return 0;
    }
    d->insertQuery(query);
    return query;
}

KDbQuerySchema* KDbConnection::querySchema(const QString& queryName)
{
    QString m_queryName = queryName.toLower();
    KDbQuerySchema *q = d->query(m_queryName);
    if (q)
        return q;
    //not found: retrieve schema
    KDbRecordData data;
    if (true != querySingleRecord(
            KDbEscapedString("SELECT o_id, o_type, o_name, o_caption, o_desc FROM kexi__objects "
                          "WHERE o_name='%1' AND o_type=%2")
                          .arg(KDbEscapedString(m_queryName), KDbEscapedString::number(KDb::QueryObjectType)),
            &data))
    {
        return 0;
    }
    return setupQuerySchema(data);
}

KDbQuerySchema* KDbConnection::querySchema(int queryId)
{
    KDbQuerySchema *q = d->query(queryId);
    if (q)
        return q;
    //not found: retrieve schema
    clearResult();
    KDbRecordData data;
    if (true != querySingleRecord(
            KDbEscapedString("SELECT o_id, o_type, o_name, o_caption, o_desc FROM kexi__objects WHERE o_id=%1")
                          .arg(queryId),
            &data))
    {
        return 0;
    }
    return setupQuerySchema(data);
}

bool KDbConnection::setQuerySchemaObsolete(const QString& queryName)
{
    KDbQuerySchema* oldQuery = querySchema(queryName);
    if (!oldQuery)
        return false;
    d->setQueryObsolete(oldQuery);
    return true;
}

void KDbConnection::insertInternalTable(KDbTableSchema* tableSchema)
{
    d->insertInternalTable(tableSchema);
}

KDbTableSchema* KDbConnection::newKDbSystemTableSchema(const QString& tableName)
{
    KDbTableSchema *ts = new KDbTableSchema(tableName.toLower());
    insertInternalTable(ts);
    return ts;
}

bool KDbConnection::isInternalTableSchema(const QString& tableName)
{
    return (d->kdbSystemTables().contains(tableSchema(tableName)))
           // these are here for compatiblility because we're no longer instantiate
           // them but can exist in projects created with previous Kexi versions:
           || tableName == QLatin1String("kexi__final") || tableName == QLatin1String("kexi__useractions");
}

bool KDbConnection::setupKDbSystemSchema()
{
    if (!d->kdbSystemTables().isEmpty())
        return true; //already set up

    KDbTableSchema *t_objects = newKDbSystemTableSchema(QLatin1String("kexi__objects"));
    t_objects->addField(new KDbField(QLatin1String("o_id"),
                                  KDbField::Integer, KDbField::PrimaryKey | KDbField::AutoInc, KDbField::Unsigned))
    .addField(new KDbField(QLatin1String("o_type"), KDbField::Byte, 0, KDbField::Unsigned))
    .addField(new KDbField(QLatin1String("o_name"), KDbField::Text))
    .addField(new KDbField(QLatin1String("o_caption"), KDbField::Text))
    .addField(new KDbField(QLatin1String("o_desc"), KDbField::LongText));

    kdbDebug() << *t_objects;

    KDbTableSchema *t_objectdata = newKDbSystemTableSchema(QLatin1String("kexi__objectdata"));
    t_objectdata->addField(new KDbField(QLatin1String("o_id"),
                                     KDbField::Integer, KDbField::NotNull, KDbField::Unsigned))
    .addField(new KDbField(QLatin1String("o_data"), KDbField::LongText))
    .addField(new KDbField(QLatin1String("o_sub_id"), KDbField::Text));

    KDbTableSchema *t_fields = newKDbSystemTableSchema(QLatin1String("kexi__fields"));
    t_fields->addField(new KDbField(QLatin1String("t_id"), KDbField::Integer, 0, KDbField::Unsigned))
    .addField(new KDbField(QLatin1String("f_type"), KDbField::Byte, 0, KDbField::Unsigned))
    .addField(new KDbField(QLatin1String("f_name"), KDbField::Text))
    .addField(new KDbField(QLatin1String("f_length"), KDbField::Integer))
    .addField(new KDbField(QLatin1String("f_precision"), KDbField::Integer))
    .addField(new KDbField(QLatin1String("f_constraints"), KDbField::Integer))
    .addField(new KDbField(QLatin1String("f_options"), KDbField::Integer))
    .addField(new KDbField(QLatin1String("f_default"), KDbField::Text))
    //these are additional properties:
    .addField(new KDbField(QLatin1String("f_order"), KDbField::Integer))
    .addField(new KDbField(QLatin1String("f_caption"), KDbField::Text))
    .addField(new KDbField(QLatin1String("f_help"), KDbField::LongText));

    KDbTableSchema *t_db = newKDbSystemTableSchema(QLatin1String("kexi__db"));
    t_db->addField(new KDbField(QLatin1String("db_property"),
                             KDbField::Text, KDbField::NoConstraints, KDbField::NoOptions, 32))
    .addField(new KDbField(QLatin1String("db_value"), KDbField::LongText));

    return true;
}

void KDbConnection::removeMe(KDbTableSchema *table)
{
    if (table && !m_destructor_started)
        d->takeTable(table);
}

QString KDbConnection::anyAvailableDatabaseName()
{
    if (!d->availableDatabaseName.isEmpty()) {
        return d->availableDatabaseName;
    }
    return m_driver->beh->ALWAYS_AVAILABLE_DATABASE_NAME;
}

void KDbConnection::setAvailableDatabaseName(const QString& dbName)
{
    d->availableDatabaseName = dbName;
}

//! @internal used in updateRecord(), insertRecord(),
inline void updateRecordDataWithNewValues(KDbQuerySchema* query, KDbRecordData* data, const KDbRecordEditBuffer::DBMap& b,
                                          QHash<KDbQueryColumnInfo*, int>* columnsOrderExpanded)
{
    *columnsOrderExpanded = query->columnsOrder(KDbQuerySchema::ExpandedList);
    QHash<KDbQueryColumnInfo*, int>::ConstIterator columnsOrderExpandedIt;
    for (KDbRecordEditBuffer::DBMap::ConstIterator it = b.constBegin();it != b.constEnd();++it) {
        columnsOrderExpandedIt = columnsOrderExpanded->constFind(it.key());
        if (columnsOrderExpandedIt == columnsOrderExpanded->constEnd()) {
            kdbWarning() << "(KDbConnection) \"now also assign new value in memory\" step"
                       "- could not find item" << it.key()->aliasOrName();
            continue;
        }
        (*data)[ columnsOrderExpandedIt.value() ] = it.value();
    }
}

bool KDbConnection::updateRecord(KDbQuerySchema* query, KDbRecordData* data, KDbRecordEditBuffer* buf, bool useRecordId)
{
// Each SQL identifier needs to be escaped in the generated query.
// kdbDebug() << *query;

    clearResult();
    //--get PKEY
    if (buf->dbBuffer().isEmpty()) {
        kdbDebug() << " -- NO CHANGES DATA!";
        return true;
    }
    KDbTableSchema *mt = query->masterTable();
    if (!mt) {
        kdbWarning() << " -- NO MASTER TABLE!";
        m_result = KDbResult(ERR_UPDATE_NO_MASTER_TABLE,
                          QObject::tr("Could not update record because there is no master table defined."));
        return false;
    }
    KDbIndexSchema *pkey = (mt->primaryKey() && !mt->primaryKey()->fields()->isEmpty()) ? mt->primaryKey() : 0;
    if (!useRecordId && !pkey) {
        kdbWarning() << " -- NO MASTER TABLE's PKEY!";
        m_result = KDbResult(ERR_UPDATE_NO_MASTER_TABLES_PKEY,
                          QObject::tr("Could not update record because master table has no primary key defined."));
//! @todo perhaps we can try to update without using PKEY?
        return false;
    }
    //update the record:
    KDbEscapedString sql;
    sql.reserve(4096);
    sql = KDbEscapedString("UPDATE ") + escapeIdentifier(mt->name()) + " SET ";
    KDbEscapedString sqlset, sqlwhere;
    sqlset.reserve(1024);
    sqlwhere.reserve(1024);
    KDbRecordEditBuffer::DBMap b = buf->dbBuffer();

    //gather the fields which are updated ( have values in KDbRecordEditBuffer)
    KDbFieldList affectedFields;
    for (KDbRecordEditBuffer::DBMap::ConstIterator it = b.constBegin();it != b.constEnd();++it) {
        if (it.key()->field->table() != mt)
            continue; // skip values for fields outside of the master table (e.g. a "visible value" of the lookup field)
        if (!sqlset.isEmpty())
            sqlset += ',';
        KDbField* currentField = it.key()->field;
        affectedFields.addField(currentField);
        sqlset += KDbEscapedString(escapeIdentifier(currentField->name())) + '=' +
                  m_driver->valueToSQL(currentField, it.value());
    }
    if (pkey) {
        const QVector<int> pkeyFieldsOrder(query->pkeyFieldsOrder());
        //kdbDebug() << pkey->fieldCount() << " ? " << query->pkeyFieldCount();
        if (pkey->fieldCount() != query->pkeyFieldCount()) { //sanity check
            kdbWarning() << " -- NO ENTIRE MASTER TABLE's PKEY SPECIFIED!";
            m_result = KDbResult(ERR_UPDATE_NO_ENTIRE_MASTER_TABLES_PKEY,
                              QObject::tr("Could not update record because it does not contain entire primary key of master table."));
            return false;
        }
        if (!pkey->fields()->isEmpty()) {
            uint i = 0;
            foreach(KDbField *f, *pkey->fields()) {
                if (!sqlwhere.isEmpty())
                    sqlwhere += " AND ";
                QVariant val(data->at(pkeyFieldsOrder.at(i)));
                if (val.isNull() || !val.isValid()) {
                    m_result = KDbResult(ERR_UPDATE_NULL_PKEY_FIELD,
                                      QObject::tr("Primary key's field \"%1\" cannot be empty.").arg(f->name()));
                    //js todo: pass the field's name somewhere!
                    return false;
                }
                sqlwhere += KDbEscapedString(escapeIdentifier(f->name())) + '=' +
                            m_driver->valueToSQL(f, val);
                i++;
            }
        }
    } else { //use RecordId
        sqlwhere = KDbEscapedString(escapeIdentifier(m_driver->beh->ROW_ID_FIELD_NAME)) + '='
                   + m_driver->valueToSQL(KDbField::BigInteger, (*data)[data->size() - 1]);
    }
    sql += (sqlset + " WHERE " + sqlwhere);
    //kdbDebug() << " -- SQL == " << ((sql.length() > 400) ? (sql.left(400) + "[.....]") : sql);

    // preprocessing before update
    if (!drv_beforeUpdate(mt->name(), &affectedFields))
        return false;

    bool res = executeSQL(sql);

    // postprocessing after update
    if (!drv_afterUpdate(mt->name(), &affectedFields))
        return false;

    if (!res) {
        m_result = KDbResult(ERR_UPDATE_SERVER_ERROR, QObject::tr("Record updating on the server failed."));
        return false;
    }
    //success: now also assign new values in memory:
    QHash<KDbQueryColumnInfo*, int> columnsOrderExpanded;
    updateRecordDataWithNewValues(query, data, b, &columnsOrderExpanded);
    return true;
}

bool KDbConnection::insertRecord(KDbQuerySchema* query, KDbRecordData* data, KDbRecordEditBuffer* buf, bool getRecordId)
{
// Each SQL identifier needs to be escaped in the generated query.
    clearResult();
    //--get PKEY
    /*disabled: there may be empty records (with autoinc)
    if (buf.dbBuffer().isEmpty()) {
      kdbDebug() << " -- NO CHANGES DATA!";
      return true; }*/
    KDbTableSchema *mt = query->masterTable();
    if (!mt) {
        kdbWarning() << " -- NO MASTER TABLE!";
        m_result = KDbResult(ERR_INSERT_NO_MASTER_TABLE,
                          QObject::tr("Could not insert record because there is no master table specified."));
        return false;
    }
    KDbIndexSchema *pkey = (mt->primaryKey() && !mt->primaryKey()->fields()->isEmpty()) ? mt->primaryKey() : 0;
    if (!getRecordId && !pkey) {
        kdbWarning() << " -- WARNING: NO MASTER TABLE's PKEY";
    }

    KDbEscapedString sqlcols, sqlvals;
    sqlcols.reserve(1024);
    sqlvals.reserve(1024);

    //insert the record:
    KDbEscapedString sql;
    sql.reserve(4096);
    sql = KDbEscapedString("INSERT INTO ") + escapeIdentifier(mt->name()) + " (";
    KDbRecordEditBuffer::DBMap b = buf->dbBuffer();

    // add default values, if available (for any column without value explicitly set)
    const KDbQueryColumnInfo::Vector fieldsExpanded(query->fieldsExpanded(KDbQuerySchema::Unique));
    uint fieldsExpandedCount = fieldsExpanded.count();
    for (uint i = 0; i < fieldsExpandedCount; i++) {
        KDbQueryColumnInfo *ci = fieldsExpanded.at(i);
        if (ci->field && KDb::isDefaultValueAllowed(ci->field)
                && !ci->field->defaultValue().isNull()
                && !b.contains(ci))
        {
            //kdbDebug() << "adding default value" << ci->field->defaultValue().toString() << "for column" << ci->field->name();
            b.insert(ci, ci->field->defaultValue());
        }
    }

    //collect fields which have values in KDbRecordEditBuffer
    KDbFieldList affectedFields;

    if (b.isEmpty()) {
        // empty record inserting requested:
        if (!getRecordId && !pkey) {
            kdbWarning() << "MASTER TABLE's PKEY REQUIRED FOR INSERTING EMPTY RECORDS: INSERT CANCELLED";
            m_result = KDbResult(ERR_INSERT_NO_MASTER_TABLES_PKEY,
                              QObject::tr("Could not insert record because master table has no primary key specified."));
            return false;
        }
        if (pkey) {
            const QVector<int> pkeyFieldsOrder(query->pkeyFieldsOrder());
//   kdbDebug() << pkey->fieldCount() << " ? " << query->pkeyFieldCount();
            if (pkey->fieldCount() != query->pkeyFieldCount()) { //sanity check
                kdbWarning() << "NO ENTIRE MASTER TABLE's PKEY SPECIFIED!";
                m_result = KDbResult(ERR_INSERT_NO_ENTIRE_MASTER_TABLES_PKEY,
                                  QObject::tr("Could not insert record because it does not contain entire master table's primary key."));
                return false;
            }
        }
        //at least one value is needed for VALUES section: find it and set to NULL:
        KDbField *anyField = mt->anyNonPKField();
        if (!anyField) {
            if (!pkey) {
                kdbWarning() << "WARNING: NO FIELD AVAILABLE TO SET IT TO NULL";
                return false;
            } else {
                //try to set NULL in pkey field (could not work for every SQL engine!)
                anyField = pkey->fields()->first();
            }
        }
        sqlcols += escapeIdentifier(anyField->name());
        sqlvals += m_driver->valueToSQL(anyField, QVariant()/*NULL*/);
        affectedFields.addField(anyField);
    } else {
        // non-empty record inserting requested:
        for (KDbRecordEditBuffer::DBMap::ConstIterator it = b.constBegin();it != b.constEnd();++it) {
            if (it.key()->field->table() != mt)
                continue; // skip values for fields outside of the master table (e.g. a "visible value" of the lookup field)
            if (!sqlcols.isEmpty()) {
                sqlcols += ',';
                sqlvals += ',';
            }
            KDbField* currentField = it.key()->field;
            affectedFields.addField(currentField);
            sqlcols += escapeIdentifier(currentField->name());
            sqlvals += m_driver->valueToSQL(currentField, it.value());
        }
    }
    sql += (sqlcols + ") VALUES (" + sqlvals + ')');
// kdbDebug() << " -- SQL == " << sql;

    // do driver specific pre-processing
    if (!drv_beforeInsert(mt->name(), &affectedFields))
        return false;

    bool res = executeSQL(sql);

    // do driver specific post-processing
    if (!drv_afterInsert(mt->name(), &affectedFields))
        return false;

    if (!res) {
        m_result = KDbResult(ERR_INSERT_SERVER_ERROR, QObject::tr("Record inserting on the server failed."));
        return false;
    }
    //success: now also assign a new value in memory:
    QHash<KDbQueryColumnInfo*, int> columnsOrderExpanded;
    updateRecordDataWithNewValues(query, data, b, &columnsOrderExpanded);

    //fetch autoincremented values
    KDbQueryColumnInfo::List *aif_list = query->autoIncrementFields();
    quint64 recordId = 0;
    if (pkey && !aif_list->isEmpty()) {
        //! @todo now only if PKEY is present, this should also work when there's no PKEY
        KDbQueryColumnInfo *id_columnInfo = aif_list->first();
//! @todo safe to cast it?
        quint64 last_id = lastInsertedAutoIncValue(
                              id_columnInfo->field->name(), id_columnInfo->field->table()->name(), &recordId);
        if (last_id == (quint64) - 1 || last_id <= 0) {
            //! @todo show error
//! @todo remove just inserted record. How? Using ROLLBACK?
            return false;
        }
        KDbRecordData aif_data;
        KDbEscapedString getAutoIncForInsertedValue("SELECT "
                                             + query->autoIncrementSQLFieldsList(this)
                                             + " FROM "
                                             + escapeIdentifier(id_columnInfo->field->table()->name())
                                             + " WHERE "
                                             + escapeIdentifier(id_columnInfo->field->name()) + '='
                                             + QByteArray::number(last_id));
        if (true != querySingleRecord(getAutoIncForInsertedValue, &aif_data)) {
            //! @todo show error
            return false;
        }
        uint i = 0;
        foreach(KDbQueryColumnInfo *ci, *aif_list) {
//   kdbDebug() << "AUTOINCREMENTED FIELD" << fi->field->name() << "==" << aif_data[i].toInt();
            ((*data)[ columnsOrderExpanded.value(ci)] = aif_data.value(i)).convert(ci->field->variantType());        //cast to get proper type
            i++;
        }
    } else {
        recordId = drv_lastInsertRecordId();
//  kdbDebug() << "new recordId ==" << (uint)recordId;
        if (m_driver->beh->ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE) {
            kdbWarning() << "m_driver->beh->ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE";
            return false;
        }
    }
    if (getRecordId && /*sanity check*/data->size() > (int)fieldsExpanded.size()) {
//  kdbDebug() << "new ROWID ==" << (uint)ROWID;
        (*data)[data->size() - 1] = recordId;
    }
    return true;
}

bool KDbConnection::deleteRecord(KDbQuerySchema* query, KDbRecordData* data, bool useRecordId)
{
// Each SQL identifier needs to be escaped in the generated query.
    clearResult();
    KDbTableSchema *mt = query->masterTable();
    if (!mt) {
        kdbWarning() << " -- NO MASTER TABLE!";
        m_result = KDbResult(ERR_DELETE_NO_MASTER_TABLE,
                          QObject::tr("Could not delete record because there is no master table specified."));
        return false;
    }
    KDbIndexSchema *pkey = (mt->primaryKey() && !mt->primaryKey()->fields()->isEmpty()) ? mt->primaryKey() : 0;

//! @todo allow to delete from a table without pkey
    if (!useRecordId && !pkey) {
        kdbWarning() << " -- WARNING: NO MASTER TABLE's PKEY";
        m_result = KDbResult(ERR_DELETE_NO_MASTER_TABLES_PKEY,
                          QObject::tr("Could not delete record because there is no primary key for master table specified."));
        return false;
    }

    //update the record:
    KDbEscapedString sql;
    sql.reserve(4096);
    sql = KDbEscapedString("DELETE FROM ") + escapeIdentifier(mt->name()) + " WHERE ";
    KDbEscapedString sqlwhere;
    sqlwhere.reserve(1024);

    if (pkey) {
        const QVector<int> pkeyFieldsOrder(query->pkeyFieldsOrder());
        //kdbDebug() << pkey->fieldCount() << " ? " << query->pkeyFieldCount();
        if (pkey->fieldCount() != query->pkeyFieldCount()) { //sanity check
            kdbWarning() << " -- NO ENTIRE MASTER TABLE's PKEY SPECIFIED!";
            m_result = KDbResult(ERR_DELETE_NO_ENTIRE_MASTER_TABLES_PKEY,
                              QObject::tr("Could not delete record because it does not contain entire master table's primary key."));
            return false;
        }
        uint i = 0;
        foreach(KDbField *f, *pkey->fields()) {
            if (!sqlwhere.isEmpty())
                sqlwhere += " AND ";
            QVariant val(data->at(pkeyFieldsOrder.at(i)));
            if (val.isNull() || !val.isValid()) {
                m_result = KDbResult(ERR_DELETE_NULL_PKEY_FIELD,
                                  QObject::tr("Primary key's field \"%1\" cannot be empty.").arg(f->name()));
//js todo: pass the field's name somewhere!
                return false;
            }
            sqlwhere += KDbEscapedString(escapeIdentifier(f->name())) + '=' +
                         m_driver->valueToSQL(f, val);
            i++;
        }
    } else {//use RecordId
        sqlwhere = KDbEscapedString(escapeIdentifier(m_driver->beh->ROW_ID_FIELD_NAME)) + '='
                    + m_driver->valueToSQL(KDbField::BigInteger, (*data)[data->size() - 1]);
    }
    sql += sqlwhere;
    //kdbDebug() << " -- SQL == " << sql;

    if (!executeSQL(sql)) {
        m_result = KDbResult(ERR_DELETE_SERVER_ERROR, QObject::tr("Record deletion on the server failed."));
        return false;
    }
    return true;
}

bool KDbConnection::deleteAllRecords(KDbQuerySchema* query)
{
    clearResult();
    KDbTableSchema *mt = query->masterTable();
    if (!mt) {
        kdbWarning() << " -- NO MASTER TABLE!";
        return false;
    }
    KDbIndexSchema *pkey = mt->primaryKey();
    if (!pkey || pkey->fields()->isEmpty()) {
        kdbWarning() << "-- WARNING: NO MASTER TABLE's PKEY";
    }
    KDbEscapedString sql = KDbEscapedString("DELETE FROM ") + escapeIdentifier(mt->name());
    //kdbDebug() << "-- SQL == " << sql;

    if (!executeSQL(sql)) {
        m_result = KDbResult(ERR_DELETE_SERVER_ERROR, QObject::tr("Record deletion on the server failed."));
        return false;
    }
    return true;
}

void KDbConnection::registerForTableSchemaChanges(TableSchemaChangeListenerInterface* listener,
                                               KDbTableSchema* schema)
{
    QSet<TableSchemaChangeListenerInterface*>* listeners = d->tableSchemaChangeListeners.value(schema);
    if (!listeners) {
        listeners = new QSet<TableSchemaChangeListenerInterface*>();
        d->tableSchemaChangeListeners.insert(schema, listeners);
    }
    listeners->insert(listener);
}

void KDbConnection::unregisterForTableSchemaChanges(TableSchemaChangeListenerInterface* listener,
                                                 KDbTableSchema* schema)
{
    QSet<TableSchemaChangeListenerInterface*>* listeners = d->tableSchemaChangeListeners.value(schema);
    if (!listeners)
        return;
    listeners->remove(listener);
}

void KDbConnection::unregisterForTablesSchemaChanges(TableSchemaChangeListenerInterface* listener)
{
    foreach(QSet<TableSchemaChangeListenerInterface*> *listeners, d->tableSchemaChangeListeners) {
        listeners->remove(listener);
    }
}

QSet<KDbConnection::TableSchemaChangeListenerInterface*>* KDbConnection::tableSchemaChangeListeners(KDbTableSchema* schema) const
{
    //kdbDebug() << d->tableSchemaChangeListeners.count();
    return d->tableSchemaChangeListeners.value(schema);
}

tristate KDbConnection::closeAllTableSchemaChangeListeners(KDbTableSchema* schema)
{
    QSet<KDbConnection::TableSchemaChangeListenerInterface*> *listeners = d->tableSchemaChangeListeners.value(schema);
    if (!listeners)
        return true;

    //try to close every window
    tristate res = true;
    QList<KDbConnection::TableSchemaChangeListenerInterface*> list(listeners->toList());
    foreach (KDbConnection::TableSchemaChangeListenerInterface* listener, list) {
        res = listener->closeListener();
    }
    return res;
}

KDbConnectionOptions* KDbConnection::options()
{
    return &d->options;
}

void KDbConnection::addCursor(KDbCursor* cursor)
{
    d->cursors.insert(cursor);
}

void KDbConnection::takeCursor(KDbCursor* cursor)
{
    d->cursors.remove(cursor);
}

KDbPreparedStatement KDbConnection::prepareStatement(KDbPreparedStatement::Type type,
    KDbFieldList* fields, const QStringList& whereFieldNames)
{
//! @todo move to ConnectionInterface just like we moved execute() and prepare() to KDbPreparedStatementInterface...
    KDbPreparedStatementInterface *iface = prepareStatementInternal();
    if (!iface)
        return KDbPreparedStatement();
    return KDbPreparedStatement(iface, type, fields, whereFieldNames);
}

KDbEscapedString KDbConnection::recentSQLString() const {
    return result().errorSql().isEmpty() ? m_result.sql() : result().errorSql();
}

KDbEscapedString KDbConnection::escapeString(const QString& str) const
{
    return m_driver->escapeString(str);
}

//! @todo extraMessages
#if 0
static const char *extraMessages[] = {
    QT_TR_NOOP("Unknown error.")
};
#endif

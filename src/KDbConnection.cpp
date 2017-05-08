/* This file is part of the KDE project
   Copyright (C) 2003-2016 Jarosław Staniek <staniek@kde.org>

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
#include "KDbCursor.h"
#include "kdb_debug.h"
#include "KDbDriverBehavior.h"
#include "KDbDriverMetaData.h"
#include "KDbDriver_p.h"
#include "KDbLookupFieldSchema.h"
#include "KDbNativeStatementBuilder.h"
#include "KDbQuerySchema.h"
#include "KDbRecordData.h"
#include "KDbRecordEditBuffer.h"
#include "KDbRelationship.h"
#include "KDbSqlRecord.h"
#include "KDbSqlResult.h"
#include "KDbTableSchemaChangeListener.h"

#include <QDir>
#include <QFileInfo>
#include <QDomDocument>

/*! Version number of extended table schema.

  List of changes:
  * 2: (Kexi 2.5.0) Added maxLengthIsDefault property (type: bool, if true, KDbField::maxLengthStrategy() == KDbField::DefaultMaxLength)
  * 1: (Kexi 1.x) Initial version
*/
#define KDB_EXTENDED_TABLE_SCHEMA_VERSION 2

KDbConnectionInternal::KDbConnectionInternal(KDbConnection *conn)
        : connection(conn)
{
}

class CursorDeleter
{
public:
    explicit CursorDeleter(KDbCursor *cursor) {
        delete cursor;
    }
};

//================================================

class Q_DECL_HIDDEN KDbConnectionOptions::Private
{
public:
    Private() : connection(nullptr) {}
    Private(const Private &other) {
        copy(other);
    }
#define KDbConnectionOptionsPrivateArgs(o) std::tie(o.connection)
    void copy(const Private &other) {
        KDbConnectionOptionsPrivateArgs((*this)) = KDbConnectionOptionsPrivateArgs(other);
    }
    bool operator==(const Private &other) const {
        return KDbConnectionOptionsPrivateArgs((*this)) == KDbConnectionOptionsPrivateArgs(other);
    }
    KDbConnection *connection;
};

KDbConnectionOptions::KDbConnectionOptions()
 : d(new Private)
{
    KDbUtils::PropertySet::insert("readOnly", false, tr("Read only", "Read only connection"));
}

KDbConnectionOptions::KDbConnectionOptions(const KDbConnectionOptions &other)
 : KDbUtils::PropertySet(other)
 , d(new Private(*other.d))
{
}

KDbConnectionOptions::~KDbConnectionOptions()
{
    delete d;
}

KDbConnectionOptions& KDbConnectionOptions::operator=(const KDbConnectionOptions &other)
{
    if (this != &other) {
        KDbUtils::PropertySet::operator=(other);
        d->copy(*other.d);
    }
    return *this;
}

bool KDbConnectionOptions::operator==(const KDbConnectionOptions &other) const
{
    return KDbUtils::PropertySet::operator==(other) && *d == *other.d;
}

bool KDbConnectionOptions::isReadOnly() const
{
    return property("readOnly").value().toBool();
}

void KDbConnectionOptions::insert(const QByteArray &name, const QVariant &value,
                                  const QString &caption)
{
    if (name == "readOnly") {
        setReadOnly(value.toBool());
        return;
    }
    QString realCaption;
    if (property(name).caption().isEmpty()) { // don't allow to change the caption
        realCaption = caption;
    }
    KDbUtils::PropertySet::insert(name, value, realCaption);
}

void KDbConnectionOptions::setCaption(const QByteArray &name, const QString &caption)
{
    if (name == "readOnly") {
        return;
    }
    KDbUtils::PropertySet::setCaption(name, caption);
}

void KDbConnectionOptions::setValue(const QByteArray &name, const QVariant &value)
{
    if (name == "readOnly") {
        setReadOnly(value.toBool());
        return;
    }
    KDbUtils::PropertySet::setValue(name, value);
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
    if (d->connection && d->connection->isConnected()) {
        return; //sanity
    }
    KDbUtils::PropertySet::setValue("readOnly", set);
}

void KDbConnectionOptions::setConnection(KDbConnection *connection)
{
    d->connection = connection;
}

//================================================

KDbConnectionPrivate::KDbConnectionPrivate(KDbConnection* const conn, KDbDriver *drv, const KDbConnectionData& _connData,
                  const KDbConnectionOptions &_options)
        : conn(conn)
        , connData(_connData)
        , options(_options)
        , driver(drv)
        , dbProperties(conn)
{
    options.setConnection(conn);
}

KDbConnectionPrivate::~KDbConnectionPrivate()
{
    options.setConnection(nullptr);
    deleteAllCursors();
    delete m_parser;
    qDeleteAll(tableSchemaChangeListeners);
    qDeleteAll(obsoleteQueries);
}

void KDbConnectionPrivate::deleteAllCursors()
{
    QSet<KDbCursor*> cursorsToDelete(cursors);
    cursors.clear();
    for(KDbCursor* c : cursorsToDelete) {
        CursorDeleter deleter(c);
    }
}

void KDbConnectionPrivate::errorInvalidDBContents(const QString& details)
{
    conn->m_result = KDbResult(ERR_INVALID_DATABASE_CONTENTS,
                               KDbConnection::tr("Invalid database contents. %1").arg(details));
}

QString KDbConnectionPrivate::strItIsASystemObject() const
{
    return KDbConnection::tr("It is a system object.");
}

void KDbConnectionPrivate::setupKDbSystemSchema()
{
    if (!m_internalKDbTables.isEmpty()) {
        return; //already set up
    }
    {
        KDbInternalTableSchema *t_objects = new KDbInternalTableSchema(QLatin1String("kexi__objects"));
        t_objects->addField(new KDbField(QLatin1String("o_id"),
                                      KDbField::Integer, KDbField::PrimaryKey | KDbField::AutoInc, KDbField::Unsigned));
        t_objects->addField(new KDbField(QLatin1String("o_type"), KDbField::Byte, nullptr, KDbField::Unsigned));
        t_objects->addField(new KDbField(QLatin1String("o_name"), KDbField::Text));
        t_objects->addField(new KDbField(QLatin1String("o_caption"), KDbField::Text));
        t_objects->addField(new KDbField(QLatin1String("o_desc"), KDbField::LongText));
        //kdbDebug() << *t_objects;
        insertTable(t_objects);
    }
    {
        KDbInternalTableSchema *t_objectdata = new KDbInternalTableSchema(QLatin1String("kexi__objectdata"));
        t_objectdata->addField(new KDbField(QLatin1String("o_id"),
                                         KDbField::Integer, KDbField::NotNull, KDbField::Unsigned));
        t_objectdata->addField(new KDbField(QLatin1String("o_data"), KDbField::LongText));
        t_objectdata->addField(new KDbField(QLatin1String("o_sub_id"), KDbField::Text));
        insertTable(t_objectdata);
    }
    {
        KDbInternalTableSchema *t_fields = new KDbInternalTableSchema(QLatin1String("kexi__fields"));
        t_fields->addField(new KDbField(QLatin1String("t_id"), KDbField::Integer, nullptr, KDbField::Unsigned));
        t_fields->addField(new KDbField(QLatin1String("f_type"), KDbField::Byte, nullptr, KDbField::Unsigned));
        t_fields->addField(new KDbField(QLatin1String("f_name"), KDbField::Text));
        t_fields->addField(new KDbField(QLatin1String("f_length"), KDbField::Integer));
        t_fields->addField(new KDbField(QLatin1String("f_precision"), KDbField::Integer));
        t_fields->addField(new KDbField(QLatin1String("f_constraints"), KDbField::Integer));
        t_fields->addField(new KDbField(QLatin1String("f_options"), KDbField::Integer));
        t_fields->addField(new KDbField(QLatin1String("f_default"), KDbField::Text));
        //these are additional properties:
        t_fields->addField(new KDbField(QLatin1String("f_order"), KDbField::Integer));
        t_fields->addField(new KDbField(QLatin1String("f_caption"), KDbField::Text));
        t_fields->addField(new KDbField(QLatin1String("f_help"), KDbField::LongText));
        insertTable(t_fields);
    }
    {
        KDbInternalTableSchema *t_db = new KDbInternalTableSchema(QLatin1String("kexi__db"));
        t_db->addField(new KDbField(QLatin1String("db_property"),
                                 KDbField::Text, KDbField::NoConstraints, KDbField::NoOptions, 32));
        t_db->addField(new KDbField(QLatin1String("db_value"), KDbField::LongText));
        insertTable(t_db);
    }
}

void KDbConnectionPrivate::insertTable(KDbTableSchema* tableSchema)
{
    KDbInternalTableSchema* internalTable = dynamic_cast<KDbInternalTableSchema*>(tableSchema);
    if (internalTable) {
        m_internalKDbTables.insert(internalTable);
    } else {
        m_tables.insert(tableSchema->id(), tableSchema);
    }
    m_tablesByName.insert(tableSchema->name(), tableSchema);
}

void KDbConnectionPrivate::removeTable(const KDbTableSchema& tableSchema)
{
    m_tablesByName.remove(tableSchema.name());
    KDbTableSchema *toDelete = m_tables.take(tableSchema.id());
    delete toDelete;
}

void KDbConnectionPrivate::takeTable(KDbTableSchema* tableSchema)
{
    if (m_tables.isEmpty()) {
        return;
    }
    m_tables.take(tableSchema->id());
    m_tablesByName.take(tableSchema->name());
}

void KDbConnectionPrivate::renameTable(KDbTableSchema* tableSchema, const QString& newName)
{
    m_tablesByName.take(tableSchema->name());
    tableSchema->setName(newName);
    m_tablesByName.insert(tableSchema->name(), tableSchema);
}

void KDbConnectionPrivate::changeTableId(KDbTableSchema* tableSchema, int newId)
{
    m_tables.take(tableSchema->id());
    m_tables.insert(newId, tableSchema);
}

void KDbConnectionPrivate::clearTables()
{
    m_tablesByName.clear();
    qDeleteAll(m_internalKDbTables);
    m_internalKDbTables.clear();
    QHash<int, KDbTableSchema*> tablesToDelete(m_tables);
    m_tables.clear();
    qDeleteAll(tablesToDelete);
}

void KDbConnectionPrivate::insertQuery(KDbQuerySchema* query)
{
    m_queries.insert(query->id(), query);
    m_queriesByName.insert(query->name(), query);
}

void KDbConnectionPrivate::removeQuery(KDbQuerySchema* querySchema)
{
    m_queriesByName.remove(querySchema->name());
    m_queries.remove(querySchema->id());
    delete querySchema;
}

void KDbConnectionPrivate::setQueryObsolete(KDbQuerySchema* query)
{
    obsoleteQueries.insert(query);
    m_queriesByName.take(query->name());
    m_queries.take(query->id());
}

void KDbConnectionPrivate::clearQueries()
{
    qDeleteAll(m_queries);
    m_queries.clear();
}

//================================================

namespace {
//! @internal static: list of internal KDb system table names
class SystemTables : public QStringList
{
public:
    SystemTables()
        : QStringList({
            QLatin1String("kexi__objects"),
            QLatin1String("kexi__objectdata"),
            QLatin1String("kexi__fields"),
            QLatin1String("kexi__db")})
    {}
};
}

Q_GLOBAL_STATIC(SystemTables, g_kdbSystemTableNames)

KDbConnection::KDbConnection(KDbDriver *driver, const KDbConnectionData& connData,
                             const KDbConnectionOptions &options)
        : d(new KDbConnectionPrivate(this, driver, connData, options))
{
    if (d->connData.driverId().isEmpty()) {
        d->connData.setDriverId(d->driver->metaData()->id());
    }
}

void KDbConnection::destroy()
{
    disconnect();
    //do not allow the driver to touch me: I will kill myself.
    d->driver->d->connections.remove(this);
}

KDbConnection::~KDbConnection()
{
    KDbConnectionPrivate *thisD = d;
    d = nullptr; // make sure d is nullptr before destructing
    delete thisD;
}

KDbConnectionData KDbConnection::data() const
{
    return d->connData;
}

KDbDriver* KDbConnection::driver() const
{
    return d->driver;
}

bool KDbConnection::connect()
{
    clearResult();
    if (d->isConnected) {
        m_result = KDbResult(ERR_ALREADY_CONNECTED,
                             tr("Connection already established."));
        return false;
    }

    d->serverVersion.clear();
    if (!(d->isConnected = drv_connect())) {
        if (m_result.code() == ERR_NONE) {
            m_result.setCode(ERR_OTHER);
        }
        m_result.setMessage(d->driver->metaData()->isFileBased() ?
                    tr("Could not open \"%1\" project file.")
                       .arg(QDir::fromNativeSeparators(QFileInfo(d->connData.databaseName()).fileName()))
                 :  tr("Could not connect to \"%1\" database server.")
                       .arg(d->connData.toUserVisibleString()));
    }
    if (d->isConnected && !d->driver->beh->USING_DATABASE_REQUIRED_TO_CONNECT) {
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
    m_result = KDbResult(ERR_NO_CONNECTION,
                         tr("Not connected to the database server."));
    return false;
}

bool KDbConnection::checkIsDatabaseUsed()
{
    if (isDatabaseUsed()) {
        clearResult();
        return true;
    }
    m_result = KDbResult(ERR_NO_DB_USED,
                         tr("Currently no database is used."));
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

    QStringList list;
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
    for (QMutableListIterator<QString> it(list); it.hasNext();) {
        if (d->driver->isSystemDatabaseName(it.next())) {
            it.remove();
        }
    }
    return list;
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
            m_result = KDbResult(ERR_OBJECT_NOT_FOUND,
                                 tr("The database \"%1\" does not exist.").arg(dbName));
        return false;
    }

    return true;
}

bool KDbConnection::databaseExists(const QString &dbName, bool ignoreErrors)
{
// kdbDebug() << dbName << ignoreErrors;
    if (d->driver->beh->CONNECTION_REQUIRED_TO_CHECK_DB_EXISTENCE && !checkConnected())
        return false;
    clearResult();

    if (d->driver->metaData()->isFileBased()) {
        //for file-based db: file must exists and be accessible
        QFileInfo file(d->connData.databaseName());
        if (!file.exists() || (!file.isFile() && !file.isSymLink())) {
            if (!ignoreErrors)
                m_result = KDbResult(ERR_OBJECT_NOT_FOUND,
                                     tr("The database file \"%1\" does not exist.")
                                        .arg(QDir::fromNativeSeparators(QFileInfo(d->connData.databaseName()).fileName())));
            return false;
        }
        if (!file.isReadable()) {
            if (!ignoreErrors)
                m_result = KDbResult(ERR_ACCESS_RIGHTS,
                                     tr("Database file \"%1\" is not readable.")
                                        .arg(QDir::fromNativeSeparators(QFileInfo(d->connData.databaseName()).fileName())));
            return false;
        }
        if (!d->options.isReadOnly() && !file.isWritable()) {
            if (!ignoreErrors)
                m_result = KDbResult(ERR_ACCESS_RIGHTS,
                                     tr("Database file \"%1\" is not writable.")
                                        .arg(QDir::fromNativeSeparators(QFileInfo(d->connData.databaseName()).fileName())));
            return false;
        }
        return true;
    }

    QString tmpdbName;
    //some engines need to have opened any database before executing "create database"
    const bool orig_skipDatabaseExistsCheckInUseDatabase = d->skipDatabaseExistsCheckInUseDatabase;
    d->skipDatabaseExistsCheckInUseDatabase = true;
    bool ret = useTemporaryDatabaseIfNeeded(&tmpdbName);
    d->skipDatabaseExistsCheckInUseDatabase = orig_skipDatabaseExistsCheckInUseDatabase;
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
            m_result = KDbResult(KDbConnection::tr("Database \"%1\" has been created but " \
                                 "could not be closed after creation.").arg(dbName)); \
            return false; \
        } }

#define createDatabase_ERROR \
    { createDatabase_CLOSE; return false; }


bool KDbConnection::createDatabase(const QString &dbName)
{
    if (d->driver->beh->CONNECTION_REQUIRED_TO_CREATE_DB && !checkConnected())
        return false;

    if (databaseExists(dbName)) {
        m_result = KDbResult(ERR_OBJECT_EXISTS,
                             tr("Database \"%1\" already exists.").arg(dbName));
        return false;
    }
    if (d->driver->isSystemDatabaseName(dbName)) {
        m_result = KDbResult(ERR_SYSTEM_NAME_RESERVED,
                             tr("Could not create database \"%1\". This name is reserved for system database.").arg(dbName));
        return false;
    }
    if (d->driver->metaData()->isFileBased()) {
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
        m_result.prependMessage(tr("Error creating database \"%1\" on the server.").arg(dbName));
        (void)closeDatabase();//sanity
        return false;
    }

    if (!tmpdbName.isEmpty()) {
        //whatever result is - now we have to close temporary opened database:
        if (!closeDatabase())
            return false;
    }

    if (!tmpdbName.isEmpty() || !d->driver->beh->IS_DB_OPEN_AFTER_CREATE) {
        //db need to be opened
        if (!useDatabase(dbName, false/*not yet kexi compatible!*/)) {
            m_result = KDbResult(tr("Database \"%1\" has been created but could not be opened.").arg(dbName));
            return false;
        }
    } else {
        //just for the rule
        d->usedDatabase = dbName;
        d->isConnected = true;
    }

    KDbTransaction trans;
    if (d->driver->transactionsSupported()) {
        trans = beginTransaction();
        if (!trans.active())
            return false;
    }

    //-create system tables schema objects
    d->setupKDbSystemSchema();

    //-physically create internal KDb tables
    foreach(KDbInternalTableSchema* t, d->internalKDbTables()) {
        if (!drv_createTable(*t))
            createDatabase_ERROR;
    }

    //-insert KDb version info:
    // (for compatibility with Kexi expect the legacy kexidb_major_ver/kexidb_minor_ver values)
    KDbTableSchema *table = d->table(QLatin1String("kexi__db"));
    if (!table)
        createDatabase_ERROR;
    if (!insertRecord(table, QLatin1String("kexidb_major_ver"), KDb::version().major())
            || !insertRecord(table, QLatin1String("kexidb_minor_ver"), KDb::version().minor()))
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

    if (!d->skipDatabaseExistsCheckInUseDatabase) {
        if (!databaseExists(my_dbName, false /*don't ignore errors*/))
            return false; //database must exist
    }

    if (!d->usedDatabase.isEmpty() && !closeDatabase()) //close db if already used
        return false;

    d->usedDatabase.clear();

    if (!drv_useDatabase(my_dbName, cancelled, msgHandler)) {
        if (cancelled && *cancelled)
            return false;
        QString msg(tr("Opening database \"%1\" failed.").arg(my_dbName));
        m_result.prependMessage(msg);
        return false;
    }
    if (d->serverVersion.isNull() && d->driver->beh->USING_DATABASE_REQUIRED_TO_CONNECT) {
        // get version just now, it was not possible earlier
        if (!drv_getServerVersion(&d->serverVersion))
            return false;
    }

    //-create system tables schema objects
    d->setupKDbSystemSchema();

    if (kexiCompatible && my_dbName.compare(anyAvailableDatabaseName(), Qt::CaseInsensitive) != 0) {
        //-get global database information
        bool ok;
        const int major = d->dbProperties.value(QLatin1String("kexidb_major_ver")).toInt(&ok);
        if (!ok) {
            m_result = d->dbProperties.result();
            return false;
        }
        const int minor = d->dbProperties.value(QLatin1String("kexidb_minor_ver")).toInt(&ok);
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
    if (d->driver->transactionsSupported()) {
        //rollback all transactions
        d->dontRemoveTransactions = true; //lock!
        foreach(const KDbTransaction& tr, d->transactions) {
            if (!rollbackTransaction(tr)) {//rollback as much as you can, don't stop on prev. errors
                ret = false;
            } else {
                kdbDebug() << "transaction rolled back!";
                kdbDebug() << "trans.refcount==" << (tr.m_data ? QString::number(tr.m_data->refcount) : QLatin1String("(null)"));
            }
        }
        d->dontRemoveTransactions = false; //unlock!
        d->transactions.clear(); //free trans. data
    }

    //delete own cursors:
    d->deleteAllCursors();
    //delete own schemas
    d->clearTables();
    d->clearQueries();

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
    if (d->driver->beh->USE_TEMPORARY_DATABASE_FOR_CONNECTION_IF_NEEDED && !isDatabaseUsed()) {
        //we have no db used, but it is required by engine to have used any!
        *name = anyAvailableDatabaseName();
        if (name->isEmpty()) {
            m_result = KDbResult(ERR_NO_DB_USED,
                                 tr("Could not find any database for temporary connection."));
            return false;
        }
        const bool orig_skipDatabaseExistsCheckInUseDatabase = d->skipDatabaseExistsCheckInUseDatabase;
        d->skipDatabaseExistsCheckInUseDatabase = true;
        bool ret = useDatabase(*name, false);
        d->skipDatabaseExistsCheckInUseDatabase = orig_skipDatabaseExistsCheckInUseDatabase;
        if (!ret) {
            m_result = KDbResult(m_result.code(),
                                 tr("Error during starting temporary connection using \"%1\" database name.").arg(*name));
            return false;
        }
    }
    return true;
}

bool KDbConnection::dropDatabase(const QString &dbName)
{
    if (d->driver->beh->CONNECTION_REQUIRED_TO_DROP_DB && !checkConnected())
        return false;

    QString dbToDrop;
    if (dbName.isEmpty() && d->usedDatabase.isEmpty()) {
        if (!d->driver->metaData()->isFileBased()
                || (d->driver->metaData()->isFileBased() && d->connData.databaseName().isEmpty()))
        {
            m_result = KDbResult(ERR_NO_NAME_SPECIFIED,
                                 tr("Could not delete database. Name is not specified."));
            return false;
        }
        //this is a file driver so reuse previously passed filename
        dbToDrop = d->connData.databaseName();
    } else {
        if (dbName.isEmpty()) {
            dbToDrop = d->usedDatabase;
        } else {
            if (d->driver->metaData()->isFileBased()) //lets get full path
                dbToDrop = QFileInfo(dbName).absoluteFilePath();
            else
                dbToDrop = dbName;
        }
    }

    if (dbToDrop.isEmpty()) {
        m_result = KDbResult(ERR_NO_NAME_SPECIFIED,
                             tr("Could not delete database. Name is not specified."));
        return false;
    }

    if (d->driver->isSystemDatabaseName(dbToDrop)) {
        m_result = KDbResult(ERR_SYSTEM_NAME_RESERVED,
                             tr("Could not delete system database \"%1\".").arg(dbToDrop));
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
    if (!checkIsDatabaseUsed()) {
        if (ok) {
            *ok = false;
        }
        return QStringList();
    }
    KDbEscapedString sql;
    if (objectType == KDb::AnyObjectType) {
        sql = "SELECT o_name FROM kexi__objects ORDER BY o_id";
    } else {
        sql = KDbEscapedString("SELECT o_name FROM kexi__objects WHERE o_type=%1"
                               " ORDER BY o_id").arg(d->driver->valueToSQL(KDbField::Integer, objectType));
    }
    QStringList list;
    const bool success = queryStringListInternal(&sql, &list, nullptr, nullptr, 0, KDb::isIdentifier);
    if (ok) {
        *ok = success;
    }
    if (!success) {
        m_result.prependMessage(tr("Could not retrieve list of object names."));
    }
    return list;
}

QStringList KDbConnection::tableNames(bool alsoSystemTables, bool* ok)
{
    bool success;
    QStringList list = objectNames(KDb::TableObjectType, &success);
    if (ok) {
        *ok = success;
    }
    if (!success) {
        m_result.prependMessage(tr("Could not retrieve list of table names."));
    }
    if (alsoSystemTables && success) {
        list += kdbSystemTableNames();
    }
    return list;
}

tristate KDbConnection::containsTable(const QString &tableName)
{
    return drv_containsTable(tableName);
}

QStringList KDbConnection::kdbSystemTableNames()
{
    return *g_kdbSystemTableNames;
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

QList<int> KDbConnection::tableIds(bool* ok)
{
    return objectIds(KDb::TableObjectType, ok);
}

QList<int> KDbConnection::queryIds(bool* ok)
{
    return objectIds(KDb::QueryObjectType, ok);
}

QList<int> KDbConnection::objectIds(int objectType, bool* ok)
{
    if (!checkIsDatabaseUsed())
        return QList<int>();

    KDbEscapedString sql;
    if (objectType == KDb::AnyObjectType)
        sql = "SELECT o_id, o_name FROM kexi__objects ORDER BY o_id";
    else
        sql = "SELECT o_id, o_name FROM kexi__objects WHERE o_type=" + QByteArray::number(objectType)
              + " ORDER BY o_id";

    KDbCursor *c = executeQuery(sql);
    if (!c) {
        if (ok) {
            *ok = false;
        }
        m_result.prependMessage(tr("Could not retrieve list of object identifiers."));
        return QList<int>();
    }
    QList<int> list;
    for (c->moveFirst(); !c->eof(); c->moveNext()) {
        QString tname = c->value(1).toString(); //kexi__objects.o_name
        if (KDb::isIdentifier(tname)) {
            list.append(c->value(0).toInt()); //kexi__objects.o_id
        }
    }
    deleteCursor(c);
    if (ok) {
        *ok = true;
    }
    return list;
}

//yeah, it is very efficient:
#define C_A(a) , const QVariant& c ## a

#define V_A0 d->driver->valueToSQL( tableSchema->field(0), c0 )
#define V_A(a) + ',' + d->driver->valueToSQL( \
        tableSchema->field(a) ? tableSchema->field(a)->type() : KDbField::Text, c ## a )

//  kdbDebug() << "******** " << QString("INSERT INTO ") +
//   escapeIdentifier(tableSchema->name()) +
//   " VALUES (" + vals + ")";

QSharedPointer<KDbSqlResult> KDbConnection::insertRecordInternal(const QString &tableSchemaName,
                                                                 KDbFieldList *fields,
                                                                 const KDbEscapedString &sql)
{
    QSharedPointer<KDbSqlResult> res;
    if (!drv_beforeInsert(tableSchemaName,fields )) {
        return res;
    }
    res = prepareSql(sql);
    if (!res || res->lastResult().isError()) {
        res.clear();
        return res;
    }
    if (!drv_afterInsert(tableSchemaName, fields)) {
        res.clear();
        return res;
    }
    {
        // Fetching is needed to perform real execution at least for some backends.
        // Also we are not expecting record but let's delete if there's any.
        (void)res->fetchRecord();
    }
    if (res->lastResult().isError()) {
        res.clear();
    }
    return res;
}

#define C_INS_REC(args, vals) \
    QSharedPointer<KDbSqlResult> KDbConnection::insertRecord(KDbTableSchema* tableSchema args) { \
        return insertRecordInternal(tableSchema->name(), tableSchema, \
                   KDbEscapedString("INSERT INTO ") + escapeIdentifier(tableSchema->name()) \
                       + " (" \
                       + tableSchema->sqlFieldsList(this) \
                       + ") VALUES (" + vals + ')'); \
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

#define V_A0 value += d->driver->valueToSQL( it.next(), c0 );
#define V_A( a ) value += (',' + d->driver->valueToSQL( it.next(), c ## a ));

#define C_INS_REC(args, vals) \
    QSharedPointer<KDbSqlResult> KDbConnection::insertRecord(KDbFieldList* fields args) \
    { \
        KDbEscapedString value; \
        const KDbField::List *flist = fields->fields(); \
        QListIterator<KDbField*> it(*flist); \
        vals \
        it.toFront(); \
        QString tableName((it.hasNext() && it.peekNext()->table()) ? it.next()->table()->name() : QLatin1String("??")); \
        return insertRecordInternal(tableName, fields, \
                   KDbEscapedString(QLatin1String("INSERT INTO ") + escapeIdentifier(tableName)) \
                   + " (" + fields->sqlFieldsList(this) \
                   + ") VALUES (" + value + ')'); \
    }

C_INS_REC_ALL

#undef C_A
#undef V_A
#undef V_ALAST
#undef C_INS_REC
#undef C_INS_REC_ALL

QSharedPointer<KDbSqlResult> KDbConnection::insertRecord(KDbTableSchema *tableSchema,
                                                         const QList<QVariant> &values)
{
// Each SQL identifier needs to be escaped in the generated query.
    QSharedPointer<KDbSqlResult> res;
    const KDbField::List *flist = tableSchema->fields();
    if (flist->isEmpty()) {
        return res;
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
        sql += d->driver->valueToSQL(f, *it);
//  kdbDebug() << "val" << i++ << ": " << d->driver->valueToSQL( f, *it );
        ++it;
        ++fieldsIt;
    }
    sql += ')';
    m_result.setSql(sql);
    res = insertRecordInternal(tableSchema->name(), tableSchema, sql);
    return res;
}

QSharedPointer<KDbSqlResult> KDbConnection::insertRecord(KDbFieldList *fields,
                                                         const QList<QVariant> &values)
{
// Each SQL identifier needs to be escaped in the generated query.
    QSharedPointer<KDbSqlResult> res;
    const KDbField::List *flist = fields->fields();
    if (flist->isEmpty()) {
        return res;
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
        sql += d->driver->valueToSQL(f, *it);
//  kdbDebug() << "val" << i++ << ": " << d->driver->valueToSQL( f, *it );
        ++it;
        ++fieldsIt;
        if (fieldsIt == flist->constEnd())
            break;
    }
    sql += ')';
    m_result.setSql(sql);
    res = insertRecordInternal(tableName, fields, sql);
    return res;
}

inline static bool checkSql(const KDbEscapedString& sql, KDbResult* result)
{
    Q_ASSERT(result);
    if (!sql.isValid()) {
        *result = KDbResult(ERR_SQL_EXECUTION_ERROR,
                            KDbConnection::tr("SQL statement for execution is invalid or empty."));
        result->setErrorSql(sql); //remember for error handling
        return false;
    }
    return true;
}

QSharedPointer<KDbSqlResult> KDbConnection::prepareSql(const KDbEscapedString& sql)
{
    m_result.setSql(sql);
    return QSharedPointer<KDbSqlResult>(drv_prepareSql(sql));
}

bool KDbConnection::executeSql(const KDbEscapedString& sql)
{
    m_result.setSql(sql);
    if (!checkSql(sql, &m_result)) {
        return false;
    }
    if (!drv_executeSql(sql)) {
        m_result.setMessage(QString()); //clear as this could be most probably just "Unknown error" string.
        m_result.setErrorSql(sql);
        m_result.prependMessage(ERR_SQL_EXECUTION_ERROR,
                                tr("Error while executing SQL statement."));
        qWarning() << m_result;
        return false;
    }
    return true;
}

KDbField* KDbConnection::findSystemFieldName(const KDbFieldList& fieldlist)
{
    for (KDbField::ListIterator it(fieldlist.fieldsIterator()); it != fieldlist.fieldsIteratorConstEnd(); ++it) {
        if (d->driver->isSystemFieldName((*it)->name()))
            return *it;
    }
    return nullptr;
}

//! Creates a KDbField list for kexi__fields, for sanity. Used by createTable()
static KDbFieldList* createFieldListForKexi__Fields(KDbTableSchema *kexi__fieldsSchema)
{
    if (!kexi__fieldsSchema)
        return nullptr;
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
static void buildValuesForKexi__Fields(QList<QVariant>& vals, KDbField* f)
{
    const KDbField::Type type = f->type(); // cache: evaluating type of expressions can be expensive
    vals.clear();
    vals
    << QVariant(f->table()->id())
    << QVariant(type)
    << QVariant(f->name())
    << buildLengthValue(*f)
    << QVariant(KDbField::isFPNumericType(type) ? f->precision() : 0)
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
                   f->name() + QLatin1Char('=') + d->driver->valueToSQL(f, *valsIt));
        if (first)
            first = false;
        ++valsIt;
    }
    delete fl;

    sql.append(KDbEscapedString(" WHERE t_id=%1 AND f_name=%2")
                .arg(d->driver->valueToSQL(KDbField::Integer, field->table()->id()))
                .arg(escapeString(field->name())));
    return executeSql(sql);
}

#define createTable_ERR \
    { kdbDebug() << "ERROR!"; \
        m_result.prependMessage(KDbConnection::tr("Creating table failed.")); \
        rollbackAutoCommitTransaction(tg.transaction()); \
        return false; }

bool KDbConnection::createTable(KDbTableSchema* tableSchema, bool replaceExisting)
{
    if (!tableSchema || !checkIsDatabaseUsed())
        return false;

    //check if there are any fields
    if (tableSchema->fieldCount() < 1) {
        clearResult();
        m_result = KDbResult(ERR_CANNOT_CREATE_EMPTY_OBJECT,
                             tr("Could not create table without fields."));
        return false;
    }
    KDbInternalTableSchema* internalTable = dynamic_cast<KDbInternalTableSchema*>(tableSchema);
    const QString tableName(tableSchema->name());

    if (!internalTable) {
        if (d->driver->isSystemObjectName(tableName)) {
            clearResult();
            m_result = KDbResult(ERR_SYSTEM_NAME_RESERVED,
                                 tr("System name \"%1\" cannot be used as table name.")
                                    .arg(tableSchema->name()));
            return false;
        }

        KDbField *sys_field = findSystemFieldName(*tableSchema);
        if (sys_field) {
            clearResult();
            m_result = KDbResult(ERR_SYSTEM_NAME_RESERVED,
                                 tr("System name \"%1\" cannot be used as one of fields in \"%2\" table.")
                                    .arg(sys_field->name(), tableName));
            return false;
        }
    }

    bool previousSchemaStillKept = false;

    KDbTableSchema *existingTable = nullptr;
    if (replaceExisting) {
        //get previous table (do not retrieve, though)
        existingTable = this->tableSchema(tableName);
        if (existingTable) {
            if (existingTable == tableSchema) {
                clearResult();
                m_result = KDbResult(ERR_OBJECT_EXISTS,
                                     tr("Could not create the same table \"%1\" twice.").arg(tableSchema->name()));
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
        if (this->tableSchema(tableSchema->name()) != nullptr) {
            clearResult();
            m_result = KDbResult(ERR_OBJECT_EXISTS,
                                 tr("Table \"%1\" already exists.").arg(tableSchema->name()));
            return false;
        }
    }
    KDbTransactionGuard tg;
    if (!beginAutoCommitTransaction(&tg))
        return false;

    if (internalTable) {
        if (!drv_containsTable(internalTable->name())) { // internal table may exist
            if (!drv_createTable(*tableSchema)) {
                createTable_ERR;
            }
        }
    } else {
        if (!drv_createTable(*tableSchema)) {
            createTable_ERR;
        }
    }

    //add the object data to kexi__* tables
    if (!internalTable) {
        //update kexi__objects
        if (!storeNewObjectData(tableSchema))
            createTable_ERR;

        KDbTableSchema *ts = d->table(QLatin1String("kexi__fields"));
        if (!ts)
            return false;
        //for sanity: remove field info (if any) for this table id
        if (!KDb::deleteRecords(this, *ts, QLatin1String("t_id"), tableSchema->id()))
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
        if (!internalTable) {
            if (previousSchemaStillKept) {
                //remove previous table schema
                d->removeTable(*tableSchema);
            }
        }
        //store locally
        d->insertTable(tableSchema);
        //ok, this table is not created by the connection
        tableSchema->setConnection(this);
    }
    return res;
}

KDbTableSchema *KDbConnection::copyTable(const KDbTableSchema &tableSchema, const KDbObject &newData)
{
    clearResult();
    if (this->tableSchema(tableSchema.name()) != &tableSchema) {
        m_result = KDbResult(ERR_OBJECT_NOT_FOUND,
                             tr("Table \"%1\" does not exist.").arg(tableSchema.name()));
        return nullptr;
    }
    KDbTableSchema *copiedTable = new KDbTableSchema(tableSchema, false /* !copyId*/);
    // copy name, caption, description
    copiedTable->setName(newData.name());
    copiedTable->setCaption(newData.caption());
    copiedTable->setDescription(newData.description());
    // copy the structure and data
    if (!createTable(copiedTable, false /* !replaceExisting */)) {
        delete copiedTable;
        return nullptr;
    }
    if (!drv_copyTableData(tableSchema, *copiedTable)) {
        dropTable(copiedTable);
        delete copiedTable;
        return nullptr;
    }
    return copiedTable;
}

KDbTableSchema *KDbConnection::copyTable(const QString &tableName, const KDbObject &newData)
{
    clearResult();
    KDbTableSchema* ts = tableSchema(tableName);
    if (!ts) {
        m_result = KDbResult(ERR_OBJECT_NOT_FOUND,
                             tr("Table \"%1\" does not exist.").arg(tableName));
        return nullptr;
    }
    return copyTable(*ts, newData);
}

bool KDbConnection::drv_copyTableData(const KDbTableSchema &tableSchema,
                                   const KDbTableSchema &destinationTableSchema)
{
    KDbEscapedString sql = KDbEscapedString("INSERT INTO %1 SELECT * FROM %2")
                .arg(escapeIdentifier(destinationTableSchema.name()))
                .arg(escapeIdentifier(tableSchema.name()));
    return executeSql(sql);
}

bool KDbConnection::removeObject(int objId)
{
    clearResult();
    //remove table schema from kexi__* tables
    KDbTableSchema *kexi__objects = d->table(QLatin1String("kexi__objects"));
    KDbTableSchema *kexi__objectdata = d->table(QLatin1String("kexi__objectdata"));
    if (!kexi__objects || !kexi__objectdata
        || !KDb::deleteRecords(this, *kexi__objects, QLatin1String("o_id"), objId) //schema entry
        || !KDb::deleteRecords(this, *kexi__objectdata, QLatin1String("o_id"), objId)) //data blocks
    {
        m_result = KDbResult(ERR_DELETE_SERVER_ERROR,
                             tr("Could not delete object's data."));
        return false;
    }
    return true;
}

bool KDbConnection::drv_dropTable(const QString& tableName)
{
    return executeSql(KDbEscapedString("DROP TABLE %1").arg(escapeIdentifier(tableName)));
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

    //be sure that we handle the correct KDbTableSchema object:
    if (tableSchema->id() < 0
            || this->tableSchema(tableSchema->name()) != tableSchema
            || this->tableSchema(tableSchema->id()) != tableSchema) {
        m_result = KDbResult(ERR_OBJECT_NOT_FOUND,
                             tr("Could not delete table \"%1\". %2")
                               .arg(tr("Unexpected name or identifier."),
                                    tableSchema->name()));
        return false;
    }

    tristate res = KDbTableSchemaChangeListener::closeListeners(this, tableSchema);
    if (true != res)
        return res;

    //sanity checks:
    if (d->driver->isSystemObjectName(tableSchema->name())) {
        m_result = KDbResult(ERR_SYSTEM_NAME_RESERVED,
                             tr("Could not delete table \"%1\". %2")
                                .arg(tableSchema->name(),
                                     d->strItIsASystemObject()));
        return false;
    }

    KDbTransactionGuard tg;
    if (!beginAutoCommitTransaction(&tg))
        return false;

    //for sanity we're checking if this table exists physically
    const tristate result = drv_containsTable(tableSchema->name());
    if (~result) {
        return cancelled;
    }
    if (result == true) {
        if (!drv_dropTable(tableSchema->name()))
            return false;
    }

    KDbTableSchema *ts = d->table(QLatin1String("kexi__fields"));
    if (!ts || !KDb::deleteRecords(this, *ts, QLatin1String("t_id"), tableSchema->id())) //field entries
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
        m_result = KDbResult(ERR_OBJECT_NOT_FOUND,
                             tr("Table \"%1\" does not exist.").arg(tableName));
        return false;
    }
    return dropTable(ts);
}

tristate KDbConnection::alterTable(KDbTableSchema* tableSchema, KDbTableSchema* newTableSchema)
{
    clearResult();
    tristate res = KDbTableSchemaChangeListener::closeListeners(this, tableSchema);
    if (true != res)
        return res;

    if (tableSchema == newTableSchema) {
        m_result = KDbResult(ERR_OBJECT_THE_SAME,
                             tr("Could not alter table \"%1\" using the same table as destination.")
                                .arg(tableSchema->name()));
        return false;
    }
//! @todo (js) implement real altering
//! @todo (js) update any structure (e.g. query) that depend on this table!
    bool ok = true;
    bool empty;
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
        m_result = KDbResult(ERR_OBJECT_NOT_FOUND,
                             tr("Unknown table \"%1\".").arg(tableSchema->name()));
        return false;
    }
    if (newName.isEmpty() || !KDb::isIdentifier(newName)) {
        m_result = KDbResult(ERR_INVALID_IDENTIFIER,
                             tr("Invalid table name \"%1\".").arg(newName));
        return false;
    }
    const QString oldTableName = tableSchema->name();
    const QString newTableName = newName.trimmed();
    if (oldTableName.trimmed() == newTableName) {
        m_result = KDbResult(ERR_OBJECT_THE_SAME,
                             tr("Could not rename table \"%1\" using the same name.")
                                .arg(newTableName));
        return false;
    }
//! @todo alter table name for server DB backends!
//! @todo what about objects (queries/forms) that use old name?
    KDbTableSchema *tableToReplace = this->tableSchema(newName);
    const bool destTableExists = tableToReplace != nullptr;
    const int origID = destTableExists ? tableToReplace->id() : -1; //will be reused in the new table
    if (!replace && destTableExists) {
        m_result = KDbResult(ERR_OBJECT_EXISTS,
                             tr("Could not rename table \"%1\" to \"%2\". Table \"%3\" already exists.")
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
        if (!dropTable(newName)) {
            return false;
        }

        // the new table owns the previous table's id:
        if (!executeSql(
                    KDbEscapedString("UPDATE kexi__objects SET o_id=%1 WHERE o_id=%2 AND o_type=%3")
                    .arg(d->driver->valueToSQL(KDbField::Integer, origID))
                    .arg(d->driver->valueToSQL(KDbField::Integer, tableSchema->id()))
                    .arg(d->driver->valueToSQL(KDbField::Integer, int(KDb::TableObjectType)))))
        {
            return false;
        }
        if (!executeSql(KDbEscapedString("UPDATE kexi__fields SET t_id=%1 WHERE t_id=%2")
                        .arg(d->driver->valueToSQL(KDbField::Integer, origID))
                        .arg(d->driver->valueToSQL(KDbField::Integer, tableSchema->id()))))
        {
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
    if (!executeSql(KDbEscapedString("UPDATE kexi__objects SET o_name=%1 WHERE o_id=%2")
                    .arg(escapeString(tableSchema->name()))
                    .arg(d->driver->valueToSQL(KDbField::Integer, tableSchema->id()))))
    {
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

    if (!executeSql(KDbEscapedString("ALTER TABLE %1 RENAME TO %2")
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
        m_result = KDbResult(ERR_OBJECT_NOT_FOUND,
                             tr("Query \"%1\" does not exist.").arg(queryName));
        return false;
    }
    return dropQuery(qs);
}

bool KDbConnection::drv_createTable(const KDbTableSchema& tableSchema)
{
    const KDbNativeStatementBuilder builder(this);
    KDbEscapedString sql;
    if (!builder.generateCreateTableStatement(&sql,tableSchema)) {
        return false;
    }
    //kdbDebug() << "******** " << sql;
    return executeSql(sql);
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
    if ((d->driver->beh->features & KDbDriver::IgnoreTransactions)
            || !d->autoCommit) {
        tg->setTransaction(KDbTransaction());
        return true;
    }

    // commit current transaction (if present) for drivers
    // that allow single transaction per connection
    if (d->driver->beh->features & KDbDriver::SingleTransactions) {
        if (d->defaultTransactionStartedInside) //only commit internally started transaction
            if (!commitTransaction(d->default_trans, true)) {
                tg->setTransaction(KDbTransaction());
                return false; //we have a real error
            }

        d->defaultTransactionStartedInside = d->default_trans.isNull();
        if (!d->defaultTransactionStartedInside) {
            tg->setTransaction(d->default_trans);
            tg->doNothing();
            return true; //reuse externally started transaction
        }
    } else if (!(d->driver->beh->features & KDbDriver::MultipleTransactions)) {
        tg->setTransaction(KDbTransaction());
        return true; //no trans. supported at all - just return
    }
    tg->setTransaction(beginTransaction());
    return !m_result.isError();
}

bool KDbConnection::commitAutoCommitTransaction(const KDbTransaction& trans)
{
    if (d->driver->beh->features & KDbDriver::IgnoreTransactions)
        return true;
    if (trans.isNull() || !d->driver->transactionsSupported())
        return true;
    if (d->driver->beh->features & KDbDriver::SingleTransactions) {
        if (!d->defaultTransactionStartedInside) //only commit internally started transaction
            return true; //give up
    }
    return commitTransaction(trans, true);
}

bool KDbConnection::rollbackAutoCommitTransaction(const KDbTransaction& trans)
{
    if (trans.isNull() || !d->driver->transactionsSupported())
        return true;
    return rollbackTransaction(trans);
}

#define SET_ERR_TRANS_NOT_SUPP \
    { m_result = KDbResult(ERR_UNSUPPORTED_DRV_FEATURE, \
                           KDbConnection::tr("Transactions are not supported for \"%1\" driver.").arg( d->driver->metaData()->name() )); }

#define SET_BEGIN_TR_ERROR \
    { if (!m_result.isError()) \
            m_result = KDbResult(ERR_ROLLBACK_OR_COMMIT_TRANSACTION, \
                                 KDbConnection::tr("Begin transaction failed.")); }

KDbTransaction KDbConnection::beginTransaction()
{
    if (!checkIsDatabaseUsed())
        return KDbTransaction();
    KDbTransaction trans;
    if (d->driver->beh->features & KDbDriver::IgnoreTransactions) {
        //we're creating dummy transaction data here,
        //so it will look like active
        trans.m_data = new KDbTransactionData(this);
        d->transactions.append(trans);
        return trans;
    }
    if (d->driver->beh->features & KDbDriver::SingleTransactions) {
        if (d->default_trans.active()) {
            m_result = KDbResult(ERR_TRANSACTION_ACTIVE,
                                 tr("Transaction already started."));
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
    if (d->driver->beh->features & KDbDriver::MultipleTransactions) {
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
    if (!d->driver->transactionsSupported()
            && !(d->driver->beh->features & KDbDriver::IgnoreTransactions)) {
        SET_ERR_TRANS_NOT_SUPP;
        return false;
    }
    KDbTransaction t = trans;
    if (!t.active()) { //try default tr.
        if (!d->default_trans.active()) {
            if (ignore_inactive)
                return true;
            clearResult();
            m_result = KDbResult(ERR_NO_TRANSACTION_ACTIVE,
                                 tr("Transaction not started."));
            return false;
        }
        t = d->default_trans;
        d->default_trans = KDbTransaction(); //now: no default tr.
    }
    bool ret = true;
    if (!(d->driver->beh->features & KDbDriver::IgnoreTransactions))
        ret = drv_commitTransaction(t.m_data);
    if (t.m_data)
        t.m_data->m_active = false; //now this transaction if inactive
    if (!d->dontRemoveTransactions) //true=transaction obj will be later removed from list
        d->transactions.removeAt(d->transactions.indexOf(t));
    if (!ret && !m_result.isError())
        m_result = KDbResult(ERR_ROLLBACK_OR_COMMIT_TRANSACTION,
                             tr("Error on commit transaction."));
    return ret;
}

bool KDbConnection::rollbackTransaction(const KDbTransaction trans, bool ignore_inactive)
{
    if (!isDatabaseUsed())
        return false;
    if (!d->driver->transactionsSupported()
            && !(d->driver->beh->features & KDbDriver::IgnoreTransactions)) {
        SET_ERR_TRANS_NOT_SUPP;
        return false;
    }
    KDbTransaction t = trans;
    if (!t.active()) { //try default tr.
        if (!d->default_trans.active()) {
            if (ignore_inactive)
                return true;
            clearResult();
            m_result = KDbResult(ERR_NO_TRANSACTION_ACTIVE,
                                 tr("Transaction not started."));
            return false;
        }
        t = d->default_trans;
        d->default_trans = KDbTransaction(); //now: no default tr.
    }
    bool ret = true;
    if (!(d->driver->beh->features & KDbDriver::IgnoreTransactions))
        ret = drv_rollbackTransaction(t.m_data);
    if (t.m_data)
        t.m_data->m_active = false; //now this transaction if inactive
    if (!d->dontRemoveTransactions) //true=transaction obj will be later removed from list
        d->transactions.removeAt(d->transactions.indexOf(t));
    if (!ret && !m_result.isError())
        m_result = KDbResult(ERR_ROLLBACK_OR_COMMIT_TRANSACTION,
                             tr("Error on rollback transaction."));
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
    if (!(d->driver->beh->features & KDbDriver::IgnoreTransactions)
            && (!trans.active() || !d->driver->transactionsSupported())) {
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
    if (d->autoCommit == on || d->driver->beh->features & KDbDriver::IgnoreTransactions)
        return true;
    if (!drv_setAutoCommit(on))
        return false;
    d->autoCommit = on;
    return true;
}

KDbTransactionData* KDbConnection::drv_beginTransaction()
{
    if (!executeSql(KDbEscapedString("BEGIN")))
        return nullptr;
    return new KDbTransactionData(this);
}

bool KDbConnection::drv_commitTransaction(KDbTransactionData *)
{
    return executeSql(KDbEscapedString("COMMIT"));
}

bool KDbConnection::drv_rollbackTransaction(KDbTransactionData *)
{
    return executeSql(KDbEscapedString("ROLLBACK"));
}

bool KDbConnection::drv_setAutoCommit(bool /*on*/)
{
    return true;
}

KDbCursor* KDbConnection::executeQuery(const KDbEscapedString& sql, KDbCursor::Options options)
{
    if (sql.isEmpty())
        return nullptr;
    KDbCursor *c = prepareQuery(sql, options);
    if (!c)
        return nullptr;
    if (!c->open()) {//err - kill that
        m_result = c->result();
        CursorDeleter deleter(c);
        return nullptr;
    }
    return c;
}

KDbCursor* KDbConnection::executeQuery(KDbQuerySchema* query, const QList<QVariant>& params,
                                       KDbCursor::Options options)
{
    KDbCursor *c = prepareQuery(query, params, options);
    if (!c)
        return nullptr;
    if (!c->open()) {//err - kill that
        m_result = c->result();
        CursorDeleter deleter(c);
        return nullptr;
    }
    return c;
}

KDbCursor* KDbConnection::executeQuery(KDbQuerySchema* query, KDbCursor::Options options)
{
    return executeQuery(query, QList<QVariant>(), options);
}

KDbCursor* KDbConnection::executeQuery(KDbTableSchema* table, KDbCursor::Options options)
{
    return executeQuery(table->query(), options);
}

KDbCursor* KDbConnection::prepareQuery(KDbTableSchema* table, KDbCursor::Options options)
{
    return prepareQuery(table->query(), options);
}

KDbCursor* KDbConnection::prepareQuery(KDbQuerySchema* query, const QList<QVariant>& params,
                                       KDbCursor::Options options)
{
    KDbCursor* cursor = prepareQuery(query, options);
    if (cursor)
        cursor->setQueryParameters(params);
    return cursor;
}

bool KDbConnection::deleteCursor(KDbCursor *cursor)
{
    if (!cursor)
        return false;
    if (cursor->connection() != this) {//illegal call
        kdbWarning() << "Could not delete the cursor not owned by the same connection!";
        return false;
    }
    const bool ret = cursor->close();
    CursorDeleter deleter(cursor);
    return ret;
}

//! @todo IMPORTANT: fix KDbConnection::setupObjectData() after refactoring
bool KDbConnection::setupObjectData(const KDbRecordData &data, KDbObject *object)
{
    if (data.count() < 5) {
        kdbWarning() << "Aborting, object data should have at least 5 elements, found" << data.count();
        return false;
    }
    bool ok;
    const int id = data[0].toInt(&ok);
    if (!ok)
        return false;
    object->setId(id);
    const QString name(data[2].toString());
    if (!KDb::isIdentifier(name)) {
        m_result = KDbResult(ERR_INVALID_IDENTIFIER,
                             tr("Invalid object name \"%1\".").arg(name));
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
                             .arg(d->driver->valueToSQL(KDbField::Integer, id)),
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
                          .arg(d->driver->valueToSQL(KDbField::Integer, type))
                          .arg(escapeString(name)),
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
                                 .arg(d->driver->valueToSQL(KDbField::Integer, object->type()))
                                 .arg(escapeString(object->name())), &existingID))
        {
            //we already have stored an object data with the same name and type:
            //just update it's properties as it would be existing object
            object->setId(existingID);
            newObject = false;
        }
    }
    if (newObject) {
        if (object->id() <= 0) {//get new ID
            QScopedPointer<KDbFieldList> fl(ts->subList(
                QList<QByteArray>() << "o_type" << "o_name" << "o_caption" << "o_desc"));
            if (!fl) {
                return false;
            }
            QSharedPointer<KDbSqlResult> result
                = insertRecord(fl.data(), QVariant(object->type()), QVariant(object->name()),
                               QVariant(object->caption()), QVariant(object->description()));
            if (!result) {
                return false;
            }
            //fetch newly assigned ID
//! @todo safe to cast it?
            quint64 obj_id = KDb::lastInsertedAutoIncValue(result, QLatin1String("o_id"), *ts);
            //kdbDebug() << "NEW obj_id == " << obj_id;
            if (obj_id == std::numeric_limits<quint64>::max()) {
                return false;
            }
            object->setId(obj_id);
            return true;
        } else {
            QScopedPointer<KDbFieldList> fl(ts->subList(
                QList<QByteArray>() << "o_id" << "o_type" << "o_name" << "o_caption" << "o_desc"));
            return fl && insertRecord(fl.data(), QVariant(object->id()), QVariant(object->type()),
                                      QVariant(object->name()), QVariant(object->caption()),
                                      QVariant(object->description()));
        }
    }
    //existing object:
    return executeSql(
               KDbEscapedString("UPDATE kexi__objects SET o_type=%2, o_caption=%3, o_desc=%4 WHERE o_id=%1")
               .arg(d->driver->valueToSQL(KDbField::Integer, object->id()))
               .arg(d->driver->valueToSQL(KDbField::Integer, object->type()))
               .arg(escapeString(object->caption()))
               .arg(escapeString(object->description())));
}

bool KDbConnection::storeObjectData(KDbObject* object)
{
    return storeObjectDataInternal(object, false);
}

bool KDbConnection::storeNewObjectData(KDbObject* object)
{
    return storeObjectDataInternal(object, true);
}

QString KDbConnection::escapeIdentifier(const QString& id, KDb::IdentifierEscapingType escapingType) const
{
    return escapingType == KDb::KDbEscaping
        ? KDb::escapeIdentifier(id) : escapeIdentifier(id);
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
        return nullptr;
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
        m_result.setSql(d->driver->addLimitTo1(*sql, addLimitTo1));
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
    return querySingleRecordInternal(data, &sql, nullptr, nullptr, addLimitTo1);
}

tristate KDbConnection::querySingleRecord(KDbQuerySchema* query, KDbRecordData* data, bool addLimitTo1)
{
    return querySingleRecordInternal(data, nullptr, query, nullptr, addLimitTo1);
}

tristate KDbConnection::querySingleRecord(KDbQuerySchema* query, KDbRecordData* data,
                                          const QList<QVariant>& params, bool addLimitTo1)
{
    return querySingleRecordInternal(data, nullptr, query, &params, addLimitTo1);
}

bool KDbConnection::checkIfColumnExists(KDbCursor *cursor, int column)
{
    if (column >= cursor->fieldCount()) {
        m_result = KDbResult(ERR_CURSOR_RECORD_FETCHING,
                             tr("Column \"%1\" does not exist in the query.").arg(column));
        return false;
    }
    return true;
}

tristate KDbConnection::querySingleStringInternal(const KDbEscapedString* sql,
                                                  QString* value, KDbQuerySchema* query,
                                                  const QList<QVariant>* params,
                                                  int column, bool addLimitTo1)
{
    Q_ASSERT(sql || query);
    if (sql) {
        //! @todo does not work with non-SQL data sources
        m_result.setSql(d->driver->addLimitTo1(*sql, addLimitTo1));
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
                                          int column, bool addLimitTo1)
{
    return querySingleStringInternal(&sql, value, nullptr, nullptr, column, addLimitTo1);
}

tristate KDbConnection::querySingleString(KDbQuerySchema* query, QString* value, int column,
                                          bool addLimitTo1)
{
    return querySingleStringInternal(nullptr, value, query, nullptr, column, addLimitTo1);
}

tristate KDbConnection::querySingleString(KDbQuerySchema* query, QString* value,
                                          const QList<QVariant>& params, int column,
                                          bool addLimitTo1)
{
    return querySingleStringInternal(nullptr, value, query, &params, column, addLimitTo1);
}

tristate KDbConnection::querySingleNumberInternal(const KDbEscapedString* sql,
                                                  int* number, KDbQuerySchema* query,
                                                  const QList<QVariant>* params,
                                                  int column, bool addLimitTo1)
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
                                          int column, bool addLimitTo1)
{
    return querySingleNumberInternal(&sql, number, nullptr, nullptr, column, addLimitTo1);
}

tristate KDbConnection::querySingleNumber(KDbQuerySchema* query, int* number, int column,
                                          bool addLimitTo1)
{
    return querySingleNumberInternal(nullptr, number, query, nullptr, column, addLimitTo1);
}

tristate KDbConnection::querySingleNumber(KDbQuerySchema* query, int* number,
                                          const QList<QVariant>& params, int column,
                                          bool addLimitTo1)
{
    return querySingleNumberInternal(nullptr, number, query, &params, column, addLimitTo1);
}

bool KDbConnection::queryStringListInternal(const KDbEscapedString* sql,
                                            QStringList* list, KDbQuerySchema* query,
                                            const QList<QVariant>* params,
                                            int column,
                                            bool (*filterFunction)(const QString&))
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
        const QString str(cursor->value(column).toString());
        if (!filterFunction || filterFunction(str)) {
            list->append(str);
        }
        if (!cursor->moveNext() && cursor->result().isError()) {
            m_result = cursor->result();
            deleteCursor(cursor);
            return false;
        }
    }
    return deleteCursor(cursor);
}

bool KDbConnection::queryStringList(const KDbEscapedString& sql, QStringList* list,
                                    int column)
{
    return queryStringListInternal(&sql, list, nullptr, nullptr, column, nullptr);
}

bool KDbConnection::queryStringList(KDbQuerySchema* query, QStringList* list, int column)
{
    return queryStringListInternal(nullptr, list, query, nullptr, column, nullptr);
}

bool KDbConnection::queryStringList(KDbQuerySchema* query, QStringList* list,
                                    const QList<QVariant>& params, int column)
{
    return queryStringListInternal(nullptr, list, query, &params, column, nullptr);
}

tristate KDbConnection::resultExists(const KDbEscapedString& sql, bool addLimitTo1)
{
    //optimization
    if (d->driver->beh->SELECT_1_SUBQUERY_SUPPORTED) {
        //this is at least for sqlite
        if (addLimitTo1 && sql.left(6).toUpper() == "SELECT") {
            m_result.setSql(
                d->driver->addLimitTo1("SELECT 1 FROM (" + sql + ')', addLimitTo1));
        }
        else {
            m_result.setSql(sql);
        }
    } else {
        if (addLimitTo1 && sql.startsWith("SELECT")) {
            m_result.setSql(d->driver->addLimitTo1(sql, addLimitTo1));
        }
        else {
            m_result.setSql(sql);
        }
    }
    KDbCursor *cursor = executeQuery(m_result.sql());
    if (!cursor) {
        kdbWarning() << "!executeQuery()" << m_result.sql();
        return cancelled;
    }
    if (!cursor->moveFirst() || cursor->eof()) {
        kdbWarning() << "!cursor->moveFirst() || cursor->eof()" << m_result.sql();
        m_result = cursor->result();
        deleteCursor(cursor);
        return m_result.isError() ? cancelled : tristate(false);
    }
    return deleteCursor(cursor) ? tristate(true) : cancelled;
}

tristate KDbConnection::isEmpty(KDbTableSchema* table)
{
    const KDbNativeStatementBuilder builder(this);
    KDbEscapedString sql;
    if (!builder.generateSelectStatement(&sql, table)) {
        return cancelled;
    }
    const tristate result = resultExists(sql);
    if (~result) {
        return cancelled;
    }
    return result == false;
}

//! Used by addFieldPropertyToExtendedTableSchemaData()
static void createExtendedTableSchemaMainElementIfNeeded(
    QDomDocument* doc, QDomElement* extendedTableSchemaMainEl,
    bool* extendedTableSchemaStringIsEmpty)
{
    if (!*extendedTableSchemaStringIsEmpty)
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
        const KDbField::Type type = f->type(); // cache: evaluating type of expressions can be expensive
        if (f->visibleDecimalPlaces() >= 0/*nondefault*/ && KDb::supportsVisibleDecimalPlacesProperty(type)) {
            addFieldPropertyToExtendedTableSchemaData(
                *f, "visibleDecimalPlaces", f->visibleDecimalPlaces(), &doc,
                &extendedTableSchemaMainEl, &extendedTableSchemaFieldEl,
                &extendedTableSchemaStringIsEmpty);
        }
        if (type == KDbField::Text) {
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
    { m_result = KDbResult(tr("Error while loading extended table schema.", \
                              "Extended schema for a table: loading error")); \
      return false; }
#define loadExtendedTableSchemaData_ERR2(details) \
    { m_result = KDbResult(details); \
      m_result.setMessageTitle(tr("Error while loading extended table schema.", \
                                  "Extended schema for a table: loading error")); \
      return false; }
#define loadExtendedTableSchemaData_ERR3(data) \
    { m_result = KDbResult(tr("Invalid XML data: %1").arg(data.left(1024))); \
      m_result.setMessageTitle(tr("Error while loading extended table schema.", \
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
            tr("Error in XML data: \"%1\" in line %2, column %3.\nXML data: %4")
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
        return nullptr;
    KDbField::Type f_type = (KDbField::Type)f_int_type;
    int f_len = qMax(0, data.at(3).toInt(&ok)); // defined limit
    if (!ok) {
        return nullptr;
    }
    if (f_len < 0) {
        f_len = 0;
    }
//! @todo load maxLengthStrategy info to see if the maxLength is the default

    int f_prec = data.at(4).toInt(&ok);
    if (!ok)
        return nullptr;
    KDbField::Constraints f_constr = (KDbField::Constraints)data.at(5).toInt(&ok);
    if (!ok)
        return nullptr;
    KDbField::Options f_opts = (KDbField::Options)data.at(6).toInt(&ok);
    if (!ok)
        return nullptr;

    QString name(data.at(2).toString());
    if (!KDb::isIdentifier(name)) {
        name = KDb::stringToIdentifier(name);
    }

    KDbField *f = new KDbField(
        name, f_type, f_constr, f_opts, f_len, f_prec);

    QVariant defaultVariant = data.at(7);
    if (defaultVariant.isValid()) {
        defaultVariant = KDb::stringToVariant(defaultVariant.toString(), KDbField::variantType(f_type), &ok);
        if (ok) {
            f->setDefaultValue(defaultVariant);
        } else {
            kdbWarning() << "problem with KDb::stringToVariant(" << defaultVariant << ')';
            ok = true; //problem with defaultValue is not critical
        }
    }

    f->setCaption(data.at(9).toString());
    f->setDescription(data.at(10).toString());
    return f;
}

KDbTableSchema* KDbConnection::setupTableSchema(const KDbRecordData &data)
{
    KDbTableSchema *t = new KDbTableSchema(this);
    if (!setupObjectData(data, t)) {
        delete t;
        return nullptr;
    }

    KDbCursor *cursor;
    if (!(cursor = executeQuery(
            KDbEscapedString("SELECT t_id, f_type, f_name, f_length, f_precision, f_constraints, "
                             "f_options, f_default, f_order, f_caption, f_help "
                             "FROM kexi__fields WHERE t_id=%1 ORDER BY f_order")
                            .arg(d->driver->valueToSQL(KDbField::Integer, t->id())))))
    {
        delete t;
        return nullptr;
    }
    if (!cursor->moveFirst()) {
        if (!cursor->result().isError() && cursor->eof()) {
            m_result = KDbResult(tr("Table has no fields defined."));
        }
        deleteCursor(cursor);
        delete t;
        return nullptr;
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
        if (!f || !t->addField(f)) {
            ok = false;
            break;
        }
        cursor->moveNext();
    }

    if (!ok) {//error:
        deleteCursor(cursor);
        delete t;
        return nullptr;
    }

    if (!deleteCursor(cursor)) {
        delete t;
        return nullptr;
    }

    if (!loadExtendedTableSchemaData(t)) {
        delete t;
        return nullptr;
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
                             "WHERE o_name=%1 AND o_type=%2")
                             .arg(escapeString(tableName))
                             .arg(d->driver->valueToSQL(KDbField::Integer, KDb::TableObjectType)), &data))
    {
        return nullptr;
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
                             .arg(d->driver->valueToSQL(KDbField::Integer, tableId)), &data))
    {
        return nullptr;
    }
    return setupTableSchema(data);
}

tristate KDbConnection::loadDataBlock(int objectID, QString* dataString, const QString& dataID)
{
    if (objectID <= 0)
        return false;
    return querySingleString(
               KDbEscapedString("SELECT o_data FROM kexi__objectdata WHERE o_id=%1 AND ")
                                .arg(d->driver->valueToSQL(KDbField::Integer, objectID))
                                + KDbEscapedString(KDb::sqlWhere(d->driver, KDbField::Text,
                                                QLatin1String("o_sub_id"),
                                                dataID.isEmpty() ? QVariant() : QVariant(dataID))),
               dataString);
}

bool KDbConnection::storeDataBlock(int objectID, const QString &dataString, const QString& dataID)
{
    if (objectID <= 0)
        return false;
    KDbEscapedString sql(
        KDbEscapedString("SELECT kexi__objectdata.o_id FROM kexi__objectdata WHERE o_id=%1")
                        .arg(d->driver->valueToSQL(KDbField::Integer, objectID)));
    KDbEscapedString sql_sub(KDb::sqlWhere(d->driver, KDbField::Text, QLatin1String("o_sub_id"),
                                              dataID.isEmpty() ? QVariant() : QVariant(dataID)));

    const tristate result = resultExists(sql + " AND " + sql_sub);
    if (~result) {
        return false;
    }
    if (result == true) {
        return executeSql(KDbEscapedString("UPDATE kexi__objectdata SET o_data=%1 WHERE o_id=%2 AND ")
                          .arg(d->driver->valueToSQL(KDbField::LongText, dataString))
                          .arg(d->driver->valueToSQL(KDbField::Integer, objectID))
                          + sql_sub);
    }
    return executeSql(
               KDbEscapedString("INSERT INTO kexi__objectdata (o_id, o_data, o_sub_id) VALUES (")
               + KDbEscapedString::number(objectID) + ',' + d->driver->valueToSQL(KDbField::LongText, dataString)
               + ',' + d->driver->valueToSQL(KDbField::Text, dataID) + ')');
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
         .arg(d->driver->valueToSQL(KDbField::Integer, destObjectID))
         .arg(d->driver->valueToSQL(KDbField::Integer, sourceObjectID));
    if (!dataID.isEmpty()) {
        sql += KDbEscapedString(" AND ") + KDb::sqlWhere(d->driver, KDbField::Text,
                                                            QLatin1String("o_sub_id"), dataID);
    }
    return executeSql(sql);
}

bool KDbConnection::removeDataBlock(int objectID, const QString& dataID)
{
    if (objectID <= 0)
        return false;
    if (dataID.isEmpty())
        return KDb::deleteRecords(this, QLatin1String("kexi__objectdata"),
                                       QLatin1String("o_id"), QString::number(objectID));
    else
        return KDb::deleteRecords(this, QLatin1String("kexi__objectdata"),
                                       QLatin1String("o_id"), KDbField::Integer, objectID,
                                       QLatin1String("o_sub_id"), KDbField::Text, dataID);
}

KDbQuerySchema* KDbConnection::setupQuerySchema(const KDbRecordData &data)
{
    bool ok = true;
    const int objID = data[0].toInt(&ok);
    if (!ok)
        return nullptr;
    QString sql;
    if (!loadDataBlock(objID, &sql, QLatin1String("sql"))) {
        m_result = KDbResult(ERR_OBJECT_NOT_FOUND,
                             tr("Could not find definition for query \"%1\". Deleting this query is recommended.").arg(data[2].toString()));
        return nullptr;
    }
    KDbQuerySchema *query = nullptr;
    if (d->parser()->parse(KDbEscapedString(sql))) {
        query = d->parser()->query();
    }
    //error?
    if (!query) {
        m_result = KDbResult(ERR_SQL_PARSE_ERROR,
                             tr("<p>Could not load definition for query \"%1\". "
                                "SQL statement for this query is invalid:<br><tt>%2</tt></p>\n"
                                "<p>This query can be edited only in Text View.</p>")
                                .arg(data[2].toString(), sql));
        return nullptr;
    }
    if (!setupObjectData(data, query)) {
        delete query;
        return nullptr;
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
                             "WHERE o_name=%1 AND o_type=%2")
                             .arg(escapeString(m_queryName))
                             .arg(d->driver->valueToSQL(KDbField::Integer, int(KDb::QueryObjectType))),
            &data))
    {
        return nullptr;
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
                             .arg(d->driver->valueToSQL(KDbField::Integer, queryId)),
            &data))
    {
        return nullptr;
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

QString KDbConnection::escapeIdentifier(const QString& id) const
{
    return d->driver->escapeIdentifier(id);
}

bool KDbConnection::isInternalTableSchema(const QString& tableName)
{
    KDbTableSchema* schema = d->table(tableName);
    return (schema && schema->isInternal())
           // these are here for compatiblility because we're no longer instantiate
           // them but can exist in projects created with previous Kexi versions:
           || tableName == QLatin1String("kexi__final") || tableName == QLatin1String("kexi__useractions");
}

void KDbConnection::removeMe(KDbTableSchema *table)
{
    if (table && d) {
        d->takeTable(table);
    }
}

QString KDbConnection::anyAvailableDatabaseName()
{
    if (!d->availableDatabaseName.isEmpty()) {
        return d->availableDatabaseName;
    }
    return d->driver->beh->ALWAYS_AVAILABLE_DATABASE_NAME;
}

void KDbConnection::setAvailableDatabaseName(const QString& dbName)
{
    d->availableDatabaseName = dbName;
}

//! @internal used in updateRecord(), insertRecord(),
inline static void updateRecordDataWithNewValues(KDbQuerySchema* query, KDbRecordData* data,
                                                 const KDbRecordEditBuffer::DbHash& b,
                                                 QHash<KDbQueryColumnInfo*, int>* columnsOrderExpanded)
{
    *columnsOrderExpanded = query->columnsOrder(KDbQuerySchema::ExpandedList);
    QHash<KDbQueryColumnInfo*, int>::ConstIterator columnsOrderExpandedIt;
    for (KDbRecordEditBuffer::DbHash::ConstIterator it = b.constBegin();it != b.constEnd();++it) {
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
                             tr("Could not update record because there is no master table defined."));
        return false;
    }
    KDbIndexSchema *pkey = (mt->primaryKey() && !mt->primaryKey()->fields()->isEmpty()) ? mt->primaryKey() : nullptr;
    if (!useRecordId && !pkey) {
        kdbWarning() << " -- NO MASTER TABLE's PKEY!";
        m_result = KDbResult(ERR_UPDATE_NO_MASTER_TABLES_PKEY,
                             tr("Could not update record because master table has no primary key defined."));
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
    KDbRecordEditBuffer::DbHash b = buf->dbBuffer();

    //gather the fields which are updated ( have values in KDbRecordEditBuffer)
    KDbFieldList affectedFields;
    for (KDbRecordEditBuffer::DbHash::ConstIterator it = b.constBegin();it != b.constEnd();++it) {
        if (it.key()->field()->table() != mt)
            continue; // skip values for fields outside of the master table (e.g. a "visible value" of the lookup field)
        if (!sqlset.isEmpty())
            sqlset += ',';
        KDbField* currentField = it.key()->field();
        const bool affectedFieldsAddOk = affectedFields.addField(currentField);
        Q_ASSERT(affectedFieldsAddOk);
        sqlset += KDbEscapedString(escapeIdentifier(currentField->name())) + '=' +
                  d->driver->valueToSQL(currentField, it.value());
    }
    if (pkey) {
        const QVector<int> pkeyFieldsOrder(query->pkeyFieldsOrder());
        //kdbDebug() << pkey->fieldCount() << " ? " << query->pkeyFieldCount();
        if (pkey->fieldCount() != query->pkeyFieldCount()) { //sanity check
            kdbWarning() << " -- NO ENTIRE MASTER TABLE's PKEY SPECIFIED!";
            m_result = KDbResult(ERR_UPDATE_NO_ENTIRE_MASTER_TABLES_PKEY,
                                 tr("Could not update record because it does not contain entire primary key of master table."));
            return false;
        }
        if (!pkey->fields()->isEmpty()) {
            int i = 0;
            foreach(KDbField *f, *pkey->fields()) {
                if (!sqlwhere.isEmpty())
                    sqlwhere += " AND ";
                QVariant val(data->at(pkeyFieldsOrder.at(i)));
                if (val.isNull() || !val.isValid()) {
                    m_result = KDbResult(ERR_UPDATE_NULL_PKEY_FIELD,
                                         tr("Primary key's field \"%1\" cannot be empty.").arg(f->name()));
                    //js todo: pass the field's name somewhere!
                    return false;
                }
                sqlwhere += KDbEscapedString(escapeIdentifier(f->name())) + '=' +
                            d->driver->valueToSQL(f, val);
                i++;
            }
        }
    } else { //use RecordId
        sqlwhere = KDbEscapedString(escapeIdentifier(d->driver->beh->ROW_ID_FIELD_NAME)) + '='
                   + d->driver->valueToSQL(KDbField::BigInteger, (*data)[data->size() - 1]);
    }
    sql += (sqlset + " WHERE " + sqlwhere);
    //kdbDebug() << " -- SQL == " << ((sql.length() > 400) ? (sql.left(400) + "[.....]") : sql);

    // preprocessing before update
    if (!drv_beforeUpdate(mt->name(), &affectedFields))
        return false;

    bool res = executeSql(sql);

    // postprocessing after update
    if (!drv_afterUpdate(mt->name(), &affectedFields))
        return false;

    if (!res) {
        m_result = KDbResult(ERR_UPDATE_SERVER_ERROR,
                             tr("Record updating on the server failed."));
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
                             tr("Could not insert record because there is no master table specified."));
        return false;
    }
    KDbIndexSchema *pkey = (mt->primaryKey() && !mt->primaryKey()->fields()->isEmpty()) ? mt->primaryKey() : nullptr;
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
    KDbRecordEditBuffer::DbHash b = buf->dbBuffer();

    // add default values, if available (for any column without value explicitly set)
    const KDbQueryColumnInfo::Vector fieldsExpanded(query->fieldsExpanded(KDbQuerySchema::Unique));
    int fieldsExpandedCount = fieldsExpanded.count();
    for (int i = 0; i < fieldsExpandedCount; i++) {
        KDbQueryColumnInfo *ci = fieldsExpanded.at(i);
        if (ci->field() && KDb::isDefaultValueAllowed(*ci->field())
                && !ci->field()->defaultValue().isNull()
                && !b.contains(ci))
        {
            //kdbDebug() << "adding default value" << ci->field->defaultValue().toString() << "for column" << ci->field->name();
            b.insert(ci, ci->field()->defaultValue());
        }
    }

    //collect fields which have values in KDbRecordEditBuffer
    KDbFieldList affectedFields;

    if (b.isEmpty()) {
        // empty record inserting requested:
        if (!getRecordId && !pkey) {
            kdbWarning() << "MASTER TABLE's PKEY REQUIRED FOR INSERTING EMPTY RECORDS: INSERT CANCELLED";
            m_result = KDbResult(ERR_INSERT_NO_MASTER_TABLES_PKEY,
                                 tr("Could not insert record because master table has no primary key specified."));
            return false;
        }
        if (pkey) {
            const QVector<int> pkeyFieldsOrder(query->pkeyFieldsOrder());
//   kdbDebug() << pkey->fieldCount() << " ? " << query->pkeyFieldCount();
            if (pkey->fieldCount() != query->pkeyFieldCount()) { //sanity check
                kdbWarning() << "NO ENTIRE MASTER TABLE's PKEY SPECIFIED!";
                m_result = KDbResult(ERR_INSERT_NO_ENTIRE_MASTER_TABLES_PKEY,
                                     tr("Could not insert record because it does not contain entire master table's primary key."));
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
        sqlvals += d->driver->valueToSQL(anyField, QVariant()/*NULL*/);
        const bool affectedFieldsAddOk = affectedFields.addField(anyField);
        Q_ASSERT(affectedFieldsAddOk);
    } else {
        // non-empty record inserting requested:
        for (KDbRecordEditBuffer::DbHash::ConstIterator it = b.constBegin();it != b.constEnd();++it) {
            if (it.key()->field()->table() != mt)
                continue; // skip values for fields outside of the master table (e.g. a "visible value" of the lookup field)
            if (!sqlcols.isEmpty()) {
                sqlcols += ',';
                sqlvals += ',';
            }
            KDbField* currentField = it.key()->field();
            const bool affectedFieldsAddOk = affectedFields.addField(currentField);
            Q_ASSERT(affectedFieldsAddOk);
            sqlcols += escapeIdentifier(currentField->name());
            sqlvals += d->driver->valueToSQL(currentField, it.value());
        }
    }
    sql += (sqlcols + ") VALUES (" + sqlvals + ')');
// kdbDebug() << " -- SQL == " << sql;

    // low-level insert
    QSharedPointer<KDbSqlResult> result = insertRecordInternal(mt->name(), &affectedFields, sql);
    if (!result) {
        m_result = KDbResult(ERR_INSERT_SERVER_ERROR,
                             tr("Record inserting on the server failed."));
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
        quint64 last_id
            = KDb::lastInsertedAutoIncValue(result, id_columnInfo->field()->name(),
                                            id_columnInfo->field()->table()->name(), &recordId);
        if (last_id == std::numeric_limits<quint64>::max()) {
            //! @todo show error
//! @todo remove just inserted record. How? Using ROLLBACK?
            return false;
        }
        KDbRecordData aif_data;
        KDbEscapedString getAutoIncForInsertedValue("SELECT "
                                             + query->autoIncrementSQLFieldsList(this)
                                             + " FROM "
                                             + escapeIdentifier(id_columnInfo->field()->table()->name())
                                             + " WHERE "
                                             + escapeIdentifier(id_columnInfo->field()->name()) + '='
                                             + QByteArray::number(last_id));
        if (true != querySingleRecord(getAutoIncForInsertedValue, &aif_data)) {
            //! @todo show error
            return false;
        }
        int i = 0;
        foreach(KDbQueryColumnInfo *ci, *aif_list) {
//   kdbDebug() << "AUTOINCREMENTED FIELD" << fi->field->name() << "==" << aif_data[i].toInt();
            ((*data)[ columnsOrderExpanded.value(ci)]
                = aif_data.value(i)).convert(ci->field()->variantType()); //cast to get proper type
            i++;
        }
    } else {
        recordId = result->lastInsertRecordId();
//  kdbDebug() << "new recordId ==" << recordId;
        if (d->driver->beh->ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE) {
            kdbWarning() << "d->driver->beh->ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE";
            return false;
        }
    }
    if (getRecordId && /*sanity check*/data->size() > fieldsExpanded.size()) {
//  kdbDebug() << "new ROWID ==" << ROWID;
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
                             tr("Could not delete record because there is no master table specified."));
        return false;
    }
    KDbIndexSchema *pkey = (mt->primaryKey() && !mt->primaryKey()->fields()->isEmpty()) ? mt->primaryKey() : nullptr;

//! @todo allow to delete from a table without pkey
    if (!useRecordId && !pkey) {
        kdbWarning() << " -- WARNING: NO MASTER TABLE's PKEY";
        m_result = KDbResult(ERR_DELETE_NO_MASTER_TABLES_PKEY,
                             tr("Could not delete record because there is no primary key for master table specified."));
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
                                 tr("Could not delete record because it does not contain entire master table's primary key."));
            return false;
        }
        int i = 0;
        foreach(KDbField *f, *pkey->fields()) {
            if (!sqlwhere.isEmpty())
                sqlwhere += " AND ";
            QVariant val(data->at(pkeyFieldsOrder.at(i)));
            if (val.isNull() || !val.isValid()) {
                m_result = KDbResult(ERR_DELETE_NULL_PKEY_FIELD,
                                     tr("Primary key's field \"%1\" cannot be empty.").arg(f->name()));
//js todo: pass the field's name somewhere!
                return false;
            }
            sqlwhere += KDbEscapedString(escapeIdentifier(f->name())) + '=' +
                         d->driver->valueToSQL(f, val);
            i++;
        }
    } else {//use RecordId
        sqlwhere = KDbEscapedString(escapeIdentifier(d->driver->beh->ROW_ID_FIELD_NAME)) + '='
                    + d->driver->valueToSQL(KDbField::BigInteger, (*data)[data->size() - 1]);
    }
    sql += sqlwhere;
    //kdbDebug() << " -- SQL == " << sql;

    if (!executeSql(sql)) {
        m_result = KDbResult(ERR_DELETE_SERVER_ERROR,
                             tr("Record deletion on the server failed."));
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

    if (!executeSql(sql)) {
        m_result = KDbResult(ERR_DELETE_SERVER_ERROR,
                             tr("Record deletion on the server failed."));
        return false;
    }
    return true;
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
    if (d && !d->cursors.isEmpty()) { // checking because this may be called from ~KDbConnection()
        d->cursors.remove(cursor);
    }
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

KDbEscapedString KDbConnection::recentSqlString() const {
    return result().errorSql().isEmpty() ? m_result.sql() : result().errorSql();
}

KDbEscapedString KDbConnection::escapeString(const QString& str) const
{
    return d->driver->escapeString(str);
}

//! @todo extraMessages
#if 0
static const char *extraMessages[] = {
    QT_TRANSLATE_NOOP("KDbConnection", "Unknown error.")
};
#endif

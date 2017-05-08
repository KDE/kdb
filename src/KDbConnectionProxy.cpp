/* This file is part of the KDE project
   Copyright (C) 2016 Jarosław Staniek <staniek@kde.org>

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

#include "KDbConnectionProxy.h"
#include "KDbConnectionData.h"
#include "KDbProperties.h"
#include "KDbVersionInfo.h"

class Q_DECL_HIDDEN KDbConnectionProxy::Private
{
public:
    Private()
     : connectionIsOwned(true)
    {
    }
    ~Private() {
        if (connectionIsOwned) {
            connection->disconnect();
            delete connection;
        }
    }
    bool connectionIsOwned;
    KDbConnection *connection;
private:
    Q_DISABLE_COPY(Private)
};

KDbConnectionProxy::KDbConnectionProxy(KDbConnection *parentConnection)
 : KDbConnection(parentConnection->driver(), parentConnection->data(), *parentConnection->options())
 , d(new Private)
{
    Q_ASSERT(parentConnection);
    d->connection = parentConnection;
}

KDbConnectionProxy::~KDbConnectionProxy()
{
    delete d;
}

KDbConnection* KDbConnectionProxy::parentConnection()
{
    return d->connection;
}

const KDbConnection* KDbConnectionProxy::parentConnection() const
{
    return d->connection;
}

void KDbConnectionProxy::setParentConnectionIsOwned(bool set)
{
    d->connectionIsOwned = set;
}

KDbConnectionData KDbConnectionProxy::data() const
{
    return d->connection->data();
}

KDbDriver* KDbConnectionProxy::driver() const
{
    return d->connection->driver();
}

bool KDbConnectionProxy::connect()
{
    return d->connection->connect();
}

bool KDbConnectionProxy::isConnected() const
{
    return d->connection->isConnected();
}

bool KDbConnectionProxy::isDatabaseUsed() const
{
    return d->connection->isDatabaseUsed();
}

KDbConnectionOptions* KDbConnectionProxy::options()
{
    return d->connection->options();
}

void KDbConnectionProxy::clearResult()
{
    d->connection->clearResult();
}

KDbResult KDbConnectionProxy::result() const
{
    return d->connection->result();
}

KDbResultable KDbConnectionProxy::resultable() const
{
    return *d->connection;
}

bool KDbConnectionProxy::disconnect()
{
    return d->connection->disconnect();
}

QStringList KDbConnectionProxy::databaseNames(bool also_system_db)
{
    return d->connection->databaseNames(also_system_db);
}

bool KDbConnectionProxy::databaseExists(const QString &dbName, bool ignoreErrors)
{
    return d->connection->databaseExists(dbName, ignoreErrors);
}

bool KDbConnectionProxy::createDatabase(const QString &dbName)
{
    return d->connection->createDatabase(dbName);
}

bool KDbConnectionProxy::useDatabase(const QString &dbName, bool kexiCompatible, bool *cancelled,
                                     KDbMessageHandler* msgHandler)
{
    return d->connection->useDatabase(dbName, kexiCompatible, cancelled, msgHandler);
}

bool KDbConnectionProxy::closeDatabase()
{
    return d->connection->closeDatabase();
}

QString KDbConnectionProxy::currentDatabase() const
{
    return d->connection->currentDatabase();
}

bool KDbConnectionProxy::dropDatabase(const QString &dbName)
{
    return d->connection->dropDatabase(dbName);
}

QStringList KDbConnectionProxy::objectNames(int objectType, bool* ok)
{
    return d->connection->objectNames(objectType, ok);
}

QStringList KDbConnectionProxy::tableNames(bool alsoSystemTables, bool* ok)
{
    return d->connection->tableNames(alsoSystemTables, ok);
}

tristate KDbConnectionProxy::containsTable(const QString &tableName)
{
    return d->connection->containsTable(tableName);
}

KDbServerVersionInfo KDbConnectionProxy::serverVersion() const
{
    return d->connection->serverVersion();
}

KDbVersionInfo KDbConnectionProxy::databaseVersion() const
{
    return d->connection->databaseVersion();
}

KDbProperties KDbConnectionProxy::databaseProperties() const
{
    return d->connection->databaseProperties();
}

QList<int> KDbConnectionProxy::tableIds(bool* ok)
{
    return d->connection->tableIds(ok);
}

QList<int> KDbConnectionProxy::queryIds(bool* ok)
{
    return d->connection->queryIds(ok);
}

QList<int> KDbConnectionProxy::objectIds(int objectType, bool* ok)
{
    return d->connection->objectIds(objectType, ok);
}

KDbTransaction KDbConnectionProxy::beginTransaction()
{
    return d->connection->beginTransaction();
}

bool KDbConnectionProxy::commitTransaction(KDbTransaction trans, bool ignore_inactive)
{
    return d->connection->commitTransaction(trans, ignore_inactive);
}

bool KDbConnectionProxy::rollbackTransaction(KDbTransaction trans, bool ignore_inactive)
{
    return d->connection->rollbackTransaction(trans, ignore_inactive);
}

KDbTransaction KDbConnectionProxy::defaultTransaction() const
{
    return d->connection->defaultTransaction();
}

void KDbConnectionProxy::setDefaultTransaction(const KDbTransaction& trans)
{
    d->connection->setDefaultTransaction(trans);
}

QList<KDbTransaction> KDbConnectionProxy::transactions()
{
    return d->connection->transactions();
}

bool KDbConnectionProxy::autoCommit() const
{
    return d->connection->autoCommit();
}

bool KDbConnectionProxy::setAutoCommit(bool on)
{
    return d->connection->setAutoCommit(on);
}

KDbEscapedString KDbConnectionProxy::escapeString(const QString& str) const
{
    return d->connection->escapeString(str);
}

KDbCursor* KDbConnectionProxy::prepareQuery(const KDbEscapedString& sql, KDbCursor::Options options)
{
    return d->connection->prepareQuery(sql, options);
}

KDbCursor* KDbConnectionProxy::prepareQuery(KDbQuerySchema* query, KDbCursor::Options options)
{
    return d->connection->prepareQuery(query, options);
}

KDbCursor* KDbConnectionProxy::prepareQuery(KDbTableSchema* table, KDbCursor::Options options)
{
    return d->connection->prepareQuery(table, options);
}

KDbCursor* KDbConnectionProxy::executeQuery(const KDbEscapedString& sql, KDbCursor::Options options)
{
    return d->connection->executeQuery(sql, options);
}

KDbCursor* KDbConnectionProxy::executeQuery(KDbQuerySchema* query, const QList<QVariant>& params,
                        KDbCursor::Options options)
{
    return d->connection->executeQuery(query, params, options);
}

KDbCursor* KDbConnectionProxy::executeQuery(KDbQuerySchema* query, KDbCursor::Options options)
{
    return d->connection->executeQuery(query, options);
}

KDbCursor* KDbConnectionProxy::executeQuery(KDbTableSchema* table, KDbCursor::Options options)
{
    return d->connection->executeQuery(table, options);
}

bool KDbConnectionProxy::deleteCursor(KDbCursor *cursor)
{
    return d->connection->deleteCursor(cursor);
}

KDbTableSchema* KDbConnectionProxy::tableSchema(int tableId)
{
    return d->connection->tableSchema(tableId);
}

KDbTableSchema* KDbConnectionProxy::tableSchema(const QString& tableName)
{
    return d->connection->tableSchema(tableName);
}

KDbQuerySchema* KDbConnectionProxy::querySchema(int queryId)
{
    return d->connection->querySchema(queryId);
}

KDbQuerySchema* KDbConnectionProxy::querySchema(const QString& queryName)
{
    return d->connection->querySchema(queryName);
}

bool KDbConnectionProxy::setQuerySchemaObsolete(const QString& queryName)
{
    return d->connection->setQuerySchemaObsolete(queryName);
}

tristate KDbConnectionProxy::querySingleRecord(const KDbEscapedString& sql, KDbRecordData* data, bool addLimitTo1)
{
    return d->connection->querySingleRecord(sql, data, addLimitTo1);
}

tristate KDbConnectionProxy::querySingleRecord(KDbQuerySchema* query, KDbRecordData* data, bool addLimitTo1)
{
    return d->connection->querySingleRecord(query, data, addLimitTo1);
}

tristate KDbConnectionProxy::querySingleRecord(KDbQuerySchema* query, KDbRecordData* data,
                           const QList<QVariant>& params, bool addLimitTo1)
{
    return d->connection->querySingleRecord(query, data, params, addLimitTo1);
}

tristate KDbConnectionProxy::querySingleString(const KDbEscapedString& sql, QString* value, int column,
                           bool addLimitTo1)
{
    return d->connection->querySingleString(sql, value, column, addLimitTo1);
}

tristate KDbConnectionProxy::querySingleString(KDbQuerySchema* query, QString* value, int column,
                           bool addLimitTo1)
{
    return d->connection->querySingleString(query, value, column, addLimitTo1);
}

tristate KDbConnectionProxy::querySingleString(KDbQuerySchema* query, QString* value,
                           const QList<QVariant>& params, int column,
                           bool addLimitTo1)
{
    return d->connection->querySingleString(query, value, params, column, addLimitTo1);
}

tristate KDbConnectionProxy::querySingleNumber(const KDbEscapedString& sql, int* number, int column,
                           bool addLimitTo1)
{
    return d->connection->querySingleNumber(sql, number, column, addLimitTo1);
}

 tristate KDbConnectionProxy::querySingleNumber(KDbQuerySchema* query, int* number, int column,
                            bool addLimitTo1)
 {
     return d->connection->querySingleNumber(query, number, column, addLimitTo1);
 }

tristate KDbConnectionProxy::querySingleNumber(KDbQuerySchema* query, int* number,
                           const QList<QVariant>& params, int column,
                           bool addLimitTo1)
{
    return d->connection->querySingleNumber(query, number, params, column, addLimitTo1);
}

bool KDbConnectionProxy::queryStringList(const KDbEscapedString& sql, QStringList* list, int column)
{
    return d->connection->queryStringList(sql, list, column);
}

bool KDbConnectionProxy::queryStringList(KDbQuerySchema* query, QStringList* list, int column)
{
    return d->connection->queryStringList(query, list, column);
}

bool KDbConnectionProxy::queryStringList(KDbQuerySchema* query, QStringList* list,
                     const QList<QVariant>& params, int column)
{
    return d->connection->queryStringList(query, list, params, column);
}

tristate KDbConnectionProxy::resultExists(const KDbEscapedString& sql, bool addLimitTo1)
{
    return d->connection->resultExists(sql, addLimitTo1);
}

tristate KDbConnectionProxy::isEmpty(KDbTableSchema* table)
{
    return d->connection->isEmpty(table);
}

KDbEscapedString KDbConnectionProxy::recentSqlString() const
{
    return d->connection->recentSqlString();
}

//PROTOTYPE:
#define A , const QVariant&
#define H_INS_REC(args, ...) bool KDbConnectionProxy::insertRecord(KDbTableSchema* tableSchema args) \
{ \
    return d->connection->insertRecord(tableSchema, __VA_ARGS__); \
}
#define H_INS_REC_ALL \
H_INS_REC(A a1, a1) \
H_INS_REC(A a1 A a2, a1, a2) \
H_INS_REC(A a1 A a2 A a3, a1, a2, a3) \
H_INS_REC(A a1 A a2 A a3 A a4, a1, a2, a3, a4) \
H_INS_REC(A a1 A a2 A a3 A a4 A a5, a1, a2, a3, a4, a5) \
H_INS_REC(A a1 A a2 A a3 A a4 A a5 A a6, a1, a2, a3, a4, a5, a6) \
H_INS_REC(A a1 A a2 A a3 A a4 A a5 A a6 A a7, a1, a2, a3, a4, a5, a6, a7) \
H_INS_REC(A a1 A a2 A a3 A a4 A a5 A a6 A a7 A a8, a1, a2, a3, a4, a5, a6, a7, a8)
H_INS_REC_ALL

#undef H_INS_REC
#define H_INS_REC(args, ...) bool KDbConnectionProxy::insertRecord(KDbFieldList* fields args) \
{ \
    return d->connection->insertRecord(fields, __VA_ARGS__); \
}

H_INS_REC_ALL
#undef H_INS_REC_ALL
#undef H_INS_REC
#undef A

bool KDbConnectionProxy::insertRecord(KDbTableSchema* tableSchema, const QList<QVariant>& values)
{
    return d->connection->insertRecord(tableSchema, values);
}

bool KDbConnectionProxy::insertRecord(KDbFieldList* fields, const QList<QVariant>& values)
{
    return d->connection->insertRecord(fields, values);
}

bool KDbConnectionProxy::createTable(KDbTableSchema* tableSchema, bool replaceExisting)
{
    return d->connection->createTable(tableSchema, replaceExisting);
}

KDbTableSchema *KDbConnectionProxy::copyTable(const KDbTableSchema &tableSchema, const KDbObject &newData)
{
    return d->connection->copyTable(tableSchema, newData);
}

KDbTableSchema *KDbConnectionProxy::copyTable(const QString& tableName, const KDbObject &newData)
{
    return d->connection->copyTable(tableName, newData);
}

tristate KDbConnectionProxy::dropTable(KDbTableSchema* tableSchema)
{
    return d->connection->dropTable(tableSchema);
}

tristate KDbConnectionProxy::dropTable(const QString& tableName)
{
    return d->connection->dropTable(tableName);
}

tristate KDbConnectionProxy::alterTable(KDbTableSchema* tableSchema, KDbTableSchema* newTableSchema)
{
    return d->connection->alterTable(tableSchema, newTableSchema);
}

bool KDbConnectionProxy::alterTableName(KDbTableSchema* tableSchema, const QString& newName, bool replace)
{
    return d->connection->alterTableName(tableSchema, newName, replace);
}

bool KDbConnectionProxy::dropQuery(KDbQuerySchema* querySchema)
{
    return d->connection->dropQuery(querySchema);
}

bool KDbConnectionProxy::dropQuery(const QString& queryName)
{
    return d->connection->dropQuery(queryName);
}

bool KDbConnectionProxy::removeObject(int objId)
{
    return d->connection->removeObject(objId);
}

KDbField* KDbConnectionProxy::findSystemFieldName(const KDbFieldList& fieldlist)
{
    return d->connection->findSystemFieldName(fieldlist);
}

QString KDbConnectionProxy::anyAvailableDatabaseName()
{
    return d->connection->anyAvailableDatabaseName();
}

void KDbConnectionProxy::setAvailableDatabaseName(const QString& dbName)
{
    d->connection->setAvailableDatabaseName(dbName);
}

bool KDbConnectionProxy::useTemporaryDatabaseIfNeeded(QString* name)
{
    return d->connection->useTemporaryDatabaseIfNeeded(name);
}

QSharedPointer<KDbSqlResult> KDbConnectionProxy::prepareSql(const KDbEscapedString& sql)
{
    return d->connection->prepareSql(sql);
}

bool KDbConnectionProxy::executeSql(const KDbEscapedString& sql)
{
    return d->connection->executeSql(sql);
}

bool KDbConnectionProxy::storeObjectData(KDbObject* object)
{
    return d->connection->storeObjectData(object);
}

bool KDbConnectionProxy::storeNewObjectData(KDbObject* object)
{
    return d->connection->storeNewObjectData(object);
}

tristate KDbConnectionProxy::loadObjectData(int id, KDbObject* object)
{
    return d->connection->loadObjectData(id, object);
}

tristate KDbConnectionProxy::loadObjectData(int type, const QString& name, KDbObject* object)
{
    return d->connection->loadObjectData(type, name, object);
}

tristate KDbConnectionProxy::loadDataBlock(int objectID, QString* dataString, const QString& dataID)
{
    return d->connection->loadDataBlock(objectID, dataString, dataID);
}

bool KDbConnectionProxy::storeDataBlock(int objectID, const QString &dataString,
                    const QString& dataID)
{
    return d->connection->storeDataBlock(objectID, dataString, dataID);
}

bool KDbConnectionProxy::copyDataBlock(int sourceObjectID, int destObjectID, const QString& dataID)
{
    return d->connection->copyDataBlock(sourceObjectID, destObjectID, dataID);
}

bool KDbConnectionProxy::removeDataBlock(int objectID, const QString& dataID)
{
    return d->connection->removeDataBlock(objectID, dataID);
}

KDbPreparedStatement KDbConnectionProxy::prepareStatement(KDbPreparedStatement::Type type,
    KDbFieldList* fields, const QStringList& whereFieldNames)
{
    return d->connection->prepareStatement(type, fields, whereFieldNames);
}

bool KDbConnectionProxy::isInternalTableSchema(const QString& tableName)
{
    return d->connection->isInternalTableSchema(tableName);
}

QString KDbConnectionProxy::escapeIdentifier(const QString& id) const
{
    return d->connection->escapeIdentifier(id);
}

bool KDbConnectionProxy::drv_connect()
{
    return d->connection->drv_connect();
}

bool KDbConnectionProxy::drv_disconnect()
{
    return d->connection->drv_disconnect();
}

bool KDbConnectionProxy::drv_getServerVersion(KDbServerVersionInfo* version)
{
    return d->connection->drv_getServerVersion(version);
}

tristate KDbConnectionProxy::drv_containsTable(const QString &tableName)
{
    return d->connection->drv_containsTable(tableName);
}

bool KDbConnectionProxy::drv_createTable(const KDbTableSchema& tableSchema)
{
    return d->connection->drv_createTable(tableSchema);
}

bool KDbConnectionProxy::drv_alterTableName(KDbTableSchema* tableSchema, const QString& newName)
{
    return d->connection->drv_alterTableName(tableSchema, newName);
}

bool KDbConnectionProxy::drv_copyTableData(const KDbTableSchema &tableSchema,
                                           const KDbTableSchema &destinationTableSchema)
{
    return d->connection->drv_copyTableData(tableSchema, destinationTableSchema);
}

bool KDbConnectionProxy::drv_dropTable(const QString& tableName)
{
    return d->connection->drv_dropTable(tableName);
}

tristate KDbConnectionProxy::dropTable(KDbTableSchema* tableSchema, bool alsoRemoveSchema)
{
    return d->connection->dropTable(tableSchema, alsoRemoveSchema);
}

bool KDbConnectionProxy::setupObjectData(const KDbRecordData& data, KDbObject* object)
{
    return d->connection->setupObjectData(data, object);
}

KDbField* KDbConnectionProxy::setupField(const KDbRecordData& data)
{
    return d->connection->setupField(data);
}

KDbSqlResult* KDbConnectionProxy::drv_prepareSql(const KDbEscapedString& sql)
{
    return d->connection->drv_prepareSql(sql);
}

bool KDbConnectionProxy::drv_executeSql(const KDbEscapedString& sql)
{
    return d->connection->drv_executeSql(sql);
}

bool KDbConnectionProxy::drv_getDatabasesList(QStringList* list)
{
    return d->connection->drv_getDatabasesList(list);
}

bool KDbConnectionProxy::drv_databaseExists(const QString &dbName, bool ignoreErrors)
{
    return d->connection->drv_databaseExists(dbName, ignoreErrors);
}

bool KDbConnectionProxy::drv_createDatabase(const QString &dbName)
{
    return d->connection->drv_createDatabase(dbName);
}

bool KDbConnectionProxy::drv_useDatabase(const QString &dbName, bool *cancelled,
                                         KDbMessageHandler* msgHandler)
{
    return d->connection->drv_useDatabase(dbName, cancelled, msgHandler);
}

bool KDbConnectionProxy::drv_closeDatabase()
{
    return d->connection->drv_closeDatabase();
}

bool KDbConnectionProxy::drv_isDatabaseUsed() const
{
    return d->connection->drv_isDatabaseUsed();
}

bool KDbConnectionProxy::drv_dropDatabase(const QString &dbName)
{
    return d->connection->drv_dropDatabase(dbName);
}

bool KDbConnectionProxy::drv_createTable(const QString& tableName)
{
    return d->connection->drv_createTable(tableName);
}

KDbTransactionData* KDbConnectionProxy::drv_beginTransaction()
{
    return d->connection->drv_beginTransaction();
}

bool KDbConnectionProxy::drv_commitTransaction(KDbTransactionData* trans)
{
    return d->connection->drv_commitTransaction(trans);
}

bool KDbConnectionProxy::drv_rollbackTransaction(KDbTransactionData* trans)
{
    return d->connection->drv_rollbackTransaction(trans);
}

bool KDbConnectionProxy::drv_beforeInsert(const QString& tableName, KDbFieldList* fields)
{
    return d->connection->drv_beforeInsert(tableName, fields);
}

bool KDbConnectionProxy::drv_afterInsert(const QString& tableName, KDbFieldList* fields)
{
    return d->connection->drv_afterInsert(tableName, fields);
}

bool KDbConnectionProxy::drv_beforeUpdate(const QString& tableName, KDbFieldList* fields)
{
    return d->connection->drv_beforeUpdate(tableName, fields);
}

bool KDbConnectionProxy::drv_afterUpdate(const QString& tableName, KDbFieldList* fields)
{
    return d->connection->drv_afterUpdate(tableName, fields);
}

bool KDbConnectionProxy::drv_setAutoCommit(bool on)
{
    return d->connection->drv_setAutoCommit(on);
}

KDbPreparedStatementInterface* KDbConnectionProxy::prepareStatementInternal()
{
    return d->connection->prepareStatementInternal();
}

bool KDbConnectionProxy::beginAutoCommitTransaction(KDbTransactionGuard* tg)
{
    return d->connection->beginAutoCommitTransaction(tg);
}

bool KDbConnectionProxy::commitAutoCommitTransaction(const KDbTransaction& trans)
{
    return d->connection->commitAutoCommitTransaction(trans);
}

bool KDbConnectionProxy::rollbackAutoCommitTransaction(const KDbTransaction& trans)
{
    return d->connection->rollbackAutoCommitTransaction(trans);
}

bool KDbConnectionProxy::checkConnected()
{
    return d->connection->checkConnected();
}

bool KDbConnectionProxy::checkIsDatabaseUsed()
{
    return d->connection->checkIsDatabaseUsed();
}

KDbTableSchema* KDbConnectionProxy::setupTableSchema(const KDbRecordData& data)
{
    return d->connection->setupTableSchema(data);
}

KDbQuerySchema* KDbConnectionProxy::setupQuerySchema(const KDbRecordData& data)
{
    return d->connection->setupQuerySchema(data);
}

bool KDbConnectionProxy::updateRecord(KDbQuerySchema* query, KDbRecordData* data, KDbRecordEditBuffer* buf, bool useRecordId)
{
    return d->connection->updateRecord(query, data, buf, useRecordId);
}

bool KDbConnectionProxy::insertRecord(KDbQuerySchema* query, KDbRecordData* data, KDbRecordEditBuffer* buf, bool getRecordId)
{
    return d->connection->insertRecord(query, data, buf, getRecordId);
}

bool KDbConnectionProxy::deleteRecord(KDbQuerySchema* query, KDbRecordData* data, bool useRecordId)
{
    return d->connection->deleteRecord(query, data, useRecordId);
}

bool KDbConnectionProxy::deleteAllRecords(KDbQuerySchema* query)
{
    return d->connection->deleteAllRecords(query);
}

bool KDbConnectionProxy::checkIfColumnExists(KDbCursor *cursor, int column)
{
    return d->connection->checkIfColumnExists(cursor, column);
}

tristate KDbConnectionProxy::querySingleRecordInternal(KDbRecordData* data, const KDbEscapedString* sql,
                                   KDbQuerySchema* query, const QList<QVariant>* params,
                                   bool addLimitTo1)
{
    return d->connection->querySingleRecordInternal(data, sql, query, params, addLimitTo1);
}

tristate KDbConnectionProxy::querySingleStringInternal(const KDbEscapedString* sql, QString* value,
                                   KDbQuerySchema* query, const QList<QVariant>* params,
                                   int column, bool addLimitTo1)
{
    return d->connection->querySingleStringInternal(sql, value, query, params, column, addLimitTo1);
}

tristate KDbConnectionProxy::querySingleNumberInternal(const KDbEscapedString* sql, int* number,
                                   KDbQuerySchema* query, const QList<QVariant>* params,
                                   int column, bool addLimitTo1)
{
    return d->connection->querySingleNumberInternal(sql, number, query, params, column, addLimitTo1);
}

bool KDbConnectionProxy::queryStringListInternal(const KDbEscapedString *sql, QStringList* list,
                             KDbQuerySchema* query, const QList<QVariant>* params,
                             int column, bool (*filterFunction)(const QString&))
{
    return d->connection->queryStringListInternal(sql, list, query, params, column, filterFunction);
}

KDbCursor* KDbConnectionProxy::executeQueryInternal(const KDbEscapedString& sql, KDbQuerySchema* query,
                                const QList<QVariant>* params)
{
    return d->connection->executeQueryInternal(sql, query, params);
}

bool KDbConnectionProxy::loadExtendedTableSchemaData(KDbTableSchema* tableSchema)
{
    return d->connection->loadExtendedTableSchemaData(tableSchema);
}

bool KDbConnectionProxy::storeExtendedTableSchemaData(KDbTableSchema* tableSchema)
{
    return d->connection->storeExtendedTableSchemaData(tableSchema);
}

bool KDbConnectionProxy::storeMainFieldSchema(KDbField *field)
{
    return d->connection->storeMainFieldSchema(field);
}

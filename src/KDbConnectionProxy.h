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

#ifndef KDB_CONNECTIONPROXY_H
#define KDB_CONNECTIONPROXY_H

#include <KDbConnection>

//! The KDbConnectionProxy class gives access to protected (low-level) API of KDbConnection
/**
 * The connection object specified in constructor of the proxy is called a parent connection.
 * All inherited methods of the KDbConnection API call equivalent methods of the parent
 * connection. The KDbConnectionProxy class also provides non-virtual methods that are
 * equivalent to KDbConnection ones. These KDbConnection's equivalent methods are called
 * by the proxy too.
 *
 * Example use of this class is Kexi's database database migration plugins when the source
 * database is only accessibly using low-level routines.
 */
class KDB_EXPORT KDbConnectionProxy : protected KDbConnection
{
public:
    //! Creates a proxy object for parent @a connection.
    //! @a connection must not be @c nullptr.
    //! It is owned by this proxy unless setConnectionIsOwned(false) is called.
    explicit KDbConnectionProxy(KDbConnection *connection);

    //! Deletes this proxy. Owned connection is closed and destroyed.
    ~KDbConnectionProxy() override;

    //! @return parent connection for this proxy
    KDbConnection *parentConnection();

    //! @overload KDbConnection *parentConnection()
    const KDbConnection *parentConnection() const;

    //! Control owhership of parent connection that is assigned to this proxy.
    //! Owned connection is closed and destroyed upon destruction of the KDbConnectionProxy
    //! object.
    void setParentConnectionIsOwned(bool set);

    KDbConnectionData data() const;

    KDbDriver* driver() const;

    bool connect();

    bool isConnected() const;

    bool isDatabaseUsed() const;

    KDbConnectionOptions *options();

    void clearResult();

    KDbResult result() const;

    KDbResultable resultable() const;

    bool disconnect();

    QStringList databaseNames(bool also_system_db = false);

    bool databaseExists(const QString &dbName, bool ignoreErrors = true);

    bool createDatabase(const QString &dbName);

    bool useDatabase(const QString &dbName = QString(), bool kexiCompatible = true, bool *cancelled = nullptr,
                     KDbMessageHandler* msgHandler = nullptr);

    bool closeDatabase();

    QString currentDatabase() const;

    bool dropDatabase(const QString &dbName = QString());

    QStringList objectNames(int objectType = KDb::AnyObjectType, bool* ok = nullptr);

    QStringList tableNames(bool alsoSystemTables = false, bool* ok = nullptr);

    tristate containsTable(const QString &tableName);

    KDbServerVersionInfo serverVersion() const;

    KDbVersionInfo databaseVersion() const;

    KDbProperties databaseProperties() const;

    QList<int> tableIds(bool* ok = nullptr);

    QList<int> queryIds(bool* ok = nullptr);

    QList<int> objectIds(int objectType, bool* ok = nullptr);

    KDbTransaction beginTransaction();

    bool commitTransaction(KDbTransaction trans = KDbTransaction(),
                           TransactionOptions options = TransactionOptions());

    bool rollbackTransaction(KDbTransaction trans = KDbTransaction(),
                             TransactionOptions options = TransactionOptions());

    KDbTransaction defaultTransaction() const;

    void setDefaultTransaction(const KDbTransaction& trans);

    QList<KDbTransaction> transactions();

    bool autoCommit() const;

    bool setAutoCommit(bool on);

    KDbEscapedString escapeString(const QString& str) const override;

    KDbCursor *prepareQuery(const KDbEscapedString &sql,
                            KDbCursor::Options options = KDbCursor::Option::None) override;

    KDbCursor *prepareQuery(KDbQuerySchema *query,
                            KDbCursor::Options options = KDbCursor::Option::None) override;

    KDbCursor *prepareQuery(KDbTableSchema *table,
                            KDbCursor::Options options = KDbCursor::Option::None);

    KDbCursor *executeQuery(const KDbEscapedString &sql,
                            KDbCursor::Options options = KDbCursor::Option::None);

    KDbCursor* executeQuery(KDbQuerySchema* query, const QList<QVariant>& params,
                            KDbCursor::Options options = KDbCursor::Option::None);

    KDbCursor* executeQuery(KDbQuerySchema* query, KDbCursor::Options options = KDbCursor::Option::None);

    KDbCursor* executeQuery(KDbTableSchema* table, KDbCursor::Options options = KDbCursor::Option::None);

    bool deleteCursor(KDbCursor *cursor);

    KDbTableSchema* tableSchema(int tableId);

    KDbTableSchema* tableSchema(const QString& tableName);

    KDbQuerySchema* querySchema(int queryId);

    KDbQuerySchema* querySchema(const QString& queryName);

    bool setQuerySchemaObsolete(const QString& queryName);

    tristate querySingleRecord(const KDbEscapedString& sql, KDbRecordData* data,
                               QueryRecordOptions options = QueryRecordOption::Default);

    tristate querySingleRecord(KDbQuerySchema* query, KDbRecordData* data,
                               QueryRecordOptions options = QueryRecordOption::Default);

    tristate querySingleRecord(KDbQuerySchema* query, KDbRecordData* data,
                               const QList<QVariant>& params,
                               QueryRecordOptions options = QueryRecordOption::Default);

    tristate querySingleString(const KDbEscapedString& sql, QString* value, int column = 0,
                               QueryRecordOptions options = QueryRecordOption::Default);

    tristate querySingleString(KDbQuerySchema* query, QString* value, int column = 0,
                               QueryRecordOptions options = QueryRecordOption::Default);

    tristate querySingleString(KDbQuerySchema* query, QString* value,
                               const QList<QVariant>& params, int column = 0,
                               QueryRecordOptions options = QueryRecordOption::Default);

    tristate querySingleNumber(const KDbEscapedString& sql, int* number, int column = 0,
                               QueryRecordOptions options = QueryRecordOption::Default);

     tristate querySingleNumber(KDbQuerySchema* query, int* number, int column = 0,
                                QueryRecordOptions options = QueryRecordOption::Default);

    tristate querySingleNumber(KDbQuerySchema* query, int* number,
                               const QList<QVariant>& params, int column = 0,
                               QueryRecordOptions options = QueryRecordOption::Default);

    bool queryStringList(const KDbEscapedString& sql, QStringList* list, int column = 0);

    bool queryStringList(KDbQuerySchema* query, QStringList* list, int column = 0);

    bool queryStringList(KDbQuerySchema* query, QStringList* list,
                         const QList<QVariant>& params, int column = 0);

    tristate resultExists(const KDbEscapedString &sql, QueryRecordOptions options
                          = QueryRecordOption::Default);

    tristate isEmpty(KDbTableSchema* table);

    KDbEscapedString recentSqlString() const override;

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

    bool createTable(KDbTableSchema* tableSchema, bool replaceExisting = false);

    KDbTableSchema *copyTable(const KDbTableSchema &tableSchema, const KDbObject &newData);

    KDbTableSchema *copyTable(const QString& tableName, const KDbObject &newData);

    tristate dropTable(KDbTableSchema* tableSchema);

    tristate dropTable(const QString& tableName);

    tristate alterTable(KDbTableSchema* tableSchema, KDbTableSchema* newTableSchema);

    bool alterTableName(KDbTableSchema* tableSchema, const QString& newName,
                        AlterTableNameOptions options = AlterTableNameOption::Default);

    bool dropQuery(KDbQuerySchema* querySchema);

    bool dropQuery(const QString& queryName);

    bool removeObject(int objId);

    KDbField* findSystemFieldName(const KDbFieldList& fieldlist);

    QString anyAvailableDatabaseName() override;

    void setAvailableDatabaseName(const QString& dbName);

    bool useTemporaryDatabaseIfNeeded(QString* name);

    QSharedPointer<KDbSqlResult> prepareSql(const KDbEscapedString& sql) Q_REQUIRED_RESULT;

    bool executeSql(const KDbEscapedString& sql);

    bool storeObjectData(KDbObject* object);

    bool storeNewObjectData(KDbObject* object);

    tristate loadObjectData(int id, KDbObject* object);

    tristate loadObjectData(int type, const QString& name, KDbObject* object);

    tristate loadDataBlock(int objectID, QString* dataString, const QString& dataID = QString());

    bool storeDataBlock(int objectID, const QString &dataString,
                        const QString& dataID = QString());

    bool copyDataBlock(int sourceObjectID, int destObjectID, const QString& dataID = QString());

    bool removeDataBlock(int objectID, const QString& dataID = QString());

    KDbPreparedStatement prepareStatement(KDbPreparedStatement::Type type,
        KDbFieldList* fields, const QStringList& whereFieldNames = QStringList());

    bool isInternalTableSchema(const QString& tableName);

    QString escapeIdentifier(const QString& id) const override;

    bool drv_connect() override;

    bool drv_disconnect() override;

    bool drv_getServerVersion(KDbServerVersionInfo* version) override;

    tristate drv_containsTable(const QString &tableName) override;

    bool drv_createTable(const KDbTableSchema& tableSchema) override;

    bool drv_alterTableName(KDbTableSchema* tableSchema, const QString& newName) override;

    bool drv_copyTableData(const KDbTableSchema &tableSchema,
                           const KDbTableSchema &destinationTableSchema) override;

    bool drv_dropTable(const QString& tableName) override;

    tristate dropTable(KDbTableSchema* tableSchema, bool alsoRemoveSchema);

    bool setupObjectData(const KDbRecordData& data, KDbObject* object);

    KDbField* setupField(const KDbRecordData& data);

    KDbSqlResult* drv_prepareSql(const KDbEscapedString& sql) override Q_REQUIRED_RESULT;

    bool drv_executeSql(const KDbEscapedString& sql) override;

    bool drv_getDatabasesList(QStringList* list) override;

    bool drv_databaseExists(const QString &dbName, bool ignoreErrors = true) override;

    bool drv_createDatabase(const QString &dbName = QString()) override;

    bool drv_useDatabase(const QString &dbName = QString(), bool *cancelled = nullptr,
                         KDbMessageHandler* msgHandler = nullptr) override;

    bool drv_closeDatabase() override;

    bool drv_isDatabaseUsed() const override;

    bool drv_dropDatabase(const QString &dbName = QString()) override;

    bool drv_createTable(const QString& tableName) override;

    KDbTransactionData* drv_beginTransaction() override;

    bool drv_commitTransaction(KDbTransactionData* trans) override;

    bool drv_rollbackTransaction(KDbTransactionData* trans) override;

    bool drv_beforeInsert(const QString& tableName, KDbFieldList* fields) override;

    bool drv_afterInsert(const QString& tableName, KDbFieldList* fields) override;

    bool drv_beforeUpdate(const QString& tableName, KDbFieldList* fields) override;

    bool drv_afterUpdate(const QString& tableName, KDbFieldList* fields) override;

    bool drv_setAutoCommit(bool on) override;

    KDbPreparedStatementInterface* prepareStatementInternal() override;

    bool beginAutoCommitTransaction(KDbTransactionGuard* tg);

    bool commitAutoCommitTransaction(const KDbTransaction& trans);

    bool rollbackAutoCommitTransaction(const KDbTransaction& trans);

    bool checkConnected();

    bool checkIsDatabaseUsed();

    KDbTableSchema* setupTableSchema(const KDbRecordData& data);

    KDbQuerySchema* setupQuerySchema(const KDbRecordData& data);

    bool updateRecord(KDbQuerySchema* query, KDbRecordData* data, KDbRecordEditBuffer* buf, bool useRecordId = false);

    bool insertRecord(KDbQuerySchema* query, KDbRecordData* data, KDbRecordEditBuffer* buf, bool getRecordId = false);

    bool deleteRecord(KDbQuerySchema* query, KDbRecordData* data, bool useRecordId = false);

    bool deleteAllRecords(KDbQuerySchema* query);

    bool checkIfColumnExists(KDbCursor *cursor, int column);

    tristate querySingleRecordInternal(KDbRecordData* data, const KDbEscapedString* sql,
                                       KDbQuerySchema* query, const QList<QVariant>* params,
                                       QueryRecordOptions options);

    tristate querySingleStringInternal(const KDbEscapedString* sql, QString* value,
                                       KDbQuerySchema* query, const QList<QVariant>* params,
                                       int column, QueryRecordOptions options);

    tristate querySingleNumberInternal(const KDbEscapedString* sql, int* number,
                                       KDbQuerySchema* query, const QList<QVariant>* params,
                                       int column, QueryRecordOptions options);

    bool queryStringListInternal(const KDbEscapedString *sql, QStringList* list,
                                 KDbQuerySchema* query, const QList<QVariant>* params,
                                 int column, bool (*filterFunction)(const QString&));

    KDbCursor* executeQueryInternal(const KDbEscapedString& sql, KDbQuerySchema* query,
                                    const QList<QVariant>* params);

    bool loadExtendedTableSchemaData(KDbTableSchema* tableSchema);

    bool storeExtendedTableSchemaData(KDbTableSchema* tableSchema);

    bool storeMainFieldSchema(KDbField *field);

private:
    Q_DISABLE_COPY(KDbConnectionProxy)
    class Private;
    Private * const d;
};

#endif

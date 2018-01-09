/* This file is part of the KDE project
   Copyright (C) 2005 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_CONNECTION_P_H
#define KDB_CONNECTION_P_H

#include "KDbConnectionData.h"
#include "KDbConnection.h"
#include "KDbConnectionOptions.h"
#include "kdb_export.h"
#include "KDbParser.h"
#include "KDbProperties.h"
#include "KDbQuerySchema_p.h"
#include "KDbVersionInfo.h"

//! Interface for accessing connection's internal result, for use by drivers.
class KDB_EXPORT KDbConnectionInternal
{
public:
    explicit KDbConnectionInternal(KDbConnection *conn);
    KDbConnection* const connection;
private:
    Q_DISABLE_COPY(KDbConnectionInternal)
};

class KDbConnectionPrivate
{
    Q_DECLARE_TR_FUNCTIONS(KDbConnectionPrivate)
public:
    KDbConnectionPrivate(KDbConnection* const conn, KDbDriver *drv,
                         const KDbConnectionData& _connData,
                         const KDbConnectionOptions &_options);

    ~KDbConnectionPrivate();

    void deleteAllCursors();

    void errorInvalidDBContents(const QString& details);

    QString strItIsASystemObject() const;

    inline KDbParser *parser() {
        return m_parser ? m_parser : (m_parser = new KDbParser(conn));
    }

    inline KDbTableSchema* table(const QString& name) const {
        return m_tablesByName.value(name);
    }

    inline KDbTableSchema* table(int id) const {
        return m_tables.value(id);
    }

    //! used just for removing system KDbTableSchema objects on db close.
    inline QSet<KDbInternalTableSchema*> internalKDbTables() const {
        return m_internalKDbTables;
    }

    /*! Allocates all needed table KDb system objects for kexi__* KDb library's
     system tables schema.
     These objects are used internally in this connection and are added to list of tables
     (by name,      not by id because these have no ids).
    */
    void setupKDbSystemSchema();

    void insertTable(KDbTableSchema* tableSchema);

    /*! Removes table schema having identifier @a id from internal structures and destroys it.
     Does not make any change at the backend. */
    void removeTable(int id);

    void takeTable(KDbTableSchema* tableSchema);

    void renameTable(KDbTableSchema* tableSchema, const QString& newName);

    void changeTableId(KDbTableSchema* tableSchema, int newId);

    void clearTables();

    inline KDbQuerySchema* query(const QString& name) const {
        return m_queriesByName.value(name);
    }

    inline KDbQuerySchema* query(int id) const {
        return m_queries.value(id);
    }

    void insertQuery(KDbQuerySchema* query);

    /*! Removes @a querySchema from internal structures and
     destroys it. Does not make any change at the backend. */
    void removeQuery(KDbQuerySchema* querySchema);

    void setQueryObsolete(KDbQuerySchema* query);

    void clearQueries();

    /*! @return a full table schema for a table retrieved using 'kexi__*' system tables.
     Connection keeps ownership of the returned object.
     Used internally by tableSchema() methods.
     On failure deletes @a table and returns @c nullptr. */
    KDbTableSchema* setupTableSchema(KDbTableSchema *table) Q_REQUIRED_RESULT;

    /*! @return a full query schema for a query using 'kexi__*' system tables.
     Connection keeps ownership of the returned object.
     Used internally by querySchema() methods.
     On failure deletes @a query and returns @c nullptr. */
    KDbQuerySchema* setupQuerySchema(KDbQuerySchema *query) Q_REQUIRED_RESULT;

    //! @return cached fields expanded information for @a query
    KDbQuerySchemaFieldsExpanded *fieldsExpanded(const KDbQuerySchema *query);

    //! Inserts cached fields expanded information for @a query
    void insertFieldsExpanded(const KDbQuerySchema *query, KDbQuerySchemaFieldsExpanded *cache);

    KDbConnection* const conn; //!< The @a KDbConnection instance this @a KDbConnectionPrivate belongs to.
    KDbConnectionData connData; //!< the @a KDbConnectionData used within that connection.

    //! True for read only connection. Used especially for file-based drivers.
    KDbConnectionOptions options;

    //!< The driver this @a KDbConnection instance uses.
    KDbDriver * const driver;

    /*! Default transaction handle.
    If transactions are supported: Any operation on database (e.g. inserts)
    that is started without specifying transaction context, will be performed
    in the context of this transaction. */
    KDbTransaction default_trans;
    QList<KDbTransaction> transactions;

    QHash<const KDbTableSchema*, QSet<KDbTableSchemaChangeListener*>* > tableSchemaChangeListeners;

    QHash<const KDbQuerySchema*, QSet<KDbTableSchemaChangeListener*>* > queryTableSchemaChangeListeners;

    //! Used in KDbConnection::setQuerySchemaObsolete( const QString& queryName )
    //! to collect obsolete queries. THese are deleted on connection deleting.
    QSet<KDbQuerySchema*> obsoleteQueries;

    //! server version information for this connection.
    KDbServerVersionInfo serverVersion;

    //! Database version information for this connection.
    KDbVersionInfo databaseVersion;

    KDbParser *m_parser = nullptr;

    //! cursors created for this connection
    QSet<KDbCursor*> cursors;

    //! Database properties
    KDbProperties dbProperties;

    QString availableDatabaseName; //!< used by anyAvailableDatabaseName()
    QString usedDatabase; //!< database name that is opened now (the currentDatabase() name)

    //! true if rollbackTransaction() and commitTransaction() shouldn't remove
    //! the transaction object from 'transactions' list; used by closeDatabase()
    bool dontRemoveTransactions = false;

    //! used to avoid endless recursion between useDatabase() and databaseExists()
    //! when useTemporaryDatabaseIfNeeded() works
    bool skipDatabaseExistsCheckInUseDatabase = false;

    /*! Used when single transactions are only supported (KDbDriver::SingleTransactions).
     True value means default KDbTransaction has been started inside connection object
     (by beginAutoCommitTransaction()), otherwise default transaction has been started outside
     of the object (e.g. before createTable()), so we shouldn't autocommit the transaction
     in commitAutoCommitTransaction(). Also, beginAutoCommitTransaction() doesn't restarts
     transaction if default_trans_started_inside is false. Such behavior allows user to
     execute a sequence of actions like CREATE TABLE...; INSERT DATA...; within a single transaction
     and commit it or rollback by hand. */
    bool defaultTransactionStartedInside = false;

    bool isConnected = false;

    bool autoCommit = true;

    bool insideCloseDatabase = false; //!< helper: true while closeDatabase() is executed

private:
    //! Table schemas retrieved on demand with tableSchema()
    QHash<int, KDbTableSchema*> m_tables;
    QHash<QString, KDbTableSchema*> m_tablesByName;
    //! used just for removing system KDbTableSchema objects on db close.
    QSet<KDbInternalTableSchema*> m_internalKDbTables;
    //! Query schemas retrieved on demand with querySchema()
    QHash<int, KDbQuerySchema*> m_queries;
    QHash<QString, KDbQuerySchema*> m_queriesByName;
    KDbUtils::AutodeletedHash<const KDbQuerySchema*, KDbQuerySchemaFieldsExpanded*> m_fieldsExpandedCache;
    Q_DISABLE_COPY(KDbConnectionPrivate)
};

#endif

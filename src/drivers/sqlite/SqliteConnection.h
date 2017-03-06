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

#ifndef KDB_CONN_SQLITE_H
#define KDB_CONN_SQLITE_H

#include <QStringList>

#include "KDbConnection.h"

class SqliteConnectionInternal;
class KDbDriver;

/*! @brief SQLite-specific connection
    Following connection options are supported (see KDbConnectionOptions):
    - extraSqliteExtensionPaths (read/write, QStringList): adds extra seach paths for SQLite
                                extensions. Set them before KDbConnection::useDatabase()
                                is called. Absolute paths are recommended.
*/
class SqliteConnection : public KDbConnection
{
    Q_DECLARE_TR_FUNCTIONS(SqliteConnection)
public:
    virtual ~SqliteConnection();

    KDbCursor* prepareQuery(const KDbEscapedString& sql,
                            KDbCursor::Options options = KDbCursor::Option::None) Q_DECL_OVERRIDE Q_REQUIRED_RESULT;
    KDbCursor* prepareQuery(KDbQuerySchema* query,
                            KDbCursor::Options options = KDbCursor::Option::None) Q_DECL_OVERRIDE Q_REQUIRED_RESULT;

    KDbPreparedStatementInterface* prepareStatementInternal() Q_DECL_OVERRIDE Q_REQUIRED_RESULT;

protected:
    /*! Used by driver */
    SqliteConnection(KDbDriver *driver, const KDbConnectionData& connData,
                     const KDbConnectionOptions &options);

    virtual bool drv_connect();
    virtual bool drv_getServerVersion(KDbServerVersionInfo* version);
    virtual bool drv_disconnect();
    virtual bool drv_getDatabasesList(QStringList* list);

#if 0 // TODO
//! @todo move this somewhere to low level class (MIGRATION?)
    virtual bool drv_getTablesList(QStringList* list);
#endif

//! @todo move this somewhere to low level class (MIGRATION?)
    virtual tristate drv_containsTable(const QString &tableName);

    /*! Creates new database using connection. Note: Do not pass @a dbName
      arg because for file-based engine (that has one database per connection)
      it is defined during connection. */
    virtual bool drv_createDatabase(const QString &dbName = QString());

    /*! Opens existing database using connection. Do not pass @a dbName
      arg because for file-based engine (that has one database per connection)
      it is defined during connection. If you pass it,
      database file name will be changed. */
    virtual bool drv_useDatabase(const QString &dbName = QString(), bool *cancelled = 0,
                                 KDbMessageHandler* msgHandler = 0);

    virtual bool drv_closeDatabase();

    /*! Drops database from the server using connection.
      After drop, database shouldn't be accessible
      anymore, so database file is just removed. See note from drv_useDatabase(). */
    virtual bool drv_dropDatabase(const QString &dbName = QString());

    virtual KDbSqlResult* drv_executeSQL(const KDbEscapedString& sql);
    virtual bool drv_executeVoidSQL(const KDbEscapedString& sql);

    //! Implemented for KDbResultable
    virtual QString serverResultName() const;

    void storeResult();

    virtual tristate drv_changeFieldProperty(KDbTableSchema* table, KDbField* field,
            const QString& propertyName, const QVariant& value);

    //! for drv_changeFieldProperty()
    tristate changeFieldType(KDbTableSchema *table, KDbField *field, KDbField::Type type);

    SqliteConnectionInternal* d;

private:
    bool drv_useDatabaseInternal(bool *cancelled, KDbMessageHandler* msgHandler, bool createIfMissing);

    //! Closes database without altering stored result number and message
    void drv_closeDatabaseSilently();

    //! Finds a native SQLite extension @a name in the search path and loads it.
    //! Path and filename extension should not be provided.
    //! @return true on success
    bool findAndLoadExtension(const QString & name);

    //! Loads extension from plugin at @a path (absolute path is recommended)
    //! @return true on success
    bool loadExtension(const QString& path);

    friend class SqliteDriver;
    friend class SqliteCursor;
    friend class SqliteSqlResult;
    Q_DISABLE_COPY(SqliteConnection)
};

#endif

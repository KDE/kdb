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

#include "SqliteConnection.h"
#include "SqliteConnection_p.h"
#include "SqliteCursor.h"
#include "SqlitePreparedStatement.h"
#include "SqliteFunctions.h"
#include "sqlite_debug.h"

#include <sqlite3.h>

#include "KDbConnectionData.h"
#include "KDbConnectionOptions.h"
#include "KDbUtils.h"
#include "KDbVersionInfo.h"

#include <QFile>
#include <QDir>
#include <QRegularExpression>

SqliteConnection::SqliteConnection(KDbDriver *driver, const KDbConnectionData& connData,
                                   const KDbConnectionOptions &options)
        : KDbConnection(driver, connData, options)
        , d(new SqliteConnectionInternal(this))
{
    this->options()->setCaption("extraSqliteExtensionPaths",
                                SqliteConnection::tr("Extra paths for SQLite plugins"));
}

SqliteConnection::~SqliteConnection()
{
    destroy();
    delete d;
}

void SqliteConnection::storeResult()
{
    d->storeResult(&m_result);
}

bool SqliteConnection::drv_connect()
{
    return true;
}

bool SqliteConnection::drv_getServerVersion(KDbServerVersionInfo* version)
{
    version->setString(QLatin1String(SQLITE_VERSION)); //defined in sqlite3.h
    QRegularExpression re(QLatin1String("^(\\d+)\\.(\\d+)\\.(\\d+)$"));
    QRegularExpressionMatch match  = re.match(version->string());
    if (match.hasMatch()) {
        version->setMajor(match.captured(1).toInt());
        version->setMinor(match.captured(2).toInt());
        version->setRelease(match.captured(3).toInt());
    }
    return true;
}

bool SqliteConnection::drv_disconnect()
{
    return true;
}

bool SqliteConnection::drv_getDatabasesList(QStringList* list)
{
    //this is one-db-per-file database
    list->append(data().databaseName());
    return true;
}

tristate SqliteConnection::drv_containsTable(const QString &tableName)
{
    return resultExists(KDbEscapedString("SELECT name FROM sqlite_master WHERE type='table' AND name LIKE %1")
                            .arg(escapeString(tableName)));
}

#if 0 // TODO
bool SqliteConnection::drv_getTablesList(QStringList* list)
{
    KDbCursor *cursor;
    if (!(cursor = executeQuery(KDbEscapedString("SELECT name FROM sqlite_master WHERE type='table'")))) {
        sqliteWarning() << "!executeQuery()";
        return false;
    }
    list->clear();
    cursor->moveFirst();
    while (!cursor->eof() && !cursor->result().isError()) {
        *list += cursor->value(0).toString();
        cursor->moveNext();
    }
    if (cursor->result().isError()) {
        deleteCursor(cursor);
        return false;
    }
    return deleteCursor(cursor);
}
#endif

bool SqliteConnection::drv_createDatabase(const QString &dbName)
{
    Q_UNUSED(dbName);
    return drv_useDatabaseInternal(0, 0, true/*create if missing*/);
}

bool SqliteConnection::drv_useDatabase(const QString &dbName, bool *cancelled,
                                       KDbMessageHandler* msgHandler)
{
    Q_UNUSED(dbName);
    return drv_useDatabaseInternal(cancelled, msgHandler, false/*do not create if missing*/);
}

bool SqliteConnection::drv_useDatabaseInternal(bool *cancelled,
                                               KDbMessageHandler* msgHandler, bool createIfMissing)
{
//! @todo add option (command line or in kdbrc?)
//! @todo   int exclusiveFlag = KDbConnection::isReadOnly() ? SQLITE_OPEN_READONLY : SQLITE_OPEN_WRITE_LOCKED; // <-- shared read + (if !r/o): exclusive write
    int openFlags = 0;
    if (options()->isReadOnly()) {
        openFlags |= SQLITE_OPEN_READONLY;
    }
    else {
        openFlags |= SQLITE_OPEN_READWRITE;
        if (createIfMissing) {
            openFlags |= SQLITE_OPEN_CREATE;
        }
    }

//! @todo add option
//    int allowReadonly = 1;
//    const bool wasReadOnly = KDbConnection::isReadOnly();

    //sqliteDebug() << data().databaseName();
    int res = sqlite3_open_v2(
                 /* unicode expected since SQLite 3.1 */
                 QDir::toNativeSeparators(data().databaseName()).toUtf8().constData(),
                 &d->data,
                 openFlags, /*exclusiveFlag,
                 allowReadonly *//* If 1 and locking fails, try opening in read-only mode */
                 0
             );
    if (res != SQLITE_OK) {
        m_result.setServerErrorCode(res);
    }
    storeResult();

    if (!m_result.isError()) {
        // Set the secure-delete on, so SQLite overwrites deleted content with zeros.
        // The default setting is determined by the SQLITE_SECURE_DELETE compile-time option but we overwrite it here.
        // Works with 3.6.23. Earlier version just ignore this pragma.
        // See http://www.sqlite.org/pragma.html#pragma_secure_delete
//! @todo add connection flags to the driver and global setting to control the "secure delete" pragma
        if (!drv_executeVoidSQL(KDbEscapedString("PRAGMA secure_delete = on"))) {
            drv_closeDatabaseSilently();
            return false;
        }
        // Load ICU extension for unicode collations
        if (!findAndLoadExtension(QLatin1String("kdb_sqlite_icu"))) {
            drv_closeDatabaseSilently();
            return false;
        }
        // load ROOT collation for use as default collation
        if (!drv_executeVoidSQL(KDbEscapedString("SELECT icu_load_collation('', '')"))) {
            drv_closeDatabaseSilently();
            return false;
        }
        if (!createCustomSQLiteFunctions(d->data)) {
            drv_closeDatabaseSilently();
            return false;
        }
    }

//! @todo check exclusive status
    Q_UNUSED(cancelled);
    Q_UNUSED(msgHandler);
//! @todo removed in kdb - reenable?
/*
    if (d->res == SQLITE_OK && cancelled && !wasReadOnly && allowReadonly && isReadOnly()) {
        //opened as read only, ask
        if (KDbMessageHandler::Continue !=
                askQuestion(
                    KDbMessageHandler::WarningContinueCancel,
                    futureTr("Do you want to open file \"%1\" as read-only?\n\n"
                        "The file is probably already open on this or another computer. "
                        "Could not gain exclusive access for writing the file.")
                    .arg(QDir::fromNativeSeparators(data()->databaseName())),
                    futureTr("Opening As Read-Only"),
                    KDbMessageHandler::Continue,
                    KDbMessageHandler::KDbGuiItem()
                            .setProperty("text", futureTr("Open As Read-Only"))
                            .setProperty("icon", "document-open"),
                    KDbMessageHandler::KDbGuiItem(),
                    "askBeforeOpeningFileReadOnly",
                    KDbMessageHandler::Notify,
                    msgHandler)
        {
            clearError();
            if (!drv_closeDatabase())
                return false;
            *cancelled = true;
            return false;
        }
    }

    if (d->res == SQLITE_CANTOPEN_WITH_LOCKED_READWRITE) {
        setError(ERR_ACCESS_RIGHTS,
                 futureTr("The file is probably already open on this or another computer.\n\n"
                          "Could not gain exclusive access for reading and writing the file. "
                          "Check the file's permissions and whether it is already opened and locked by another application."));
    } else if (d->res == SQLITE_CANTOPEN_WITH_LOCKED_WRITE) {
        setError(ERR_ACCESS_RIGHTS,
                 futureTr("The file is probably already open on this or another computer.\n\n"
                          "Could not gain exclusive access for writing the file. "
                          "Check the file's permissions and whether it is already opened and locked by another application."));
    }*/
    return res == SQLITE_OK;
}

void SqliteConnection::drv_closeDatabaseSilently()
{
    KDbResult result = this->result(); // save
    drv_closeDatabase();
    m_result = result;
}

bool SqliteConnection::drv_closeDatabase()
{
    if (!d->data)
        return false;

    const int res = sqlite3_close(d->data);
    if (SQLITE_OK == res) {
        d->data = 0;
        return true;
    }
    if (SQLITE_BUSY == res) {
#if 0 //this is ANNOYING, needs fixing (by closing cursors or waiting)
        setError(ERR_CLOSE_FAILED, futureTr("Could not close busy database."));
#else
        return true;
#endif
    }
    return false;
}

bool SqliteConnection::drv_dropDatabase(const QString &dbName)
{
    Q_UNUSED(dbName); // Each database is one single SQLite file.
    const QString filename = data().databaseName();
    if (QFile::exists(filename) && !QFile::remove(filename)) {
        m_result = KDbResult(ERR_ACCESS_RIGHTS,
                             SqliteConnection::tr("Could not delete file \"%1\". "
                             "Check the file's permissions and whether it is already "
                             "opened and locked by another application.")
                   .arg(QDir::fromNativeSeparators(filename)));
        return false;
    }
    return true;
}

KDbCursor* SqliteConnection::prepareQuery(const KDbEscapedString& sql, int cursor_options)
{
    return new SqliteCursor(this, sql, cursor_options);
}

KDbCursor* SqliteConnection::prepareQuery(KDbQuerySchema* query, int cursor_options)
{
    return new SqliteCursor(this, query, cursor_options);
}

KDbSqlResult* SqliteConnection::drv_executeSQL(const KDbEscapedString& sql)
{
#ifdef KDB_DEBUG_GUI
    KDb::debugGUI(QLatin1String("ExecuteSQL (SQLite): ") + sql.toString());
#endif

    sqlite3_stmt *prepared_st = nullptr;
    const int res = sqlite3_prepare(
                 d->data,            /* Database handle */
                 sql.constData(),    /* SQL statement, UTF-8 encoded */
                 sql.length(),       /* Length of zSql in bytes. */
                 &prepared_st,       /* OUT: Statement handle */
                 nullptr/*const char **pzTail*/     /* OUT: Pointer to unused portion of zSql */
             );
    if (res != SQLITE_OK) {
        m_result.setServerErrorCode(res);
        storeResult();
#ifdef KDB_DEBUG_GUI
        KDb::debugGUI(QLatin1String("  Failure"));
#endif
        return nullptr;
    }

#ifdef KDB_DEBUG_GUI
    KDb::debugGUI(QLatin1String("  Success"));
#endif
    return new SqliteSqlResult(this, prepared_st);
}

bool SqliteConnection::drv_executeVoidSQL(const KDbEscapedString& sql)
{
#ifdef KDB_DEBUG_GUI
    KDb::debugGUI(QLatin1String("ExecuteVoidSQL (SQLite): ") + sql.toString());
#endif

    char *errmsg_p = nullptr;
    const int res = sqlite3_exec(
                 d->data,
                 sql.constData(),
                 nullptr/*callback*/,
                 nullptr,
                 &errmsg_p);
    if (res != SQLITE_OK) {
        m_result.setServerErrorCode(res);
    }
    if (errmsg_p) {
        clearResult();
        m_result.setServerMessage(QLatin1String(errmsg_p));
        sqlite3_free(errmsg_p);
    } else {
        storeResult();
    }

#ifdef KDB_DEBUG_GUI
    KDb::debugGUI(QLatin1String( res == SQLITE_OK ? "  Success" : "  Failure"));
#endif
    return res == SQLITE_OK;
}

QString SqliteConnection::serverResultName() const
{
    return SqliteConnectionInternal::serverResultName(m_result.serverErrorCode());
}

KDbPreparedStatementInterface* SqliteConnection::prepareStatementInternal()
{
    return new SqlitePreparedStatement(d);
}

bool SqliteConnection::findAndLoadExtension(const QString & name)
{
    QStringList pluginPaths;
    foreach (const QString& path, KDb::libraryPaths()) {
        pluginPaths += path + QLatin1String("/sqlite3");
    }
    pluginPaths += options()->property("extraSqliteExtensionPaths").value.toStringList();
    foreach (const QString& path, pluginPaths) {
        if (loadExtension(path + QLatin1Char('/') + name + QLatin1String(KDB_SHARED_LIB_EXTENSION))) {
            return true;
        }
    }
    clearResult();
    m_result = KDbResult(ERR_CANNOT_LOAD_OBJECT,
                         SqliteConnection::tr("Could not load SQLite plugin \"%1\".").arg(name));
    return false;
}

bool SqliteConnection::loadExtension(const QString& path)
{
    bool tempEnable = false;
    clearResult();
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        m_result = KDbResult(ERR_OBJECT_NOT_FOUND,
                             SqliteConnection::tr("Could not find SQLite plugin file \"%1\".").arg(path));
        //sqliteWarning() << "SqliteConnection::loadExtension(): Could not find SQLite extension";
        return false;
    }
    if (!d->extensionsLoadingEnabled()) {
        tempEnable = true;
        d->setExtensionsLoadingEnabled(true);
    }
    char *errmsg_p = 0;
    int res = sqlite3_load_extension(d->data, QDir::toNativeSeparators(path).toUtf8().constData(), 0, &errmsg_p);
    bool ok = res == SQLITE_OK;
    if (!ok) {
        m_result.setServerErrorCode(res);
        m_result = KDbResult(ERR_CANNOT_LOAD_OBJECT,
                             SqliteConnection::tr("Could not load SQLite extension \"%1\".").arg(path));
        sqliteWarning() << "SqliteConnection::loadExtension(): Could not load SQLite extension"
                << path << ":" << errmsg_p;
        if (errmsg_p) {
            m_result.setServerMessage(QLatin1String(errmsg_p));
            sqlite3_free(errmsg_p);
        }
    }
    if (tempEnable) {
        d->setExtensionsLoadingEnabled(false);
    }
    return ok;
}

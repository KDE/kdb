/* This file is part of the KDE project
   Copyright (C) 2003-2006 Jarosław Staniek <staniek@kde.org>

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
#include "sqliteCursor.h"
#include "SqlitePreparedStatement.h"

//#include "kexisql.h" //for isReadOnly()
#include <sqlite3.h>

#include <Predicate/Driver.h>
#include <Predicate/Cursor.h>
#include <Predicate/Error.h>
#include <Predicate/tools/Utils.h>

#include <qfile.h>
#include <qdir.h>
#include <qregexp.h>

#include <QtDebug>

//remove debug
#undef PreDrvDbg
#define PreDrvDbg if (true); else qDebug()

using namespace Predicate;

SQLiteConnectionInternal::SQLiteConnectionInternal(Connection *connection)
        : ConnectionInternal(connection)
        , data(0)
        , data_owned(true)
        , errmsg_p(0)
        , res(SQLITE_OK)
        , result_name(0)
{
}

SQLiteConnectionInternal::~SQLiteConnectionInternal()
{
    if (data_owned && data) {
        free(data);
        data = 0;
    }
//sqlite_freemem does this if (errmsg) {
//  free( errmsg );
//  errmsg = 0;
// }
}

void SQLiteConnectionInternal::storeResult()
{
    if (errmsg_p) {
        errmsg = errmsg_p;
        sqlite3_free(errmsg_p);
        errmsg_p = 0;
    }
    errmsg = (data && res != SQLITE_OK) ? sqlite3_errmsg(data) : 0;
}

/*! Used by driver */
SQLiteConnection::SQLiteConnection(Driver *driver, ConnectionData &conn_data)
        : Connection(driver, conn_data)
        , d(new SQLiteConnectionInternal(this))
{
}

SQLiteConnection::~SQLiteConnection()
{
    PreDrvDbg;
    //disconnect if was connected
// disconnect();
    destroy();
    delete d;
    PreDrvDbg << "ok";
}

bool SQLiteConnection::drv_connect(Predicate::ServerVersionInfo& version)
{
    PreDrvDbg;
    version.string = QString(SQLITE_VERSION); //defined in sqlite3.h
    QRegExp re("(\\d+)\\.(\\d+)\\.(\\d+)");
    if (re.exactMatch(version.string)) {
        version.major = re.cap(1).toUInt();
        version.minor = re.cap(2).toUInt();
        version.release = re.cap(3).toUInt();
    }
    return true;
}

bool SQLiteConnection::drv_disconnect()
{
    PreDrvDbg;
    return true;
}

bool SQLiteConnection::drv_getDatabasesList(QStringList &list)
{
    //this is one-db-per-file database
    list.append(data()->fileName());   //more consistent than dbFileName() ?
    return true;
}

bool SQLiteConnection::drv_containsTable(const QString &tableName)
{
    bool success;
    return resultExists(QString("select name from sqlite_master where type='table' and name LIKE %1")
                        .arg(driver()->escapeString(tableName)), success) && success;
}

bool SQLiteConnection::drv_getTablesList(QStringList &list)
{
    Predicate::Cursor *cursor;
    m_sql = "select lower(name) from sqlite_master where type='table'";
    if (!(cursor = executeQuery(m_sql))) {
        PreWarn << "!executeQuery()";
        return false;
    }
    list.clear();
    cursor->moveFirst();
    while (!cursor->eof() && !cursor->error()) {
        list += cursor->value(0).toString();
        cursor->moveNext();
    }
    if (cursor->error()) {
        deleteCursor(cursor);
        return false;
    }
    return deleteCursor(cursor);
}

bool SQLiteConnection::drv_createDatabase(const QString &dbName)
{
    // SQLite creates a new db is it does not exist
    return drv_useDatabase(dbName);
#if 0
    d->data = sqlite_open(QFile::encodeName(data()->fileName()), 0/*mode: unused*/,
                          &d->errmsg_p);
    d->storeResult();
    return d->data != 0;
#endif
}

bool SQLiteConnection::drv_useDatabase(const QString &dbName, bool *cancelled,
                                       MessageHandler* msgHandler)
{
    Q_UNUSED(dbName);
// PreDrvDbg << "drv_useDatabase(): " << data()->fileName();
    //TODO: perhaps allow to use sqlite3_open16() as well for SQLite ~ 3.3 ?
//! @todo add option (command line or in kexirc?)
    int flags = Connection::isReadOnly() ? SQLITE_OPEN_READONLY : SQLITE_OPEN_READWRITE;
//! @todo add option
//removed in predicate    int allowReadonly = 1;
//removed in predicate    const bool wasReadOnly = Connection::isReadOnly();

    d->res = sqlite3_open_v2(
                 data()->fileName().toUtf8().constData(), /* unicode expected since SQLite 3.1 */
                 &d->data,
                 flags,
//removed in predicate                 allowReadonly /* If 1 and locking fails, try opening in read-only mode */
                 0
             );
    d->storeResult();

// @todo removed in predicate - reenable?
/*
    if (d->res == SQLITE_OK && cancelled && !wasReadOnly && allowReadonly && isReadOnly()) {
        //opened as read only, ask
        if (MessageHandler::Continue !=
                askQuestion(
                    MessageHandler::WarningContinueCancel,
                    tr("Do you want to open file \"%1\" as read-only?\n\n"
                        "The file is probably already open on this or another computer. "
                        "Could not gain exclusive access for writing the file.")
                    .arg(QDir::convertSeparators(data()->fileName())),
                    QObject::tr("Opening As Read-Only"),
                    MessageHandler::Continue,
                    MessageHandler::GuiItem()
                            .setProperty("text", QObject::tr("Open As Read-Only"))
                            .setProperty("icon", "document-open"),
                    MessageHandler::GuiItem(),
                    "askBeforeOpeningFileReadOnly",
                    MessageHandler::Notify,
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
                 tr("The file is probably already open on this or another computer.") + "\n\n"
                 + tr("Could not gain exclusive access for reading and writing the file.") + " "
                 + tr("Check the file's permissions and whether it is already opened and locked by another application."));
    } else if (d->res == SQLITE_CANTOPEN_WITH_LOCKED_WRITE) {
        setError(ERR_ACCESS_RIGHTS,
                 tr("The file is probably already open on this or another computer.") + "\n\n"
                 + tr("Could not gain exclusive access for writing the file.") + " "
                 + tr("Check the file's permissions and whether it is already opened and locked by another application."));
    }*/
    return d->res == SQLITE_OK;
}

bool SQLiteConnection::drv_closeDatabase()
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
        setError(ERR_CLOSE_FAILED, tr("Could not close busy database."));
#else
        return true;
#endif
    }
    return false;
}

bool SQLiteConnection::drv_dropDatabase(const QString &dbName)
{
    Q_UNUSED(dbName); // Each database is one single SQLite file.
    const QString filename = data()->fileName();
    if (QFile(filename).exists() && !QDir().remove(filename)) {
        setError(ERR_ACCESS_RIGHTS, tr("Could not remove file \"%1\". "
                 "Check the file's permissions and whether it is already opened and locked by another application.")
                   .arg(QDir::convertSeparators(filename)));
        return false;
    }
    return true;
}

//CursorData* SQLiteConnection::drv_createCursor( const QString& statement )
Cursor* SQLiteConnection::prepareQuery(const QString& statement, uint cursor_options)
{
    return new SQLiteCursor(this, statement, cursor_options);
}

Cursor* SQLiteConnection::prepareQuery(QuerySchema& query, uint cursor_options)
{
    return new SQLiteCursor(this, query, cursor_options);
}

bool SQLiteConnection::drv_executeSQL(const QString& statement)
{
// PreDrvDbg << statement;
// QCString st(statement.length()*2);
// st = escapeString( statement.local8Bit() ); //?
#ifdef SQLITE_UTF8
    d->temp_st = statement.toUtf8();
#else
    d->temp_st = statement.toLocal8Bit(); //latin1 only
#endif

#ifdef KEXI_DEBUG_GUI
    Utils::addKexiDBDebug(QString("ExecuteSQL (SQLite): ") + statement);
#endif

    d->res = sqlite3_exec(
                 d->data,
                 (const char*)d->temp_st,
                 0/*callback*/,
                 0,
                 &d->errmsg_p);
    d->storeResult();
#ifdef KEXI_DEBUG_GUI
    Utils::addKexiDBDebug(d->res == SQLITE_OK ? "  Success" : "  Failure");
#endif
    return d->res == SQLITE_OK;
}

quint64 SQLiteConnection::drv_lastInsertRowID()
{
    return (quint64)sqlite3_last_insert_rowid(d->data);
}

int SQLiteConnection::serverResult()
{
    return d->res == 0 ? Connection::serverResult() : d->res;
}

static const char* serverResultNames[] = {
    "SQLITE_OK", // 0
    "SQLITE_ERROR",
    "SQLITE_INTERNAL",
    "SQLITE_PERM",
    "SQLITE_ABORT",
    "SQLITE_BUSY",
    "SQLITE_LOCKED",
    "SQLITE_NOMEM",
    "SQLITE_READONLY",
    "SQLITE_INTERRUPT",
    "SQLITE_IOERR",
    "SQLITE_CORRUPT",
    "SQLITE_NOTFOUND",
    "SQLITE_FULL",
    "SQLITE_CANTOPEN",
    "SQLITE_PROTOCOL",
    "SQLITE_EMPTY",
    "SQLITE_SCHEMA",
    "SQLITE_TOOBIG",
    "SQLITE_CONSTRAINT",
    "SQLITE_MISMATCH",
    "SQLITE_MISUSE",
    "SQLITE_NOLFS",
    "SQLITE_AUTH",
    "SQLITE_FORMAT",
    "SQLITE_RANGE",
    "SQLITE_NOTADB", // 26
};

QString SQLiteConnection::serverResultName()
{
    if (d->res >= 0 && d->res <= SQLITE_NOTADB)
        return QString::fromLatin1(serverResultNames[d->res]);
    else if (d->res == SQLITE_ROW)
        return QLatin1String("SQLITE_ROW");
    else if (d->res == SQLITE_DONE)
        return QLatin1String("SQLITE_DONE");
    return QString();
}

void SQLiteConnection::drv_clearServerResult()
{
    if (!d)
        return;
    d->res = SQLITE_OK;
// d->result_name = 0;
}

QString SQLiteConnection::serverErrorMsg()
{
    return d->errmsg.isEmpty() ? Connection::serverErrorMsg() : d->errmsg;
}

PreparedStatementInterface* SQLiteConnection::prepareStatementInternal()
{
    return new SQLitePreparedStatement(*d);
}

bool SQLiteConnection::isReadOnly() const
{
    return Connection::isReadOnly();
//! @todo port
    //return (d->data ? sqlite3_is_readonly(d->data) : false)
    //       || Connection::isReadOnly();
}

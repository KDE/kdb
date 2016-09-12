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

#ifndef KDB_SQLITECONN_P_H
#define KDB_SQLITECONN_P_H

#include "KDbConnection_p.h"
#include "SqliteConnection.h"
#include "KDbSqlField.h"
#include "KDbSqlRecord.h"
#include "KDbSqlResult.h"
#include "KDbSqlString.h"

#include <sqlite3.h>

/*! Internal SQLite connection data. Also used by SqliteCursor. */
class SqliteConnectionInternal : public KDbConnectionInternal
{
public:
    explicit SqliteConnectionInternal(KDbConnection *connection);
    virtual ~SqliteConnectionInternal();

    //! @return true is loading extensions is enabled
    bool extensionsLoadingEnabled() const;

    //! Sets loading extensions flag to @a set
    void setExtensionsLoadingEnabled(bool set);

    static QString serverResultName(int serverResultCode);

    void storeResult(KDbResult *result);

    sqlite3 *data;
    bool data_owned; //!< true if data pointer should be freed on destruction

private:
    bool m_extensionsLoadingEnabled;
    Q_DISABLE_COPY(SqliteConnectionInternal)
};

class SqliteSqlField : public KDbSqlField
{
public:
    inline SqliteSqlField(sqlite3_stmt *st, int i) : prepared_st(st), index(i)
    {
    }
    //! @return column name
    inline QString name() Q_DECL_OVERRIDE {
        return QString::fromUtf8(sqlite3_column_name(prepared_st, index));
    }
    //! @return column type
    inline int type() Q_DECL_OVERRIDE {
        return sqlite3_column_type(prepared_st, index);
    }
    //! @return length limit - no limits for SQLite
    inline int length() Q_DECL_OVERRIDE {
        return std::numeric_limits<quint64>::max();
    }
private:
    sqlite3_stmt * const prepared_st;
    const int index;
    Q_DISABLE_COPY(SqliteSqlField)
};

class SqliteSqlRecord : public KDbSqlRecord
{
public:
    inline SqliteSqlRecord(sqlite3_stmt *st)
        : prepared_st(st)
    {
        Q_ASSERT(st);
    }
    inline ~SqliteSqlRecord() {
    }
    inline QString stringValue(int index) Q_DECL_OVERRIDE {
        return QString::fromUtf8(
                        (const char*)sqlite3_column_text(prepared_st, index),
                        sqlite3_column_bytes(prepared_st, index));
    }
    inline KDbSqlString cstringValue(int index) Q_DECL_OVERRIDE {
        // sqlite3_column_text() returns UTF-8 but it's OK if the data is a C string
        return KDbSqlString((const char*)sqlite3_column_text(prepared_st, index),
                            sqlite3_column_bytes(prepared_st, index));
    }
    inline QByteArray toByteArray(int index) Q_DECL_OVERRIDE {
        return QByteArray((const char*)sqlite3_column_blob(prepared_st, index),
                          sqlite3_column_bytes(prepared_st, index));
    }

private:
    sqlite3_stmt * const prepared_st;
    Q_DISABLE_COPY(SqliteSqlRecord)
};

//! Used by SqliteSqlResult::cacheFieldInfo(const QString&)
struct SqliteSqlFieldInfo {
    void setConstraints(KDbField* field);
    QString defaultValue;
    bool isNotNull;
    bool isPrimaryKey;
};

class SqliteSqlResult : public KDbSqlResult
{
public:
    inline SqliteSqlResult(SqliteConnection *c, sqlite3_stmt *st)
        : conn(c), prepared_st(st)
    {
        Q_ASSERT(c);
    }

    inline ~SqliteSqlResult() {
        // don't check result here, done elsewhere already
        (void)sqlite3_finalize(prepared_st);
    }

    inline KDbConnection *connection() const Q_DECL_OVERRIDE {
        return conn;
    }

    inline int fieldsCount() Q_DECL_OVERRIDE {
        // We're using sqlite3_column_count instead of sqlite3_data_count to know
        // the column count before fetching. User will know if fetching succeeded anyway.
        return sqlite3_column_count(prepared_st);
    }

    inline KDbSqlField *field(int index) Q_DECL_OVERRIDE Q_REQUIRED_RESULT {
        return prepared_st ? new SqliteSqlField(prepared_st, index) : nullptr;
    }

    KDbField *createField(const QString &tableName, int index) Q_DECL_OVERRIDE Q_REQUIRED_RESULT;

    inline KDbSqlRecord* fetchRecord() Q_DECL_OVERRIDE Q_REQUIRED_RESULT {
        SqliteSqlRecord *record;
        const int res = sqlite3_step(prepared_st);
        if (res == SQLITE_ROW) {
            record = new SqliteSqlRecord(prepared_st);
        } else {
            record = nullptr;
        }
        return record;
    }

    inline KDbResult lastResult() Q_DECL_OVERRIDE {
        KDbResult res;
        const int err = sqlite3_errcode(conn->d->data);
        if (err != SQLITE_ROW && err != SQLITE_OK && err != SQLITE_DONE) {
            res.setCode(ERR_OTHER);
            res.setServerErrorCode(err);
            conn->d->storeResult(&res);
        }
        return res;
    }

    inline quint64 lastInsertRecordId() Q_DECL_OVERRIDE {
        return static_cast<quint64>(sqlite3_last_insert_rowid(conn->d->data));
    }

protected:
    //! @return a KDb type for a SQLite type
    //! The returned type is a guess, for example KDbField::Integer is returned for SQLITE_INTEGER.
    //! For unsupported types returns KDbField::InvalidType.
    //! See https://www.sqlite.org/c3ref/c_blob.html
    static KDbField::Type type(int sqliteType);

    //! Sets constraints to a @a field based on SQLite's internal schema:
    //! - whether the column can be NULL
    //! - the default value for the column
    //! - whether the column is primary key
    //! @note @a field should have its name set
    //! See https://www.sqlite.org/pragma.html#pragma_table_info
    //! @todo support keys
    bool setConstraints(const QString &tableName, KDbField* field);

    //! Caches information about the the fields, for setConstraints()
    //! @todo Support composite primary keys
    //! @todo Default values are only encoded as string
    bool cacheFieldInfo(const QString &tableName);

private:
    SqliteConnection * const conn;
    sqlite3_stmt * const prepared_st;
    KDbUtils::AutodeletedHash<QString, SqliteSqlFieldInfo*> cachedFieldInfos;
    friend class SqlitePreparedStatement;
    Q_DISABLE_COPY(SqliteSqlResult)
};

#endif

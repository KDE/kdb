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

#include "SqliteConnection_p.h"

SqliteConnectionInternal::SqliteConnectionInternal(KDbConnection *connection)
        : KDbConnectionInternal(connection)
        , data(nullptr)
        , data_owned(true)
        , m_extensionsLoadingEnabled(false)
{
}

SqliteConnectionInternal::~SqliteConnectionInternal()
{
    if (data_owned && data) {
        sqlite3_close(data);
        data = nullptr;
    }
}

static const char* const serverResultNames[] = {
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

// static
QString SqliteConnectionInternal::serverResultName(int serverResultCode)
{
    if (serverResultCode >= 0 && serverResultCode <= SQLITE_NOTADB)
        return QString::fromLatin1(serverResultNames[serverResultCode]);
    else if (serverResultCode == SQLITE_ROW)
        return QLatin1String("SQLITE_ROW");
    else if (serverResultCode == SQLITE_DONE)
        return QLatin1String("SQLITE_DONE");
    return QString();
}

void SqliteConnectionInternal::storeResult(KDbResult *result)
{
    result->setServerMessage(
        (data && result->isError()) ? QString::fromUtf8(sqlite3_errmsg(data))
                                    : QString());
}

bool SqliteConnectionInternal::extensionsLoadingEnabled() const
{
    return m_extensionsLoadingEnabled;
}

void SqliteConnectionInternal::setExtensionsLoadingEnabled(bool set)
{
    if (set == m_extensionsLoadingEnabled)
        return;
    sqlite3_enable_load_extension(data, set);
    m_extensionsLoadingEnabled = set;
}

//static
KDbField::Type SqliteSqlResult::type(int sqliteType)
{
    KDbField::Type t;
    switch(sqliteType) {
    case SQLITE_INTEGER: t = KDbField::Integer; break;
    case SQLITE_FLOAT: t = KDbField::Double; break;
    case SQLITE_BLOB: t = KDbField::BLOB; break;
    case SQLITE_NULL: t = KDbField::Null; break;
    case SQLITE_TEXT: t = KDbField::LongText; break;
    default: t = KDbField::InvalidType; break;
    }
    return t;
}

bool SqliteSqlResult::setConstraints(const QString &tableName, KDbField* field)
{
    Q_ASSERT(field);
    if (!cacheFieldInfo(tableName)) {
        return false;
    }
    SqliteSqlFieldInfo* info = cachedFieldInfos.value(field->name());
    if (info) {
        info->setConstraints(field);
        return true;
    } else {
        return false;
    }
}

void SqliteSqlFieldInfo::setConstraints(KDbField* field)
{
    field->setDefaultValue(KDbField::convertToType(defaultValue, field->type()));
    field->setNotNull(isNotNull);
    field->setPrimaryKey(isPrimaryKey);
}

bool SqliteSqlResult::cacheFieldInfo(const QString &tableName)
{
    if (!cachedFieldInfos.isEmpty()) {
        return true;
    }
    QSharedPointer<KDbSqlResult> tableInfoResult = conn->prepareSql(
        KDbEscapedString("PRAGMA table_info(%1)").arg(conn->escapeIdentifier(tableName)));
    if (!tableInfoResult) {
        return false;
    }
    // Forward-compatible approach: find columns of table_info that we need
    const int columns = tableInfoResult->fieldsCount();
    enum TableInfoColumns {
        TableInfoFieldName,
        TableInfoNotNull,
        TableInfoDefault,
        TableInfoPK
    };
    QVector<int> columnIndex(TableInfoPK + 1);
    int found = 0;
    for(int col = 0; col < columns; ++col) {
        QScopedPointer<KDbSqlField> f(tableInfoResult->field(col));
        if (!f) {
            return false;
        }
        if (f->name() == QLatin1String("name")) {
            columnIndex[TableInfoFieldName] = col;
            ++found;
        } else if (f->name() == QLatin1String("notnull")) {
            columnIndex[TableInfoNotNull] = col;
            ++found;
        } else if (f->name() == QLatin1String("dflt_value")) {
            columnIndex[TableInfoDefault] = col;
            ++found;
        } else if (f->name() == QLatin1String("pk")) {
            columnIndex[TableInfoPK] = col;
            ++found;
        }
    }
    if (found != TableInfoPK + 1) { // not all columns found
        return false;
    }

    bool ok = true;
    Q_FOREVER {
        QSharedPointer<KDbSqlRecord> record = tableInfoResult->fetchRecord();
        if (!record) {
            ok = !tableInfoResult->lastResult().isError();
            break;
        }
        QScopedPointer<SqliteSqlFieldInfo> info(new SqliteSqlFieldInfo);
        const QString name = record->stringValue(columnIndex[TableInfoFieldName]);
        if (name.isEmpty()) {
            ok = false;
            break;
        }
        info->defaultValue = record->stringValue(columnIndex[TableInfoDefault]);
        info->isNotNull
            = record->cstringValue(columnIndex[TableInfoNotNull]).rawDataToByteArray() == "1";
        //! @todo Support composite primary keys:
        //! The "pk" column in the result set is zero for columns that are not part of
        //! the primary key, and is the index of the column in the primary key for columns
        //! that are part of the primary key.
        //! https://www.sqlite.org/pragma.html#pragma_table_info
        info->isPrimaryKey
            = record->cstringValue(columnIndex[TableInfoPK]).rawDataToByteArray() != "0";
        cachedFieldInfos.insert(name, info.take());
    }
    if (!ok) {
        cachedFieldInfos.clear();
    }
    return ok;
}

KDbField *SqliteSqlResult::createField(const QString &tableName, int index)
{
    QScopedPointer<SqliteSqlField> f(static_cast<SqliteSqlField*>(field(index)));
    if (!f) {
        return nullptr;
    }
    const QString caption(f->name());
    QString realFieldName(KDb::stringToIdentifier(caption.toLower()));
    KDbField *kdbField = new KDbField(realFieldName, type(f->type()));
    kdbField->setCaption(caption);
    setConstraints(tableName, kdbField);
    return kdbField;
}

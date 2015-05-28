/* This file is part of the KDE project
   Copyright (C) 2003-2012 Jarosław Staniek <staniek@kde.org>

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

#include "SqliteDriver.h"
#include "KDbConnection.h"
#include "KDbDriverManager.h"
#include "KDbDriver_p.h"
#include "KDb.h"

#include "SqliteConnection.h"
#include "SqliteConnection_p.h"
#include "SqliteAdmin.h"


#include <sqlite3.h>

KDB_DRIVER_PLUGIN_FACTORY(SQLiteDriver, "kdb_sqlitedriver.json")

//! driver specific private data
//! @internal
class SQLiteDriverPrivate
{
public:
    SQLiteDriverPrivate() 
     : collate(QLatin1String(" COLLATE ''"))
    {
    }
    KDbEscapedString collate;
};

SQLiteDriver::SQLiteDriver(QObject *parent, const QVariantList &args)
        : KDbDriver(parent, args)
        , dp(new SQLiteDriverPrivate)
{
    d->isDBOpenedAfterCreate = true;
    d->features = SingleTransactions | CursorForward
                  | CompactingDatabaseSupported;

    //special method for autoincrement definition
    beh->SPECIAL_AUTO_INCREMENT_DEF = true;
    beh->AUTO_INCREMENT_FIELD_OPTION = QString(); //not available
    beh->AUTO_INCREMENT_TYPE = QLatin1String("INTEGER");
    beh->AUTO_INCREMENT_PK_FIELD_OPTION = QLatin1String("PRIMARY KEY");
    beh->AUTO_INCREMENT_REQUIRES_PK = true;
    beh->ROW_ID_FIELD_NAME = QLatin1String("OID");
    beh->_1ST_ROW_READ_AHEAD_REQUIRED_TO_KNOW_IF_THE_RESULT_IS_EMPTY = true;
    beh->QUOTATION_MARKS_FOR_IDENTIFIER = '"';
    beh->SELECT_1_SUBQUERY_SUPPORTED = true;
    beh->CONNECTION_REQUIRED_TO_CHECK_DB_EXISTENCE = false;
    beh->CONNECTION_REQUIRED_TO_CREATE_DB = false;
    beh->CONNECTION_REQUIRED_TO_DROP_DB = false;

    initDriverSpecificKeywords(keywords);

    // internal properties
    d->properties.insert("client_library_version", QLatin1String(sqlite3_libversion()));
    d->properties.insert("default_server_encoding", QLatin1String("UTF8")); //OK?

    d->typeNames[KDbField::Byte] = QLatin1String("Byte");
    d->typeNames[KDbField::ShortInteger] = QLatin1String("ShortInteger");
    d->typeNames[KDbField::Integer] = QLatin1String("Integer");
    d->typeNames[KDbField::BigInteger] = QLatin1String("BigInteger");
    d->typeNames[KDbField::Boolean] = QLatin1String("Boolean");
    d->typeNames[KDbField::Date] = QLatin1String("Date"); // In fact date/time types could be declared as datetext etc.
    d->typeNames[KDbField::DateTime] = QLatin1String("DateTime"); // to force text affinity..., see http://sqlite.org/datatype3.html
    d->typeNames[KDbField::Time] = QLatin1String("Time");
    d->typeNames[KDbField::Float] = QLatin1String("Float");
    d->typeNames[KDbField::Double] = QLatin1String("Double");
    d->typeNames[KDbField::Text] = QLatin1String("Text");
    d->typeNames[KDbField::LongText] = QLatin1String("CLOB");
    d->typeNames[KDbField::BLOB] = QLatin1String("BLOB");
}

SQLiteDriver::~SQLiteDriver()
{
    delete dp;
}


KDbConnection*
SQLiteDriver::drv_createConnection(const KDbConnectionData& connData)
{
    return new SQLiteConnection(this, connData);
}

bool SQLiteDriver::isSystemObjectName(const QString& n) const
{
    return KDbDriver::isSystemObjectName(n)
           || n.startsWith(QLatin1String("sqlite_"), Qt::CaseInsensitive);
}

bool SQLiteDriver::drv_isSystemFieldName(const QString& n) const
{
    const QString lcName(n.toLower());
    return (lcName == QLatin1String("_rowid_"))
           || (lcName == QLatin1String("rowid"))
           || (lcName == QLatin1String("oid"));
}

KDbEscapedString SQLiteDriver::escapeString(const QString& str) const
{
    return KDbEscapedString("'") + KDbEscapedString(str).replace('\'', "''") + '\'';
}

KDbEscapedString SQLiteDriver::escapeString(const QByteArray& str) const
{
    return KDbEscapedString("'") + KDbEscapedString(str).replace('\'', "''") + '\'';
}

KDbEscapedString SQLiteDriver::escapeBLOB(const QByteArray& array) const
{
    return KDbEscapedString(KDb::escapeBLOB(array, KDb::BLOBEscapeXHex));
}

QString SQLiteDriver::drv_escapeIdentifier(const QString& str) const
{
    return QString(str).replace(QLatin1Char('"'), QLatin1String("\"\""));
}

QByteArray SQLiteDriver::drv_escapeIdentifier(const QByteArray& str) const
{
    return QByteArray(str).replace('"', "\"\"");
}

KDbAdminTools* SQLiteDriver::drv_createAdminTools() const
{
    return new SQLiteAdminTools();
}

KDbEscapedString SQLiteDriver::collationSQL() const
{
    return dp->collate;
}

#include "SqliteDriver.moc"

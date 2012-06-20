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

#include <Predicate/Connection>
#include <Predicate/DriverManager>
#include <Predicate/Private/Driver>
#include <Predicate/Utils>

#include "SqliteDriver.h"
#include "SqliteConnection.h"
#include "SqliteConnection_p.h"
#include "SqliteAdmin.h"

#include <QtDebug>

#include <sqlite3.h>

using namespace Predicate;

EXPORT_PREDICATE_DRIVER(SQLiteDriver, sqlite)

//! driver specific private data
//! @internal
class Predicate::SQLiteDriverPrivate
{
public:
    SQLiteDriverPrivate() 
     : collate(QLatin1String(" COLLATE ''"))
    {
    }
    EscapedString collate;
};

SQLiteDriver::SQLiteDriver()
        : Driver()
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

    //predefined properties
    d->properties["client_library_version"] = QLatin1String(sqlite3_libversion());
    d->properties["default_server_encoding"] = QLatin1String("UTF8"); //OK?

    d->typeNames[Field::Byte] = QLatin1String("Byte");
    d->typeNames[Field::ShortInteger] = QLatin1String("ShortInteger");
    d->typeNames[Field::Integer] = QLatin1String("Integer");
    d->typeNames[Field::BigInteger] = QLatin1String("BigInteger");
    d->typeNames[Field::Boolean] = QLatin1String("Boolean");
    d->typeNames[Field::Date] = QLatin1String("Date"); // In fact date/time types could be declared as datetext etc.
    d->typeNames[Field::DateTime] = QLatin1String("DateTime"); // to force text affinity..., see http://sqlite.org/datatype3.html
    d->typeNames[Field::Time] = QLatin1String("Time");
    d->typeNames[Field::Float] = QLatin1String("Float");
    d->typeNames[Field::Double] = QLatin1String("Double");
    d->typeNames[Field::Text] = QLatin1String("Text");
    d->typeNames[Field::LongText] = QLatin1String("CLOB");
    d->typeNames[Field::BLOB] = QLatin1String("BLOB");
}

SQLiteDriver::~SQLiteDriver()
{
    delete dp;
}


Predicate::Connection*
SQLiteDriver::drv_createConnection(const ConnectionData& connData)
{
    return new SQLiteConnection(this, connData);
}

bool SQLiteDriver::isSystemObjectName(const QString& n) const
{
    return Driver::isSystemObjectName(n)
           || n.toLower().startsWith(QLatin1String("sqlite_"));
}

bool SQLiteDriver::drv_isSystemFieldName(const QString& n) const
{
    QString lcName(n.toLower());
    return (lcName == QLatin1String("_rowid_"))
           || (lcName == QLatin1String("rowid"))
           || (lcName == QLatin1String("oid"));
}

EscapedString SQLiteDriver::escapeString(const QString& str) const
{
    return EscapedString("'") + EscapedString(str).replace('\'', "''") + '\'';
}

EscapedString SQLiteDriver::escapeString(const QByteArray& str) const
{
    return EscapedString("'") + EscapedString(str).replace('\'', "''") + '\'';
}

EscapedString SQLiteDriver::escapeBLOB(const QByteArray& array) const
{
    return EscapedString(Predicate::escapeBLOB(array, Predicate::BLOBEscapeXHex));
}

QString SQLiteDriver::drv_escapeIdentifier(const QString& str) const
{
    return QString(str).replace(QLatin1Char('"'), QLatin1String("\"\""));
}

QByteArray SQLiteDriver::drv_escapeIdentifier(const QByteArray& str) const
{
    return QByteArray(str).replace('"', "\"\"");
}

AdminTools* SQLiteDriver::drv_createAdminTools() const
{
    return new SQLiteAdminTools();
}

EscapedString SQLiteDriver::collationSQL() const
{
    return dp->collate;
}

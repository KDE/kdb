/* This file is part of the KDE project
Copyright (C) 2007   Sharan Rao <sharanrao@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this program; see the file COPYING. If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/


#include "SybaseDriver.h"
#include "SybaseConnection.h"
#include "KDbField.h"
#include "KDbDriverBehavior.h"
#include "KDb.h"

KDB_DRIVER_PLUGIN_FACTORY(SybaseDriver, "kdb_sybasedriver.json")

SybaseDriver::SybaseDriver(QObject *parent, const QVariantList &args)
    : KDbDriver(parent, args)
{
    // Sybase supports Nested Transactions. Ignore for now
    beh->features = IgnoreTransactions | CursorForward;

    // Last value assigned is stored in variable @@IDENTITY
    beh->ROW_ID_FIELD_NAME = "@@IDENTITY";

    beh->ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE = true ;

    beh->_1ST_ROW_READ_AHEAD_REQUIRED_TO_KNOW_IF_THE_RESULT_IS_EMPTY = false;
    beh->USING_DATABASE_REQUIRED_TO_CONNECT = false;

    // for Sybase ASA this field is "DEFAULT AUTOINCREMENT"
    // for MSSQL and Sybase ASE it's IDENTITY
    beh->AUTO_INCREMENT_FIELD_OPTION = "IDENTITY";
    beh->AUTO_INCREMENT_PK_FIELD_OPTION = beh->AUTO_INCREMENT_FIELD_OPTION + " PRIMARY KEY ";

    // confirm
    //beh->SELECT_1_SUBQUERY_SUPPORTED = true;

    beh->OPENING_QUOTATION_MARK_BEGIN_FOR_IDENTIFIER = '"';
    beh->CLOSING_QUOTATION_MARK_BEGIN_FOR_IDENTIFIER = '"';

    initDriverSpecificKeywords(m_keywords);

    //predefined properties
    beh->properties["client_library_version"] = ""; //!< @todo
    beh->properties["default_server_encoding"] = ""; //!< @todo

    // datatypes
    // integers
    beh->typeNames[KDbField::Byte] = "TINYINT";
    beh->typeNames[KDbField::ShortInteger] = "SMALLINT";
    beh->typeNames[KDbField::Integer] = "INT";
    beh->typeNames[KDbField::BigInteger] = "BIGINT";

    // boolean
    beh->typeNames[KDbField::Boolean] = "BIT";

    // date and time. There's only one integrated datetime datatype in Sybase
    // Though there are smalldatetime (4 bytes) and datetime (8 bytes) data types
    beh->typeNames[KDbField::Date] = "DATETIME";
    beh->typeNames[KDbField::DateTime] = "DATETIME";
    beh->typeNames[KDbField::Time] = "DATETIME"; // or should we use timestamp ?

    // floating point
    beh->typeNames[KDbField::Float] = "REAL"; // 4 bytes
    beh->typeNames[KDbField::Double] = "DOUBLE PRECISION"; // 8 bytes

    // strings
    beh->typeNames[KDbField::Text] = "VARCHAR";
    beh->typeNames[KDbField::LongText] = "TEXT";

    // Large Binary Objects
    beh->typeNames[KDbField::BLOB] = "IMAGE";
}

SybaseDriver::~SybaseDriver()
{
}

KDbConnection* SybaseDriver::drv_createConnection(const KDbConnectionData& connData,
                                   const KDbConnectionOptions &options)
{
    return new SybaseConnection(this, connData, options);
}

bool SybaseDriver::isSystemDatabaseName(const QString &n) const
{
    if (m_systemDatabases.isEmpty()) {
        m_systemDatabases << "master" << "model" << "sybsystemprocs" << "tempdb"
                          << "sybsecurity" << "sybsystemdb" << "pubs2" << "pubs3"
                          << "dbccdb";
    }
    return m_systemDatabases.contains(n.toLatin1().toLower());
}

bool SybaseDriver::drv_isSystemFieldName(const QString&) const
{
    return false;
}

KDbEscapedString SybaseDriver::escapeString(const QString& str) const
{
    return KDbEscapedString("'") + KDbEscapedString(str).replace("\'", "\\''") + '\'';
}

KDbEscapedString SybaseDriver::escapeBLOB(const QByteArray& array) const
{
    return KDbEscapedString(KDb::escapeBLOB(array, KDb::BLOBEscape0xHex));
}

KDbEscapedString SybaseDriver::escapeString(const QByteArray& str) const
{
    //! @todo needs any modification ?
    return KDbEscapedString("'") + KDbEscapedString(str).replace("\'", "\\''") + '\'';
}

QByteArray SybaseDriver::drv_escapeIdentifier(const QString& str) const
{
    //! @todo verify
    return QByteArray("\"") + QByteArray(str.toUtf8()).replace("\\", "\\\\").replace("\"", "\"\"")
           + "\"";
}

QByteArray SybaseDriver::drv_escapeIdentifier(const QByteArray& str) const
{
    // verify
    return QByteArray("\"") + str
           .replace("\\", "\\\\")
           .replace("\"", "\"\"")
           + "\"";
}

KDbEscapedString SybaseDriver::addLimitTo1(const QString& sql, bool add)
{
    // length of "select" is 6
    // eg: before:  select foo from foobar
    // after:   select TOP 1 foo from foobar
    return add ? KDbEscapedString(sql).trimmed().insert(6, " TOP 1 ") : KDbEscapedString(sql);
}

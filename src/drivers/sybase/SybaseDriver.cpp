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

#include <QVariant>
#include <QFile>
#include <QtDebug>

#include "SybaseDriver.h"
#include "SybaseConnection.h"
#include <Predicate/Field.h>
#include <Predicate/Driver_p.h>
#include <Predicate/Utils.h>

using namespace Predicate;

EXPORT_PREDICATE_DRIVER(SybaseDriver, sybase)

SybaseDriver::SybaseDriver()
    : Driver()
{
    // Sybase supports Nested Transactions. Ignore for now
    d->features = IgnoreTransactions | CursorForward;

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

    beh->QUOTATION_MARKS_FOR_IDENTIFIER = '"';

    initDriverSpecificKeywords(keywords);


    //predefined properties
    d->properties["client_library_version"] = "";//TODO
    d->properties["default_server_encoding"] = ""; //TODO

    // datatypes
    // integers
    d->typeNames[Field::Byte] = "TINYINT";
    d->typeNames[Field::ShortInteger] = "SMALLINT";
    d->typeNames[Field::Integer] = "INT";
    d->typeNames[Field::BigInteger] = "BIGINT";

    // boolean
    d->typeNames[Field::Boolean] = "BIT";

    // date and time. There's only one integrated datetime datatype in Sybase
    // Though there are smalldatetime (4 bytes) and datetime (8 bytes) data types
    d->typeNames[Field::Date] = "DATETIME";
    d->typeNames[Field::DateTime] = "DATETIME";
    d->typeNames[Field::Time] = "DATETIME"; // or should we use timestamp ?

    // floating point
    d->typeNames[Field::Float] = "REAL"; // 4 bytes
    d->typeNames[Field::Double] = "DOUBLE PRECISION"; // 8 bytes

    // strings
    d->typeNames[Field::Text] = "VARCHAR";
    d->typeNames[Field::LongText] = "TEXT";

    // Large Binary Objects
    d->typeNames[Field::BLOB] = "IMAGE";
}

SybaseDriver::~SybaseDriver()
{
}

Predicate::Connection*
SybaseDriver::drv_createConnection(const ConnectionData& connData)
{
    return new SybaseConnection(this, connData);
}

bool SybaseDriver::isSystemDatabaseName(const QString &n) const
{
    QStringList systemDatabases;
    systemDatabases << QString::fromLatin1("master")
    << QString::fromLatin1("model")
    << QString::fromLatin1("sybsystemprocs")
    << QString::fromLatin1("tempdb")
    << QString::fromLatin1("sybsecurity")
    << QString::fromLatin1("sybsystemdb")
    << QString::fromLatin1("pubs2")
    << QString::fromLatin1("pubs3")
    << QString::fromLatin1("dbccdb");

    QStringList::iterator i = qFind(systemDatabases.begin(), systemDatabases.end(), n.toLower());
    if (i != systemDatabases.end())
        return true;

    return Driver::isSystemObjectName(n);
}

bool SybaseDriver::drv_isSystemFieldName(const QString&) const
{
    return false;
}

EscapedString SybaseDriver::escapeString(const QString& str) const
{
    return EscapedString("'") + EscapedString(str).replace("\'", "\\''") + '\'';
}

EscapedString SybaseDriver::escapeBLOB(const QByteArray& array) const
{
    return EscapedString(Predicate::escapeBLOB(array, Predicate::BLOBEscape0xHex));
}

EscapedString SybaseDriver::escapeString(const QByteArray& str) const
{
    //! @todo needs any modification ?
    return EscapedString("'") + EscapedString(str).replace("\'", "\\''") + '\'';
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

EscapedString SybaseDriver::addLimitTo1(const QString& sql, bool add)
{
    // length of "select" is 6
    // eg: before:  select foo from foobar
    // after:   select TOP 1 foo from foobar
    return add ? EscapedString(sql).trimmed().insert(6, " TOP 1 ") : EscapedString(sql);
}

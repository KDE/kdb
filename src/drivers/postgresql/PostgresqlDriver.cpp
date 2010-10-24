/* This file is part of the KDE project
   Copyright (C) 2003 Adam Pigg <adam@piggz.co.uk>
   Copyright (C) 2010 Jarosław Staniek <staniek@kde.org>

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

#include <Predicate/Connection.h>
#include <Predicate/DriverManager.h>
#include <Predicate/Driver_p.h>
#include <Predicate/Utils.h>
#include "PostgresqlDriver.h"
#include "PostgresqlConnection.h"

#include <QtDebug>

using namespace Predicate;

EXPORT_PREDICATE_DRIVER(PostgresqlDriver, postgresql)

//==================================================================================
//
PostgresqlDriver::PostgresqlDriver()
        : Driver()
{
    d->features = SingleTransactions | CursorForward | CursorBackward;
//! @todo enable this when kexidb supports multiple: d->features = MultipleTransactions | CursorForward | CursorBackward;

    beh->UNSIGNED_TYPE_KEYWORD = QString();
    beh->ROW_ID_FIELD_NAME = QLatin1String("oid");
    beh->SPECIAL_AUTO_INCREMENT_DEF = false;
    beh->AUTO_INCREMENT_TYPE = QLatin1String("SERIAL");
    beh->AUTO_INCREMENT_FIELD_OPTION = QString();
    beh->AUTO_INCREMENT_PK_FIELD_OPTION = QLatin1String("PRIMARY KEY");
    beh->ALWAYS_AVAILABLE_DATABASE_NAME = QLatin1String("template1");
    beh->QUOTATION_MARKS_FOR_IDENTIFIER = '"';
    // Use SQL compliant TRUE or FALSE as described
    // at http://www.postgresql.org/docs/8.0/interactive/datatype-boolean.html
    // 1 or 0 does not work.
    beh->BOOLEAN_TRUE_LITERAL = QLatin1String("TRUE");
    beh->BOOLEAN_FALSE_LITERAL = QLatin1String("FALSE");

    initDriverSpecificKeywords(keywords);

    //predefined properties
    d->properties["client_library_version"] = "";//TODO
    d->properties["default_server_encoding"] = ""; //TODO

    d->typeNames[Field::Byte] = QLatin1String("SMALLINT");
    d->typeNames[Field::ShortInteger] = QLatin1String("SMALLINT");
    d->typeNames[Field::Integer] = QLatin1String("INTEGER");
    d->typeNames[Field::BigInteger] = QLatin1String("BIGINT");
    d->typeNames[Field::Boolean] = QLatin1String("BOOLEAN");
    d->typeNames[Field::Date] = QLatin1String("DATE");
    d->typeNames[Field::DateTime] = QLatin1String("TIMESTAMP");
    d->typeNames[Field::Time] = QLatin1String("TIME");
    d->typeNames[Field::Float] = QLatin1String("REAL");
    d->typeNames[Field::Double] = QLatin1String("DOUBLE PRECISION");
    d->typeNames[Field::Text] = QLatin1String("CHARACTER VARYING");
    d->typeNames[Field::LongText] = QLatin1String("TEXT");
    d->typeNames[Field::BLOB] = QLatin1String("BYTEA");
}

PostgresqlDriver::~PostgresqlDriver()
{
}

//Override the default implementation to allow for NUMERIC type natively
QString PostgresqlDriver::sqlTypeName(int id_t, int p) const
{
    if (id_t == Field::Null)
        return QLatin1String("NULL");
    if (id_t == Field::Float || id_t == Field::Double) {
        if (p > 0) {
            return QLatin1String("NUMERIC");
        } else {
            return d->typeNames[id_t];
        }
    } else {
        return d->typeNames[id_t];
    }
}

Predicate::Connection* PostgresqlDriver::drv_createConnection(const ConnectionData& connData)
{
    return new PostgresqlConnection(this, connData);
}

bool PostgresqlDriver::isSystemObjectName(const QString& n) const
{
    return Driver::isSystemObjectName(n);
}

bool PostgresqlDriver::drv_isSystemFieldName(const QString&) const
{
    return false;
}

bool PostgresqlDriver::isSystemDatabaseName(const QString& n) const
{
    return    0 == n.compare(QLatin1String("template1"), Qt::CaseInsensitive)
           || 0 == n.compare(QLatin1String("template0"), Qt::CaseInsensitive);
}

QString PostgresqlDriver::escapeString(const QString& str) const
{
    //Cannot use pqxx or libpq escape functions as they require a db connection
    //to escape using the char encoding of the database
    //see http://www.postgresql.org/docs/8.1/static/libpq-exec.html#LIBPQ-EXEC-ESCAPE-STRING
/*    return QString::fromLatin1("'")
    + QString::fromAscii(_internalWork->esc(std::string(str.toAscii().constData())).c_str())
           + QString::fromLatin1("'");
*/
//! @todo optimize
           return QString::fromLatin1("'") + QString(str)
           /*.replace('\\', "\\\\")*/
           .replace('\'', "\\''")
           .replace('"', "\\\"")
           + QString::fromLatin1("'");
}

QByteArray PostgresqlDriver::escapeString(const QByteArray& str) const
{
    //Cannot use pqxx or libpq escape functions as they require a db connection
    //to escape using the char encoding of the database
    //see http://www.postgresql.org/docs/8.1/static/libpq-exec.html#LIBPQ-EXEC-ESCAPE-STRING
    
    /*
    return QByteArray("'")
    + QByteArray(_internalWork->esc(str).c_str())
           + QByteArray("'");*/

//! @todo optimize
    return QByteArray("'") + QByteArray(str)
           /*.replace('\\', "\\\\")*/
           .replace('\'', "\\''")
           .replace('"', "\\\"")
           + QByteArray("'");
}

QString PostgresqlDriver::drv_escapeIdentifier(const QString& str) const
{
    return QByteArray(str.toLatin1()).replace('"', "\"\"");
}

QByteArray PostgresqlDriver::drv_escapeIdentifier(const QByteArray& str) const
{
    return QByteArray(str).replace('"', "\"\"");
}

QString PostgresqlDriver::escapeBLOB(const QByteArray& array) const
{
    return Predicate::escapeBLOB(array, Predicate::BLOBEscapeOctal);
}

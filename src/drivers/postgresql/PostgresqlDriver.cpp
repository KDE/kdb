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

#include "PostgresqlDriver.h"

#include <Predicate/Connection>
#include <Predicate/DriverManager>
#include <Predicate/Private/Driver>
#include <Predicate/Utils>

#include "PostgresqlConnection.h"

#include <libpq-fe.h>
#include <postgres.h>
#include <catalog/pg_type.h>

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
    beh->USE_TEMPORARY_DATABASE_FOR_CONNECTION_IF_NEEDED = true;

    initDriverSpecificKeywords(m_keywords);
    initPgsqlToVariantMap();

    //predefined properties
    //! @todo pgsql client_library_version
    d->properties["client_library_version"] = QString();
    //! @todo pgsql default_server_encoding
    d->properties["default_server_encoding"] = QString();

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

void PostgresqlDriver::initPgsqlToVariantMap()
{
    m_pgsqlToVariantTypes.insert(BOOLOID, QVariant::Bool);
    m_pgsqlToVariantTypes.insert(BYTEAOID, QVariant::ByteArray);
    m_pgsqlToVariantTypes.insert(CHAROID, QVariant::Int);
    m_pgsqlToVariantTypes.insert(NAMEOID, QVariant::ByteArray);
    m_pgsqlToVariantTypes.insert(INT8OID, QVariant::LongLong);
    m_pgsqlToVariantTypes.insert(INT2OID, QVariant::Int);
    //! @todo INT2VECTOROID? (array of int2, used in system tables)
    m_pgsqlToVariantTypes.insert(INT4OID, QVariant::Int);
    m_pgsqlToVariantTypes.insert(REGPROCOID, QVariant::Int);
    m_pgsqlToVariantTypes.insert(TEXTOID, QVariant::String);
    m_pgsqlToVariantTypes.insert(OIDOID, QVariant::Int);
    //! @todo TIDOID? (block, offset), physical location of tuple
    m_pgsqlToVariantTypes.insert(XIDOID, QVariant::Int);
    m_pgsqlToVariantTypes.insert(CIDOID, QVariant::Int);
    //! @todo OIDVECTOROID? (array of oids, used in system tables)
    // PG_TYPE_RELTYPE_OID
    // PG_ATTRIBUTE_RELTYPE_OID
    // PG_PROC_RELTYPE_OID
    // PG_CLASS_RELTYPE_OID
    m_pgsqlToVariantTypes.insert(XMLOID, QVariant::String);
    //! @todo POINTOID geometric point '(x, y)
    //! @todo LSEGOID geometric line segment '(pt1,pt2)
    //! @todo PATHOID geometric path '(pt1,...)'
    //! @todo BOXOID geometric box '(lower left,upper right)
    //! @todo POLYGONOID geometric polygon '(pt1,...)'
    m_pgsqlToVariantTypes.insert(FLOAT4OID, QVariant::Double);
    m_pgsqlToVariantTypes.insert(FLOAT8OID, QVariant::Double);
    m_pgsqlToVariantTypes.insert(ABSTIMEOID, QVariant::Date);
    m_pgsqlToVariantTypes.insert(RELTIMEOID, QVariant::Date);
    //! @todo TINTERVALOID (abstime,abstime), time interval
    //! @todo CIRCLEOID geometric circle '(center,radius)'
    //! @todo CASHOID monetary amounts, $d,ddd.cc
    //! @todo MACADDROID XX:XX:XX:XX:XX:XX, MAC address
    //! @todo INETOID IP address/netmask, host address, netmask optional
    //! @todo CIDROID network IP address/netmask, network address
    //! @todo INT4ARRAYOID
    //! @todo FLOAT4ARRAYOID
    //! @todo ACLITEMOID access control list
    m_pgsqlToVariantTypes.insert(CSTRINGARRAYOID, QVariant::ByteArray);
    m_pgsqlToVariantTypes.insert(BPCHAROID, QVariant::String); // char(length), blank-padded string,
                                                               // fixed storage length
    m_pgsqlToVariantTypes.insert(VARCHAROID, QVariant::String); // varchar(length), non-blank-padded string,
                                                                // variable storage length
    m_pgsqlToVariantTypes.insert(DATEOID, QVariant::Date);
    m_pgsqlToVariantTypes.insert(TIMEOID, QVariant::Time);
    m_pgsqlToVariantTypes.insert(TIMESTAMPOID, QVariant::DateTime);
    m_pgsqlToVariantTypes.insert(TIMESTAMPTZOID, QVariant::DateTime);
    //! @todo INTERVALOID @ <number> <units>, time interval
    m_pgsqlToVariantTypes.insert(TIMETZOID, QVariant::Time);
    //! @todo BITOID ok?
    m_pgsqlToVariantTypes.insert(BITOID, QVariant::ByteArray);
    //! @todo VARBITOID ok?
    m_pgsqlToVariantTypes.insert(VARBITOID, QVariant::ByteArray);
    m_pgsqlToVariantTypes.insert(NUMERICOID, QVariant::Double);
    //! @todo REFCURSOROID reference cursor (portal name)
    //! @todo REGPROCEDUREOID registered procedure (with args)
    //! @todo REGOPEROID registered operator
    //! @todo REGOPERATOROID registered operator (with args)
    //! @todo REGCLASSOID registered class
    //! @todo REGTYPEOID registered type
    //! @todo REGTYPEARRAYOID
    //! @todo TSVECTOROID text representation for text search
    //! @todo GTSVECTOROID GiST index internal text representation for text search
    //! @todo TSQUERYOID query representation for text search
    //! @todo REGCONFIGOID registered text search configuration
    //! @todo REGDICTIONARYOID registered text search dictionary
    //! @todo RECORDOID
    //! @todo CSTRINGOID
    //! @todo ANYOID
    //! @todo ANYARRAYOID
    //! @todo VOIDOID
    //! @todo TRIGGEROID
    //! @todo LANGUAGE_HANDLEROID
    //! @todo INTERNALOID
    //! @todo OPAQUEOID
    //! @todo ANYELEMENTOID
    //! @todo ANYNONARRAYOID
    //! @todo ANYENUMOID
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
           || 0 == n.compare(QLatin1String("template0"), Qt::CaseInsensitive)
           || 0 == n.compare(QLatin1String("postgres"), Qt::CaseInsensitive);
}

EscapedString PostgresqlDriver::escapeString(const QString& str) const
{
    //Cannot use libpq escape functions as they require a db connection
    //to escape using the char encoding of the database
    //see http://www.postgresql.org/docs/8.1/static/libpq-exec.html#LIBPQ-EXEC-ESCAPE-STRING
    return EscapedString("E'")
           + EscapedString(str).replace("\\", "\\\\").replace("'", "\\\'")
           + "'";
};

EscapedString PostgresqlDriver::escapeString(const QByteArray& str) const
{
    //Cannot use libpq escape functions as they require a db connection
    //to escape using the char encoding of the database
    //see http://www.postgresql.org/docs/8.1/static/libpq-exec.html#LIBPQ-EXEC-ESCAPE-STRING
    return EscapedString("'")
           + QByteArray(str).replace("\\", "\\\\").replace("'", "\\\'")
           + "'";
}

QString PostgresqlDriver::drv_escapeIdentifier(const QString& str) const
{
    return QString(str).replace(QLatin1Char('"'), QLatin1String("\"\""));
}

QByteArray PostgresqlDriver::drv_escapeIdentifier(const QByteArray& str) const
{
    return QByteArray(str).replace('"', "\"\"");
}

EscapedString PostgresqlDriver::escapeBLOB(const QByteArray& array) const
{
    return EscapedString(Predicate::escapeBLOB(array, Predicate::BLOBEscapeOctal));
}

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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4200) // "nonstandard extension used : zero-sized array in struct/union"
#endif

#ifdef __GNUC__
# pragma GCC diagnostic push
// remove c.h: warning: ISO C++ does not support ‘__int128’ for ‘int128’ [-Wpedantic]
# pragma GCC diagnostic ignored "-Wpedantic"
# include <postgres.h>
# pragma GCC diagnostic pop
#else
# include <postgres.h>
#endif
#include <libpq-fe.h>
#include <catalog/pg_type.h> // needed for BOOLOID, etc.
#include <pg_config.h> // needed for PG_VERSION_NUM

#ifdef _MSC_VER
#pragma warning( pop )
#endif

void PostgresqlDriver::initPgsqlToKDbMap()
{
    m_pgsqlToKDbTypes.insert(BOOLOID, KDbField::Boolean);
    m_pgsqlToKDbTypes.insert(BYTEAOID, KDbField::BLOB);
    m_pgsqlToKDbTypes.insert(CHAROID, KDbField::Integer);
    m_pgsqlToKDbTypes.insert(NAMEOID, KDbField::BLOB);
    m_pgsqlToKDbTypes.insert(INT8OID, KDbField::BigInteger);
    m_pgsqlToKDbTypes.insert(INT2OID, KDbField::Integer);
    //! @todo INT2VECTOROID? (array of int2, used in system tables)
    m_pgsqlToKDbTypes.insert(INT4OID, KDbField::Integer);
    m_pgsqlToKDbTypes.insert(REGPROCOID, KDbField::Integer);
    m_pgsqlToKDbTypes.insert(TEXTOID, KDbField::LongText);
    m_pgsqlToKDbTypes.insert(OIDOID, KDbField::Integer);
    //! @todo TIDOID? (block, offset), physical location of tuple
    m_pgsqlToKDbTypes.insert(XIDOID, KDbField::Integer);
    m_pgsqlToKDbTypes.insert(CIDOID, KDbField::Integer);
    //! @todo OIDVECTOROID? (array of oids, used in system tables)
    // PG_TYPE_RELTYPE_OID
    // PG_ATTRIBUTE_RELTYPE_OID
    // PG_PROC_RELTYPE_OID
    // PG_CLASS_RELTYPE_OID
    m_pgsqlToKDbTypes.insert(XMLOID, KDbField::LongText);
    //! @todo POINTOID geometric point '(x, y)
    //! @todo LSEGOID geometric line segment '(pt1,pt2)
    //! @todo PATHOID geometric path '(pt1,...)'
    //! @todo BOXOID geometric box '(lower left,upper right)
    //! @todo POLYGONOID geometric polygon '(pt1,...)'
    m_pgsqlToKDbTypes.insert(FLOAT4OID, KDbField::Double);
    m_pgsqlToKDbTypes.insert(FLOAT8OID, KDbField::Double);
#if PG_VERSION_NUM < 120000
    m_pgsqlToKDbTypes.insert(ABSTIMEOID, KDbField::Date);
    m_pgsqlToKDbTypes.insert(RELTIMEOID, KDbField::Date);
#endif
    //! @todo TINTERVALOID (abstime,abstime), time interval
    //! @todo CIRCLEOID geometric circle '(center,radius)'
    //! @todo CASHOID monetary amounts, $d,ddd.cc
    //! @todo MACADDROID XX:XX:XX:XX:XX:XX, MAC address
    //! @todo INETOID IP address/netmask, host address, netmask optional
    //! @todo CIDROID network IP address/netmask, network address
    //! @todo INT4ARRAYOID
    //! @todo FLOAT4ARRAYOID
    //! @todo ACLITEMOID access control list
    m_pgsqlToKDbTypes.insert(CSTRINGARRAYOID, KDbField::BLOB);
    m_pgsqlToKDbTypes.insert(BPCHAROID, KDbField::LongText);  // char(length), blank-padded string,
                                                              // fixed storage length
    m_pgsqlToKDbTypes.insert(VARCHAROID, KDbField::LongText); // varchar(length), non-blank-padded string,
                                                              // variable storage length
    m_pgsqlToKDbTypes.insert(DATEOID, KDbField::Date);
    m_pgsqlToKDbTypes.insert(TIMEOID, KDbField::Time);
    m_pgsqlToKDbTypes.insert(TIMESTAMPOID, KDbField::DateTime);
    m_pgsqlToKDbTypes.insert(TIMESTAMPTZOID, KDbField::DateTime);
    //! @todo INTERVALOID @ <number> <units>, time interval
    m_pgsqlToKDbTypes.insert(TIMETZOID, KDbField::Time);
    //! @todo BITOID ok?
    m_pgsqlToKDbTypes.insert(BITOID, KDbField::BLOB);
    //! @todo VARBITOID ok?
    m_pgsqlToKDbTypes.insert(VARBITOID, KDbField::BLOB);
    m_pgsqlToKDbTypes.insert(NUMERICOID, KDbField::Double);
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

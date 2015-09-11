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

#include <postgres.h>
#include <libpq-fe.h>
#include <catalog/pg_type.h> // needed for BOOLOID, etc.

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

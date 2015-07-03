/* This file is part of the KDE project
   Copyright (C) 2008 Sharan Rao <sharanrao@gmail.com>

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

#include "XbaseDriver.h"

#include "KDbConnection.h"
#include "KDbDriverManager.h"
#include "KDbDriver_p.h"
#include "KDb.h"


#include "XbaseConnection.h"

KDB_DRIVER_PLUGIN_FACTORY(xBaseDriver, "kdb_xbasedriver.json")

class KDbxBaseDriverPrivate {

public:
  xBaseDriverPrivate()
    : internalDriver(0)
  {
  }

  KDbDriver* internalDriver;

};

xBaseDriver::xBaseDriver()
  : KDbDriver()
  ,dp( new xBaseDriverPrivate() )
{
  KDbDriverManager manager;
  dp->internalDriver = manager.driver(KDb::defaultFileBasedDriverId());

//  d->isFileDriver = true ;
  d->isDBOpenedAfterCreate = true;
  d->features = SingleTransactions | CursorForward;

  // Everything below is for the SQLite (default file based) driver

  //special method for autoincrement definition
  beh->SPECIAL_AUTO_INCREMENT_DEF = true;
  beh->AUTO_INCREMENT_FIELD_OPTION = ""; //not available
  beh->AUTO_INCREMENT_TYPE = "INTEGER";
  beh->AUTO_INCREMENT_PK_FIELD_OPTION = "PRIMARY KEY";
  beh->AUTO_INCREMENT_REQUIRES_PK = true;
  beh->ROW_ID_FIELD_NAME = "OID";

  beh->QUOTATION_MARKS_FOR_IDENTIFIER='"';
  beh->SELECT_1_SUBQUERY_SUPPORTED = true;

  // As we provide a wrapper over SQLite, this aspect will be hidden by SQLite to us.
  beh->_1ST_ROW_READ_AHEAD_REQUIRED_TO_KNOW_IF_THE_RESULT_IS_EMPTY=false;

  initDriverSpecificKeywords(keywords);

  // Ditto like SQLite , as it won't matter
  d->typeNames[KDbField::Byte]="Byte";
  d->typeNames[KDbField::ShortInteger]="ShortInteger";
  d->typeNames[KDbField::Integer]="Integer";
  d->typeNames[KDbField::BigInteger]="BigInteger";
  d->typeNames[KDbField::Boolean]="Boolean";
  d->typeNames[KDbField::Date]="Date";
  d->typeNames[KDbField::DateTime]="DateTime";
  d->typeNames[KDbField::Time]="Time";
  d->typeNames[KDbField::Float]="Float";
  d->typeNames[KDbField::Double]="Double";
  d->typeNames[KDbField::Text]="Text";
  d->typeNames[KDbField::LongText]="CLOB";
  d->typeNames[KDbField::BLOB]="BLOB";
}

xBaseDriver::~xBaseDriver()
{
  delete dp;
}

KDbConnection* xBaseDriver::drv_createConnection(const ConnectionData& connData)
{
    if ( !dp->internalDriver ) {
        return 0;
    }
    return new xBaseConnection( this, dp->internalDriver, connData );
}

bool xBaseDriver::isSystemObjectName( const QString& n ) const
{
  if ( !dp->internalDriver ) {
    return false;
  }
  return KDbDriver::isSystemObjectName(n) || dp->internalDriver->isSystemObjectName(n);
}

bool xBaseDriver::drv_isSystemFieldName( const QString& n ) const
{
  if ( !dp->internalDriver ) {
    return false;
  }
  return dp->internalDriver->isSystemFieldName(n);
}

KDbEscapedString xBaseDriver::escapeString(const QString& str) const
{
  if ( !dp->internalDriver ) {
    return KDbEscapedString("'") + str + '\'';
  }
  return dp->internalDriver->escapeString(str);
}

KDbEscapedString xBaseDriver::escapeString(const QByteArray& str) const
{
  if ( !dp->internalDriver ) {
    return KDbEscapedString("'") + str + '\'';
  }
  return dp->internalDriver->escapeString(str);
}

KDbEscapedString xBaseDriver::escapeBLOB(const QByteArray& array) const
{
  if ( !dp->internalDriver ) {
    return array;
  }
  return dp->internalDriver->escapeBLOB(array);
}

QByteArray xBaseDriver::drv_escapeIdentifier( const QString& str) const
{
  if ( !dp->internalDriver ) {
    return str;
  }
  return dp->internalDriver->escapeIdentifier(str);
}

QByteArray xBaseDriver::drv_escapeIdentifier( const QByteArray& str) const
{
  if ( !dp->internalDriver ) {
    return str;
  }
  return dp->internalDriver->escapeIdentifier(str);
}

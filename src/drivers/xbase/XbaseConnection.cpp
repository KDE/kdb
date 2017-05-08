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
*  Boston, MA 02110-1301, USA.
*/


#include "XbaseDriver.h"
#include "XbaseCursor.h"
#include "XbaseConnection.h"
#include "XbaseConnection_p.h"
#include "KDbError.h"

xBaseConnection::xBaseConnection(KDbDriver *driver, KDbDriver* internalDriver, const KDbConnectionData& connData)
  : KDbConnection(driver, connData)
  , d(new xBaseConnectionInternal(this, internalDriver))
{
}

xBaseConnection::~xBaseConnection() {
  destroy();
}

bool xBaseConnection::drv_connect(KDbServerVersionInfo* version)
{
  Q_UNUSED(version);
  const bool ok = d->db_connect(*data());
  if (!ok)
    return false;

  //! @todo xBase version here
  //version.string = mysql_get_host_info(d->mysql);

  return true;
}

bool xBaseConnection::drv_disconnect() {
  return d->db_disconnect(*data());
}

KDbCursor* xBaseConnection::prepareQuery(const KDbEscapedString& sql, int cursor_options)
{
  if ( !d->internalConn ) {
    return 0;
  }
  KDbCursor* internalCursor = d->internalConn->prepareQuery(sql,cursor_options);
  return new xBaseCursor( this, internalCursor, sql, cursor_options );
}

KDbCursor* xBaseConnection::prepareQuery(KDbQuerySchema* query, int cursor_options) {
  if ( !d->internalConn ) {
    return 0;
  }
  KDbCursor* internalCursor = d->internalConn->prepareQuery(query, cursor_options);
  return new xBaseCursor( this, internalCursor, query, cursor_options );
}

bool xBaseConnection::drv_getDatabasesList(QStringList* list)
{
  //! @todo Check whether this is the right thing to do
  *list += QStringList( d->dbMap.keys() );
//        list<<d->internalConn->databaseNames();
  return true;
}

bool xBaseConnection::drv_createDatabase( const QString &dbName) {
  //! @todo Check whether this function has any use.
  //xbaseDebug() << dbName;
// return d->internalConn->createDatabase(d->dbMap[dbName]);
  return true;
}

bool xBaseConnection::drv_useDatabase(const QString &dbName, bool *cancelled, KDbMessageHandler* msgHandler)
{
  Q_UNUSED(cancelled);
  Q_UNUSED(msgHandler);
//! @todo is here escaping needed?
  return d->useDatabase(dbName);
}

bool xBaseConnection::drv_closeDatabase() {
  if (!d->internalConn || !d->internalConn->closeDatabase() ) {
    return false;
  }
  return true;
}

bool xBaseConnection::drv_dropDatabase(const QString &dbName) {
    Q_UNUSED(dbName);
//! @todo is here escaping needed
  // Delete the directory ?
  return true;
}

bool xBaseConnection::drv_executeSql( const KDbEscapedString& sql ) {
  return d->executeSql(sql);
}

quint64 xBaseConnection::drv_lastInsertRecordId()
{
  //! @todo
  quint64 rowID = -1;
  if (d->internalConn)
    d->internalConn->lastInsertedAutoIncValue(QString(), QString(), &rowID );

  return rowID;
}

int xBaseConnection::serverResult()
{
  return d->res;
}

QString xBaseConnection::serverResultName() const
{
  if (!d->internalConn) {
    return QString();
  }
  return d->internalConn->serverResultName();
}

/*void xBaseConnection::drv_clearServerResult()
{
  if (!d || !d->internalConn)
    return;
  d->internalConn->clearError();
  d->res = 0;
}*/

QString xBaseConnection::serverErrorMsg()
{
  return d->errmsg;
}

bool xBaseConnection::drv_containsTable( const QString &tableName )
{
  return resultExists(KDbEscapedString("SHOW TABLES LIKE %1")
                      .arg(escapeString(tableName)));
}

bool xBaseConnection::drv_getTablesList(QStringList* list)
{
  if ( !d->internalConn ) {
    return false;
  }
  *list += d->internalConn->tableNames();
  return true;
}

KDbPreparedStatementInterface* xBaseConnection::prepareStatementInternal()
{
    if ( !d->internalConn )
        return 0;
//! @todo   return new XBasePreparedStatement(*d);
        return 0;
}

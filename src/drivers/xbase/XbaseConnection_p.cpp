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


#include <QTemporaryFile>

#include "XbaseConnection_p.h"
#include "XbaseExport.h"

#include "KDbDriverManager.h"
#include "KDb.h"
#include "KDbConnectionData.h"
#include <migration/keximigrate.h>
#include <migration/migratemanager.h>

#include <core/kexiprojectdata.h>

xBaseConnectionInternal::xBaseConnectionInternal(KDbConnection* connection, KDbDriver* internalDriver )
  : ConnectionInternal(connection),
  internalDriver(internalDriver)
{
}

xBaseConnectionInternal::~xBaseConnectionInternal()
{
// deletion of internalDriver and internalConn will be handled by KDbDriver* class ( creator )
}

void xBaseConnectionInternal::storeResult()
{
  if (internalConn) {
    res = internalConn->serverResult();
    errmsg = internalConn->serverErrorMsg();
  }
}

bool xBaseConnectionInternal::db_connect(const KDbConnectionData& data)
{
  // we have to migrate the xbase source database into a .kexi file
  // xbase source database directory will be in connectiondata
  // we can choose a QTemporaryFile for the destination .kexi file

  KexiMigration::MigrateManager xBase2KexiMigrateManager;

  // create a temporary .kexi file
  QTemporaryFile temporaryKexiFile(QDir::tempPath() + QLatin1String("/kdb_xbase_XXXXXX.kexi"));
  temporaryKexiFile.setAutoRemove( false );

  if ( !temporaryKexiFile.open() ) {
    xbaseWarning() << "Couldn't create .kexi file for exporting from xBase to .kexi";
    return false;
  }

        tempDatabase = temporaryKexiFile.fileName();

  KDbConnectionData* kexiConnectionData = 0;
  kexiConnectionData = new KDbConnectionData();

  // set destination file name here.
  kexiConnectionData->driverId = KDb::defaultFileBasedDriverId();
  kexiConnectionData->setFileName( tempDatabase );
  //xbaseDebug() << "Current file name: " << tempDatabase;

  QString sourceDriverId = "xbase";
  // get the source migration driver
  KexiMigration::KexiMigrate* sourceDriver = 0;
  sourceDriver = xBase2KexiMigrateManager.driver( sourceDriverId );
  if(!sourceDriver || xBase2KexiMigrateManager.error()) {
    xbaseWarning() << "Import migrate driver error...";
    return false;
  }

  KexiMigration::Data* md = new KexiMigration::Data();
  md->keepData = true;
  md->destination = new KexiProjectData(*kexiConnectionData, tempDatabase);

  // Setup XBase connection data from input connection data passed
  //! @todo Check sanity of this
  md->source = new KDbConnectionData(data);
  md->sourceName = "";

  sourceDriver->setData(md);
  if ( !sourceDriver->performImport() ) {
    xbaseWarning() << "Import failed";
    return false;
  }

  // finished transferring xBase database into .kexi file

  // Get a driver to the destination database

  if ( internalDriver )
    internalConn = internalDriver->createConnection(*kexiConnectionData);
  else
    return false;

  if (!internalConn || internalDriver->error()) {
    internalDriver->debugError();
    return false;
  }
  if (!internalConn->connect()) {
    internalConn->debugError();
    storeResult();
    return false;
  }

        if (!internalConn->useDatabase(tempDatabase)) {
                internalConn->debugError();
                storeResult();
                return false;
        }

  // store mapping from xbase directory to .kexi file name for future use
  // Note: When a directory is specified ( as has to be done for xBase ), fileName()
  // will give directory name with an additional forward slash. dbPath() won't do so.
  // Need some more maintainable solution.

  dbMap[data.fileName()] = tempDatabase;

  return true;
}

bool xBaseConnectionInternal::db_disconnect(const KDbConnectionData& data)
{
  //! Export back to xBase
  xBaseExport export2xBase;
  KexiMigration::Data* migrateData = new KexiMigration::Data();
  migrateData->source = internalConn->data();
  migrateData->sourceName = tempDatabase;
  migrateData->destination = new KexiProjectData( data );
  migrateData->keepData = true;

  export2xBase.setData( migrateData );

  if (!export2xBase.performExport()) {
    return false;
  }

  return internalConn->disconnect();
}

bool xBaseConnectionInternal::useDatabase(const QString &dbName)
{
  if ( !internalConn ) {
    return false;
  }
  return internalConn->useDatabase(dbMap[dbName]);
}

bool xBaseConnectionInternal::executeSql(const KDbEscapedString& sql)
{
//xbaseDebug() << statement;
  if ( !internalConn ) {
    return false;
  }
  return internalConn->executeSql(sql);
}

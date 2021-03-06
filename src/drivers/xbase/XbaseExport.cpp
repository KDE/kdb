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

#include "XbaseExport.h"

#include <QHash>
#include <QDir>


#include "KDbField.h"
#include "KDbRecordData.h"
#include "KDbCursor.h"
#include "KDbDriverManager.h"
#include <core/kexi.h>
#include <migration/keximigratedata.h>

#include <cstring>

#include "xbase.h"

class KDbxBaseExportPrivate {
  public:
    xBaseExportPrivate() {
    }

    //! Converts KDbField types to xbase types
    char type(KDbField::Type fieldType);

    //! Appends record to xbase table
    bool appendRecord(const QString& sourceTableName , KDbRecordData* recordData);

    //! Returns max fieldlengths for xBase table
    int fieldLength(KDbField* f );

    //! converts QVariant data to a format understood by xBase
    QByteArray fieldData(QVariant data, char type);

    //! Creates xBase indexes for the table
    bool createIndexes(const QString& sourceTableName, KDbTableSchema* tableSchema);

    xbXBase xbase;
    QHash<QString, QString> tableNamePathMap;
};

char xBaseExportPrivate::type(KDbField::Type fieldType)
{
  char xBaseType = '\0';

  switch( fieldType ) {
    case KDbField::Text:
    case KDbField::LongText:
      xBaseType = XB_CHAR_FLD;
      break;

    case KDbField::Boolean:
      xBaseType = XB_LOGICAL_FLD;
      break;

    case KDbField::Float:
    case KDbField::Double:
      xBaseType = XB_FLOAT_FLD;

    case KDbField::ShortInteger:
    case KDbField::Integer:
    case KDbField::BigInteger:
      xBaseType = XB_NUMERIC_FLD;
      break;

    case KDbField::DateTime:
    case KDbField::Date:
    case KDbField::Time:
      xBaseType = XB_DATE_FLD;
      break;

    case KDbField::BLOB:
      xBaseType = XB_MEMO_FLD;
      break;

    default:
      xBaseType = '\0';
  }

  return xBaseType;
}

bool xBaseExportPrivate::appendRecord( const QString& sourceTableName , KDbRecordData* recordData ) {

//     xbaseDebug() << recordData->debugString();
  QString pathName = tableNamePathMap.value( sourceTableName );
  QByteArray pathNameBa = pathName.toLatin1();
  xbDbf* table = xbase.GetDbfPtr( pathNameBa.constData() );

  int returnCode;
  table->BlankRecord();
  for (int i=0;i < recordData->size();++i) {
  char fieldType = table->GetFieldType(i);
  QByteArray stringData = fieldData(recordData->value(i), fieldType);

  if (fieldType == XB_MEMO_FLD) {
  #ifdef XB_MEMO_FIELDS
    // we use size()+1 as size to accommodate `\0`
    table->UpdateMemoData(i, stringData.size()+1, stringData.constData(), F_SETLKW );
  #else
    xbaseDebug() << "XB_MEMO_FIELDS support disabled during compilation of XBase libraries";
  #endif
  } else {
    if ((returnCode = table->PutField( i, stringData.constData())) != XB_NO_ERROR ) {
      switch(returnCode) {
      case XB_INVALID_FIELDNO:
        xbaseWarning() << "Invalid field number" << i;
        return false;
      case XB_INVALID_DATA:
        xbaseWarning() << "Invalid data" << stringData;
        return false;
      default:
        xbaseWarning() << "Error number" << returnCode << "has occurred";
        return false;
    }
    }
  }
  }

  if((returnCode = table->AppendRecord()) != XB_NO_ERROR) {
    xbaseWarning() << "xBase Error" << returnCode << "appending data record.";
    return false;
  }

//     // for debugging purposes only
//     for ( int i=0; i< recordData->size(); ++i ) {
//         xbaseDebug() << table->GetField(i);
//     }

  return true;
}

int xBaseExportPrivate::fieldLength(KDbField* f)
{
  const Field::Type t = f->type(); // cache: evaluating type of expressions can be expensive
  if (KDbField::isTextType(t)) {
    return f->maxLength();
  }
  // return the max possible (string)length of the types
  // see https://linux.techass.com/projects/xdb/xbasedocs/xbase_c3.html
  switch(type(t)) {
    case XB_CHAR_FLD:
      return 254;
    case XB_LOGICAL_FLD:
      return 1;
    case XB_FLOAT_FLD:
    case XB_NUMERIC_FLD:
      return 17;
    case XB_DATE_FLD:
      return 8;
    case XB_MEMO_FLD:
      return 10;
    default:
      return 0;
  }
}

QByteArray xBaseExportPrivate::fieldData(QVariant data, char type) {

  switch(type) {
    case XB_CHAR_FLD:
    case XB_FLOAT_FLD:
    case XB_NUMERIC_FLD:
      return data.toString().toUtf8();

    case XB_LOGICAL_FLD:
      if (data.toBool()) {
        return QString( "t" ).toLatin1();
      } else
        return QString( "f" ).toLatin1();

    case XB_DATE_FLD:
      return data.toDate().toString("yyyyMMdd").toLatin1();

    case XB_MEMO_FLD:
      return data.toByteArray();
    default:
      return QByteArray();
  }
}

bool xBaseExportPrivate::createIndexes(const QString& sourceTableName, KDbTableSchema* tableSchema) {

  QString pathName = tableNamePathMap.value( sourceTableName );
  QByteArray pathNameBa = pathName.toLatin1();
  xbDbf* table = xbase.GetDbfPtr( pathNameBa.constData() );
  int fieldCount = tableSchema->fieldCount();

  QString dirName = QFileInfo( pathName ).path();

  for (int i=0; i< fieldCount ; ++i) {
    KDbField* f = tableSchema->field(i);

    int returnCode;
    QString fieldName = f->name();
    QString indexName = dirName + QDir::separator() + sourceTableName + '_' + fieldName + ".ndx";
    QByteArray indexNameBa = indexName.toLatin1();
    QByteArray fieldNameBa = fieldName.toLatin1();

    xbNdx index(table);
    if (f->isUniqueKey() || f->isPrimaryKey()) {

      if ((returnCode = index.CreateIndex(indexNameBa.constData(), fieldNameBa.constData(), XB_UNIQUE, XB_OVERLAY)) != XB_NO_ERROR ) {
        xbaseWarning() << "Couldn't create unique index for fieldName"
                       << fieldName << "on table" << sourceTableName << "Error Code" << returnCode;
        return false;
      }
      index.CloseIndex();

    } else if ( f->isIndexed() ) {

      if ((returnCode = index.CreateIndex(indexNameBa.constData(), fieldNameBa.constData(), XB_NOT_UNIQUE, XB_OVERLAY)) != XB_NO_ERROR ) {
        xbaseWarning() << "Couldn't create index for fieldName" << fieldName << "on table"
                       << sourceTableName << "Error Code" << returnCode;
        return false;
      }
      index.CloseIndex();

    }
  }
  return true;
}


xBaseExport::xBaseExport()
: m_migrateData( 0 ),
d(new xBaseExportPrivate)
{
}

void xBaseExport::setData(KexiMigration::Data* migrateData) {
  m_migrateData = migrateData;
}

bool xBaseExport::performExport(Kexi::ObjectStatus* result) {

  if (result)
    result->clearStatus();


  KDbDriverManager drvManager;

  if (!m_migrateData) {
    xbaseWarning() << "Migration Data not set yet";
    result->setStatus(&drvManager, tr("Data not set for migration."));
    return false;
  }

  KDbDriver *sourceDriver = drvManager.driver(m_migrateData->source->driverId);
  if (!sourceDriver) {
    result->setStatus(&drvManager,
      tr("Could not export back to destination database."));
    return false;
  }

  // connect to destination database
  if (!dest_connect()) {
    xbaseWarning() << "Couldn't connect to destination database";
    if (result)
      result->setStatus(tr("Could not connect to data source \"%1\".",
        m_migrateData->destination->connectionData()->serverInfoString()), "");
    return false;
  }

  KDbConnection* sourceConn = sourceDriver->createConnection(*(m_migrateData->source));

  if (!sourceConn || sourceDriver->error()) {
    xbaseWarning() << "Export failed";
    return false;
  }
  if (!sourceConn->connect()) {
    xbaseWarning() << "Export failed.Could not connect";
    return false;
  }

  if (!sourceConn->useDatabase(m_migrateData->sourceName)) {
    xbaseWarning() << "Couldn't use database "<<m_migrateData->sourceName;
    return false;
  }

  QStringList tables = sourceConn->tableNames();

  // Check if there are any tables
  if (tables.isEmpty()) {
    xbaseDebug() << "There were no tables to export";
    if (result)
      result->setStatus(
        tr("No tables to export found in data source \"%1\".",
          m_migrateData->source->serverInfoString()), "");
    return false;
  }

  tables.sort();

  // -- read table schemas and create them in memory (only for non-KDb-compat tables)
  foreach (const QString& tableCaption, tables) {
    if (dest_isSystemObjectName( tableCaption )) {
      return false;
    }

    KDbTableSchema *tableSchema = sourceConn->tableSchema( tableCaption );

    if (!dest_createTable(tableCaption, tableSchema)) {
      if (result)
        result->setStatus(tr("Could not create table in destination \"%1\". Error reading table \"%2\".",    m_migrateData->destination->connectionData()->serverInfoString(), tableCaption), "");
      return false;
    }

    if (m_migrateData->keepData) {
      if (!dest_copyTable(tableCaption, sourceConn, tableSchema)) {
        xbaseWarning() << "Failed to copy table " << tableCaption;
        if (result)
          result->setStatus(sourceConn,
              tr("Could not copy table \"%1\" to destination database.", tableCaption));
      }
    }

  }

  if (dest_disconnect()) {
    bool ok = false;
    if (sourceConn)
      ok = sourceConn->disconnect();
    return ok;
  }

  // Finally: Error.handling
  if (result && result->error())
    result->setStatus(sourceConn,
      tr("Could not export data to \"%1\".",
        m_migrateData->source->serverInfoString()));
  dest_disconnect();
  if (sourceConn) {
    sourceConn->disconnect();
  }
  return false;
}

bool xBaseExport::dest_connect() {
  return true;
}

bool xBaseExport::dest_disconnect() {
  QList<QString> pathNameList = d->tableNamePathMap.values();
  foreach(const QString& pathName, pathNameList) {
    QByteArray ba = pathName.toLatin1();
    xbDbf* tablePtr = d->xbase.GetDbfPtr(ba.constData());
    tablePtr->CloseDatabase();
    // delete tablePtr ?
  }
  return true;
}

bool xBaseExport::dest_createTable(const QString& originalName, KDbTableSchema* tableSchema) {
  // Algorithm
  // 1. For each fields in the table schema.
  // 2.   Create a xbSchema entry and add it to xbSchema array.
  // 3. End for
  // 4. Create table in overlay mode ( overwrite )

  int fieldCount = tableSchema->fieldCount();
  const int arrayLength = fieldCount + 1; // and extra space for the `null`
  xbSchema xBaseTableSchema[arrayLength];// = new xbSchema[fieldCount+1][4];

  int i = 0;
  for (i = 0; i < fieldCount ; ++i) {
    KDbField* f = tableSchema->field(i);

    QByteArray ba = f->name().toLatin1();
    //! @todo Fieldname can only be 11 characters
    strcpy(xBaseTableSchema[i].FieldName, ba.data());
    xBaseTableSchema[i].Type = d->type(f->type());
    xBaseTableSchema[i].FieldLen = d->fieldLength( f ); //!< @todo Check semantics
    xBaseTableSchema[i].NoOfDecs = ( xBaseTableSchema[i].Type != XB_CHAR_FLD )? f->scale() : 0 ;

  }

  // last member should be all 0
  strcpy( xBaseTableSchema[i].FieldName , "" );
  xBaseTableSchema[i].Type = 0;
  xBaseTableSchema[i].FieldLen = 0;
  xBaseTableSchema[i].NoOfDecs = 0;

  const KDbConnectionData* connData = m_migrateData->destination->connectionData();
  QString dirName = connData->fileName(); // this includes the forward slash after the dir name

  QString pathName = dirName + originalName + ".dbf";
  d->tableNamePathMap[originalName] = pathName;

  QByteArray pathNameBa = pathName.toLatin1();

  xbDbf* xBaseTable = new xbDbf( &d->xbase );
  xBaseTable->SetVersion( 4 ); // create dbase IV style files
  xbShort returnCode;
  if (( returnCode = xBaseTable->CreateDatabase( pathNameBa.constData() , xBaseTableSchema, XB_OVERLAY ))  != XB_NO_ERROR ) {
    xbaseWarning() << "Error creating table" << originalName << "Error Code" << returnCode;
    return false;
  }

  if (!d->createIndexes(originalName, tableSchema)) {
    return false;
  }

  return true;
}

bool xBaseExport::dest_copyTable(const QString& srcTableName, KDbConnection *srcConn,
        KDbTableSchema* /*srcTable*/) {
  // Algorithm
  // 1. pick each row
  // 2. Insert it into the xBase table

  // using the tableSchema as argument automatically appends rowid
  // info to the recordData which we don't want. Hence we use SQL query
  KDbCursor* cursor = srcConn->executeQuery(KDbEscapedString( "SELECT * FROM %1" ).arg(srcTableName));

  if (!cursor)
    return false;

  if (!cursor->moveFirst() && cursor->error())
    return false;

  while (!cursor->eof()) {
    KDbRecordData *record = cursor->storeCurrentRecord();
    if (!record) {
      return false;
    }
    if (!d->appendRecord(srcTableName, record)) {
      xbaseWarning() << "Couldn't append record";
      return false;
    }

    if (!cursor->moveNext() && cursor->error()) {
      return false;
    }
  }
  return true;
}

bool xBaseExport::dest_isSystemObjectName( const QString& /* objectName */ ) {
  return false;
}

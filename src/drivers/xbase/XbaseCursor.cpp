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

#include "XbaseCursor.h"
#include "XbaseConnection.h"

#include <Predicate/Error>
#include <Predicate/Utils>

#include <QtDebug>
#include <limits.h>


using namespace Predicate;

class Predicate::xBaseCursorData {
  public:
    explicit xBaseCursorData(Predicate::Cursor* cursor = 0)
      : internalCursor(cursor)
    {
    }

    Predicate::Cursor* internalCursor;

};

xBaseCursor::xBaseCursor(Predicate::Connection* conn, Predicate::Cursor* internalCursor, const EscapedString& statement, uint cursor_options)
  : Cursor(conn,statement,cursor_options)
  , d( new xBaseCursorData(internalCursor) )
{
  init();
}

xBaseCursor::xBaseCursor(Connection* conn, Predicate::Cursor* internalCursor, QuerySchema* query, uint options)
  : Cursor( conn, query, options )
  , d( new xBaseCursorData(internalCursor) )
{
  init();
}

xBaseCursor::~xBaseCursor() {
  close();
}

void xBaseCursor::init() {
  
  if (d->internalCursor) {
    m_options |= d->internalCursor->options();
  }
  // SQLite does buffering. So all calls to moving the cursor to SQLite will read from the buffer
  // ( if the rows are already buffered ) inside. Any point in double buffering ?
  setBuffered(false);
}

bool xBaseCursor::drv_open(const EscapedString& sql)
{
//	PreDrvDbg << m_sql;
  if (!d->internalCursor) {
    return false;
  }
  return d->internalCursor->open();
}

bool xBaseCursor::drv_close() {
  if (!d->internalCursor) {
    return false;
  }
  m_opened = false;
  Connection* internalConn = d->internalCursor->connection();
  internalConn->deleteCursor( d->internalCursor );
  return true;
}

void xBaseCursor::drv_getNextRecord() {
  if (!d->internalCursor) {
    m_fetchResult = FetchError;
    return;
  }

  if ( !d->internalCursor->moveNext() ) {
    if ( d->internalCursor->eof() )
      m_fetchResult = FetchEnd;
    else
      m_fetchResult = FetchError;
  } else {
    m_fetchResult = FetchOK;
    m_fieldCount = d->internalCursor->fieldCount();
    m_fieldsToStoreInRecord = m_fieldCount;
  }
}

QVariant xBaseCursor::value(uint pos) {
  if (!d->internalCursor) {
    // Construct an invalid QVariant
    return QVariant();
  }
  return d->internalCursor->value(pos);
}


bool xBaseCursor::drv_storeCurrentRecord(RecordData* data) const
{
  if (!d->internalCursor) {
    return false;
  }

  RecordData* rData = d->internalCursor->storeCurrentRecord();
  if (!rData) {
    return false;
  }
  *data = *rData;
  return true;
}

void xBaseCursor::drv_appendCurrentRecordToBuffer() {
}


void xBaseCursor::drv_bufferMovePointerNext() {
}

void xBaseCursor::drv_bufferMovePointerPrev() {
}


void xBaseCursor::drv_bufferMovePointerTo(qint64 to) {
  Q_UNUSED(to);
}

const char** xBaseCursor::recordData() const {
  if (!d->internalCursor) {
    return 0;
  }
  return d->internalCursor->recordData();
}

int xBaseCursor::serverResult()
{
  if (!d->internalCursor) {
    // Any better value to return ?
    return -1;
  }
  return d->internalCursor->serverResult();
}

QString xBaseCursor::serverResultName() const
{
  if (!d->internalCursor) {
    return QString();
  }
  return d->internalCursor->serverResultName();
}

/*void xBaseCursor::drv_clearServerResult()
{
  //! TODO
}*/

QString xBaseCursor::serverErrorMsg()
{
  if (!d->internalCursor) {
    return QString();
  }
  return d->internalCursor->serverErrorMsg();
}

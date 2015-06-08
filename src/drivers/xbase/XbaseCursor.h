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

#ifndef _XBASECURSOR_H_
#define _XBASECURSOR_H_

#include "KDbCursor.h"
#include "KDbConnection.h"

class xBaseCursorData;

class xBaseCursor: public KDbCursor {
  public:
    xBaseCursor(KDbConnection* conn, KDbCursor* internalCursor, const QString& sql = QString(),
      uint cursor_options = NoOptions );
    xBaseCursor(KDbConnection* conn, KDbCursor* internalCursor, KDbQuerySchema* query, uint options = NoOptions);
    virtual ~xBaseCursor();

    virtual bool drv_open(const KDbEscapedString& sql);
    virtual bool drv_close();
    virtual void drv_getNextRecord();
    virtual QVariant value(uint);

//    virtual void drv_clearServerResult();
    virtual void drv_appendCurrentRecordToBuffer();
    virtual void drv_bufferMovePointerNext();
    virtual void drv_bufferMovePointerPrev();
    virtual void drv_bufferMovePointerTo(qint64 to);
    virtual const char** recordData() const;
    virtual bool drv_storeCurrentRecord(KDbRecordData* data) const;

    //! Implemented for KDbResultable
    virtual QString serverResultName() const;

  protected:
    xBaseCursorData *d;

  private:
    void init();


};

#endif

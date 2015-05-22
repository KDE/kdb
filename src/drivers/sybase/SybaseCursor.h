/* This file is part of the KDE project
   Copyright (C) 2007 Sharan Rao <sharanrao@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this program; see the file COPYING. If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _SYBASECURSOR_H_
#define _SYBASECURSOR_H_

#include "KDbCursor.h"
#include "KDbConnection.h"

class SybaseCursorData;

class SybaseCursor: public KDbCursor
{
public:
    SybaseCursor(KDbConnection* conn, const KDbEscapedString& sql,
                 uint cursor_options = NoOptions);
    SybaseCursor(KDbConnection* conn, KDbQuerySchema* query, uint options = NoOptions);
    virtual ~SybaseCursor();
    virtual bool drv_open(const KDbEscapedString& sql);
    virtual bool drv_close();
//   virtual bool drv_moveFirst();
    virtual void drv_getNextRecord();
    //virtual bool drv_getPrevRecord();
    virtual QVariant value(uint);

//    virtual void drv_clearServerResult();
    virtual void drv_appendCurrentRecordToBuffer();
    virtual void drv_bufferMovePointerNext();
    virtual void drv_bufferMovePointerPrev();
    virtual void drv_bufferMovePointerTo(qint64 to);
    virtual const char** recordData() const;
    virtual bool drv_storeCurrentRecord(KDbRecordData* data) const;
//   virtual bool save(KDbRecordData& data, RowEditBuffer& buf);

    //! Implemented for KDbResultable
    virtual QString serverResultName() const;

protected:
    QVariant pValue(uint pos) const;

    SybaseCursorData *d;
};

#endif

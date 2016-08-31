/* This file is part of the KDE project
   Copyright (C) 2003-2010 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_SQLITECURSOR_H
#define KDB_SQLITECURSOR_H

#include <QString>

#include "KDbCursor.h"

class SqliteCursorData;
class SqliteConnection;

/*!

*/
class SqliteCursor : public KDbCursor
{
public:
    virtual ~SqliteCursor();
    virtual QVariant value(int i);

    /*! [PROTOTYPE] @return internal buffer data. */
//! @todo virtual const char *** bufferData()
    /*! [PROTOTYPE] @return current record data or NULL if there is no current records. */
    virtual const char ** recordData() const;

    virtual bool drv_storeCurrentRecord(KDbRecordData* data) const;

    //! Implemented for KDbResultable
    virtual QString serverResultName() const;

protected:
    /*! KDbCursor will operate on @a conn, raw @a sql statement will be used to execute query. */
    SqliteCursor(SqliteConnection* conn, const KDbEscapedString& sql, int options = NoOptions);

    /*! KDbCursor will operate on @a conn, @a query schema will be used to execute query. */
    SqliteCursor(SqliteConnection* conn, KDbQuerySchema* query,
                 int options = NoOptions);

    virtual bool drv_open(const KDbEscapedString& sql);

    virtual bool drv_close();
    virtual void drv_getNextRecord();

    virtual void drv_appendCurrentRecordToBuffer();
    virtual void drv_bufferMovePointerNext();
    virtual void drv_bufferMovePointerPrev();
    virtual void drv_bufferMovePointerTo(qint64 at);

//! @todo virtual void drv_storeCurrentRecord();

    //PROTOTYPE:
    /*! Method called when cursor's buffer need to be cleared
      (only for buffered cursor type), eg. in close(). */
    virtual void drv_clearBuffer();

    void storeResult();

    SqliteCursorData * const d;

    friend class SqliteConnection;
};

#endif

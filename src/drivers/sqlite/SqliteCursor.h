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
    ~SqliteCursor() override;
    QVariant value(int i) override;

    /*! [PROTOTYPE] @return internal buffer data. */
//! @todo virtual const char *** bufferData()
    /*! [PROTOTYPE] @return current record data or @c nullptr if there is no current records. */
    const char ** recordData() const override;

    bool drv_storeCurrentRecord(KDbRecordData* data) const override;

    //! Implemented for KDbResultable
    QString serverResultName() const override;

protected:
    /*! KDbCursor will operate on @a conn, raw @a sql statement will be used to execute query. */
    SqliteCursor(SqliteConnection* conn, const KDbEscapedString& sql,
                 Options options = KDbCursor::Option::None);

    /*! KDbCursor will operate on @a conn, @a query schema will be used to execute query. */
    SqliteCursor(SqliteConnection* conn, KDbQuerySchema* query,
                 Options options = KDbCursor::Option::None);

    bool drv_open(const KDbEscapedString& sql) override;

    bool drv_close() override;
    void drv_getNextRecord() override;

    void drv_appendCurrentRecordToBuffer() override;
    void drv_bufferMovePointerNext() override;
    void drv_bufferMovePointerPrev() override;
    void drv_bufferMovePointerTo(qint64 at) override;

//! @todo virtual void drv_storeCurrentRecord();

    //PROTOTYPE:
    /*! Method called when cursor's buffer need to be cleared
      (only for buffered cursor type), eg. in close(). */
    void drv_clearBuffer() override;

    void storeResult();

    SqliteCursorData * const d;

    friend class SqliteConnection;
    Q_DISABLE_COPY(SqliteCursor)
};

#endif

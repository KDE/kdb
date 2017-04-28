/* This file is part of the KDE project
   Copyright (C) 2003 Joseph Wenninger<jowenn@kde.org>
   Copyright (C) 2005-2016 Jarosław Staniek <staniek@kde.org>

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

#include "MysqlCursor.h"
#include "MysqlConnection.h"
#include "MysqlConnection_p.h"
#include "KDbError.h"
#include "KDb.h"
#include "KDbRecordData.h"

#include <limits.h>

#define BOOL bool

MysqlCursor::MysqlCursor(KDbConnection* conn, const KDbEscapedString& sql, int cursor_options)
        : KDbCursor(conn, sql, cursor_options)
        , d(new MysqlCursorData(conn))
{
    m_options |= Buffered;
}

MysqlCursor::MysqlCursor(KDbConnection* conn, KDbQuerySchema* query, int options)
        : KDbCursor(conn, query, options)
        , d(new MysqlCursorData(conn))
{
    m_options |= Buffered;
}

MysqlCursor::~MysqlCursor()
{
    close();
    delete d;
}

bool MysqlCursor::drv_open(const KDbEscapedString& sql)
{
    if (mysql_real_query(d->mysql, sql.constData(), sql.length()) == 0) {
        if (mysql_errno(d->mysql) == 0) {
            //! @todo Add option somewhere so we can use more optimal mysql_num_rows().
            //!       In this case mysql_num_rows() does not work however.
            d->mysqlres = mysql_store_result(d->mysql);
            m_fieldCount = mysql_num_fields(d->mysqlres);
            m_fieldsToStoreInRecord = m_fieldCount;
            d->numRows = mysql_num_rows(d->mysqlres);

            m_records_in_buf = d->numRows;
            m_buffering_completed = true;
            return true;
        }
    }

    storeResult();
    return false;
}

bool MysqlCursor::drv_close()
{
    mysql_free_result(d->mysqlres);
    d->mysqlres = nullptr;
    d->mysqlrow = nullptr;
    d->lengths = nullptr;
    d->numRows = 0;
    return true;
}

void MysqlCursor::drv_getNextRecord()
{
    if (at() >= d->numRows) {
        m_fetchResult = FetchEnd;
    }
    else if (at() < 0) {
        // control will reach here only when at() < 0 ( which is usually -1 )
        // -1 is same as "1 beyond the End"
        m_fetchResult = FetchEnd;
    }
    else {  // 0 <= at() < d->numRows
        d->lengths = mysql_fetch_lengths(d->mysqlres);
        m_fetchResult = FetchOK;
    }
}

// This isn't going to work right now as it uses d->mysqlrow
QVariant MysqlCursor::value(int pos)
{
    if (!d->mysqlrow || pos >= m_fieldCount || d->mysqlrow[pos] == nullptr)
        return QVariant();

    KDbField *f = (m_visibleFieldsExpanded && pos < m_visibleFieldsExpanded->count())
                       ? m_visibleFieldsExpanded->at(pos)->field : nullptr;

//! @todo js: use MYSQL_FIELD::type here!

    bool ok;
    return KDb::cstringToVariant(d->mysqlrow[pos], f ? f->type() : KDbField::Text,
                                            &ok, d->lengths[pos]);
}

/* As with sqlite, the DB library returns all values (including numbers) as
   strings. So just put that string in a QVariant and let KDb deal with it.
 */
bool MysqlCursor::drv_storeCurrentRecord(KDbRecordData* data) const
{
// mysqlDebug() << "position is " << (long)m_at;
    if (d->numRows == 0)
        return false;

    if (!m_visibleFieldsExpanded) {//simple version: without types
        for (int i = 0; i < m_fieldCount; ++i) {
            (*data)[i] = QString::fromUtf8(d->mysqlrow[i], d->lengths[i]);
        }
        return true;
    }
    for (int i = 0; i < m_fieldCount; ++i) {
        KDbField *f = m_visibleFieldsExpanded->at(i)->field;
        bool ok;
        (*data)[i] = KDb::cstringToVariant(d->mysqlrow[i], f ? f->type() : KDbField::Text,
                                           &ok, d->lengths[i]);
        if (!ok) {
            return false;
        }
    }
    return true;
}

void MysqlCursor::drv_appendCurrentRecordToBuffer()
{
}


void MysqlCursor::drv_bufferMovePointerNext()
{
    d->mysqlrow = mysql_fetch_row(d->mysqlres);
    d->lengths = mysql_fetch_lengths(d->mysqlres);
}

void MysqlCursor::drv_bufferMovePointerPrev()
{
    mysql_data_seek(d->mysqlres, m_at - 1);
    d->mysqlrow = mysql_fetch_row(d->mysqlres);
    d->lengths = mysql_fetch_lengths(d->mysqlres);
}


void MysqlCursor::drv_bufferMovePointerTo(qint64 to)
{
    mysql_data_seek(d->mysqlres, to);
    d->mysqlrow = mysql_fetch_row(d->mysqlres);
    d->lengths = mysql_fetch_lengths(d->mysqlres);
}

const char** MysqlCursor::recordData() const
{
    //! @todo
    return nullptr;
}

QString MysqlCursor::serverResultName() const
{
    return MysqlConnectionInternal::serverResultName(d->mysql);
}

void MysqlCursor::storeResult()
{
    d->storeResult(&m_result);
}

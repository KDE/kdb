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
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/


#include "SybaseCursor.h"
#include "SybaseConnection.h"
#include "SybaseConnection_p.h"


#include "KDbError.h"
#include "KDbUtils.h"

#include <limits.h>
#include <cstring>

#include <sqldb.h>

SybaseCursor::SybaseCursor(KDbConnection* conn, const KDbEscapedString& sql, int cursor_options)
        : KDbCursor(conn, sql, cursor_options)
        , d(new SybaseCursorData(conn))
{

    //m_options |= Buffered;

    d->dbProcess = static_cast<SybaseConnection*>(conn)->d->dbProcess;
// sybaseDebug() << "SybaseCursor: constructor for query statement";
}

SybaseCursor::SybaseCursor(KDbConnection* conn, KDbQuerySchema* query, int options)
        : KDbCursor(conn, query, options)
        , d(new SybaseCursorData(conn))
{
    //  m_options |= Buffered;

    d->dbProcess = static_cast<SybaseConnection*>(conn)->d->dbProcess;
// sybaseDebug() << "SybaseCursor: constructor for query statement";
}

SybaseCursor::~SybaseCursor()
{
    close();
}

bool SybaseCursor::drv_open(const KDbEscapedString& sql)
{

    /* Pseudo Code
     *
     * Execute Query
     * If no error
     *   Store Result in buffer ( d-> ?? )
     *   Store fieldcount ( no. of columns ) in m_fieldCount
     *   Set m_fieldsToStoreInRecord equal to m_fieldCount
     *   Store number of rows in d->numRows
     *   Set pointer at 0 ( m_at = 0 )
     *
     *   Set opened flag as true ( m_opened = true )
     *   Set numberoOfRecordsInbuffer as d->numRows ( m_records_in_buf = d->numRows )
     *   Set Buffering Complete flag = true
     *   Set After Last flag = false
     *
     */

    // clear all previous results ( if remaining )
    if (dbcancel(d->dbProcess) == FAIL)
        sybaseWarning() << "drv_open" << "dead DBPROCESS ?";

    // insert into command buffer
    dbcmd(d->dbProcess, sql.toByteArray());
    // execute query
    dbsqlexec(d->dbProcess);

    if (dbresults(d->dbProcess) == SUCCEED) {
        // result set goes directly into dbProcess' buffer
        m_fieldCount = dbnumcols(d->dbProcess);
        m_fieldsToStoreInRecord = m_fieldCount;

        // only relevant if buffering will ever work
        // <ignore>
        d->numRows = DBLASTROW(d->dbProcess);   // only true if buffering enabled
        m_records_in_buf = d->numRows;
        m_buffering_completed = true;
        // </ignore>

        m_afterLast = false;
        m_opened = true;
        m_at = 0;

        return true;
    }

    setError(ERR_DB_SPECIFIC, static_cast<SybaseConnection*>(connection())->d->errmsg);
    return false;
}


bool SybaseCursor::drv_close()
{

    m_opened = false;
    d->numRows = 0;
    return true;
}

//! @todo SybaseCursor::drv_moveFirst()
/*bool SybaseCursor::drv_moveFirst() {
  return true;
}*/

void SybaseCursor::drv_getNextRecord()
{
// sybaseDebug();

    // no buffering , and we don't know how many rows are there in result set

    if (dbnextrow(d->dbProcess) != NO_MORE_ROWS)
        m_fetchResult = FetchResult::Ok;
    else {
        m_fetchResult = FetchResult::End;
    }

}


QVariant SybaseCursor::value(int pos)
{
    if (!d->dbProcess || pos >= m_fieldCount)
        return QVariant();

    KDbField *f = (m_visibleFieldsExpanded && pos < m_visibleFieldsExpanded->count())
                       ? m_visibleFieldsExpanded->at(pos)->field : 0;

    // db-library indexes its columns from 1
    pos = pos + 1;

    long int columnDataLength = dbdatlen(d->dbProcess, pos);

    // 512 is
    // 1. the length used internally in dblib for allocating data to each column in function dbprrow()
    // 2. it's greater than all the values returned in the dblib internal function _get_printable_size
    long int pointerLength = qMax(columnDataLength , (long int)512);

    BYTE* columnValue = new unsigned char[pointerLength + 1] ;

    // convert to string representation. All values are convertible to string
    dbconvert(d->dbProcess , dbcoltype(d->dbProcess , pos), dbdata(d->dbProcess , pos), columnDataLength , (SYBCHAR), columnValue, -2);

    QVariant returnValue = KDbcstringToVariant((const char*)columnValue , f, strlen((const char*)columnValue));

    delete[] columnValue;

    return returnValue;
}


/* As with sqlite, the DB library returns all values (including numbers) as
   strings. So just put that string in a QVariant and let KDb deal with it.
 */
bool SybaseCursor::drv_storeCurrentRecord(KDbRecordData* data) const
{
// sybaseDebug() << "Position is" << (long)m_at;
// if (d->numRows<=0)
//  return false;

    const int fieldsExpandedCount = m_visibleFieldsExpanded
                                    ? m_visibleFieldsExpanded->count() : INT_MAX;
    const int realCount = qMin(fieldsExpandedCount, m_fieldsToStoreInRecord);
    for (int i = 0; i < realCount; i++) {
        KDbField *f = m_visibleFieldsExpanded ? m_visibleFieldsExpanded->at(i)->field : 0;
        if (m_visibleFieldsExpanded && !f)
            continue;

        long int columnDataLength = dbdatlen(d->dbProcess, i + 1);

        // 512 is
        // 1. the length used internally in dblib for allocating data to each column in function dbprrow()
        // 2. it's greater than all the values returned in the dblib internal function _get_printable_size
        long int pointerLength = qMax(columnDataLength , (long int)512);

        BYTE* columnValue = new unsigned char[pointerLength + 1] ;

        // convert to string representation. All values are convertible to string
        dbconvert(d->dbProcess , dbcoltype(d->dbProcess , i + 1), dbdata(d->dbProcess , i + 1), columnDataLength , (SYBCHAR), columnValue, -2);

        (*data)[i] =  KDbcstringToVariant((const char*)columnValue , f,  strlen((const char*)columnValue));

        delete[] columnValue;
    }
    return true;
}

void SybaseCursor::drv_appendCurrentRecordToBuffer()
{
}


void SybaseCursor::drv_bufferMovePointerNext()
{
    //dbgetrow( d->dbProcess, m_at + 1 );
}

void SybaseCursor::drv_bufferMovePointerPrev()
{
    //dbgetrow( d->dbProcess, m_at - 1 );
}


void SybaseCursor::drv_bufferMovePointerTo(qint64 to)
{
    //dbgetrow( d->dbProcess, to );
}

const char** SybaseCursor::recordData() const
{
    //! @todo
    return 0;
}

int SybaseCursor::serverResult()
{
    return d->res;
}

QString SybaseCursor::serverResultName() const
{
    return QString();
}

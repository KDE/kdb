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

#include "KDbCursor.h"
#include "KDbDriver.h"
#include "KDbDriver_p.h"
#include "KDbError.h"
#include "KDbRecordEditBuffer.h"
#include "KDb.h"

#include <QtDebug>


#include <assert.h>
#include <stdlib.h>

#ifdef __GNUC__
#warning replace QPointer<KDbConnection> m_conn;
#else
#pragma WARNING(replace QPointer<KDbConnection> m_conn;)
#endif

KDbCursor::KDbCursor(KDbConnection* conn, const KDbEscapedString& statement, uint options)
        : m_conn(conn)
        , m_query(0)
        , m_rawStatement(statement)
        , m_options(options)
{
#ifdef KDB_DEBUG_GUI
    KDb::debugGUI(QLatin1String("Create cursor: ") + statement.toString());
#endif
    init();
}

KDbCursor::KDbCursor(KDbConnection* conn, KDbQuerySchema* query, uint options)
        : m_conn(conn)
        , m_query(query)
        , m_options(options)
{
#ifdef KDB_DEBUG_GUI
    KDb::debugGUI(QString::fromLatin1("Create cursor for query \"%1\": ").arg(query->name())
                        + KDbUtils::debugString(query));
#endif
    init();
}

void KDbCursor::init()
{
    assert(m_conn);
    m_conn->addCursor(this);
    m_opened = false;
    m_atLast = false;
    m_afterLast = false;
    m_readAhead = false;
    m_at = 0;
    m_records_in_buf = 0;
    m_buffering_completed = false;
    m_at_buffer = false;
    m_fetchResult = FetchInvalid;

    m_containsRecordIdInfo = (m_query && m_query->masterTable())
                          && m_conn->driver()->beh->ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE == false;

    if (m_query) {
        //get list of all fields
        m_fieldsExpanded = new KDbQueryColumnInfo::Vector();
        *m_fieldsExpanded = m_query->fieldsExpanded(
                                m_containsRecordIdInfo ? KDbQuerySchema::WithInternalFieldsAndRecordId : KDbQuerySchema::WithInternalFields);
        m_logicalFieldCount = m_fieldsExpanded->count()
                              - m_query->internalFields().count() - (m_containsRecordIdInfo ? 1 : 0);
        m_fieldCount = m_fieldsExpanded->count();
        m_fieldsToStoreInRecord = m_fieldCount;
    } else {
        m_fieldsExpanded = 0;
        m_logicalFieldCount = 0;
        m_fieldCount = 0;
        m_fieldsToStoreInRecord = 0;
    }
    m_orderByColumnList = 0;
    m_queryParameters = 0;
}

KDbCursor::~KDbCursor()
{
#ifdef KDB_DEBUG_GUI
    if (m_query)
        KDb::debugGUI(QLatin1String("~ Delete cursor for query"));
    else
        KDb::debugGUI(QLatin1String("~ Delete cursor: ") + m_rawStatement.toString());
#endif
    /* if (!m_query)
        KDbDbg << "KDbCursor::~KDbCursor() '" << m_rawStatement.toLatin1() << "'";
      else
        KDbDbg << "KDbCursor::~KDbCursor() ";*/

    //take me if delete was
    if (!m_conn->m_insideCloseDatabase) {
        if (!m_conn->m_destructor_started) {
            m_conn->takeCursor(this);
        } else {
            KDbFatal << "can be destroyed with Conenction::deleteCursor(), not with delete operator!";
        }
    }
    delete m_fieldsExpanded;
    delete m_queryParameters;
}

bool KDbCursor::open()
{
    if (m_opened) {
        if (!close())
            return false;
    }
    if (!m_rawStatement.isEmpty()) {
        m_result.setSql(m_rawStatement);
    }
    else {
        if (!m_query) {
            KDbDbg << "no query statement (or schema) defined!";
            m_result = KDbResult(ERR_SQL_EXECUTION_ERROR, QObject::tr("No query statement or schema defined."));
            return false;
        }
        KDbConnection::SelectStatementOptions options;
        options.alsoRetrieveRecordId = m_containsRecordIdInfo; /*get record Id if needed*/
        m_result.setSql(m_queryParameters
                        ? m_conn->selectStatement(m_query, *m_queryParameters, options)
                        : m_conn->selectStatement(m_query, options));
        if (m_result.sql().isEmpty()) {
            KDbDbg << "empty statement!";
            m_result = KDbResult(ERR_SQL_EXECUTION_ERROR, QObject::tr("Query statement is empty."));
            return false;
        }
    }
    m_opened = drv_open(m_result.sql());
    m_afterLast = false; //we are not @ the end
    m_at = 0; //we are before 1st rec
    if (!m_opened) {
        m_result.setCode(ERR_SQL_EXECUTION_ERROR);
        m_result.setMessage(QObject::tr("Error opening database cursor."));
        return false;
    }
    m_validRecord = false;

    if (m_conn->driver()->beh->_1ST_ROW_READ_AHEAD_REQUIRED_TO_KNOW_IF_THE_RESULT_IS_EMPTY) {
//  KDbDbg << "READ AHEAD:";
        m_readAhead = getNextRecord(); //true if any record in this query
//  KDbDbg << "READ AHEAD = " << m_readAhead;
    }
    m_at = 0; //we are still before 1st rec
    return !m_result.isError();
}

bool KDbCursor::close()
{
    if (!m_opened)
        return true;
    bool ret = drv_close();

    clearBuffer();

    m_opened = false;
    m_afterLast = false;
    m_readAhead = false;
    m_fieldCount = 0;
    m_fieldsToStoreInRecord = 0;
    m_logicalFieldCount = 0;
    m_at = -1;

// KDbDbg << ret;
    return ret;
}

bool KDbCursor::reopen()
{
    if (!m_opened)
        return open();
    return close() && open();
}

bool KDbCursor::moveFirst()
{
    if (!m_opened)
        return false;
    if (!m_readAhead) {
        if (m_options & Buffered) {
            if (m_records_in_buf == 0 && m_buffering_completed) {
                //eof and bof should now return true:
                m_afterLast = true;
                m_at = 0;
                return false; //buffering completed and there is no records!
            }
            if (m_records_in_buf > 0) {
                //set state as we would be before first rec:
                m_at_buffer = false;
                m_at = 0;
                //..and move to next, ie. 1st record
                m_afterLast = !getNextRecord();
                return !m_afterLast;
            }
        } else if (!(m_conn->driver()->beh->_1ST_ROW_READ_AHEAD_REQUIRED_TO_KNOW_IF_THE_RESULT_IS_EMPTY))  {
            // not buffered
            m_at = 0;
            m_afterLast = !getNextRecord();
            return !m_afterLast;
        }

        if (m_afterLast && m_at == 0) //failure if already no records
            return false;
        if (!reopen()) //try reopen
            return false;
        if (m_afterLast) //eof
            return false;
    } else {
        //we have a record already read-ahead: we now point @ that:
        m_at = 1;
    }
    //get first record
    m_afterLast = false;
    m_readAhead = false; //1st record had been read
    return m_validRecord;
}

bool KDbCursor::moveLast()
{
    if (!m_opened)
        return false;
    if (m_afterLast || m_atLast) {
        return m_validRecord; //we already have valid last record retrieved
    }
    if (!getNextRecord()) { //at least next record must be retrieved
        m_afterLast = true;
        m_validRecord = false;
        m_atLast = false;
        return false; //no records
    }
    while (getNextRecord()) //move after last rec.
        ;
    m_afterLast = false;
    //cursor shows last record data
    m_atLast = true;
    return true;
}

bool KDbCursor::moveNext()
{
    if (!m_opened || m_afterLast)
        return false;
    if (getNextRecord()) {
        return true;
    }
    return false;
}

bool KDbCursor::movePrev()
{
    if (!m_opened /*|| m_beforeFirst*/ || !(m_options & Buffered))
        return false;

    //we're after last record and there are records in the buffer
    //--let's move to last record
    if (m_afterLast && (m_records_in_buf > 0)) {
        drv_bufferMovePointerTo(m_records_in_buf - 1);
        m_at = m_records_in_buf;
        m_at_buffer = true; //now current record is stored in the buffer
        m_validRecord = true;
        m_afterLast = false;
        return true;
    }
    //we're at first record: go BOF
    if ((m_at <= 1) || (m_records_in_buf <= 1/*sanity*/)) {
        m_at = 0;
        m_at_buffer = false;
        m_validRecord = false;
        return false;
    }

    m_at--;
    if (m_at_buffer) {//we already have got a pointer to buffer
        drv_bufferMovePointerPrev(); //just move to prev record in the buffer
    } else {//we have no pointer
        //compute a place in the buffer that contain next record's data
        drv_bufferMovePointerTo(m_at - 1);
        m_at_buffer = true; //now current record is stored in the buffer
    }
    m_validRecord = true;
    m_afterLast = false;
    return true;
}

bool KDbCursor::isBuffered() const
{
    return m_options & Buffered;
}

void KDbCursor::setBuffered(bool buffered)
{
    if (!m_opened)
        return;
    if (isBuffered() == buffered)
        return;
    m_options ^= Buffered;
}

void KDbCursor::clearBuffer()
{
    if (!isBuffered() || m_fieldCount == 0)
        return;

    drv_clearBuffer();

    m_records_in_buf = 0;
    m_at_buffer = false;
}

bool KDbCursor::getNextRecord()
{
    m_fetchResult = FetchInvalid; //by default: invalid result of record fetching

    if (m_options & Buffered) {//this cursor is buffered:
//  KDbDbg << "m_at < m_records_in_buf :: " << (long)m_at << " < " << m_records_in_buf;
        if (m_at < m_records_in_buf) {//we have next record already buffered:
            if (m_at_buffer) {//we already have got a pointer to buffer
                drv_bufferMovePointerNext(); //just move to next record in the buffer
            } else {//we have no pointer
                //compute a place in the buffer that contain next record's data
                drv_bufferMovePointerTo(m_at - 1 + 1);
                m_at_buffer = true; //now current record is stored in the buffer
            }
        } else {//we are after last retrieved record: we need to physically fetch next record:
            if (!m_readAhead) {//we have no record that was read ahead
                if (!m_buffering_completed) {
                    //retrieve record only if we are not after
                    //the last buffer's item (i.e. when buffer is not fully filled):
//     KDbDbg<<"==== buffering: drv_getNextRecord() ====";
                    drv_getNextRecord();
                }
                if (m_fetchResult != FetchOK) {//there is no record
                    m_buffering_completed = true; //no more records for buffer
//     KDbDbg<<"m_fetchResult != FetchOK ********";
                    m_validRecord = false;
                    m_afterLast = true;
                    m_at = -1; //position is invalid now and will not be used
                    if (m_fetchResult == FetchError) {
                        m_result = KDbResult(ERR_CURSOR_RECORD_FETCHING, QObject::tr("Cannot fetch next record."));
                        return false;
                    }
                    return false; // in case of m_fetchResult = FetchEnd or m_fetchResult = FetchInvalid
                }
                //we have a record: store this record's values in the buffer
                drv_appendCurrentRecordToBuffer();
                m_records_in_buf++;
            } else //we have a record that was read ahead: eat this
                m_readAhead = false;
        }
    } else {//we are after last retrieved record: we need to physically fetch next record:
        if (!m_readAhead) {//we have no record that was read ahead
//   KDbDbg<<"==== no prefetched record ====";
            drv_getNextRecord();
            if (m_fetchResult != FetchOK) {//there is no record
//    KDbDbg<<"m_fetchResult != FetchOK ********";
                m_validRecord = false;
                m_afterLast = true;
                m_at = -1;
                if (m_fetchResult == FetchEnd) {
                    return false;
                }
                m_result = KDbResult(ERR_CURSOR_RECORD_FETCHING, QObject::tr("Cannot fetch next record."));
                return false;
            }
        } else //we have a record that was read ahead: eat this
            m_readAhead = false;
    }

    m_at++;

// if (m_data->curr_colname && m_data->curr_coldata)
//  for (int i=0;i<m_data->curr_cols;i++) {
//   KDbDbg<<i<<": "<< m_data->curr_colname[i]<<" == "<< m_data->curr_coldata[i];
//  }
// KDbDbg<<"m_at == "<<(long)m_at;

    m_validRecord = true;
    return true;
}

bool KDbCursor::updateRecord(KDbRecordData* data, KDbRecordEditBuffer* buf, bool useRecordId)
{
//! @todo doesn't update cursor's buffer YET!
    clearResult();
    if (!m_query)
        return false;
    return m_conn->updateRecord(m_query, data, buf, useRecordId);
}

bool KDbCursor::insertRecord(KDbRecordData* data, KDbRecordEditBuffer* buf, bool useRecordId)
{
//! @todo doesn't update cursor's buffer YET!
    clearResult();
    if (!m_query)
        return false;
    return m_conn->insertRecord(m_query, data, buf, useRecordId);
}

bool KDbCursor::deleteRecord(KDbRecordData* data, bool useRecordId)
{
//! @todo doesn't update cursor's buffer YET!
    clearResult();
    if (!m_query)
        return false;
    return m_conn->deleteRecord(m_query, data, useRecordId);
}

bool KDbCursor::deleteAllRecords()
{
//! @todo doesn't update cursor's buffer YET!
    clearResult();
    if (!m_query)
        return false;
    return m_conn->deleteAllRecords(m_query);
}

QDebug operator<<(QDebug dbg, const KDbCursor& cursor)
{
    dbg.nospace() << "CURSOR(";
    if (!cursor.query()) {
        dbg.nospace() << "RAW STATEMENT:" << cursor.rawStatement().toString()
                      << "\n";
    }
    else {
        dbg.nospace() << "KDbQuerySchema:"
                      << cursor.connection()->selectStatement(cursor.query()).toString()
                      << "\n";
    }
    if (cursor.isOpened()) {
        dbg.space() << "OPENED";
    }
    else {
        dbg.space() << "NOT_OPENED";
    }
    if (cursor.isBuffered()) {
        dbg.space() << "BUFFERED";
    }
    else {
        dbg.space() << "NOT_BUFFERED";
    }
    dbg.nospace() << "AT=" << cursor.at() << ")";
    return dbg.space();
}

void KDbCursor::setOrderByColumnList(const QStringList& columnNames)
{
    Q_UNUSED(columnNames);
//! @todo implement this:
// all field names should be fooun, exit otherwise ..........

    // OK
//! @todo if (!m_orderByColumnList)
}

/*! Convenience method, similar to setOrderBy(const QStringList&). */
void KDbCursor::setOrderByColumnList(const QString& column1, const QString& column2,
                                  const QString& column3, const QString& column4, const QString& column5)
{
    Q_UNUSED(column1);
    Q_UNUSED(column2);
    Q_UNUSED(column3);
    Q_UNUSED(column4);
    Q_UNUSED(column5);
//! @todo implement this, like above
//! @todo add ORDER BY info to debugString()
}

KDbQueryColumnInfo::Vector KDbCursor::orderByColumnList() const
{
    return m_orderByColumnList ? *m_orderByColumnList : KDbQueryColumnInfo::Vector();
}

QList<QVariant> KDbCursor::queryParameters() const
{
    return m_queryParameters ? *m_queryParameters : QList<QVariant>();
}

void KDbCursor::setQueryParameters(const QList<QVariant>& params)
{
    if (!m_queryParameters)
        m_queryParameters = new QList<QVariant>(params);
    else
        *m_queryParameters = params;
}

static const char *extraMessages[] = {
    QT_TR_NOOP("No connection for cursor open operation specified.")
};

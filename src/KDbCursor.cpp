/* This file is part of the KDE project
   Copyright (C) 2003-2016 Jarosław Staniek <staniek@kde.org>

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
#include "KDbConnection.h"
#include "KDbDriver.h"
#include "KDbDriverBehavior.h"
#include "KDbError.h"
#include "KDb.h"
#include "KDbNativeStatementBuilder.h"
#include "KDbQuerySchema.h"
#include "KDbRecordData.h"
#include "KDbRecordEditBuffer.h"
#include "kdb_debug.h"

#include <assert.h>
#include <stdlib.h>

class KDbCursor::Private
{
public:
    Private()
        : opened(false)
        , atLast(false)
        , readAhead(false)
        , validRecord(false)
        , atBuffer(false)
    {
    }

    ~Private() {
    }

    bool containsRecordIdInfo; //!< true if result contains extra column for record id;
                               //!< used only for PostgreSQL now
    //! @todo IMPORTANT: use something like QPointer<KDbConnection> conn;
    KDbConnection *conn;
    KDbEscapedString rawSql;
    bool opened;
    bool atLast;
    bool readAhead;
    bool validRecord; //!< true if valid record is currently retrieved @ current position

    //! Used by setOrderByColumnList()
    KDbQueryColumnInfo::Vector orderByColumnList;
    QList<QVariant> queryParameters;

    //<members related to buffering>
    bool atBuffer; //!< true if we already point to the buffer with curr_coldata
    //</members related to buffering>
};

KDbCursor::KDbCursor(KDbConnection* conn, const KDbEscapedString& sql, int options)
        : m_query(nullptr)
        , m_options(options)
        , d(new Private)
{
#ifdef KDB_DEBUG_GUI
    KDb::debugGUI(QLatin1String("Create cursor for raw SQL: ") + sql.toString());
#endif
    init(conn);
    d->rawSql = sql;
}

KDbCursor::KDbCursor(KDbConnection* conn, KDbQuerySchema* query, int options)
        : m_query(query)
        , m_options(options)
        , d(new Private)
{
#ifdef KDB_DEBUG_GUI
    KDb::debugGUI(QString::fromLatin1("Create cursor for query \"%1\":\n")
                  .arg(KDb::iifNotEmpty(query->name(), QString::fromLatin1("<unnamed>")))
                  + KDbUtils::debugString(query));
#endif
    init(conn);
}

void KDbCursor::init(KDbConnection* conn)
{
    Q_ASSERT(conn);
    d->conn = conn;
    d->conn->addCursor(this);
    m_afterLast = false;
    m_at = 0;
    m_records_in_buf = 0;
    m_buffering_completed = false;
    m_fetchResult = FetchInvalid;

    d->containsRecordIdInfo = (m_query && m_query->masterTable())
                              && d->conn->driver()->beh->ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE == false;

    if (m_query) {
        //get list of all fields
        m_visibleFieldsExpanded = new KDbQueryColumnInfo::Vector();
        *m_visibleFieldsExpanded = m_query->visibleFieldsExpanded(
                    d->containsRecordIdInfo ? KDbQuerySchema::WithInternalFieldsAndRecordId
                                            : KDbQuerySchema::WithInternalFields);
        m_logicalFieldCount = m_visibleFieldsExpanded->count()
                              - m_query->internalFields().count() - (d->containsRecordIdInfo ? 1 : 0);
        m_fieldCount = m_visibleFieldsExpanded->count();
        m_fieldsToStoreInRecord = m_fieldCount;
    } else {
        m_visibleFieldsExpanded = nullptr;
        m_logicalFieldCount = 0;
        m_fieldCount = 0;
        m_fieldsToStoreInRecord = 0;
    }
}

KDbCursor::~KDbCursor()
{
#ifdef KDB_DEBUG_GUI
#if 0 // too many details
    if (m_query)
        KDb::debugGUI(QLatin1String("~ Delete cursor for query"));
    else
        KDb::debugGUI(QLatin1String("~ Delete cursor: ") + m_rawSql.toString());
#endif
#endif
    /* if (!m_query)
        kdbDebug() << "KDbCursor::~KDbCursor() '" << m_rawSql.toLatin1() << "'";
      else
        kdbDebug() << "KDbCursor::~KDbCursor() ";*/

    d->conn->takeCursor(this);
    delete m_visibleFieldsExpanded;
    delete d;
}

bool KDbCursor::readAhead() const
{
    return d->readAhead;
}

KDbConnection* KDbCursor::connection() const
{
    return d->conn;
}

KDbQuerySchema *KDbCursor::query() const
{
    return m_query;
}

KDbEscapedString KDbCursor::rawSql() const
{
    return d->rawSql;
}

int KDbCursor::options() const
{
    return m_options;
}

bool KDbCursor::isOpened() const
{
    return d->opened;
}

bool KDbCursor::containsRecordIdInfo() const
{
    return d->containsRecordIdInfo;
}

KDbRecordData* KDbCursor::storeCurrentRecord() const
{
    KDbRecordData* data = new KDbRecordData(m_fieldsToStoreInRecord);
    if (!drv_storeCurrentRecord(data)) {
        delete data;
        return nullptr;
    }
    return data;
}

bool KDbCursor::storeCurrentRecord(KDbRecordData* data) const
{
    Q_ASSERT(data);
    data->resize(m_fieldsToStoreInRecord);
    return drv_storeCurrentRecord(data);
}

bool KDbCursor::open()
{
    if (d->opened) {
        if (!close())
            return false;
    }
    if (!d->rawSql.isEmpty()) {
        m_result.setSql(d->rawSql);
    }
    else {
        if (!m_query) {
            kdbDebug() << "no query statement (or schema) defined!";
            m_result = KDbResult(ERR_SQL_EXECUTION_ERROR,
                                 tr("No query statement or schema defined."));
            return false;
        }
        KDbSelectStatementOptions options;
        options.alsoRetrieveRecordId = d->containsRecordIdInfo; /*get record Id if needed*/
        KDbNativeStatementBuilder builder(d->conn);
        KDbEscapedString sql;
        if (!builder.generateSelectStatement(&sql, m_query, options, d->queryParameters)
            || sql.isEmpty())
        {
            kdbDebug() << "no statement generated!";
            m_result = KDbResult(ERR_SQL_EXECUTION_ERROR,
                                 tr("Could not generate query statement."));
            return false;
        }
        m_result.setSql(sql);
#ifdef KDB_DEBUG_GUI
        KDb::debugGUI(QString::fromLatin1("SQL for query \"%1\": ")
                         .arg(KDb::iifNotEmpty(m_query->name(), QString::fromLatin1("<unnamed>")))
                      + m_result.sql().toString());
#endif
    }
    d->opened = drv_open(m_result.sql());
    m_afterLast = false; //we are not @ the end
    m_at = 0; //we are before 1st rec
    if (!d->opened) {
        m_result.setCode(ERR_SQL_EXECUTION_ERROR);
        m_result.setMessage(tr("Error opening database cursor."));
        return false;
    }
    d->validRecord = false;

    if (d->conn->driver()->beh->_1ST_ROW_READ_AHEAD_REQUIRED_TO_KNOW_IF_THE_RESULT_IS_EMPTY) {
//  kdbDebug() << "READ AHEAD:";
        d->readAhead = getNextRecord(); //true if any record in this query
//  kdbDebug() << "READ AHEAD = " << d->readAhead;
    }
    m_at = 0; //we are still before 1st rec
    return !m_result.isError();
}

bool KDbCursor::close()
{
    if (!d->opened) {
        return true;
    }
    bool ret = drv_close();

    clearBuffer();

    d->opened = false;
    m_afterLast = false;
    d->readAhead = false;
    m_fieldCount = 0;
    m_fieldsToStoreInRecord = 0;
    m_logicalFieldCount = 0;
    m_at = -1;

// kdbDebug() << ret;
    return ret;
}

bool KDbCursor::reopen()
{
    if (!d->opened) {
        return open();
    }
    return close() && open();
}

bool KDbCursor::moveFirst()
{
    if (!d->opened) {
        return false;
    }
    if (!d->readAhead) {
        if (m_options & Buffered) {
            if (m_records_in_buf == 0 && m_buffering_completed) {
                //eof and bof should now return true:
                m_afterLast = true;
                m_at = 0;
                return false; //buffering completed and there is no records!
            }
            if (m_records_in_buf > 0) {
                //set state as we would be before first rec:
                d->atBuffer = false;
                m_at = 0;
                //..and move to next, ie. 1st record
                m_afterLast = !getNextRecord();
                return !m_afterLast;
            }
        } else if (!(d->conn->driver()->beh->_1ST_ROW_READ_AHEAD_REQUIRED_TO_KNOW_IF_THE_RESULT_IS_EMPTY))  {
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
    d->readAhead = false; //1st record had been read
    return d->validRecord;
}

bool KDbCursor::moveLast()
{
    if (!d->opened) {
        return false;
    }
    if (m_afterLast || d->atLast) {
        return d->validRecord; //we already have valid last record retrieved
    }
    if (!getNextRecord()) { //at least next record must be retrieved
        m_afterLast = true;
        d->validRecord = false;
        d->atLast = false;
        return false; //no records
    }
    while (getNextRecord()) //move after last rec.
        ;
    m_afterLast = false;
    //cursor shows last record data
    d->atLast = true;
    return true;
}

bool KDbCursor::moveNext()
{
    if (!d->opened || m_afterLast) {
        return false;
    }
    if (getNextRecord()) {
        return true;
    }
    return false;
}

bool KDbCursor::movePrev()
{
    if (!d->opened /*|| m_beforeFirst*/ || !(m_options & Buffered)) {
        return false;
    }
    //we're after last record and there are records in the buffer
    //--let's move to last record
    if (m_afterLast && (m_records_in_buf > 0)) {
        drv_bufferMovePointerTo(m_records_in_buf - 1);
        m_at = m_records_in_buf;
        d->atBuffer = true; //now current record is stored in the buffer
        d->validRecord = true;
        m_afterLast = false;
        return true;
    }
    //we're at first record: go BOF
    if ((m_at <= 1) || (m_records_in_buf <= 1/*sanity*/)) {
        m_at = 0;
        d->atBuffer = false;
        d->validRecord = false;
        return false;
    }

    m_at--;
    if (d->atBuffer) {//we already have got a pointer to buffer
        drv_bufferMovePointerPrev(); //just move to prev record in the buffer
    } else {//we have no pointer
        //compute a place in the buffer that contain next record's data
        drv_bufferMovePointerTo(m_at - 1);
        d->atBuffer = true; //now current record is stored in the buffer
    }
    d->validRecord = true;
    m_afterLast = false;
    return true;
}

bool KDbCursor::isBuffered() const
{
    return m_options & Buffered;
}

void KDbCursor::setBuffered(bool buffered)
{
    if (!d->opened) {
        return;
    }
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
    d->atBuffer = false;
}

bool KDbCursor::getNextRecord()
{
    m_fetchResult = FetchInvalid; //by default: invalid result of record fetching

    if (m_options & Buffered) {//this cursor is buffered:
//  kdbDebug() << "m_at < m_records_in_buf :: " << (long)m_at << " < " << m_records_in_buf;
        if (m_at < m_records_in_buf) {//we have next record already buffered:
            if (d->atBuffer) {//we already have got a pointer to buffer
                drv_bufferMovePointerNext(); //just move to next record in the buffer
            } else {//we have no pointer
                //compute a place in the buffer that contain next record's data
                drv_bufferMovePointerTo(m_at - 1 + 1);
                d->atBuffer = true; //now current record is stored in the buffer
            }
        } else {//we are after last retrieved record: we need to physically fetch next record:
            if (!d->readAhead) {//we have no record that was read ahead
                if (!m_buffering_completed) {
                    //retrieve record only if we are not after
                    //the last buffer's item (i.e. when buffer is not fully filled):
//     kdbDebug()<<"==== buffering: drv_getNextRecord() ====";
                    drv_getNextRecord();
                }
                if (m_fetchResult != FetchOK) {//there is no record
                    m_buffering_completed = true; //no more records for buffer
//     kdbDebug()<<"m_fetchResult != FetchOK ********";
                    d->validRecord = false;
                    m_afterLast = true;
                    m_at = -1; //position is invalid now and will not be used
                    if (m_fetchResult == FetchError) {
                        m_result = KDbResult(ERR_CURSOR_RECORD_FETCHING,
                                             tr("Could not fetch next record."));
                        return false;
                    }
                    return false; // in case of m_fetchResult = FetchEnd or m_fetchResult = FetchInvalid
                }
                //we have a record: store this record's values in the buffer
                drv_appendCurrentRecordToBuffer();
                m_records_in_buf++;
            } else //we have a record that was read ahead: eat this
                d->readAhead = false;
        }
    } else {//we are after last retrieved record: we need to physically fetch next record:
        if (!d->readAhead) {//we have no record that was read ahead
//   kdbDebug()<<"==== no prefetched record ====";
            drv_getNextRecord();
            if (m_fetchResult != FetchOK) {//there is no record
//    kdbDebug()<<"m_fetchResult != FetchOK ********";
                d->validRecord = false;
                m_afterLast = true;
                m_at = -1;
                if (m_fetchResult == FetchEnd) {
                    return false;
                }
                m_result = KDbResult(ERR_CURSOR_RECORD_FETCHING,
                                     tr("Could not fetch next record."));
                return false;
            }
        } else { //we have a record that was read ahead: eat this
            d->readAhead = false;
        }
    }

    m_at++;

// if (m_data->curr_colname && m_data->curr_coldata)
//  for (int i=0;i<m_data->curr_cols;i++) {
//   kdbDebug()<<i<<": "<< m_data->curr_colname[i]<<" == "<< m_data->curr_coldata[i];
//  }
// kdbDebug()<<"m_at == "<<(long)m_at;

    d->validRecord = true;
    return true;
}

bool KDbCursor::updateRecord(KDbRecordData* data, KDbRecordEditBuffer* buf, bool useRecordId)
{
//! @todo doesn't update cursor's buffer YET!
    clearResult();
    if (!m_query)
        return false;
    return d->conn->updateRecord(m_query, data, buf, useRecordId);
}

bool KDbCursor::insertRecord(KDbRecordData* data, KDbRecordEditBuffer* buf, bool useRecordId)
{
//! @todo doesn't update cursor's buffer YET!
    if (!m_query) {
        clearResult();
        return false;
    }
    return d->conn->insertRecord(m_query, data, buf, useRecordId);
}

bool KDbCursor::deleteRecord(KDbRecordData* data, bool useRecordId)
{
//! @todo doesn't update cursor's buffer YET!
    clearResult();
    if (!m_query)
        return false;
    return d->conn->deleteRecord(m_query, data, useRecordId);
}

bool KDbCursor::deleteAllRecords()
{
//! @todo doesn't update cursor's buffer YET!
    clearResult();
    if (!m_query)
        return false;
    return d->conn->deleteAllRecords(m_query);
}

QDebug operator<<(QDebug dbg, const KDbCursor& cursor)
{
    dbg.nospace() << "CURSOR(";
    if (!cursor.query()) {
        dbg.nospace() << "RAW SQL STATEMENT:" << cursor.rawSql().toString()
                      << "\n";
    }
    else {
        KDbNativeStatementBuilder builder(cursor.connection());
        KDbEscapedString sql;
        QString sqlString;
        if (builder.generateSelectStatement(&sql, cursor.query())) {
            sqlString = sql.toString();
        }
        else {
            sqlString = QLatin1String("<CANNOT GENERATE!>");
        }
        dbg.nospace() << "KDbQuerySchema:" << sqlString << "\n";
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
//! @todo implement this: all field names should be found, exit otherwise

    // OK
//! @todo if (!d->orderByColumnList)
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
    return d->orderByColumnList;
}

QList<QVariant> KDbCursor::queryParameters() const
{
    return d->queryParameters;
}

void KDbCursor::setQueryParameters(const QList<QVariant>& params)
{
    d->queryParameters = params;
}

//! @todo extraMessages
#if 0
static const char *extraMessages[] = {
    QT_TRANSLATE_NOOP("KDbCursor", "No connection for cursor open operation specified.")
};
#endif

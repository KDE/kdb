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

#include "SqliteCursor.h"

#include "SqliteConnection.h"
#include "SqliteConnection_p.h"

#include <Predicate/Error>
#include <Predicate/Driver>
#include <Predicate/Tools/Utils>

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <QtDebug>


#include <QVector>
#include <QDateTime>
#include <QByteArray>

using namespace Predicate;

//! safer interpretations of boolean values for SQLite
static bool sqliteStringToBool(const QString& s)
{
    return 0 == s.compare(QLatin1String("yes"), Qt::CaseInsensitive)
        || (0 != s.compare(QLatin1String("no"), Qt::CaseInsensitive) && s != QLatin1String("0"));
}

//----------------------------------------------------

class Predicate::SQLiteCursorData : public SQLiteConnectionInternal
{
public:
    SQLiteCursorData(Connection* conn)
            :
            SQLiteConnectionInternal(conn)
            , prepared_st_handle(0)
            , utail(0)
            , curr_coldata(0)
            , curr_colname(0)
            , cols_pointers_mem_size(0)
    {
        data_owned = false;
    }

    /*
        void fetchRowDataIfNeeded()
        {
          if (!rowDataReadyToFetch)
            return true;
          rowDataReadyToFetch = false;
          m_fieldCount = sqlite3_data_count(data);
          for (int i=0; i<m_fieldCount; i++) {

          }
        }
    */

    sqlite3_stmt *prepared_st_handle;

    char *utail;
    const char **curr_coldata;
    const char **curr_colname;
    uint cols_pointers_mem_size; //!< size of record's array of pointers to values
    QVector<const char**> records; //!< buffer data

    inline QVariant getValue(Field *f, int i) {
        int type = sqlite3_column_type(prepared_st_handle, i);
        if (type == SQLITE_NULL) {
            return QVariant();
        } else if (!f || type == SQLITE_TEXT) {
//! @todo support for UTF-16
#define GET_sqlite3_column_text QString::fromUtf8( (const char*)sqlite3_column_text(prepared_st_handle, i) )
            if (!f || f->isTextType())
                return GET_sqlite3_column_text;
            else {
                switch (f->type()) {
                case Field::Date:
                    return QDate::fromString(GET_sqlite3_column_text, Qt::ISODate);
                case Field::Time:
                    //QDateTime - a hack needed because QVariant(QTime) has broken isNull()
                    return Utils::stringToHackedQTime(GET_sqlite3_column_text);
                case Field::DateTime: {
                    QString tmp(GET_sqlite3_column_text);
                    tmp[10] = 'T'; //for ISODate compatibility
                    return QDateTime::fromString(tmp, Qt::ISODate);
                }
                case Field::Boolean:
                    return sqliteStringToBool(GET_sqlite3_column_text);
                default:
                    return QVariant(); //!< @todo
                }
            }
        } else if (type == SQLITE_INTEGER) {
            switch (f->type()) {
            case Field::Byte:
            case Field::ShortInteger:
            case Field::Integer:
                return QVariant(sqlite3_column_int(prepared_st_handle, i));
            case Field::BigInteger:
                return QVariant((qint64)sqlite3_column_int64(prepared_st_handle, i));
            case Field::Boolean:
                return sqlite3_column_int(prepared_st_handle, i) != 0;
            default:;
            }
            if (f->isFPNumericType()) //WEIRD, YEAH?
                return QVariant((double)sqlite3_column_int(prepared_st_handle, i));
            else
                return QVariant(); //!< @todo
        } else if (type == SQLITE_FLOAT) {
            if (f && f->isFPNumericType())
                return QVariant(sqlite3_column_double(prepared_st_handle, i));
            else if (!f || f->isIntegerType())
                return QVariant((double)sqlite3_column_double(prepared_st_handle, i));
            else
                return QVariant(); //!< @todo
        } else if (type == SQLITE_BLOB) {
            if (f && f->type() == Field::BLOB) {
//! @todo efficient enough?
                return QByteArray((const char*)sqlite3_column_blob(prepared_st_handle, i),
                                  sqlite3_column_bytes(prepared_st_handle, i));
            } else
                return QVariant(); //!< @todo
        }
        return QVariant();
    }
};

SQLiteCursor::SQLiteCursor(Connection* conn, const EscapedString& statement, uint options)
        : Cursor(conn, statement, options)
        , d(new SQLiteCursorData(conn))
{
    d->data = static_cast<SQLiteConnection*>(conn)->d->data;
}

SQLiteCursor::SQLiteCursor(Connection* conn, QuerySchema* query, uint options)
        : Cursor(conn, query, options)
        , d(new SQLiteCursorData(conn))
{
    d->data = static_cast<SQLiteConnection*>(conn)->d->data;
}

SQLiteCursor::~SQLiteCursor()
{
    close();
    delete d;
}

bool SQLiteCursor::drv_open(const EscapedString& sql)
{
    //! @todo decode
    if (! d->data) {
        // this may as example be the case if SQLiteConnection::drv_useDatabase()
        // wasn't called before. Normaly sqlite_compile/sqlite3_prepare
        // should handle it, but it crashes in in sqlite3SafetyOn at util.c:786
        qWarning() << "SQLiteCursor::drv_open(): Database handle undefined.";
        return false;
    }

    m_result.setServerResultCode(
        sqlite3_prepare(
                 d->data,            /* Database handle */
                 sql.constData(),       /* SQL statement, UTF-8 encoded */
                 sql.length(),             /* Length of zSql in bytes. */
                 &d->prepared_st_handle,  /* OUT: Statement handle */
                 0/*const char **pzTail*/     /* OUT: Pointer to unused portion of zSql */
             )
    );
    if (m_result.serverResultCode() != SQLITE_OK) {
        storeResult();
        return false;
    }
    if (isBuffered()) {
//! @todo manage size dynamically
        d->records.resize(128);
    }

    return true;
}

bool SQLiteCursor::drv_close()
{
    m_result.setServerResultCode(
        sqlite3_finalize(d->prepared_st_handle)
    );
    if (m_result.serverResultCode() != SQLITE_OK) {
        storeResult();
        return false;
    }
    return true;
}

void SQLiteCursor::drv_getNextRecord()
{
    m_result.setServerResultCode(
        sqlite3_step(d->prepared_st_handle)
    );
    if (m_result.serverResultCode() == SQLITE_ROW) {
        m_fetchResult = FetchOK;
        m_fieldCount = sqlite3_data_count(d->prepared_st_handle);
//#else //for SQLITE3 data fetching is delayed. Now we even do not take field count information
//      // -- just set a flag that we've a data not fetched but available
        m_fieldsToStoreInRecord = m_fieldCount;
    }
    else {
        if (m_result.serverResultCode() == SQLITE_DONE)
            m_fetchResult = FetchEnd;
        else
            m_fetchResult = FetchError;
    }

    //debug
    /*
      if ((int)m_result == (int)FetchOK && d->curr_coldata) {
        for (uint i=0;i<m_fieldCount;i++) {
          PreDrvDbg<<"col."<< i<<": "<< d->curr_colname[i]<<" "<< d->curr_colname[m_fieldCount+i]
          << " = " << (d->curr_coldata[i] ? QString::fromLocal8Bit(d->curr_coldata[i]) : "(NULL)");
        }
    //  PreDrvDbg << m_fieldCount << "col(s) fetched";
      }*/
}

void SQLiteCursor::drv_appendCurrentRecordToBuffer()
{
// PreDrvDbg;
    if (!d->curr_coldata)
        return;
    if (!d->cols_pointers_mem_size)
        d->cols_pointers_mem_size = m_fieldCount * sizeof(char*);
    const char **record = (const char**)malloc(d->cols_pointers_mem_size);
    const char **src_col = d->curr_coldata;
    const char **dest_col = record;
    for (uint i = 0; i < m_fieldCount; i++, src_col++, dest_col++) {
//  PreDrvDbg << i <<": '" << *src_col << "'";
//  PreDrvDbg << "src_col: " << src_col;
        *dest_col = *src_col ? strdup(*src_col) : 0;
    }
    d->records[m_records_in_buf] = record;
// PreDrvDbg << "ok.";
}

void SQLiteCursor::drv_bufferMovePointerNext()
{
    d->curr_coldata++; //move to next record in the buffer
}

void SQLiteCursor::drv_bufferMovePointerPrev()
{
    d->curr_coldata--; //move to prev record in the buffer
}

//compute a place in the buffer that contain next record's data
//and move internal buffer pointer to that place
void SQLiteCursor::drv_bufferMovePointerTo(qint64 at)
{
    d->curr_coldata = d->records.at(at);
}

void SQLiteCursor::drv_clearBuffer()
{
    if (d->cols_pointers_mem_size > 0) {
        const uint records_in_buf = m_records_in_buf;
        const char ***r_ptr = d->records.data();
        for (uint i = 0; i < records_in_buf; i++, r_ptr++) {
            const char **field_data = *r_ptr;
            for (uint col = 0; col < m_fieldCount; col++, field_data++) {
                free((void*)*field_data); //free field memory
            }
            free(*r_ptr); //free pointers to fields array
        }
    }
    m_records_in_buf = 0;
    d->cols_pointers_mem_size = 0;
    d->records.clear();
}

//! @todo
/*
const char *** SQLiteCursor::bufferData()
{
  if (!isBuffered())
    return 0;
  return m_records.data();
}*/

const char ** SQLiteCursor::recordData() const
{
    return d->curr_coldata;
}

bool SQLiteCursor::drv_storeCurrentRecord(RecordData* data) const
{
    if (!m_fieldsExpanded) {//simple version: without types
        for (uint i = 0; i < m_fieldCount; i++) {
            (*data)[i] = QString::fromUtf8((const char*)sqlite3_column_text(d->prepared_st_handle, i));
        }
        return true;
    }
    const uint maxCount = qMin(m_fieldCount, (uint)m_fieldsExpanded->count());
    // i - visible field's index, j - physical index
    for (uint i = 0, j = 0; i < m_fieldCount; i++, j++) {
        while (j < maxCount && !m_fieldsExpanded->at(j)->visible)
            j++;
        if (j >= (maxCount /*+(m_containsROWIDInfo ? 1 : 0)*/)) {
            //ERR!
            break;
        }
        Field *f = (i >= m_fieldCount) ? 0 : m_fieldsExpanded->at(j)->field;
//  PreDrvDbg << "col=" << (col ? *col : 0);
        (*data)[i] = d->getValue(f, i);
    }
    return true;
}

QVariant SQLiteCursor::value(uint i)
{
    if (i > (m_fieldCount - 1)) //range checking
        return QVariant();
//! @todo allow disable range checking! - performance reasons
    Predicate::Field *f = (m_fieldsExpanded && i < (uint)m_fieldsExpanded->count())
                       ? m_fieldsExpanded->at(i)->field : 0;
    return d->getValue(f, i); //, i==m_logicalFieldCount/*ROWID*/);
}

QString SQLiteCursor::serverResultName() const
{
    return SQLiteConnectionInternal::serverResultName(m_result.serverResultCode());
}

void SQLiteCursor::storeResult()
{
    m_result.setServerMessage(
        QLatin1String( (d->data && m_result.serverResultCode() != SQLITE_OK) ? sqlite3_errmsg(d->data) : 0 ));
}

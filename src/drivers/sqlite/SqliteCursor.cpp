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

#include "SqliteCursor.h"

#include "SqliteConnection.h"
#include "SqliteConnection_p.h"
#include "sqlite_debug.h"

#include "KDbDriver.h"
#include "KDbError.h"
#include "KDbRecordData.h"
#include "KDbUtils.h"

#include <QVector>
#include <QDateTime>
#include <QByteArray>

//! safer interpretations of boolean values for SQLite
static bool sqliteStringToBool(const QString& s)
{
    return 0 == s.compare(QLatin1String("yes"), Qt::CaseInsensitive)
        || (0 != s.compare(QLatin1String("no"), Qt::CaseInsensitive) && s != QLatin1String("0"));
}

//----------------------------------------------------

class SqliteCursorData : public SqliteConnectionInternal
{
public:
    explicit SqliteCursorData(SqliteConnection* conn)
            : SqliteConnectionInternal(conn)
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
    int cols_pointers_mem_size; //!< size of record's array of pointers to values
    QVector<const char**> records; //!< buffer data

    inline QVariant getValue(KDbField *f, int i) {
        int type = sqlite3_column_type(prepared_st_handle, i);
        if (type == SQLITE_NULL) {
            return QVariant();
        } else if (!f || type == SQLITE_TEXT) {
//! @todo support for UTF-16
            QString text(QString::fromUtf8(
                (const char*)sqlite3_column_text(prepared_st_handle, i),
                 sqlite3_column_bytes(prepared_st_handle, i)));
            if (!f) {
                return text;
            }
            const KDbField::Type t = f->type(); // cache: evaluating type of expressions can be expensive
            if (KDbField::isTextType(t)) {
                return text;
            } else if (t == KDbField::Date) {
                return QDate::fromString(text, Qt::ISODate);
            } else if (t == KDbField::Time) {
                //QDateTime - a hack needed because QVariant(QTime) has broken isNull()
                return KDbUtils::stringToHackedQTime(text);
            } else if (t == KDbField::DateTime) {
                if (text.length() > 10) {
                    text[10] = QLatin1Char('T'); //for ISODate compatibility
                }
                return QDateTime::fromString(text, Qt::ISODate);
            } else if (t == KDbField::Boolean) {
                return sqliteStringToBool(text);
            } else {
                return QVariant(); //!< @todo
            }
        } else if (type == SQLITE_INTEGER) {
            const KDbField::Type t = f->type();  // cache: evaluating type of expressions can be expensive
            if (t == KDbField::BigInteger) {
                return QVariant(qint64(sqlite3_column_int64(prepared_st_handle, i)));
            } else if (KDbField::isIntegerType(t)) {
                return QVariant(sqlite3_column_int(prepared_st_handle, i));
            } else if (t == KDbField::Boolean) {
                return sqlite3_column_int(prepared_st_handle, i) != 0;
            } else if (KDbField::isFPNumericType(t)) { //WEIRD, YEAH?
                return QVariant(double(sqlite3_column_int(prepared_st_handle, i)));
            } else {
                return QVariant(); //!< @todo
            }
        } else if (type == SQLITE_FLOAT) {
            const KDbField::Type t = f->type(); // cache: evaluating type of expressions can be expensive
            if (KDbField::isFPNumericType(t)) {
                return QVariant(sqlite3_column_double(prepared_st_handle, i));
            } else if (t == KDbField::BigInteger) {
                return QVariant(qint64(sqlite3_column_int64(prepared_st_handle, i)));
            } else if (KDbField::isIntegerType(t)) {
                return QVariant(int(sqlite3_column_double(prepared_st_handle, i)));
            } else {
                return QVariant(); //!< @todo
            }
        } else if (type == SQLITE_BLOB) {
            if (f && f->type() == KDbField::BLOB) {
//! @todo efficient enough?
                return QByteArray((const char*)sqlite3_column_blob(prepared_st_handle, i),
                                  sqlite3_column_bytes(prepared_st_handle, i));
            } else
                return QVariant(); //!< @todo
        }
        return QVariant();
    }
};

SqliteCursor::SqliteCursor(SqliteConnection* conn, const KDbEscapedString& sql, int options)
        : KDbCursor(conn, sql, options)
        , d(new SqliteCursorData(conn))
{
    d->data = static_cast<SqliteConnection*>(conn)->d->data;
}

SqliteCursor::SqliteCursor(SqliteConnection* conn, KDbQuerySchema* query, int options)
        : KDbCursor(conn, query, options)
        , d(new SqliteCursorData(conn))
{
    d->data = static_cast<SqliteConnection*>(conn)->d->data;
}

SqliteCursor::~SqliteCursor()
{
    close();
    delete d;
}

bool SqliteCursor::drv_open(const KDbEscapedString& sql)
{
    //! @todo decode
    if (! d->data) {
        // this may as example be the case if SqliteConnection::drv_useDatabase()
        // wasn't called before. Normaly sqlite_compile/sqlite3_prepare
        // should handle it, but it crashes in in sqlite3SafetyOn at util.c:786
        sqliteWarning() << "SqliteCursor::drv_open(): Database handle undefined.";
        return false;
    }

    int res = sqlite3_prepare(
                 d->data,            /* Database handle */
                 sql.constData(),       /* SQL statement, UTF-8 encoded */
                 sql.length(),             /* Length of zSql in bytes. */
                 &d->prepared_st_handle,  /* OUT: Statement handle */
                 0/*const char **pzTail*/     /* OUT: Pointer to unused portion of zSql */
             );
    if (res != SQLITE_OK) {
        m_result.setServerErrorCode(res);
        storeResult();
        return false;
    }
    if (isBuffered()) {
//! @todo manage size dynamically
        d->records.resize(128);
    }

    return true;
}

bool SqliteCursor::drv_close()
{
    int res = sqlite3_finalize(d->prepared_st_handle);
    if (res != SQLITE_OK) {
        m_result.setServerErrorCode(res);
        storeResult();
        return false;
    }
    return true;
}

void SqliteCursor::drv_getNextRecord()
{
    int res = sqlite3_step(d->prepared_st_handle);
    if (res == SQLITE_ROW) {
        m_fetchResult = FetchOK;
        m_fieldCount = sqlite3_data_count(d->prepared_st_handle);
//#else //for SQLITE3 data fetching is delayed. Now we even do not take field count information
//      // -- just set a flag that we've a data not fetched but available
        m_fieldsToStoreInRecord = m_fieldCount;
    }
    else {
        if (res == SQLITE_DONE) {
            m_fetchResult = FetchEnd;
        } else {
            m_result.setServerErrorCode(res);
            m_fetchResult = FetchError;
        }
    }

    //debug
    /*
      if ((int)m_result == (int)FetchOK && d->curr_coldata) {
        for (int i=0;i<m_fieldCount;i++) {
          sqliteDebug()<<"col."<< i<<": "<< d->curr_colname[i]<<" "<< d->curr_colname[m_fieldCount+i]
          << " = " << (d->curr_coldata[i] ? QString::fromLocal8Bit(d->curr_coldata[i]) : "(NULL)");
        }
    //  sqliteDebug() << m_fieldCount << "col(s) fetched";
      }*/
}

void SqliteCursor::drv_appendCurrentRecordToBuffer()
{
// sqliteDebug();
    if (!d->curr_coldata)
        return;
    if (!d->cols_pointers_mem_size)
        d->cols_pointers_mem_size = m_fieldCount * sizeof(char*);
    const char **record = (const char**)malloc(d->cols_pointers_mem_size);
    const char **src_col = d->curr_coldata;
    const char **dest_col = record;
    for (int i = 0; i < m_fieldCount; i++, src_col++, dest_col++) {
//  sqliteDebug() << i <<": '" << *src_col << "'";
//  sqliteDebug() << "src_col: " << src_col;
        *dest_col = *src_col ? strdup(*src_col) : 0;
    }
    d->records[m_records_in_buf] = record;
// sqliteDebug() << "ok.";
}

void SqliteCursor::drv_bufferMovePointerNext()
{
    d->curr_coldata++; //move to next record in the buffer
}

void SqliteCursor::drv_bufferMovePointerPrev()
{
    d->curr_coldata--; //move to prev record in the buffer
}

//compute a place in the buffer that contain next record's data
//and move internal buffer pointer to that place
void SqliteCursor::drv_bufferMovePointerTo(qint64 at)
{
    d->curr_coldata = d->records.at(at);
}

void SqliteCursor::drv_clearBuffer()
{
    if (d->cols_pointers_mem_size > 0) {
        const int records_in_buf = m_records_in_buf;
        const char ***r_ptr = d->records.data();
        for (int i = 0; i < records_in_buf; i++, r_ptr++) {
            const char **field_data = *r_ptr;
            for (int col = 0; col < m_fieldCount; col++, field_data++) {
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
const char *** SqliteCursor::bufferData()
{
  if (!isBuffered())
    return 0;
  return m_records.data();
}*/

const char ** SqliteCursor::recordData() const
{
    return d->curr_coldata;
}

bool SqliteCursor::drv_storeCurrentRecord(KDbRecordData* data) const
{
    if (!m_visibleFieldsExpanded) {//simple version: without types
        for (int i = 0; i < m_fieldCount; i++) {
            (*data)[i] = QString::fromUtf8(
                            (const char*)sqlite3_column_text(d->prepared_st_handle, i),
                            sqlite3_column_bytes(d->prepared_st_handle, i));
        }
        return true;
    }
    for (int i = 0; i < m_fieldCount; ++i) {
        KDbField *f = m_visibleFieldsExpanded->at(i)->field;
//  sqliteDebug() << "col=" << (col ? *col : 0);
        (*data)[i] = d->getValue(f, i);
    }
    return true;
}

QVariant SqliteCursor::value(int i)
{
    if (i < 0 || i > (m_fieldCount - 1)) //range checking
        return QVariant();
//! @todo allow disable range checking! - performance reasons
    KDbField *f = (m_visibleFieldsExpanded && i < m_visibleFieldsExpanded->count())
                  ? m_visibleFieldsExpanded->at(i)->field : 0;
    return d->getValue(f, i); //, i==m_logicalFieldCount/*ROWID*/);
}

QString SqliteCursor::serverResultName() const
{
    return SqliteConnectionInternal::serverResultName(m_result.serverErrorCode());
}

void SqliteCursor::storeResult()
{
    d->storeResult(&m_result);
}

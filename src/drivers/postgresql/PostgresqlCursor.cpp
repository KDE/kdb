/* This file is part of the KDE project
   Copyright (C) 2003 Adam Pigg <adam@piggz.co.uk>
   Copyright (C) 2010 Jarosław Staniek <staniek@kde.org>

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

#include "PostgresqlCursor.h"
#include "PostgresqlConnection.h"
#include "PostgresqlConnection_p.h"
#include "PostgresqlDriver.h"

#include <Predicate/Error>
#include <Predicate/Global>


#include <QtDebug>

using namespace Predicate;

// Constructor based on query statement
PostgresqlCursor::PostgresqlCursor(Predicate::Connection* conn, const EscapedString& statement, uint options)
        : Cursor(conn, statement, options)
        , m_numRows(0)
        , d(new PostgresqlCursorData(conn))
{
    m_options |= Buffered;
    //m_implicityStarted = false;
}

//==================================================================================
//Constructor base on query object
PostgresqlCursor::PostgresqlCursor(Connection* conn, QuerySchema* query, uint options)
        : Cursor(conn, query, options)
        , d(new PostgresqlCursorData(conn))
{
    m_options |= Buffered;
    //m_implicityStarted = false;
}

//==================================================================================
//Destructor
PostgresqlCursor::~PostgresqlCursor()
{
    close();
    delete d;
}


//==================================================================================
//Create a cursor result set
bool PostgresqlCursor::drv_open(const EscapedString& sql)
{
    if (!d->executeSQL(sql, PGRES_TUPLES_OK))
        return false;

    m_fieldsToStoreInRecord = PQnfields(d->res);
    m_fieldCount = m_fieldsToStoreInRecord - (m_containsRecordIdInfo ? 1 : 0);
    m_numRows = PQntuples(d->res);
    m_records_in_buf = m_numRows;
    m_buffering_completed = true;

    // get real types for all fields
    PostgresqlDriver* drv = static_cast<PostgresqlDriver*>(m_conn->driver());
    
    m_realTypes.resize(m_fieldsToStoreInRecord);
    for (int i = 0; i < int(m_fieldsToStoreInRecord); i++) {
        const int pqtype = PQftype(d->res, i);
        m_realTypes[i] = drv->pgsqlToVariantType(pqtype);
    }
    return true;
}

//==================================================================================
//Delete objects
bool PostgresqlCursor::drv_close()
{
    PQclear(d->res);
    return true;
}

//==================================================================================
//Gets the next record...does not need to do much, just return fetchend if at end of result set
void PostgresqlCursor::drv_getNextRecord()
{
    if (at() >= qint64(m_numRows)) {
        m_fetchResult = FetchEnd;
    }
    else if (at() < 0) {
        // control will reach here only when at() < 0 ( which is usually -1 )
        // -1 is same as "1 beyond the End"
        m_fetchResult = FetchEnd;
    }
    else { // 0 <= at() < m_numRows
        m_fetchResult = FetchOK;
    }
}

//==================================================================================
//Check the current position is within boundaries
#if 0 
void PostgresqlCursor::drv_getPrevRecord()
{
// PreDrvDbg;

    if (at() < m_res->size() && at() >= 0) {
        m_fetchResult = FetchOK;
    } else if (at() >= m_res->size()) {
        m_fetchResult = FetchEnd;
    } else {
        m_fetchResult = FetchError;
    }
}
#endif

//==================================================================================
//Return the value for a given column for the current record
QVariant PostgresqlCursor::value(uint pos)
{
    if (pos < m_fieldCount)
        return pValue(pos);
    else
        return QVariant();
}

#if 0
inline QVariant pgsqlCStrToVariant(const pqxx::result::field& r)
{
    switch (r.type()) {
    case BOOLOID:
        return QString::fromLatin1(r.c_str(), r.size()) == "true"; //TODO check formatting
    case INT2OID:
    case INT4OID:
    case INT8OID:
        return r.as(int());
    case FLOAT4OID:
    case FLOAT8OID:
    case NUMERICOID:
        return r.as(double());
    case DATEOID:
        return QString::fromUtf8(r.c_str(), r.size()); //TODO check formatting
    case TIMEOID:
        return QString::fromUtf8(r.c_str(), r.size()); //TODO check formatting
    case TIMESTAMPOID:
        return QString::fromUtf8(r.c_str(), r.size()); //TODO check formatting
    case BYTEAOID:
        return Predicate::pgsqlByteaToByteArray(r.c_str(), r.size());
    case BPCHAROID:
    case VARCHAROID:
    case TEXTOID:
        return QString::fromUtf8(r.c_str(), r.size()); //utf8?
    default:
        return QString::fromUtf8(r.c_str(), r.size()); //utf8?
    }
}
#endif

inline bool hasTimeZone(const QString& s)
{
    return s.at(s.length() - 3) == QLatin1Char('+') || s.at(s.length() - 3) == QLatin1Char('-');
}

//==================================================================================
//Return the value for a given column for the current record - Private const version
QVariant PostgresqlCursor::pValue(uint pos) const
{
    //not needed: if (pos >= m_fieldsToStoreInRecord) {
//  PreDrvWarn << "PostgresqlCursor::value - ERROR: requested position is greater than the number of fields";
        //return QVariant();
    //}
    const qint64 row = at();

#if 0
    Predicate::Field *f = (m_fieldsExpanded && pos < qMin((uint)m_fieldsExpanded->count(), m_fieldCount))
                       ? m_fieldsExpanded->at(pos)->field : 0;
#endif
// PreDrvDbg << "pos:" << pos;

    const QVariant::Type type = m_realTypes[pos];
    if (PQgetisnull(d->res, row, pos)) {
        return QVariant(type);
    }
    const char *data = PQgetvalue(d->res, row, pos);
    const int len = PQgetlength(d->res, row, pos);

//    if (f) { //We probably have a schema type query so can use kexi to determine the row type
    switch (type) { // from most to least frequently used types:
    case QVariant::String:
        return d->unicode ? QString::fromUtf8(data, len) : QString::fromLatin1(data, len);
    case QVariant::Int:
        return atoi(data); // the fastest way
    case QVariant::Bool:
        return bool(data[0] == 't');
    case QVariant::LongLong:
        if (data[0] == '-')
            return QByteArray::fromRawData(data, len).toLongLong();
        else
            return QByteArray::fromRawData(data, len).toULongLong();
    case QVariant::Double:
//! @todo support equivalent of QSql::NumericalPrecisionPolicy, especially for NUMERICOID
        return QByteArray::fromRawData(data, len).toDouble();
    case QVariant::Date:
        if (len == 0) {
            return QVariant(QDate());
        } else {
            return QVariant(QDate::fromString(QLatin1String(QByteArray::fromRawData(data, len)), Qt::ISODate));
        }
    case QVariant::Time:
        if (len == 0) {
            return QVariant(QTime());
        } else {
            QString s(QString::fromLatin1(data, len));
            if (hasTimeZone(s)) {
                s.chop(3); // skip timezone
                return QVariant(QTime::fromString(s, Qt::ISODate));
            }
            return QVariant(QTime::fromString(s, Qt::ISODate));
        }
    case QVariant::DateTime:
        if (len < 10 /*ISO Date*/) {
            return QVariant(QDateTime());
        } else {
            QString s(QString::fromLatin1(data, len));
            if (hasTimeZone(s)) {
                s.chop(3); // skip timezone
                if (s.isEmpty())
                    return QVariant(QDateTime());
            }
            if (s.at(s.length() - 3).isPunct()) // fix ms, should be three digits
                s += QLatin1Char('0');
            return QVariant(QDateTime::fromString(s, Qt::ISODate));
        }
    case QVariant::ByteArray:
        {
            size_t unescapedLen;
            unsigned char *unescapedData = PQunescapeBytea((const unsigned char*)data, &unescapedLen);
            const QByteArray result((const char*)unescapedData, unescapedLen);
//! @todo avoid deep copy; QByteArray does not allow passing ownership of data; maybe copy PQunescapeBytea code?
            PQfreemem(unescapedData);
            return QVariant(result);
        }
    default:
        qWarning() << "PostgresqlCursor::pValue() data type?";
    }
    return QVariant();

#if 0
        if ((f->isIntegerType()) || (/*ROWID*/!f && m_containsRecordIdInfo && pos == m_fieldCount)) {
            return (*m_res)[at()][pos].as(int());
        } else if (f->isTextType()) {
            return QString::fromUtf8((*m_res)[at()][pos].c_str()); //utf8?
        } else if (f->isFPNumericType()) {
            return (*m_res)[at()][pos].as(double());
        } else if (f->type() == Field::Boolean) {
            return QString((*m_res)[at()][pos].c_str()).toLower() == "t" ? QVariant(true) : QVariant(false);
        } else if (f->typeGroup() == Field::BLOBGroup) {
//   PreDrvDbg << r.name() << ", " << r.c_str() << ", " << r.type() << ", " << r.size();
            return ::pgsqlByteaToByteArray((*m_res)[at()][pos]);
        } else {
            return pgsqlCStrToVariant((*m_res)[at()][pos]);
        }
    } else { // We probably have a raw type query so use pqxx to determin the column type
        return pgsqlCStrToVariant((*m_res)[at()][pos]);
    }

    return QString::fromUtf8((*m_res)[at()][pos].c_str(), (*m_res)[at()][pos].size()); //utf8?
#endif
}

//==================================================================================
//Return the current record as a char**
const char** PostgresqlCursor::recordData() const
{
    //! @todo
    return 0;
}

//==================================================================================
//Store the current record in [data]
bool PostgresqlCursor::drv_storeCurrentRecord(RecordData* data) const
{
// PreDrvDbg << "POSITION IS" << (long)m_at;

// const uint realCount = m_fieldCount + (m_containsRecordIdInfo ? 1 : 0);
//not needed data.resize(realCount);

    for (uint i = 0; i < m_fieldsToStoreInRecord; i++)
        (*data)[i] = pValue(i);
    return true;
}

//==================================================================================
//
/*void PostgresqlCursor::drv_clearServerResult()
{
//! @todo PostgresqlCursor: stuff with server results
}*/

//==================================================================================
//Add the current record to the internal buffer
//Implementation required but no need in this driver
//Result set is a buffer so do not need another
void PostgresqlCursor::drv_appendCurrentRecordToBuffer()
{

}

//==================================================================================
//Move internal pointer to internal buffer +1
//Implementation required but no need in this driver
void PostgresqlCursor::drv_bufferMovePointerNext()
{

}

//==================================================================================
//Move internal pointer to internal buffer -1
//Implementation required but no need in this driver
void PostgresqlCursor::drv_bufferMovePointerPrev()
{

}

//==================================================================================
//Move internal pointer to internal buffer to N
//Implementation required but no need in this driver
void PostgresqlCursor::drv_bufferMovePointerTo(qint64 to)
{
    Q_UNUSED(to);
}


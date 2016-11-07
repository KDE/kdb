/* This file is part of the KDE project
   Copyright (C) 2003 Adam Pigg <adam@piggz.co.uk>
   Copyright (C) 2016 Jarosław Staniek <staniek@kde.org>

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
#include "postgresql_debug.h"

#include "KDbError.h"
#include "KDbGlobal.h"
#include "KDbRecordData.h"

// Constructor based on query statement
PostgresqlCursor::PostgresqlCursor(KDbConnection* conn, const KDbEscapedString& sql, int options)
        : KDbCursor(conn, sql, options)
        , m_numRows(0)
        , d(new PostgresqlCursorData(conn))
{
    m_options |= Buffered;
}

//==================================================================================
//Constructor base on query object
PostgresqlCursor::PostgresqlCursor(KDbConnection* conn, KDbQuerySchema* query, int options)
        : KDbCursor(conn, query, options)
        , m_numRows(0)
        , d(new PostgresqlCursorData(conn))
{
    m_options |= Buffered;
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
bool PostgresqlCursor::drv_open(const KDbEscapedString& sql)
{
    d->res = d->executeSQL(sql);
    d->resultStatus = PQresultStatus(d->res);
    if (d->resultStatus != PGRES_TUPLES_OK && d->resultStatus != PGRES_COMMAND_OK) {
        storeResultAndClear(&d->res, d->resultStatus);
        return false;
    }
    m_fieldsToStoreInRecord = PQnfields(d->res);
    m_fieldCount = m_fieldsToStoreInRecord - (containsRecordIdInfo() ? 1 : 0);
    m_numRows = PQntuples(d->res);
    m_records_in_buf = m_numRows;
    m_buffering_completed = true;

    // get real types for all fields
    PostgresqlDriver* drv = static_cast<PostgresqlDriver*>(connection()->driver());

    m_realTypes.resize(m_fieldsToStoreInRecord);
    m_realLengths.resize(m_fieldsToStoreInRecord);
    for (int i = 0; i < int(m_fieldsToStoreInRecord); i++) {
        const int pqtype = PQftype(d->res, i);
        const int pqfmod = PQfmod(d->res, i);
        m_realTypes[i] = drv->pgsqlToKDbType(pqtype, pqfmod, &m_realLengths[i]);
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
QVariant PostgresqlCursor::value(int pos)
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
        return QString::fromLatin1(r.c_str(), r.size()) == "true"; //!< @todo check formatting
    case INT2OID:
    case INT4OID:
    case INT8OID:
        return r.as(int());
    case FLOAT4OID:
    case FLOAT8OID:
    case NUMERICOID:
        return r.as(double());
    case DATEOID:
        return QString::fromUtf8(r.c_str(), r.size()); //!< @todo check formatting
    case TIMEOID:
        return QString::fromUtf8(r.c_str(), r.size()); //!< @todo check formatting
    case TIMESTAMPOID:
        return QString::fromUtf8(r.c_str(), r.size()); //!< @todo check formatting
    case BYTEAOID:
        return KDb::pgsqlByteaToByteArray(r.c_str(), r.size());
    case BPCHAROID:
    case VARCHAROID:
    case TEXTOID:
        return QString::fromUtf8(r.c_str(), r.size()); //utf8?
    default:
        return QString::fromUtf8(r.c_str(), r.size()); //utf8?
    }
}
#endif

static inline bool hasTimeZone(const QString& s)
{
    return s.at(s.length() - 3) == QLatin1Char('+') || s.at(s.length() - 3) == QLatin1Char('-');
}

static inline QVariant convertToKDbType(bool convert, const QVariant &value, KDbField::Type kdbType)
{
    return (convert && kdbType != KDbField::InvalidType)
            ? KDbField::convertToType(value, kdbType) : value;
}

static inline QTime timeFromData(const char *data, int len)
{
    if (len == 0) {
        return QTime();
    }
    QString s(QString::fromLatin1(data, len));
    if (hasTimeZone(s)) {
        s.chop(3); // skip timezone
        return QTime::fromString(s, Qt::ISODate);
    }
    return QTime::fromString(s, Qt::ISODate);
}

static inline QDateTime dateTimeFromData(const char *data, int len)
{
    if (len < 10 /*ISO Date*/) {
        return QDateTime();
    }
    QString s(QString::fromLatin1(data, len));
    if (hasTimeZone(s)) {
        s.chop(3); // skip timezone
        if (s.isEmpty()) {
            return QDateTime();
        }
    }
    if (s.at(s.length() - 3).isPunct()) { // fix ms, should be three digits
        s += QLatin1Char('0');
    }
    return QDateTime::fromString(s, Qt::ISODate);
}

static inline QByteArray byteArrayFromData(const char *data)
{
    size_t unescapedLen;
    unsigned char *unescapedData = PQunescapeBytea((const unsigned char*)data, &unescapedLen);
    const QByteArray result((const char*)unescapedData, unescapedLen);
    //! @todo avoid deep copy; QByteArray does not allow passing ownership of data; maybe copy PQunescapeBytea code?
    PQfreemem(unescapedData);
    return result;
}

//==================================================================================
//Return the value for a given column for the current record - Private const version
QVariant PostgresqlCursor::pValue(int pos) const
{
//  postgresqlWarning() << "PostgresqlCursor::value - ERROR: requested position is greater than the number of fields";
    const qint64 row = at();

    KDbField *f = (m_visibleFieldsExpanded && pos < qMin(m_visibleFieldsExpanded->count(), m_fieldCount))
                       ? m_visibleFieldsExpanded->at(pos)->field() : nullptr;
// postgresqlDebug() << "pos:" << pos;

    const KDbField::Type type = m_realTypes[pos];
    const KDbField::Type kdbType = f ? f->type() : KDbField::InvalidType; // cache: evaluating type of expressions can be expensive
    if (PQgetisnull(d->res, row, pos) || kdbType == KDbField::Null) {
        return QVariant();
    }
    const char *data = PQgetvalue(d->res, row, pos);
    int len = PQgetlength(d->res, row, pos);

    switch (type) { // from most to least frequently used types:
    case KDbField::Text:
    case KDbField::LongText: {
        const int maxLength = m_realLengths[pos];
        if (maxLength > 0) {
            len = qMin(len, maxLength);
        }
        return convertToKDbType(!KDbField::isTextType(kdbType),
                                d->unicode ? QString::fromUtf8(data, len) : QString::fromLatin1(data, len),
                                kdbType);
    }
    case KDbField::Integer:
        return convertToKDbType(!KDbField::isIntegerType(kdbType),
                                atoi(data), // the fastest way
                                kdbType);
    case KDbField::Boolean:
        return convertToKDbType(kdbType != KDbField::Boolean,
                                bool(data[0] == 't'),
                                kdbType);
    case KDbField::BigInteger:
        return convertToKDbType(kdbType != KDbField::BigInteger,
                                (data[0] == '-') ? QByteArray::fromRawData(data, len).toLongLong()
                                                 : QByteArray::fromRawData(data, len).toULongLong(),
                                kdbType);
    case KDbField::Double:
//! @todo support equivalent of QSql::NumericalPrecisionPolicy, especially for NUMERICOID
        return convertToKDbType(!KDbField::isFPNumericType(kdbType),
                                QByteArray::fromRawData(data, len).toDouble(),
                                kdbType);
    case KDbField::Date:
        return convertToKDbType(kdbType != KDbField::Date,
                                (len == 0) ? QVariant(QDate())
                                           : QVariant(QDate::fromString(QLatin1String(QByteArray::fromRawData(data, len)), Qt::ISODate)),
                                kdbType);
    case KDbField::Time:
        return convertToKDbType(kdbType != KDbField::Time,
                                timeFromData(data, len),
                                kdbType);
    case KDbField::DateTime:
        return convertToKDbType(kdbType != KDbField::DateTime,
                                dateTimeFromData(data, len),
                                kdbType);
    case KDbField::BLOB:
        return convertToKDbType(kdbType != KDbField::BLOB,
                                byteArrayFromData(data),
                                kdbType);
    default:
        postgresqlWarning() << "PostgresqlCursor::pValue() data type?";
    }
    return QVariant();
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
bool PostgresqlCursor::drv_storeCurrentRecord(KDbRecordData* data) const
{
// postgresqlDebug() << "POSITION IS" << (long)m_at;
    for (int i = 0; i < m_fieldsToStoreInRecord; i++)
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

void PostgresqlCursor::storeResultAndClear(PGresult **pgResult, ExecStatusType execStatus)
{
    d->storeResultAndClear(&m_result, pgResult, execStatus);
}

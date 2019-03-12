/* This file is part of the KDE project
   Copyright (C) 2005-2012 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "SqlitePreparedStatement.h"
#include "KDbPreparedStatement.h"
#include "sqlite_debug.h"

SqlitePreparedStatement::SqlitePreparedStatement(SqliteConnectionInternal* conn)
        : KDbPreparedStatementInterface()
        , SqliteConnectionInternal(conn->connection)
{
    data_owned = false;
    data = conn->data; //copy
}

SqlitePreparedStatement::~SqlitePreparedStatement()
{
}

bool SqlitePreparedStatement::prepare(const KDbEscapedString& sql)
{
    m_sqlResult = connection->prepareSql(sql);
    m_result = connection->result();
    return m_sqlResult && !m_result.isError();
}

bool SqlitePreparedStatement::bindValue(KDbField *field, const QVariant& value, int par)
{
    if (value.isNull()) {
        //no value to bind or the value is null: bind NULL
        int res = sqlite3_bind_null(sqlResult()->prepared_st, par);
        if (res != SQLITE_OK) {
            m_result.setServerErrorCode(res);
            storeResult(&m_result);
            return false;
        }
        return true;
    }
    if (field->isTextType()) {
        //! @todo optimize: make a static copy so SQLITE_STATIC can be used
        const QByteArray utf8String(value.toString().toUtf8());
        int res = sqlite3_bind_text(sqlResult()->prepared_st, par,
                                    utf8String.constData(), utf8String.length(), SQLITE_TRANSIENT /*??*/);
        if (res != SQLITE_OK) {
            m_result.setServerErrorCode(res);
            storeResult(&m_result);
            return false;
        }
        return true;
    }

    switch (field->type()) {
    case KDbField::Byte:
    case KDbField::ShortInteger:
    case KDbField::Integer: {
        //! @todo what about unsigned > INT_MAX ?
        bool ok;
        const int intValue = value.toInt(&ok);
        if (ok) {
            int res = sqlite3_bind_int(sqlResult()->prepared_st, par, intValue);
            if (res != SQLITE_OK) {
                m_result.setServerErrorCode(res);
                storeResult(&m_result);
                return false;
            }
        } else {
            int res = sqlite3_bind_null(sqlResult()->prepared_st, par);
            if (res != SQLITE_OK) {
                m_result.setServerErrorCode(res);
                storeResult(&m_result);
                return false;
            }
        }
        break;
    }
    case KDbField::Float:
    case KDbField::Double: {
        int res = sqlite3_bind_double(sqlResult()->prepared_st, par, value.toDouble());
        if (res != SQLITE_OK) {
            m_result.setServerErrorCode(res);
            storeResult(&m_result);
            return false;
        }
        break;
    }
    case KDbField::BigInteger: {
        //! @todo what about unsigned > LLONG_MAX ?
        bool ok;
        const qint64 int64Value = value.toLongLong(&ok);
        if (ok) {
            int res = sqlite3_bind_int64(sqlResult()->prepared_st, par, int64Value);
            if (res != SQLITE_OK) {
                m_result.setServerErrorCode(res);
                storeResult(&m_result);
                return false;
            }
        } else {
            int res = sqlite3_bind_null(sqlResult()->prepared_st, par);
            if (res != SQLITE_OK) {
                m_result.setServerErrorCode(res);
                storeResult(&m_result);
                return false;
            }
        }
        break;
    }
    case KDbField::Boolean: {
        int res = sqlite3_bind_text(sqlResult()->prepared_st, par, value.toBool() ? "1" : "0",
                                    1, SQLITE_TRANSIENT /*??*/);
        if (res != SQLITE_OK) {
            m_result.setServerErrorCode(res);
            storeResult(&m_result);
            return false;
        }
        break;
    }
    case KDbField::Time: {
        int res = sqlite3_bind_text(sqlResult()->prepared_st, par,
                                    qPrintable(KDbUtils::toISODateStringWithMs(value.toTime())),
                                    QLatin1String("HH:MM:SS").size(), SQLITE_TRANSIENT /*??*/);
        if (res != SQLITE_OK) {
            m_result.setServerErrorCode(res);
            storeResult(&m_result);
            return false;
        }
        break;
    }
    case KDbField::Date: {
        int res = sqlite3_bind_text(sqlResult()->prepared_st, par,
                                    qPrintable(value.toDate().toString(Qt::ISODate)),
                                    QLatin1String("YYYY-MM-DD").size(), SQLITE_TRANSIENT /*??*/);
        if (res != SQLITE_OK) {
            m_result.setServerErrorCode(res);
            storeResult(&m_result);
            return false;
        }
        break;
    }
    case KDbField::DateTime: {
        int res = sqlite3_bind_text(sqlResult()->prepared_st, par,
                                qPrintable(KDbUtils::toISODateStringWithMs(value.toDateTime())),
                                QLatin1String("YYYY-MM-DDTHH:MM:SS").size(), SQLITE_TRANSIENT /*??*/);
        if (res != SQLITE_OK) {
            m_result.setServerErrorCode(res);
            storeResult(&m_result);
            return false;
        }
        break;
    }
    case KDbField::BLOB: {
        const QByteArray byteArray(value.toByteArray());
        int res = sqlite3_bind_blob(sqlResult()->prepared_st, par,
                                    byteArray.constData(), byteArray.size(), SQLITE_TRANSIENT /*??*/);
        if (res != SQLITE_OK) {
            m_result.setServerErrorCode(res);
            storeResult(&m_result);
            return false;
        }
        break;
    }
    default: {
        sqliteWarning() << "unsupported field type:"
                << field->type() << "- NULL value bound to column #" << par;
        int res = sqlite3_bind_null(sqlResult()->prepared_st, par);
        if (res != SQLITE_OK) {
            m_result.setServerErrorCode(res);
            storeResult(&m_result);
            return false;
        }
    }
    } //switch
    return true;
}

QSharedPointer<KDbSqlResult> SqlitePreparedStatement::execute(
    KDbPreparedStatement::Type type,
    const KDbField::List& selectFieldList,
    KDbFieldList* insertFieldList,
    const KDbPreparedStatementParameters& parameters)
{
    Q_UNUSED(insertFieldList);
    if (!sqlResult()->prepared_st) {
        return QSharedPointer<KDbSqlResult>();
    }

    int par = 1; // par.index counted from 1
    KDbField::ListIterator itFields(selectFieldList.constBegin());
    for (QList<QVariant>::ConstIterator it = parameters.constBegin();
         itFields != selectFieldList.constEnd();
         it += (it == parameters.constEnd() ? 0 : 1), ++itFields, par++)
    {
        if (!bindValue(*itFields, it == parameters.constEnd() ? QVariant() : *it, par))
            return QSharedPointer<KDbSqlResult>();
    }

    //real execution
    const int res = sqlite3_step(sqlResult()->prepared_st);
    if (type == KDbPreparedStatement::InsertStatement) {
        const bool ok = res == SQLITE_DONE;
        if (ok) {
            m_result = KDbResult();
        } else {
            m_result.setServerErrorCode(res);
            storeResult(&m_result);
            sqliteWarning() << m_result << QString::fromLatin1(sqlite3_sql(sqlResult()->prepared_st));
        }
        (void)sqlite3_reset(sqlResult()->prepared_st);
        return m_sqlResult;
    }
    else if (type == KDbPreparedStatement::SelectStatement) {
        //! @todo fetch result
        const bool ok = res == SQLITE_ROW;
        storeResult(&m_result);
        if (ok) {
            m_result = KDbResult();
        } else {
            m_result.setServerErrorCode(res);
            storeResult(&m_result);
            sqliteWarning() << m_result << QString::fromLatin1(sqlite3_sql(sqlResult()->prepared_st));
        }
        (void)sqlite3_reset(sqlResult()->prepared_st);
        return m_sqlResult;
    }
    return QSharedPointer<KDbSqlResult>();
}

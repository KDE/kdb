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


SQLitePreparedStatement::SQLitePreparedStatement(ConnectionInternal* conn)
        : KDbPreparedStatementInterface()
        , SQLiteConnectionInternal(conn->connection)
        , m_handle(0)
{
    data_owned = false;
    data = dynamic_cast<SQLiteConnectionInternal&>(*conn).data; //copy
}

SQLitePreparedStatement::~SQLitePreparedStatement()
{
    sqlite3_finalize(m_handle);
    m_handle = 0;
}

bool SQLitePreparedStatement::prepare(const KDbEscapedString& sql)
{
    m_result.setServerResultCode(
        sqlite3_prepare(
            data,                    /* Database handle */
            sql.toByteArray(),       /* SQL statement, UTF-8 encoded */
            sql.length(),            /* Length of zSql in bytes. */
            &m_handle,               /* OUT: Statement handle */
            0                        /* OUT: Pointer to unused portion of zSql */
        )
    );
    //! @todo copy error msg
    return m_result.serverResultCode() == SQLITE_OK;
}

bool SQLitePreparedStatement::bindValue(KDbField *field, const QVariant& value, int par)
{
    if (value.isNull()) {
        //no value to bind or the value is null: bind NULL
        m_result.setServerResultCode(sqlite3_bind_null(m_handle, par));
        if (SQLITE_OK != m_result.serverResultCode()) {
            //! @todo msg?
            return false;
        }
        return true;
    }
    if (field->isTextType()) {
        //! @todo optimize: make a static copy so SQLITE_STATIC can be used
        const QByteArray utf8String(value.toString().toUtf8());
        m_result.setServerResultCode(
            sqlite3_bind_text(m_handle, par,
                              (const char*)utf8String, utf8String.length(), SQLITE_TRANSIENT /*??*/));
        if (SQLITE_OK != m_result.serverResultCode()) {
            //! @todo msg?
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
            m_result.setServerResultCode(sqlite3_bind_int(m_handle, par, intValue));
            if (SQLITE_OK != m_result.serverResultCode()) {
                //! @todo msg?
                return false;
            }
        } else {
            m_result.setServerResultCode(sqlite3_bind_null(m_handle, par));
            if (SQLITE_OK != m_result.serverResultCode()) {
                //! @todo msg?
                return false;
            }
        }
        break;
    }
    case KDbField::Float:
    case KDbField::Double:
        m_result.setServerResultCode(sqlite3_bind_double(m_handle, par, value.toDouble()));
        if (SQLITE_OK != m_result.serverResultCode()) {
            //! @todo msg?
            return false;
        }
        break;
    case KDbField::BigInteger: {
        //! @todo what about unsigned > LLONG_MAX ?
        bool ok;
        const qint64 int64Value = value.toLongLong(&ok);
        if (ok) {
            m_result.setServerResultCode(sqlite3_bind_int64(m_handle, par, int64Value));
            if (SQLITE_OK != m_result.serverResultCode()) {
                //! @todo msg?
                return false;
            }
        } else {
            m_result.setServerResultCode(sqlite3_bind_null(m_handle, par));
            if (SQLITE_OK != m_result.serverResultCode()) {
                //! @todo msg?
                return false;
            }
        }
        break;
    }
    case KDbField::Boolean:
        m_result.setServerResultCode(
            sqlite3_bind_text(m_handle, par, value.toBool() ? "1" : "0",
                              1, SQLITE_TRANSIENT /*??*/));
        if (SQLITE_OK != m_result.serverResultCode()) {
            //! @todo msg?
            return false;
        }
        break;
    case KDbField::Time:
        m_result.setServerResultCode(
            sqlite3_bind_text(m_handle, par,
                              value.toTime().toString(Qt::ISODate).toLatin1(),
                              sizeof("HH:MM:SS"), SQLITE_TRANSIENT /*??*/));
        if (SQLITE_OK != m_result.serverResultCode()) {
            //! @todo msg?
            return false;
        }
        break;
    case KDbField::Date:
        m_result.setServerResultCode(
            sqlite3_bind_text(m_handle, par,
                              value.toDate().toString(Qt::ISODate).toLatin1(),
                              sizeof("YYYY-MM-DD"), SQLITE_TRANSIENT /*??*/));
        if (SQLITE_OK != m_result.serverResultCode()) {
            //! @todo msg?
            return false;
        }
        break;
    case KDbField::DateTime:
        m_result.setServerResultCode(
            sqlite3_bind_text(m_handle, par,
                              value.toDateTime().toString(Qt::ISODate).toLatin1(),
                              sizeof("YYYY-MM-DDTHH:MM:SS"), SQLITE_TRANSIENT /*??*/));
        if (SQLITE_OK != m_result.serverResultCode()) {
            //! @todo msg?
            return false;
        }
        break;
    case KDbField::BLOB: {
        const QByteArray byteArray(value.toByteArray());
        m_result.setServerResultCode(
            sqlite3_bind_blob(m_handle, par,
                              byteArray.constData(), byteArray.size(), SQLITE_TRANSIENT /*??*/));
        if (SQLITE_OK != m_result.serverResultCode()) {
            //! @todo msg?
            return false;
        }
        break;
    }
    default:
        KDbWarn << "unsupported field type:"
            << field->type() << "- NULL value bound to column #" << par;
        m_result.setServerResultCode(sqlite3_bind_null(m_handle, par));
        if (SQLITE_OK != m_result.serverResultCode()) {
            //! @todo msg?
            return false;
        }
    } //switch
    return true;
}

bool SQLitePreparedStatement::execute(
    KDbPreparedStatement::Type type,
    const KDbField::List& selectFieldList,
    KDbFieldList& insertFieldList,
    const KDbPreparedStatementParameters& parameters)
{
    Q_UNUSED(insertFieldList);
    if (!m_handle)
        return false;

    int par = 1; // par.index counted from 1
    KDbField::ListIterator itFields(selectFieldList.constBegin());
    for (QList<QVariant>::ConstIterator it = parameters.constBegin();
         itFields != selectFieldList.constEnd();
         it += (it == parameters.constEnd() ? 0 : 1), ++itFields, par++)
    {
        if (!bindValue(*itFields, it == parameters.constEnd() ? QVariant() : *it, par))
            return false;
    }

    //real execution
    sqlite3_step(m_handle);
    m_result.setServerResultCode(sqlite3_reset(m_handle));
    if (type == KDbPreparedStatement::InsertStatement && SQLITE_OK == m_result.serverResultCode()) {
        return true;
    }
    if (type == KDbPreparedStatement::SelectStatement) {
        //fetch result
        //! @todo
    }
    return false;
}

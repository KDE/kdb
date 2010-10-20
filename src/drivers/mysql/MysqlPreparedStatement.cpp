/* This file is part of the KDE project
   Copyright (C) 2006-2010 Jarosław Staniek <staniek@kde.org>

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

#include "MysqlPreparedStatement.h"
#include <QtDebug>
#include <errmsg.h>

using namespace Predicate;

// For example prepared MySQL statement code see:
// http://dev.mysql.com/doc/refman/4.1/en/mysql-stmt-execute.html

MysqlPreparedStatement::MysqlPreparedStatement(ConnectionInternal* conn)
        : PreparedStatementInterface()
        , MysqlConnectionInternal(conn->connection)
#ifdef PREDICATE_USE_MYSQL_STMT
        , m_statement(0)
        , m_mysqlBind(0)
#endif
        , m_resetRequired(false)
{
// PreDrvDbg;

    mysql_owned = false;
    mysql = dynamic_cast<Predicate::MysqlConnectionInternal&>(*conn).mysql; //copy
//    m_tempStatementString = generateStatementString();

    if (!init())
        done();
}

bool MysqlPreparedStatement::init()
{
//    if (m_tempStatementString.isEmpty())
//        return false;
#ifdef PREDICATE_USE_MYSQL_STMT
    m_statement = mysql_stmt_init(mysql);
    if (!m_statement) {
//! @todo err 'out of memory'
        return false;
    }
    res = mysql_stmt_prepare(m_statement,
                             (const char*)m_tempStatementString, m_tempStatementString.length());
    if (0 != res) {
//! @todo use mysql_stmt_error(stmt); to show error
        return false;
    }

    m_realParamCount = mysql_stmt_param_count(m_statement);
    if (m_realParamCount <= 0) {
//! @todo err
        return false;
    }
    m_mysqlBind = new MYSQL_BIND[ m_realParamCount ];
    memset(m_mysqlBind, 0, sizeof(MYSQL_BIND)*m_realParamCount); //safe?
#endif
    return true;
}

MysqlPreparedStatement::~MysqlPreparedStatement()
{
    done();
}

void MysqlPreparedStatement::done()
{
#ifdef PREDICATE_USE_MYSQL_STMT
    if (m_statement) {
//! @todo handle errors of mysql_stmt_close()?
        mysql_stmt_close(m_statement);
        m_statement = 0;
    }
    delete m_mysqlBind;
    m_mysqlBind = 0;
#endif
}

bool MysqlPreparedStatement::prepare(const QByteArray& statement)
{
    Q_UNUSED(statement);
    return true;
}

#ifdef PREDICATE_USE_MYSQL_STMT
#define BIND_NULL { \
        m_mysqlBind[arg].buffer_type = MYSQL_TYPE_NULL; \
        m_mysqlBind[arg].buffer = 0; \
        m_mysqlBind[arg].buffer_length = 0; \
        m_mysqlBind[arg].is_null = &dummyNull; \
        m_mysqlBind[arg].length = &str_length; }

bool MysqlPreparedStatement::bindValue(Field *field, const QVariant& value, int arg)
{
    if (value.isNull()) {
        // no value to bind or the value is null: bind NULL
        BIND_NULL;
        return true;
    }

    if (field->isTextType()) {
//! @todo optimize
        m_stringBuffer[ 1024 ]; ? ? ?
        char *str = qstrncpy(m_stringBuffer, (const char*)value.toString().toUtf8(), 1024);
        m_mysqlBind[arg].buffer_type = MYSQL_TYPE_STRING;
        m_mysqlBind[arg].buffer = m_stringBuffer;
        m_mysqlBind[arg].is_null = (my_bool*)0;
        m_mysqlBind[arg].buffer_length = 1024; //?
        m_mysqlBind[arg].length = &str_length;
        return true;
    }

    switch (field->type()) {
    case Predicate::Field::Byte:
    case Predicate::Field::ShortInteger:
    case Predicate::Field::Integer: {
        //! @todo what about unsigned > INT_MAX ?
        bool ok;
        const int intValue = value.toInt(&ok);
        if (ok) {
            if (field->type() == Predicate::Field::Byte)
                m_mysqlBind[arg].buffer_type = MYSQL_TYPE_TINY;
            else if (field->type() == Predicate::Field::ShortInteger)
                m_mysqlBind[arg].buffer_type = MYSQL_TYPE_SHORT;
            else if (field->type() == Predicate::Field::Integer)
                m_mysqlBind[arg].buffer_type = MYSQL_TYPE_LONG;

            m_mysqlBind[arg].is_null = (my_bool*)0;
            m_mysqlBind[arg].length = 0;

            res = sqlite3_bind_int(prepared_st_handle, arg, intValue);
            if (SQLITE_OK != res) {
                //! @todo msg?
                return false;
            }
        } else
            BIND_NULL;
        break;
    }
    case Predicate::Field::Float:
    case Predicate::Field::Double:
        res = sqlite3_bind_double(prepared_st_handle, arg, value.toDouble());
        if (SQLITE_OK != res) {
            //! @todo msg?
            return false;
        }
        break;
    case Predicate::Field::BigInteger: {
        //! @todo what about unsigned > LLONG_MAX ?
        bool ok;
        qint64 int64Value = value.toLongLong(&ok);
        if (ok) {
            res = sqlite3_bind_int64(prepared_st_handle, arg, value);
            if (SQLITE_OK != res) {
                //! @todo msg?
                return false;
            }
        } else {
            res = sqlite3_bind_null(prepared_st_handle, arg);
            if (SQLITE_OK != res) {
                //! @todo msg?
                return false;
            }
        }
        break;
    }
    case Predicate::Field::Boolean:
        res = sqlite3_bind_text(prepared_st_handle, arg,
                                QString::number(value.toBool() ? 1 : 0).toLatin1(),
                                1, SQLITE_TRANSIENT /*??*/);
        if (SQLITE_OK != res) {
            //! @todo msg?
            return false;
        }
        break;
    case Predicate::Field::Time:
        res = sqlite3_bind_text(prepared_st_handle, arg,
                                value.toTime().toString(Qt::ISODate).toLatin1(),
                                sizeof("HH:MM:SS"), SQLITE_TRANSIENT /*??*/);
        if (SQLITE_OK != res) {
            //! @todo msg?
            return false;
        }
        break;
    case Predicate::Field::Date:
        res = sqlite3_bind_text(prepared_st_handle, arg,
                                value.toDate().toString(Qt::ISODate).toLatin1(),
                                sizeof("YYYY-MM-DD"), SQLITE_TRANSIENT /*??*/);
        if (SQLITE_OK != res) {
            //! @todo msg?
            return false;
        }
        break;
    case Predicate::Field::DateTime:
        res = sqlite3_bind_text(prepared_st_handle, arg,
                                value.toDateTime().toString(Qt::ISODate).toLatin1(),
                                sizeof("YYYY-MM-DDTHH:MM:SS"), SQLITE_TRANSIENT /*??*/);
        if (SQLITE_OK != res) {
            //! @todo msg?
            return false;
        }
        break;
    case Predicate::Field::BLOB: {
        const QByteArray byteArray(value.toByteArray());
        res = sqlite3_bind_blob(prepared_st_handle, arg,
                                (const char*)byteArray, byteArray.size(), SQLITE_TRANSIENT /*??*/);
        if (SQLITE_OK != res) {
            //! @todo msg?
            return false;
        }
        break;
    }
    default:
        PreWarn << "unsupported field type:"
            << field->type() << "- NULL value bound to column #" << arg;
        res = sqlite3_bind_null(prepared_st_handle, arg);
        if (SQLITE_OK != res) {
            //! @todo msg?
            return false;
        }
    } //switch
    return true;
}
#endif

bool MysqlPreparedStatement::execute(
    PreparedStatement::Type type,
    const Field::List& fieldList,
    const PreparedStatementParameters& parameters)
{
#ifdef PREDICATE_USE_MYSQL_STMT
    if (!m_statement || m_realParamCount <= 0)
        return false;
    if (mysql_stmt_errno(m_statement) == CR_SERVER_LOST) {
        //sanity: connection lost: reconnect
//! @todo Predicate::Connection should be reconnected as well!
        done();
        if (!init()) {
            done();
            return false;
        }
    }

    if (m_resetRequired) {
        mysql_stmt_reset(m_statement);
        res = sqlite3_reset(prepared_st_handle);
        if (SQLITE_OK != res) {
            //! @todo msg?
            return false;
        }
        m_resetRequired = false;
    }

    int par = 0;
    bool dummyNull = true;
    unsigned long str_length;

    //for INSERT, we're iterating over inserting values
    //for SELECT, we're iterating over WHERE conditions
    Field::List *fieldList = 0;
    if (m_type == SelectStatement)
        fieldList = m_whereFields;
    else if (m_type == InsertStatement)
        fieldList = m_fields->fields();
    else
        assert(0); //impl. error

    Field::ListIterator itFields(fieldList->constBegin());
    for (QList<QVariant>::ConstIterator it(parameters.constBegin());
            itFields != fieldList->constEnd() && arg < m_realParamCount; ++it, ++itFields, par++) {
        if (!bindValue(*itFields, it == parameters.constEnd() ? QVariant() : *it, par))
            return false;

        }//else
    }//for

    //real execution
    res = sqlite3_step(prepared_st_handle);
    m_resetRequired = true;
    if (m_type == InsertStatement && res == SQLITE_DONE) {
        return true;
    }
    if (m_type == SelectStatement) {
        //fetch result

//! @todo
    }
#else
    m_resetRequired = true;
#if 0 //TODO
    if (type == PreparedStatement::InsertStatement && connection->insertRecord(fieldList, parameters)) {
        return true;
    }
#endif
//TODO handle Select...

#endif // !PREDICATE_USE_MYSQL_STMT
    return false;
}

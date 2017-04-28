/* This file is part of the KDE project
   Copyright (C) 2004 Martin Ellis <martin.ellis@kdemail.net>
   Copyright (C) 2004-2016 Jarosław Staniek <staniek@kde.org>

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

#include "MysqlConnection_p.h"
#include "mysql_debug.h"

#include "KDbConnectionData.h"

#include <QByteArray>
#include <QStringList>
#include <QFile>

static inline QString escapeIdentifier(const QString& str)
{
    return QString(str).replace(QLatin1Char('`'), QLatin1Char('\''));
}

MysqlConnectionInternal::MysqlConnectionInternal(KDbConnection* connection)
        : KDbConnectionInternal(connection)
        , mysql(nullptr)
        , mysql_owned(true)
        , res(0)
        , lowerCaseTableNames(false)
        , serverVersion(0)
{
}

MysqlConnectionInternal::~MysqlConnectionInternal()
{
    if (mysql_owned && mysql) {
        db_disconnect();
    }
}

bool MysqlConnectionInternal::db_connect(const KDbConnectionData& data)
{
    if (!(mysql = mysql_init(mysql)))
        return false;

    mysqlDebug();
    QByteArray localSocket;
    QByteArray hostName = QFile::encodeName(data.hostName());
    if (hostName.isEmpty() || 0 == qstricmp(hostName.constData(), "localhost")) {
        if (data.useLocalSocketFile()) {
            if (data.localSocketFileName().isEmpty()) {
                //! @todo move the list of default sockets to a generic method
                QStringList sockets;
#ifndef Q_OS_WIN
                sockets
                    << QLatin1String("/var/lib/mysql/mysql.sock")
                    << QLatin1String("/var/run/mysqld/mysqld.sock")
                    << QLatin1String("/var/run/mysql/mysql.sock")
                    << QLatin1String("/tmp/mysql.sock");

                foreach(const QString& socket, sockets) {
                    if (QFile(socket).exists()) {
                        localSocket = socket.toLatin1();
                        break;
                    }
                }
#endif
            } else
                localSocket = QFile::encodeName(data.localSocketFileName());
        } else {
            //we're not using local socket
            hostName = "127.0.0.1"; //this will force mysql to connect to localhost
        }
    }

    /*! @todo is latin1() encoding here valid? what about using UTF for passwords? */
    const QByteArray userName(data.userName().toLatin1());
    const QByteArray password(data.password().toLatin1());
    int client_flag = 0; //!< @todo support client_flag?
    if (mysql_real_connect(mysql, hostName.isEmpty() ? nullptr : hostName.constData(),
                           data.userName().isEmpty() ? nullptr : userName.constData(),
                           data.password().isNull() ? nullptr : password.constData(),
                           nullptr,
                           data.port(), localSocket.isEmpty() ? nullptr : localSocket.constData(),
                           client_flag))
    {
        serverVersion = mysql_get_server_version(mysql);
        return true;
    }
    return false;
}

bool MysqlConnectionInternal::db_disconnect()
{
    mysql_close(mysql);
    mysql = nullptr;
    serverVersion = 0;
    mysqlDebug();
    return true;
}

bool MysqlConnectionInternal::useDatabase(const QString &dbName)
{
//! @todo is here escaping needed?
    if (!executeVoidSQL(KDbEscapedString("USE ") + escapeIdentifier(dbName))) {
        return false;
    }
    if (!executeVoidSQL(KDbEscapedString("SET SESSION sql_mode='TRADITIONAL'"))) {
        // needed to turn warnings about trimming string values into SQL errors
        return false;
    }
    return true;
}

bool MysqlConnectionInternal::executeVoidSQL(const KDbEscapedString& sql)
{
    return 0 == mysql_real_query(mysql, sql.constData(), sql.length());
}

//static
QString MysqlConnectionInternal::serverResultName(MYSQL *mysql)
{
    //! @todo use mysql_stmt_sqlstate() for prepared statements
    return QString::fromLatin1(mysql_sqlstate(mysql));
}

void MysqlConnectionInternal::storeResult(KDbResult *result)
{
    result->setServerMessage(QString::fromLatin1(mysql_error(mysql)));
    result->setServerErrorCode(mysql_errno(mysql));
}

//--------------------------------------

MysqlCursorData::MysqlCursorData(KDbConnection* connection)
        : MysqlConnectionInternal(connection)
        , mysqlres(nullptr)
        , mysqlrow(nullptr)
        , lengths(nullptr)
        , numRows(0)
{
    mysql_owned = false;
    mysql = static_cast<MysqlConnection*>(connection)->d->mysql;
}

MysqlCursorData::~MysqlCursorData()
{
}

//--------------------------------------

static inline KDbSqlString mysqlTypeName(MysqlSqlResult *result, QScopedPointer<MysqlSqlRecord> *recordPtr)
{
    KDbSqlString name;
    if (result && result->fieldsCount() >= 2) {
        recordPtr->reset(static_cast<MysqlSqlRecord*>(result->fetchRecord()));
        if (*recordPtr) {
            name = (*recordPtr)->cstringValue(1);
        }
    }
    return name;
}

//! @todo From Kexi MysqlMigrate, unused for now because enum type isn't supported by KDb
#if 0
//! Get the strings that identify values in an enum field
/*! Parse the type of a MySQL enum field as returned by the server in a
    'DESCRIBE table' or 'SHOW COLUMNS FROM table' statement.  The string
    returned by the server is in the form 'enum('option1','option2').
    In this example, the result should be a string list containing two
    strings, "option1", "option2".
    \return list of possible values the field can take
 */
QStringList examineEnumField(const QString& table, const KDbSqlField* field)
{
    QString vals;
    const KDbEscapedString query
        = KDbEscapedString("SHOW COLUMNS FROM `") + conn->escapeIdentifier(table) +
                        "` LIKE '" + conn->escapeIdentifier(fld->name) + '\'';

    if (!conn->executeVoidSQL(query))
        // Huh? MySQL wont tell us what values it can take.
        return QStringList();

    MYSQL_RES *res = mysql_store_result(d->mysql);

    if (!res) {
        //qWarning() << "null result";
    }
    else {
        MYSQL_ROW row;
        if ((row = mysql_fetch_row(res)))
            vals = QString(row[1]);
        mysql_free_result(res);
    }

    qDebug() << "considering" << vals;

    // Crash and burn if we get confused...
    if (!vals.startsWith("enum(")) {
        // Huh? We're supposed to be parsing an enum!
        qWarning() << "1 not an enum!";
        return QStringList();
    }
    if (!vals.endsWith(')')) {
        qWarning() << "2 not an enum!";
        return QStringList();
    }

    // It'd be nice to use QString.section or QStringList.split, but we need
    // to be careful as enum values can have commas and quote marks in them
    // e.g. CREATE TABLE t(f enum('option,''') gives one option: "option,'"
    vals.remove(0, 5);
    QRegularExpression rx = QRegularExpression("^'((?:[^,']|,|'')*)'");
    QStringList values = QStringList();
    int index = 0;

    while ((index = rx.indexIn(vals, index, QRegularExpression::CaretAtOffset)) != -1) {
        int len = rx.matchedLength();
        if (len != -1) {
            //qDebug() << "3 " << rx.cap(1);
            values << rx.cap(1);
        } else {
            qDebug() << "4 lost";
        }

        QChar next = vals[index + len];
        if (next != QChar(',') && next != QChar(')')) {
            qDebug() << "5 " << next;
        }
        index += len + 1;
    }

    return values;
}
#endif // examineEnumField

KDbField::Type MysqlSqlResult::blobType(const QString& tableName, MysqlSqlField *field)
{
    KDbField::Type kdbType = KDbField::LongText;
    const KDbEscapedString sql = KDbEscapedString("SHOW COLUMNS FROM %1 LIKE '%2'")
            .arg(escapeIdentifier(tableName)).arg(field->name());
    //! @todo this conflicts with active query
    QScopedPointer<MysqlSqlResult> result(static_cast<MysqlSqlResult*>(conn->executeSQL(sql)));
    if (result) {
        QScopedPointer<MysqlSqlRecord> record;
        const KDbSqlString typeName(mysqlTypeName(result.data(), &record));
        if (typeName.rawDataToByteArray().toLower().contains("blob")) {
            // Doesn't matter how big it is, it's binary
            kdbType = KDbField::BLOB;
        } else if (field->length() < 200) {
            kdbType = KDbField::Text;
        }
    }
    return kdbType;
}

KDbField::Type MysqlSqlResult::type(const QString& tableName, MysqlSqlField *field)
{
    // Field type
    KDbField::Type kdbType = KDbField::InvalidType;

    switch (field->type()) {
        // These are in the same order as mysql_com.h.
        // MySQL names given on the right
    case FIELD_TYPE_DECIMAL:    // DECIMAL or NUMERIC
        break;
    case FIELD_TYPE_TINY:       // TINYINT (-2^7..2^7-1 or 2^8)
        kdbType = KDbField::Byte;
        break;
    case FIELD_TYPE_SHORT:      // SMALLINT (-2^15..2^15-1 or 2^16)
        kdbType = KDbField::ShortInteger;
        break;
    case FIELD_TYPE_LONG:       // INTEGER (-2^31..2^31-1 or 2^32)
        kdbType = KDbField::Integer;
        break;
    case FIELD_TYPE_FLOAT:      // FLOAT
        kdbType = KDbField::Float;
        break;
    case FIELD_TYPE_DOUBLE:     // DOUBLE or REAL (8 byte)
        kdbType = KDbField::Double;
        break;
    case FIELD_TYPE_NULL:       // WTF?
        break;
    case FIELD_TYPE_TIMESTAMP:  // TIMESTAMP (promote?)
        kdbType = KDbField::DateTime;
        break;
    case FIELD_TYPE_LONGLONG:   // BIGINT (-2^63..2^63-1 or 2^64)
    case FIELD_TYPE_INT24:      // MEDIUMINT (-2^23..2^23-1 or 2^24) (promote)
        kdbType = KDbField::BigInteger;
        break;
    case FIELD_TYPE_DATE:       // DATE
        kdbType = KDbField::Date;
        break;
    case FIELD_TYPE_TIME:       // TIME
        kdbType = KDbField::Time;
        break;
    case FIELD_TYPE_DATETIME:   // DATETIME
        kdbType = KDbField::DateTime;
        break;
    case FIELD_TYPE_YEAR:       // YEAR (promote)
        kdbType = KDbField::ShortInteger;
        break;
    case FIELD_TYPE_NEWDATE:    // WTF?
    case FIELD_TYPE_ENUM:       // ENUM
        // If MySQL did what it's documentation said it did, we would come here
        // for enum fields ...
        kdbType = KDbField::Enum;
        break;
    case FIELD_TYPE_SET:        // SET
        //! @todo: Support set column type
        break;
    case FIELD_TYPE_TINY_BLOB:
    case FIELD_TYPE_MEDIUM_BLOB:
    case FIELD_TYPE_LONG_BLOB:
    case FIELD_TYPE_BLOB:       // BLOB or TEXT
    case FIELD_TYPE_VAR_STRING: // VARCHAR
    case FIELD_TYPE_STRING:     // CHAR
        if (field->data->flags & ENUM_FLAG) {
            // ... instead we come here, using the ENUM_FLAG which is supposed to
            // be deprecated! Duh.
            kdbType = KDbField::Enum;
        } else {
            kdbType = blobType(tableName, field);
        }
        break;
    default:
        break;
    }
    return kdbType;
}

inline void copyConstraints(int mysqlFieldFlags, KDbField* field)
{
    field->setPrimaryKey(mysqlFieldFlags & PRI_KEY_FLAG);
    field->setAutoIncrement(mysqlFieldFlags & AUTO_INCREMENT_FLAG);
    field->setNotNull(mysqlFieldFlags & NOT_NULL_FLAG);
    field->setUniqueKey(mysqlFieldFlags & UNIQUE_KEY_FLAG);
    //! @todo: support keys
}

static inline void copyOptions(int mysqlFieldFlags, KDbField* field)
{
    field->setUnsigned(mysqlFieldFlags & UNSIGNED_FLAG);
}

KDbField* MysqlSqlResult::createField(const QString &tableName, int index)
{
    QScopedPointer<MysqlSqlField> f(static_cast<MysqlSqlField*>(field(index)));
    if (!f) {
        return nullptr;
    }
    const QString caption(f->name());
    QString realFieldName(KDb::stringToIdentifier(caption.toLower()));
    KDbField *kdbField = new KDbField(realFieldName, type(tableName, f.data()));
    kdbField->setCaption(caption);
    const int flags = f->data->flags;
    copyConstraints(flags, kdbField);
    copyOptions(flags, kdbField);
    return kdbField;
}

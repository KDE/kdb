/* This file is part of the KDE project
   Copyright (C) 2002 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003 Joseph Wenninger<jowenn@kde.org>

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

#include "MysqlDriver.h"
#include "MysqlConnection.h"
#include <Predicate/Field.h>
#include <Predicate/Driver_p.h>
#include <Predicate/Utils.h>

#include <QVariant>
#include <QFile>
#include <QtDebug>

#ifdef Q_WS_WIN
# include <config-win.h>
#endif
#include <mysql_version.h>
#include <mysql.h>
#define BOOL bool

using namespace Predicate;

EXPORT_PREDICATE_DRIVER(MysqlDriver, mysql)

/*! @todo Implement buffered/unbuffered cursor, rather than buffer everything.
   Each MYSQL connection can only handle at most one unbuffered cursor,
   so MysqlConnection should keep count?
 */

MysqlDriver::MysqlDriver()
    : Driver()
{
    d->features = IgnoreTransactions | CursorForward;

    beh->ROW_ID_FIELD_NAME = QLatin1String("LAST_INSERT_ID()");
    beh->ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE = true;
    beh->_1ST_ROW_READ_AHEAD_REQUIRED_TO_KNOW_IF_THE_RESULT_IS_EMPTY = false;
    beh->USING_DATABASE_REQUIRED_TO_CONNECT = false;
    beh->QUOTATION_MARKS_FOR_IDENTIFIER = '`';
    initDriverSpecificKeywords(keywords);

    //predefined properties
#if MYSQL_VERSION_ID < 40000
    d->properties["client_library_version"] = MYSQL_SERVER_VERSION; //nothing better
    d->properties["default_server_encoding"] = MYSQL_CHARSET; //nothing better
#elif MYSQL_VERSION_ID < 50000
//OK? d->properties["client_library_version"] = mysql_get_client_version();
#endif

    d->typeNames[Field::Byte] = QLatin1String("TINYINT");
    d->typeNames[Field::ShortInteger] = QLatin1String("SMALLINT");
    d->typeNames[Field::Integer] = QLatin1String("INT");
    d->typeNames[Field::BigInteger] = QLatin1String("BIGINT");
    // Can use BOOLEAN here, but BOOL has been in MySQL longer
    d->typeNames[Field::Boolean] = QLatin1String("BOOL");
    d->typeNames[Field::Date] = QLatin1String("DATE");
    d->typeNames[Field::DateTime] = QLatin1String("DATETIME");
    d->typeNames[Field::Time] = QLatin1String("TIME");
    d->typeNames[Field::Float] = QLatin1String("FLOAT");
    d->typeNames[Field::Double] = QLatin1String("DOUBLE");
    d->typeNames[Field::Text] = QLatin1String("VARCHAR");
    d->typeNames[Field::LongText] = QLatin1String("LONGTEXT");
    d->typeNames[Field::BLOB] = QLatin1String("BLOB");
}

MysqlDriver::~MysqlDriver()
{
}

Predicate::Connection*
MysqlDriver::drv_createConnection(const ConnectionData& connData)
{
    return new MysqlConnection(this, connData);
}

bool MysqlDriver::isSystemDatabaseName(const QString &n) const
{
    return 0 == n.compare(QLatin1String("mysql"), Qt::CaseInsensitive);
}

bool MysqlDriver::drv_isSystemFieldName(const QString&) const
{
    return false;
}

EscapedString MysqlDriver::escapeString(const QString& str) const
{
    //escape as in http://dev.mysql.com/doc/refman/5.0/en/string-syntax.html
//! @todo support more characters, like %, _

    const int old_length = str.length();
    int i;
    for (i = 0; i < old_length; i++) {   //anything to escape?
        const unsigned int ch = str[i].unicode();
        if (ch == '\\' || ch == '\'' || ch == '"' || ch == '\n' || ch == '\r' || ch == '\t' || ch == '\b' || ch == '\0')
            break;
    }
    if (i >= old_length) { //no characters to escape
        return EscapedString("'") + EscapedString(str) + '\'';
    }

    QChar *new_string = new QChar[ old_length * 3 + 1 ]; // a worst case approximation
//! @todo move new_string to Driver::m_new_string or so...
    int new_length = 0;
    new_string[new_length++] = '\''; //prepend '
    for (i = 0; i < old_length; i++, new_length++) {
        const unsigned int ch = str[i].unicode();
        if (ch == '\\') {
            new_string[new_length++] = '\\';
            new_string[new_length] = '\\';
        } else if (ch <= '\'') {//check for speedup
            if (ch == '\'') {
                new_string[new_length++] = '\\';
                new_string[new_length] = '\'';
            } else if (ch == '"') {
                new_string[new_length++] = '\\';
                new_string[new_length] = '"';
            } else if (ch == '\n') {
                new_string[new_length++] = '\\';
                new_string[new_length] = 'n';
            } else if (ch == '\r') {
                new_string[new_length++] = '\\';
                new_string[new_length] = 'r';
            } else if (ch == '\t') {
                new_string[new_length++] = '\\';
                new_string[new_length] = 't';
            } else if (ch == '\b') {
                new_string[new_length++] = '\\';
                new_string[new_length] = 'b';
            } else if (ch == '\0') {
                new_string[new_length++] = '\\';
                new_string[new_length] = '0';
            } else
                new_string[new_length] = str[i];
        } else
            new_string[new_length] = str[i];
    }

    new_string[new_length++] = '\''; //append '
    EscapedString result(QString(new_string, new_length));
    delete [] new_string;
    return result;
}

EscapedString MysqlDriver::escapeBLOB(const QByteArray& array) const
{
    return EscapedString(Predicate::escapeBLOB(array, Predicate::BLOBEscape0xHex));
}

EscapedString MysqlDriver::escapeString(const QByteArray& str) const
{
//! @todo optimize using mysql_real_escape_string()?
//! see http://dev.mysql.com/doc/refman/5.0/en/string-syntax.html

    return EscapedString("'") + EscapedString(str)
           .replace('\\', "\\\\")
           .replace('\'', "\\''")
           .replace('"', "\\\"")
           + '\'';
}

/*! Add back-ticks to an identifier, and replace any back-ticks within
 * the name with single quotes.
 */
QByteArray MysqlDriver::drv_escapeIdentifier(const QString& str) const
{
    return QByteArray(str.toUtf8()).replace('`', "'");
}

QByteArray MysqlDriver::drv_escapeIdentifier(const QByteArray& str) const
{
    return QByteArray(str).replace('`', "'");
}
/* This file is part of the KDE project
   Copyright (C) 2002 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003 Joseph Wenninger<jowenn@kde.org>
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

#include "MysqlDriver.h"
#include "MysqlConnection.h"
#include "KDbField.h"
#include "KDbDriver_p.h"
#include "KDb.h"

#include <mysql_version.h>
#include <mysql.h>

KDB_DRIVER_PLUGIN_FACTORY(MysqlDriver, "kdb_mysqldriver.json")

/*! @todo Implement buffered/unbuffered cursor, rather than buffer everything.
   Each MYSQL connection can only handle at most one unbuffered cursor,
   so MysqlConnection should keep count?
 */

MysqlDriver::MysqlDriver(QObject *parent, const QVariantList &args)
    : KDbDriver(parent, args)
{
    d->features = IgnoreTransactions | CursorForward;

    beh->ROW_ID_FIELD_NAME = QLatin1String("LAST_INSERT_ID()");
    beh->ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE = true;
    beh->_1ST_ROW_READ_AHEAD_REQUIRED_TO_KNOW_IF_THE_RESULT_IS_EMPTY = false;
    beh->USING_DATABASE_REQUIRED_TO_CONNECT = false;
    beh->QUOTATION_MARKS_FOR_IDENTIFIER = '`';
    //! @todo add configuration option
    beh->TEXT_TYPE_MAX_LENGTH = 255;
    beh->RANDOM_FUNCTION = QLatin1String("RAND");
    initDriverSpecificKeywords(keywords);

    //predefined properties
#if MYSQL_VERSION_ID < 40000
    d->properties["client_library_version"] = MYSQL_SERVER_VERSION; //nothing better
    d->properties["default_server_encoding"] = MYSQL_CHARSET; //nothing better
#else
    // https://dev.mysql.com/doc/refman/5.7/en/mysql-get-client-version.html
    d->properties.insert("client_library_version", int(mysql_get_client_version()));
#endif

    d->typeNames[KDbField::Byte] = QLatin1String("TINYINT");
    d->typeNames[KDbField::ShortInteger] = QLatin1String("SMALLINT");
    d->typeNames[KDbField::Integer] = QLatin1String("INT");
    d->typeNames[KDbField::BigInteger] = QLatin1String("BIGINT");
    // Can use BOOLEAN here, but BOOL has been in MySQL longer
    d->typeNames[KDbField::Boolean] = QLatin1String("BOOL");
    d->typeNames[KDbField::Date] = QLatin1String("DATE");
    d->typeNames[KDbField::DateTime] = QLatin1String("DATETIME");
    d->typeNames[KDbField::Time] = QLatin1String("TIME");
    d->typeNames[KDbField::Float] = QLatin1String("FLOAT");
    d->typeNames[KDbField::Double] = QLatin1String("DOUBLE");
    d->typeNames[KDbField::Text] = QLatin1String("VARCHAR");
    d->typeNames[KDbField::LongText] = QLatin1String("LONGTEXT");
    d->typeNames[KDbField::BLOB] = QLatin1String("BLOB");
}

MysqlDriver::~MysqlDriver()
{
}

KDbConnection* MysqlDriver::drv_createConnection(const KDbConnectionData& connData,
                                                 const KDbConnectionOptions &options)
{
    return new MysqlConnection(this, connData, options);
}

bool MysqlDriver::isSystemObjectName(const QString& n) const
{
    Q_UNUSED(n);
    return false;
}

bool MysqlDriver::isSystemDatabaseName(const QString &n) const
{
    return    0 == n.compare(QLatin1String("mysql"), Qt::CaseInsensitive)
           || 0 == n.compare(QLatin1String("information_schema"), Qt::CaseInsensitive)
           || 0 == n.compare(QLatin1String("performance_schema"), Qt::CaseInsensitive);
}

bool MysqlDriver::drv_isSystemFieldName(const QString& n) const
{
    Q_UNUSED(n);
    return false;
}

KDbEscapedString MysqlDriver::escapeString(const QString& str) const
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
        return KDbEscapedString("'") + KDbEscapedString(str) + '\'';
    }

    QChar *new_string = new QChar[ old_length * 3 + 1 ]; // a worst case approximation
//! @todo move new_string to KDbDriver::m_new_string or so...
    int new_length = 0;
    new_string[new_length++] = QLatin1Char('\''); //prepend '
    for (i = 0; i < old_length; i++, new_length++) {
        const unsigned int ch = str[i].unicode();
        if (ch == '\\') {
            new_string[new_length++] = QLatin1Char('\\');
            new_string[new_length] = QLatin1Char('\\');
        } else if (ch <= '\'') {//check for speedup
            if (ch == '\'') {
                new_string[new_length++] = QLatin1Char('\\');
                new_string[new_length] = QLatin1Char('\'');
            } else if (ch == '"') {
                new_string[new_length++] = QLatin1Char('\\');
                new_string[new_length] = QLatin1Char('"');
            } else if (ch == '\n') {
                new_string[new_length++] = QLatin1Char('\\');
                new_string[new_length] = QLatin1Char('n');
            } else if (ch == '\r') {
                new_string[new_length++] = QLatin1Char('\\');
                new_string[new_length] = QLatin1Char('r');
            } else if (ch == '\t') {
                new_string[new_length++] = QLatin1Char('\\');
                new_string[new_length] = QLatin1Char('t');
            } else if (ch == '\b') {
                new_string[new_length++] = QLatin1Char('\\');
                new_string[new_length] = QLatin1Char('b');
            } else if (ch == '\0') {
                new_string[new_length++] = QLatin1Char('\\');
                new_string[new_length] = QLatin1Char('0');
            } else
                new_string[new_length] = str[i];
        } else
            new_string[new_length] = str[i];
    }

    new_string[new_length++] = QLatin1Char('\''); //append '
    KDbEscapedString result(QString(new_string, new_length));
    delete [] new_string;
    return result;
}

KDbEscapedString MysqlDriver::escapeBLOB(const QByteArray& array) const
{
    return KDbEscapedString(KDb::escapeBLOB(array, KDb::BLOBEscape0xHex));
}

KDbEscapedString MysqlDriver::escapeString(const QByteArray& str) const
{
//! @todo optimize using mysql_real_escape_string()?
//! see http://dev.mysql.com/doc/refman/5.0/en/string-syntax.html

    return KDbEscapedString("'") + KDbEscapedString(str)
           .replace('\\', "\\\\")
           .replace('\'', "\\''")
           .replace('"', "\\\"")
           + '\'';
}

/*! Add back-ticks to an identifier, and replace any back-ticks within
 * the name with single quotes.
 */
QString MysqlDriver::drv_escapeIdentifier(const QString& str) const
{
    return QString(str).replace(QLatin1Char('"'), QLatin1String("\"\""));
}

QByteArray MysqlDriver::drv_escapeIdentifier(const QByteArray& str) const
{
    return QByteArray(str).replace('`', '\'');
}

KDbEscapedString MysqlDriver::lengthFunctionToString(const KDbNArgExpression &args,
                                                     KDbQuerySchemaParameterValueListIterator* params,
                                                     KDb::ExpressionCallStack* callStack) const
{
    return KDbFunctionExpression::toString(
                QLatin1String("CHAR_LENGTH"), this, args, params, callStack);
}

KDbEscapedString MysqlDriver::greatestOrLeastFunctionToString(const QString &name,
                                                     const KDbNArgExpression &args,
                                                     KDbQuerySchemaParameterValueListIterator* params,
                                                     KDb::ExpressionCallStack* callStack) const
{
    return KDbFunctionExpression::greatestOrLeastFunctionUsingCaseToString(
                name, this, args, params, callStack);
}

KDbEscapedString MysqlDriver::unicodeFunctionToString(
                                            const KDbNArgExpression &args,
                                            KDbQuerySchemaParameterValueListIterator* params,
                                            KDb::ExpressionCallStack* callStack) const
{
    Q_ASSERT(args.argCount() == 1);
    return KDbEscapedString("ORD(CONVERT(%1 USING UTF16))")
                            .arg(args.arg(0).toString(this, params, callStack));
}

KDbEscapedString MysqlDriver::concatenateFunctionToString(const KDbBinaryExpression &args,
                                                          KDbQuerySchemaParameterValueListIterator* params,
                                                          KDb::ExpressionCallStack* callStack) const
{
    return KDbEscapedString("CONCAT(%1, %2)").arg(args.left().toString(this, params, callStack))
                                             .arg(args.right().toString(this, params, callStack));
}

#include "MysqlDriver.moc"

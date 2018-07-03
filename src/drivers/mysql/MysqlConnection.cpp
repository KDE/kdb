/* This file is part of the KDE project
   Copyright (C) 2002 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2003 Joseph Wenninger<jowenn@kde.org>
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

#include "MysqlConnection.h"
#include "MysqlDriver.h"
#include "MysqlCursor.h"
#include "MysqlPreparedStatement.h"
#include "mysql_debug.h"
#include "KDbConnectionData.h"
#include "KDbVersionInfo.h"

#include <QRegularExpression>

MysqlConnection::MysqlConnection(KDbDriver *driver, const KDbConnectionData& connData,
                                 const KDbConnectionOptions &options)
        : KDbConnection(driver, connData, options)
        , d(new MysqlConnectionInternal(this))
{
}

MysqlConnection::~MysqlConnection()
{
    destroy();
    delete d;
}

bool MysqlConnection::drv_connect()
{
    const bool ok = d->db_connect(data());
    if (!ok) {
        storeResult(); //store error msg, if any - can be destroyed after disconnect()
        d->db_disconnect();
        return false;
    }

    // Get lower_case_table_name value so we know if there's case sensitivity supported
    // See https://dev.mysql.com/doc/refman/5.0/en/identifier-case-sensitivity.html
    int intLowerCaseTableNames = 0;
    const tristate res = querySingleNumber(
        KDbEscapedString("SHOW VARIABLES LIKE 'lower_case_table_name'"), &intLowerCaseTableNames,
        0 /*col*/,
        QueryRecordOptions(QueryRecordOption::Default) & ~QueryRecordOptions(QueryRecordOption::AddLimitTo1));
    if (res == false) // sanity
        return false;
    d->lowerCaseTableNames = intLowerCaseTableNames > 0;
    return true;
}

bool MysqlConnection::drv_getServerVersion(KDbServerVersionInfo* version)
{
    // https://dev.mysql.com/doc/refman/5.1/en/mysql-get-server-info.html
    version->setString(QLatin1String(mysql_get_server_info(d->mysql)));

    // get the version info using 'version' built-in variable:
//! @todo this is hardcoded for now; define api for retrieving variables and use this API...
    // https://dev.mysql.com/doc/refman/5.1/en/mysql-get-server-version.html
    QString versionString;
    tristate res = querySingleString(KDbEscapedString("SELECT @@version"), &versionString,
        /*column*/ 0,
        QueryRecordOptions(QueryRecordOption::Default) & ~QueryRecordOptions(QueryRecordOption::AddLimitTo1));

    QRegularExpression versionRe(QLatin1String("^(\\d+)\\.(\\d+)\\.(\\d+)$"));
    QRegularExpressionMatch match  = versionRe.match(versionString);
    if (res == false) // sanity
        return false;
    if (match.hasMatch()) {
        // (if querySingleString failed, the version will be 0.0.0...
        version->setMajor(match.captured(1).toInt());
        version->setMinor(match.captured(2).toInt());
        version->setRelease(match.captured(3).toInt());
    }
    return true;
}

bool MysqlConnection::drv_disconnect()
{
    return d->db_disconnect();
}

KDbCursor* MysqlConnection::prepareQuery(const KDbEscapedString& sql, KDbCursor::Options options)
{
    return new MysqlCursor(this, sql, options);
}

KDbCursor* MysqlConnection::prepareQuery(KDbQuerySchema* query, KDbCursor::Options options)
{
    return new MysqlCursor(this, query, options);
}

bool MysqlConnection::drv_getDatabasesList(QStringList* list)
{
    mysqlDebug();
    list->clear();
    MYSQL_RES *res = mysql_list_dbs(d->mysql, nullptr);
    if (res != nullptr) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr) {
            *list << QString::fromUtf8(row[0]);
        }
        mysql_free_result(res);
        return true;
    }
    storeResult();
    return false;
}

bool MysqlConnection::drv_databaseExists(const QString &dbName, bool ignoreErrors)
{
    /* db names can be lower case in mysql */
    const QString storedDbName(d->lowerCaseTableNames ? dbName.toLower() : dbName);
    const tristate result = resultExists(
        KDbEscapedString("SHOW DATABASES LIKE %1").arg(escapeString(storedDbName)));
    if (result == true) {
        return true;
    }
    if (!ignoreErrors) {
        m_result = KDbResult(ERR_OBJECT_NOT_FOUND,
                             tr("The database \"%1\" does not exist.").arg(storedDbName));
    }
    return false;
}

bool MysqlConnection::drv_createDatabase(const QString &dbName)
{
    const QString storedDbName(d->lowerCaseTableNames ? dbName.toLower() : dbName);
    mysqlDebug() << storedDbName;
    // mysql_create_db deprecated, use SQL here.
    // db names are lower case in mysql
    return drv_executeSql(KDbEscapedString("CREATE DATABASE %1").arg(escapeIdentifier(storedDbName)));
}

bool MysqlConnection::drv_useDatabase(const QString &dbName, bool *cancelled, KDbMessageHandler* msgHandler)
{
    Q_UNUSED(cancelled);
    Q_UNUSED(msgHandler);
//! @todo is here escaping needed?
    const QString storedDbName(d->lowerCaseTableNames ? dbName.toLower() : dbName);
    if (!d->useDatabase(storedDbName)) {
        storeResult();
        return false;
    }
    return true;
}

bool MysqlConnection::drv_closeDatabase()
{
//! @todo free resources, as far as I know, mysql doesn't support that
    return true;
}

bool MysqlConnection::drv_dropDatabase(const QString &dbName)
{
//! @todo is here escaping needed?
    const QString storedDbName(d->lowerCaseTableNames ? dbName.toLower() : dbName);
    return drv_executeSql(KDbEscapedString("DROP DATABASE %1").arg(escapeIdentifier(storedDbName)));
}

KDbSqlResult* MysqlConnection::drv_prepareSql(const KDbEscapedString& sql)
{
    if (!drv_executeSql(sql)) {
        return nullptr;
    }
    MYSQL_RES *data = mysql_use_result(d->mysql); // more optimal than mysql_store_result
    //! @todo use mysql_error()
    return new MysqlSqlResult(this, data);
}

bool MysqlConnection::drv_executeSql(const KDbEscapedString& sql)
{
    if (!d->executeSql(sql)) {
        storeResult();
        return false;
    }
    return true;
}

QString MysqlConnection::serverResultName() const
{
    return MysqlConnectionInternal::serverResultName(d->mysql);
}

tristate MysqlConnection::drv_containsTable(const QString& tableName)
{
    return resultExists(KDbEscapedString("SHOW TABLES LIKE %1")
                        .arg(escapeString(tableName)));
}

KDbPreparedStatementInterface* MysqlConnection::prepareStatementInternal()
{
    return new MysqlPreparedStatement(d);
}

void MysqlConnection::storeResult()
{
    d->storeResult(&m_result);
}

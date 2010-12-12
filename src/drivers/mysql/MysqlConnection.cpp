/* This file is part of the KDE project
   Copyright (C) 2002 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2003 Joseph Wenninger<jowenn@kde.org>
   Copyright (C) 2004-2010 Jarosław Staniek <staniek@kde.org>

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

#include <QVariant>
#include <QFile>
#include <QRegExp>
#include <QtDebug>

#include "MysqlDriver.h"
#include "MysqlConnection.h"
#include "MysqlConnection_p.h"
#include "MysqlCursor.h"
#include "MysqlPreparedStatement.h"
#include <Predicate/Global.h>
#include <Predicate/Error.h>


using namespace Predicate;

//--------------------------------------------------------------------------

MysqlConnection::MysqlConnection(Driver *driver, const ConnectionData& connData)
        : Connection(driver, connData)
        , d(new MysqlConnectionInternal(this))
{
}

MysqlConnection::~MysqlConnection()
{
    destroy();
    delete d;
}

bool MysqlConnection::drv_connect(Predicate::ServerVersionInfo* version)
{
    const bool ok = d->db_connect(data());
    if (!ok)
        return false;

    // http://dev.mysql.com/doc/refman/5.1/en/mysql-get-server-info.html
    version->setString(mysql_get_server_info(d->mysql));

    // get the version info using 'version' built-in variable:
//! @todo this is hardcoded for now; define api for retrieving variables and use this API...
    // http://dev.mysql.com/doc/refman/5.1/en/mysql-get-server-version.html
    QString versionString;
    tristate res = querySingleString(EscapedString("SELECT @@version"),
                                     &versionString, /*column*/0, false /*!addLimitTo1*/);
    QRegExp versionRe("(\\d+)\\.(\\d+)\\.(\\d+)");
    if (res == true && versionRe.exactMatch(versionString)) {
        // (if querySingleString failed, the version will be 0.0.0...
        version->setMajor(versionRe.cap(1).toInt());
        version->setMinor(versionRe.cap(2).toInt());
        version->setRelease(versionRe.cap(3).toInt());
    }

    // Get lower_case_table_name value so we know if there's case sensitivity supported
    // See http://dev.mysql.com/doc/refman/5.0/en/identifier-case-sensitivity.html
    int intLowerCaseTableNames = 0;
    res = querySingleNumber(EscapedString("SHOW VARIABLES LIKE 'lower_case_table_name'"),
                            &intLowerCaseTableNames,
                            0/*col*/, false/* !addLimitTo1 */);
    if (res == false) // sanity
        return false;
    d->lowerCaseTableNames = intLowerCaseTableNames > 0;
    return true;
}

bool MysqlConnection::drv_disconnect()
{
    return d->db_disconnect();
}

Cursor* MysqlConnection::prepareQuery(const EscapedString& statement, uint cursor_options)
{
    return new MysqlCursor(this, statement, cursor_options);
}

Cursor* MysqlConnection::prepareQuery(QuerySchema* query, uint cursor_options)
{
    return new MysqlCursor(this, query, cursor_options);
}

bool MysqlConnection::drv_getDatabasesList(QStringList* list)
{
    PreDrvDbg;
    list->clear();
    MYSQL_RES *res = mysql_list_dbs(d->mysql, 0);
    if (res != 0) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != 0) {
            *list << QString(row[0]);
        }
        mysql_free_result(res);
        return true;
    }
    d->storeResult();
    return false;
}

bool MysqlConnection::drv_databaseExists(const QString &dbName, bool ignoreErrors)
{
    bool success;
    /* db names can be lower case in mysql */
    const QString storedDbName(d->lowerCaseTableNames ? dbName.toLower() : dbName);
    bool exists = resultExists(
      EscapedString("SHOW DATABASES LIKE %1").arg(escapeString(storedDbName)), &success);
    if (!exists || !success) {
        if (!ignoreErrors) {
            m_result = Result(ERR_OBJECT_NOT_FOUND,
                              QObject::tr("The database \"%1\" does not exist.").arg(storedDbName));
        }
        return false;
    }
    return true;
}

bool MysqlConnection::drv_createDatabase(const QString &dbName)
{
    const QString storedDbName(d->lowerCaseTableNames ? dbName.toLower() : dbName);
    PreDrvDbg << storedDbName;
    // mysql_create_db deprecated, use SQL here.
    // db names are lower case in mysql
    if (drv_executeSQL(EscapedString("CREATE DATABASE %1").arg(escapeIdentifier(storedDbName))))
        return true;
    d->storeResult();
    return false;
}

bool MysqlConnection::drv_useDatabase(const QString &dbName, bool *cancelled, MessageHandler* msgHandler)
{
    Q_UNUSED(cancelled);
    Q_UNUSED(msgHandler);
//TODO is here escaping needed?
    const QString storedDbName(d->lowerCaseTableNames ? dbName.toLower() : dbName);
    return d->useDatabase(storedDbName);
}

bool MysqlConnection::drv_closeDatabase()
{
//TODO free resources
//As far as I know, mysql doesn't support that
    return true;
}

bool MysqlConnection::drv_dropDatabase(const QString &dbName)
{
//TODO is here escaping needed
    const QString storedDbName(d->lowerCaseTableNames ? dbName.toLower() : dbName);
    return drv_executeSQL(EscapedString("DROP DATABASE %1").arg(escapeIdentifier(storedDbName)));
}

bool MysqlConnection::drv_executeSQL(const EscapedString& statement)
{
    return d->executeSQL(statement);
}

quint64 MysqlConnection::drv_lastInsertRecordId()
{
    //! @todo
    return static_cast<quint64>(mysql_insert_id(d->mysql));
}

QString MysqlConnection::serverResultName() const
{
#warning TODO: MysqlConnection::serverResultName()
    return QString();
}

/*void MysqlConnection::drv_clearServerResult()
{
    if (!d)
        return;
    d->res = 0;
}*/

bool MysqlConnection::drv_containsTable(const QString& tableName)
{
    bool success;
    return resultExists(EscapedString("SHOW TABLES LIKE %1")
                        .arg(escapeString(tableName)), &success) && success;
}

bool MysqlConnection::drv_getTablesList(QStringList* list)
{
    return queryStringList(EscapedString("SHOW TABLES"), list);
}

PreparedStatementInterface* MysqlConnection::prepareStatementInternal()
{
    return new MysqlPreparedStatement(d);
}

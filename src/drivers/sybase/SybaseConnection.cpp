/* This file is part of the KDE project
   Copyright (C) 2007 Sharan Rao <sharanrao@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this program; see the file COPYING. If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <QRegExp>

#include <kgenericfactory.h>

#include "SybaseDriver.h"
#include "SybaseConnection.h"
#include "SybaseConnection_p.h"
#include "SybaseCursor.h"
#include "SybasePreparedStatement.h"
#include "KDbError.h"

SybaseConnection::SybaseConnection(KDbDriver *driver, const KDbConnectionData& connData)
        : KDbConnection(driver, connData)
        , d(new SybaseConnectionInternal(this))
{
}

SybaseConnection::~SybaseConnection()
{
    destroy();
}

bool SybaseConnection::drv_connect(KDbServerVersionInfo* version)
{
    const bool ok = d->db_connect(*data());
    if (!ok)
        return false;

    // we can retrieve the server name and the server version using global variables
    // @@servername
    // @@version

    QString serverVersionString;

    if (!querySingleString(KDbEscapedString("SELECT @@servername") , &version.string)) {
        sybaseWarning() << "Couldn't fetch server name";
    }

    if (!querySingleString(KDbEscapedString("SELECT @@version"), &serverVersionString)) {
        sybaseWarning() << "Couldn't fetch server version";
    }

    QRegExp versionRe("(\\d+)\\.(\\d+)\\.(\\d+)\\.(\\d+)");
    if (versionRe.exactMatch(serverVersionString)) {
        version.major = versionRe.cap(1).toInt();
        version.minor = versionRe.cap(2).toInt();
        version.release = versionRe.cap(3).toInt();
    }

    return true;
}

bool SybaseConnection::drv_disconnect()
{
    return d->db_disconnect();
}

KDbCursor* SybaseConnection::prepareQuery(const KDbEscapedString& sql, int cursor_options)
{
    return new SybaseCursor(this, sql, cursor_options);
}

KDbCursor* SybaseConnection::prepareQuery(KDbQuerySchema* query, int cursor_options)
{
    return new SybaseCursor(this, query, cursor_options);
}

bool SybaseConnection::drv_getDatabasesList(QStringList* list)
{
    // select * from master..sysdatabases ?
    // todo: verify.
    return queryStringList(KDbEscapedString("SELECT name FROM master..sysdatabases"), list) ;
}

bool SybaseConnection::drv_createDatabase(const QString &dbName)
{
    //sybaseDebug() << dbName;
    // mysql_create_db deprecated, use SQL here.
    if (drv_executeSQL(KDbEscapedString("CREATE DATABASE ") + dbName)) {
        // set allow_nulls_by_default option to true
        KDbEscapedString allowNullsQuery = KDbEscapedString("sp_dboption %1, allow_nulls_by_default, true").arg(dbName);
        if (drv_executeSQL(allowNullsQuery.data()))
            return true;
    }
    d->storeResult();
    return false;
}

bool SybaseConnection::drv_useDatabase(const QString &dbName, bool *cancelled, KDbMessageHandler* msgHandler)
{
    Q_UNUSED(cancelled);
    Q_UNUSED(msgHandler);

    //! @todo is here escaping needed?
    return d->useDatabase(dbName) ;
}

bool SybaseConnection::drv_closeDatabase()
{
// here we disconenct the connection
    return true;
}

bool SybaseConnection::drv_dropDatabase(const QString &dbName)
{

    return drv_executeSQL(KDbEscapedString("DROP DATABASE ") + escapeString(dbName));
}

bool SybaseConnection::drv_executeSQL(const KDbEscapedString& sql)
{
    return d->executeSQL(sql);
}

quint64 SybaseConnection::drv_lastInsertRecordId()
{
    int rowId = 0;
    querySingleNumber(KDbEscapedString("Select @@IDENTITY"), &rowId);
    return (qint64)rowId;
}

int SybaseConnection::serverResult()
{
    return d->res;
}

QString SybaseConnection::serverResultName() const
{
    return QString();
}

/*void SybaseConnection::drv_clearServerResult()
{
    if (!d)
        return;
    d->res = 0;
}*/

bool SybaseConnection::drv_containsTable(const QString &tableName)
{
    return resultExists(KDbEscapedString("SELECT name FROM sysobjects WHERE type='U' AND name=%1")
                        .arg(escapeString(tableName)));
}

bool SybaseConnection::drv_getTablesList(QStringList* list)
{
    return queryStringList(KDbEscapedString("SELECT name FROM sysobjects WHERE type='U'"), list);
}

KDbPreparedStatement SybaseConnection::prepareStatement(KDbPreparedStatement::StatementType type,
        KDbFieldList* fields)
{
    return SybasePreparedStatement(type, *d, fields);
}

bool KDbSybaseConnection::drv_beforeInsert(const QString& table, KDbFieldList* fields)
{

    if (fields.autoIncrementFields()->isEmpty())
        return true;

    // explicit insertion into IDENTITY fields !!
    return drv_executeSQL(KDbEscapedString("SET IDENTITY_INSERT %1 ON").arg(escapeIdentifier(table)));

}

bool KDbSybaseConnection::drv_afterInsert(const QString& table, KDbFieldList* fields)
{
    // should we instead just set a flag when an identity_insert has taken place and only check for that
    // flag here ?

    if (fields.autoIncrementFields()->isEmpty())
        return true;

    // explicit insertion into IDENTITY fields has taken place. Turn off IDENTITY_INSERT
    return drv_executeSQL(KDbEscapedString("SET IDENTITY_INSERT %1 OFF").arg(escapeIdentifier(table)));

}

bool KDbSybaseConnection::drv_beforeUpdate(const QString& table, KDbFieldList* fields)
{
    if (fields->autoIncrementFields()->isEmpty())
        return true;

    // explicit update of IDENTITY fields has taken place.
    return drv_executeSQL(KDbEscapedString("SET IDENTITY_UPDATE %1 ON").arg(escapeIdentifier(table)));
}

bool KDbSybaseConnection::drv_afterUpdate(const QString& table, KDbFieldList& fields)
{
    // should we instead just set a flag when an identity_update has taken place and only check for that
    // flag here ?

    if (fields.autoIncrementFields()->isEmpty())
        return true;

    // explicit insertion into IDENTITY fields has taken place. Turn off IDENTITY_INSERT
    return drv_executeSQL(KDbEscapedString("SET IDENTITY_UPDATE %1 OFF").arg(escapeIdentifier(table)));
}

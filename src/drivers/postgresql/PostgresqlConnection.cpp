/* This file is part of the KDE project
   Copyright (C) 2003 Adam Pigg <adam@piggz.co.uk>
   Copyright (C) 2010-2016 Jarosław Staniek <staniek@kde.org>

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

#include "PostgresqlConnection.h"
#include "PostgresqlConnection_p.h"
#include "PostgresqlPreparedStatement.h"
#include "PostgresqlCursor.h"
#include "postgresql_debug.h"
#include "KDbError.h"
#include "KDbGlobal.h"

#include <QFileInfo>
#include <QHostAddress>

#define MIN_SERVER_VERSION_MAJOR 7
#define MIN_SERVER_VERSION_MINOR 1

inline static QByteArray parameter(PGconn *conn, const char *paramName)
{
    return PQparameterStatus(conn, paramName);
}

PostgresqlTransactionData::PostgresqlTransactionData(KDbConnection *conn)
        : KDbTransactionData(conn)
{
}

PostgresqlTransactionData::~PostgresqlTransactionData()
{
}

//==================================================================================

PostgresqlConnection::PostgresqlConnection(KDbDriver *driver, const KDbConnectionData& connData,
                                           const KDbConnectionOptions &options)
        : KDbConnection(driver, connData, options)
        , d(new PostgresqlConnectionInternal(this))
{
}

PostgresqlConnection::~PostgresqlConnection()
{
    //delete m_trans;
    destroy();
    delete d;
}

KDbCursor* PostgresqlConnection::prepareQuery(const KDbEscapedString& sql,  int cursor_options)
{
    Q_UNUSED(cursor_options);
    return new PostgresqlCursor(this, sql, 1); //Always used buffered cursor
}

KDbCursor* PostgresqlConnection::prepareQuery(KDbQuerySchema* query, int cursor_options)
{
    Q_UNUSED(cursor_options);
    return new PostgresqlCursor(this, query, 1);//Always used buffered cursor
}

bool PostgresqlConnection::drv_connect()
{
    return true;
}

bool PostgresqlConnection::drv_getServerVersion(KDbServerVersionInfo* version)
{
    // http://www.postgresql.org/docs/8.4/static/libpq-status.html
    //postgresqlDebug() << "server_version:" << d->parameter("server_version");
    version->setString(QString::fromLatin1(parameter(d->conn, "server_version")));

    QString versionString;
    int versionNumber = PQserverVersion(d->conn);
    if (versionNumber > 0) {
        version->setMajor(versionNumber / 10000);
        version->setMinor((versionNumber % 1000) / 100);
        version->setRelease(versionNumber % 100);
    }

    if (   version->major() < MIN_SERVER_VERSION_MAJOR
        || (version->major() == MIN_SERVER_VERSION_MAJOR && version->minor() < MIN_SERVER_VERSION_MINOR))
    {
        postgresqlWarning()
            << QString::fromLatin1("PostgreSQL %d.%d is not supported and may not work. The minimum is %d.%d")
               .arg(version->major()).arg(version->minor()).arg(MIN_SERVER_VERSION_MAJOR).arg(MIN_SERVER_VERSION_MINOR);
    }
    return true;
}

bool PostgresqlConnection::drv_disconnect()
{
    return true;
}

bool PostgresqlConnection::drv_getDatabasesList(QStringList* list)
{
    return queryStringList(KDbEscapedString("SELECT datname FROM pg_database WHERE datallowconn = TRUE"), list);
}

bool PostgresqlConnection::drv_createDatabase(const QString &dbName)
{
    return executeVoidSQL(KDbEscapedString("CREATE DATABASE ") + escapeIdentifier(dbName));
}

QByteArray buildConnParameter(const QByteArray& key, const QVariant& value)
{
    QByteArray result = key;
//! @todo optimize
    result.replace('\\', "\\\\").replace('\'', "\\'");
    return key + "='" + value.toString().toUtf8() + "' ";
}

bool PostgresqlConnection::drv_useDatabase(const QString &dbName, bool *cancelled,
                                           KDbMessageHandler* msgHandler)
{
    Q_UNUSED(cancelled);
    Q_UNUSED(msgHandler);

    QByteArray conninfo;

    if (data().hostName().isEmpty()
        || 0 == QString::compare(data().hostName(), QLatin1String("localhost"), Qt::CaseInsensitive))
    {
        if (!data().localSocketFileName().isEmpty()) {
            QFileInfo fileInfo(data().localSocketFileName());
            if (fileInfo.exists()) {
                conninfo += buildConnParameter("host", fileInfo.absolutePath());
            }
        }
    }
    else {
        const QHostAddress ip(data().hostName());
        if (ip.isNull()) {
            conninfo += buildConnParameter("host", data().hostName());
        }
        else {
            conninfo += buildConnParameter("hostaddr", ip.toString());
        }
    }

    //Build up the connection string
    if (data().port() > 0)
        conninfo += buildConnParameter("port", data().port());

    QString myDbName = dbName;
    if (myDbName.isEmpty())
        myDbName = data().databaseName();
    if (!myDbName.isEmpty())
        conninfo += buildConnParameter("dbname", myDbName);

    if (!data().userName().isEmpty())
        conninfo += buildConnParameter("user", data().userName());

    if (!data().password().isEmpty())
        conninfo += buildConnParameter("password", data().password());

    //postgresqlDebug() << conninfo;

    //! @todo other parameters: connect_timeout, options, options, sslmode, sslcert, sslkey, sslrootcert, sslcrl, krbsrvname, gsslib, service
    // http://www.postgresql.org/docs/8.4/interactive/libpq-connect.html
    d->conn = PQconnectdb(conninfo.constData());

    if (!d->connectionOK()) {
        PQfinish(d->conn);
        d->conn = 0;
        return false;
    }

    // pgsql 8.1 changed the default to no oids but we need them
    PGresult* result = PQexec(d->conn, "SET DEFAULT_WITH_OIDS TO ON");
    int status = PQresultStatus(result);
    PQclear(result);

    // initialize encoding
    result = PQexec(d->conn, "SET CLIENT_ENCODING TO 'UNICODE'");
    status = PQresultStatus(result);
    PQclear(result);
    d->unicode = status == PGRES_COMMAND_OK;

    result = PQexec(d->conn, "SET DATESTYLE TO 'ISO'");
    status = PQresultStatus(result);
    if (status != PGRES_COMMAND_OK) {
        postgresqlWarning() << "Failed to set DATESTYLE to 'ISO':" << PQerrorMessage(d->conn);
    }
    //! @todo call on first use of SOUNDEX(), etc.;
    //!       it's not possible now because we don't have connection context in KDbFunctionExpressionData
    drv_executeVoidSQL(KDbEscapedString("CREATE EXTENSION fuzzystrmatch"));
    PQclear(result);
    return true;
}

bool PostgresqlConnection::drv_closeDatabase()
{
    PQfinish(d->conn);
    d->conn = 0;
    return true;
}

bool PostgresqlConnection::drv_dropDatabase(const QString &dbName)
{
    //postgresqlDebug() << dbName;

    //! @todo Maybe should check that dbname is no the currentdb
    if (executeVoidSQL(KDbEscapedString("DROP DATABASE ") + escapeIdentifier(dbName)))
        return true;

    return false;
}

KDbSqlResult* PostgresqlConnection::drv_executeSQL(const KDbEscapedString& sql)
{
    PGresult* result = d->executeSQL(sql);
    const ExecStatusType status = PQresultStatus(result);
    if (status == PGRES_TUPLES_OK || status == PGRES_COMMAND_OK) {
        return new PostgresqlSqlResult(this, result, status);
    }
    storeResult(result, status);
    return nullptr;
}

bool PostgresqlConnection::drv_executeVoidSQL(const KDbEscapedString& sql)
{
    PGresult* result = d->executeSQL(sql);
    const ExecStatusType status = PQresultStatus(result);
    d->storeResultAndClear(&m_result, &result, status);
    return status == PGRES_TUPLES_OK || status == PGRES_COMMAND_OK;
}

bool PostgresqlConnection::drv_isDatabaseUsed() const
{
    return d->conn;
}

tristate PostgresqlConnection::drv_containsTable(const QString &tableName)
{
    return resultExists(KDbEscapedString("SELECT 1 FROM pg_class WHERE relkind='r' AND relname LIKE %1")
                        .arg(escapeString(tableName)));
}

bool PostgresqlConnection::drv_getTablesList(QStringList* list)
{
    return queryStringList(KDbEscapedString("SELECT lower(relname) FROM pg_class WHERE relkind='r'"), list);
}

QString PostgresqlConnection::serverResultName() const
{
    if (m_result.code() >= 0 && m_result.code() <= PGRES_SINGLE_TUPLE) {
        return QString::fromLatin1(PQresStatus(ExecStatusType(m_result.code())));
    }
    return QString();
}

KDbPreparedStatementInterface* PostgresqlConnection::prepareStatementInternal()
{
    return new PostgresqlPreparedStatement(d);
}

KDbEscapedString PostgresqlConnection::escapeString(const QByteArray& str) const
{
    int error;
    d->escapingBuffer.resize(str.length() * 2 + 1);
    size_t count = PQescapeStringConn(d->conn,
                                      d->escapingBuffer.data(), str.constData(), str.length(),
                                      &error);
    d->escapingBuffer.resize(count);

    if (error != 0) {
        d->storeResult(const_cast<KDbResult*>(&m_result));
        const_cast<KDbResult&>(m_result) = KDbResult(ERR_INVALID_ENCODING,
                          PostgresqlConnection::tr("Escaping string failed. Invalid multibyte encoding."));
        return KDbEscapedString();
    }
    return KDbEscapedString("\'") + d->escapingBuffer + '\'';
}

KDbEscapedString PostgresqlConnection::escapeString(const QString& str) const
{
    return escapeString(d->unicode ? str.toUtf8() : str.toLocal8Bit());
}

void PostgresqlConnection::storeResult(PGresult *pgResult, ExecStatusType execStatus)
{
    d->storeResultAndClear(&m_result, &pgResult, execStatus);
}

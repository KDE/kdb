/* This file is part of the KDE project
   Copyright (C) 2003 Adam Pigg <adam@piggz.co.uk>
   Copyright (C) 2010 Jarosław Staniek <staniek@kde.org>

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

#include "PostgresqlPreparedStatement.h"
#include "PostgresqlConnection_p.h"
#include "PostgresqlCursor.h"
#include <Predicate/Error>
#include <Predicate/Global>

#include <QFileInfo>
#include <QHostAddress>
#include <QtDebug>

#define MIN_SERVER_VERSION_MAJOR 7
#define MIN_SERVER_VERSION_MINOR 1

using namespace Predicate;

PostgresqlTransactionData::PostgresqlTransactionData(Connection *conn)
        : TransactionData(conn)
{
}

PostgresqlTransactionData::~PostgresqlTransactionData()
{
}

//==================================================================================

PostgresqlConnection::PostgresqlConnection(Driver *driver, const ConnectionData& connData)
        : Connection(driver, connData)
        , d(new PostgresqlConnectionInternal(this))
{
}

//==================================================================================
//Do any tidying up before the object is deleted
PostgresqlConnection::~PostgresqlConnection()
{
    //delete m_trans;
    destroy();
    delete d;
}

//==================================================================================
//Return a new query based on a query statment
Cursor* PostgresqlConnection::prepareQuery(const EscapedString& statement,  uint cursor_options)
{
    Q_UNUSED(cursor_options);
    return new PostgresqlCursor(this, statement, 1); //Always used buffered cursor
}

//==================================================================================
//Return a new query based on a query object
Cursor* PostgresqlConnection::prepareQuery(QuerySchema* query, uint cursor_options)
{
    Q_UNUSED(cursor_options);
    return new PostgresqlCursor(this, query, 1);//Always used buffered cursor
}

//==================================================================================
//Made this a noop
//We tell we are connected, but we wont actually connect until we use a database!
bool PostgresqlConnection::drv_connect()
{
    PreDrvDbg;
    return true;
}

bool PostgresqlConnection::drv_getServerVersion(Predicate::ServerVersionInfo* version)
{
    // http://www.postgresql.org/docs/8.4/static/libpq-status.html
    qDebug() << "server_version:" << d->parameter("server_version");
    version->setString(d->parameter("server_version"));

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
        qWarning(
            "PostgreSQL %d.%d is not supported and may not work. The minimum is %d.%d",
            version->major(), version->minor(), MIN_SERVER_VERSION_MAJOR, MIN_SERVER_VERSION_MINOR);
    }
    return true;
}

//==================================================================================
//Made this a noop
//We tell kexi wehave disconnected, but it is actually handled by closeDatabse
bool PostgresqlConnection::drv_disconnect()
{
    PreDrvDbg;
    return true;
}

//==================================================================================
//Return a list of database names
bool PostgresqlConnection::drv_getDatabasesList(QStringList* list)
{
    return queryStringList(EscapedString("SELECT datname FROM pg_database WHERE datallowconn = TRUE"), list);
}

//==================================================================================
//Create a new database
bool PostgresqlConnection::drv_createDatabase(const QString &dbName)
{
    return executeSQL(EscapedString("CREATE DATABASE ") + escapeIdentifier(dbName));
}

QByteArray buildConnParameter(const QByteArray& key, const QVariant& value)
{
    QByteArray result = key;
//! @todo optimize
    result.replace('\\', "\\\\").replace('\'', "\\'");
    return key + "='" + value.toString().toUtf8() + "' ";
}

//==================================================================================
//Use this as our connection instead of connect
bool PostgresqlConnection::drv_useDatabase(const QString &dbName, bool *cancelled,
                                           MessageHandler* msgHandler)
{
    Q_UNUSED(cancelled);
    Q_UNUSED(msgHandler);

    QByteArray conninfo;

    if (data().hostName().isEmpty() || data().hostName() == "localhost") {
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

    qDebug() << conninfo;

    //! @todo other parameters: connect_timeout, options, options, sslmode, sslcert, sslkey, sslrootcert, sslcrl, krbsrvname, gsslib, service
    // http://www.postgresql.org/docs/8.4/interactive/libpq-connect.html
    d->conn = PQconnectdb(conninfo);

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
    if (status != PGRES_COMMAND_OK)
        qWarning("Failed to set DATESTYLE to 'ISO': %1", PQerrorMessage(d->conn));
    PQclear(result);
    return true;
}

//==================================================================================
//Here we close the database connection
bool PostgresqlConnection::drv_closeDatabase()
{
    PreDrvDbg;
// if (isConnected())
// {
    PQclear(d->res);
    d->res = 0;
    PQfinish(d->conn);
    d->conn = 0;
    return true;
// }
    /* js: not needed, right?
      else
      {
        d->errmsg = "Not connected to database backend";
        d->res = ERR_NO_CONNECTION;
      }
      return false;*/
}

//==================================================================================
//Drops the given database
bool PostgresqlConnection::drv_dropDatabase(const QString &dbName)
{
    PreDrvDbg << dbName;

    //FIXME Maybe should check that dbname is no the currentdb
    if (executeSQL(EscapedString("DROP DATABASE ") + escapeIdentifier(dbName)))
        return true;

    return false;
}

//==================================================================================
//Execute an SQL statement
bool PostgresqlConnection::drv_executeSQL(const EscapedString& statement)
{
    return d->executeSQL(statement, PGRES_COMMAND_OK);
}

//==================================================================================
//Return true if currently connected to a database, ignoring the m_is_connected flag.
bool PostgresqlConnection::drv_isDatabaseUsed() const
{
    return d->conn;
}

//==================================================================================
//Return the oid of the last insert - only works if sql was insert of 1 row
quint64 PostgresqlConnection::drv_lastInsertRecordId()
{
    // InvalidOid is 0, so the cast is OK
    return static_cast<quint64>(PQoidValue(d->res));
}

bool PostgresqlConnection::drv_containsTable(const QString &tableName)
{
    bool success;
    return resultExists(EscapedString("SELECT 1 FROM pg_class WHERE relkind='r' AND relname LIKE %1")
                        .arg(escapeString(tableName)), &success) && success;
}

bool PostgresqlConnection::drv_getTablesList(QStringList* list)
{
    return queryStringList(EscapedString("SELECT lower(relname) FROM pg_class WHERE relkind='r'"), list);
}

/*pred
TransactionData* PostgresqlConnection::drv_beginTransaction()
{
    return new PostgresqlTransactionData(this);
}

bool PostgresqlConnection::drv_commitTransaction(TransactionData *tdata)
{
    bool result = true;
    try {
        static_cast<PostgresqlTransactionData*>(tdata)->data->commit();
    } catch (const std::exception &e) {
        //If an error ocurred then put the error description into _dbError
        d->errmsg = QString::fromUtf8(e.what());
        result = false;
    } catch (...) {
        //! @todo
        setError();
        result = false;
    }
    if (m_trans == tdata)
        m_trans = 0;
    return result;
}

bool PostgresqlConnection::drv_rollbackTransaction(TransactionData *tdata)
{
    bool result = true;
    try {
        static_cast<PostgresqlTransactionData*>(tdata)->data->abort();
    } catch (const std::exception &e) {
        //If an error ocurred then put the error description into _dbError
        d->errmsg = QString::fromUtf8(e.what());

        result = false;
    } catch (...) {
        d->errmsg = QObject::tr("Unknown error.");
        result = false;
    }
    if (m_trans == tdata)
        m_trans = 0;
    return result;
}*/

/*pred
int PostgresqlConnection::serverResult()
{
    return d->resultCode;
}*/

QString PostgresqlConnection::serverResultName() const
{
    if (m_result.serverResultCode() >= 0 && m_result.serverResultCode() <= PGRES_FATAL_ERROR) {
        return QString::fromLatin1(PQresStatus(ExecStatusType(m_result.serverResultCode())));
    }
    return QString();
}

/*void PostgresqlConnection::drv_clearServerResult()
{
    d->resultCode = 0;
}*/

PreparedStatementInterface* PostgresqlConnection::prepareStatementInternal()
{
    return new PostgresqlPreparedStatement(d);
}

EscapedString PostgresqlConnection::escapeString(const QByteArray& str) const
{
    int error;
    d->escapingBuffer.resize(str.length() * 2 + 1);
    size_t count = PQescapeStringConn(d->conn,
                                      d->escapingBuffer.data(), str.constData(), str.length(),
                                      &error);
    d->escapingBuffer.resize(count);

    if (error != 0) {
        d->storeResult();
        const_cast<Result&>(m_result) = Result(ERR_INVALID_ENCODING,
                          QObject::tr("Escaping string failed. Invalid multibyte encoding."));
        return EscapedString();
    }
    return EscapedString("\'") + d->escapingBuffer + '\'';
}

EscapedString PostgresqlConnection::escapeString(const QString& str) const
{
    return escapeString(d->unicode ? str.toUtf8() : str.toLocal8Bit());
}

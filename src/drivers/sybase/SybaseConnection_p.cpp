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

#include <QByteArray>
#include <QStringList>
#include <QApplication>
#include <QFile>
#include <QTemporaryFile>

#include "SybaseConnection_p.h"

#include "KDbConnectionData.h"

QMap<DBPROCESS*, SybaseConnectionInternal*> SybaseConnectionInternal::dbProcessConnectionMap;


int connectionMessageHandler(DBPROCESS* dbproc, DBINT msgno, int msgstate, int severity,
                             char* msgtext, char* srvname, char* procname, int line)
{
    if (!dbproc) {
        return 0;
    }

    SybaseConnectionInternal* conn = SybaseConnectionInternal::dbProcessConnectionMap[dbproc];
    if (conn)
        conn->messageHandler(msgno, msgstate, severity, msgtext, srvname, procname, line);

    return (0);
}

SybaseConnectionInternal::SybaseConnectionInternal(KDbConnection* connection)
        : ConnectionInternal(connection)
        , dbProcess(0)
        , res(0)
{
}

SybaseConnectionInternal::~SybaseConnectionInternal()
{
    if (sybase_owned && dbProcess) {
        dbclose(dbProcess);
        dbProcess = 0;
    }
}

void SybaseConnectionInternal::storeResult()
{
    //sybaseDebug() << "Store Result!!";
    // all message numbers and message texts were handled in the messageHandler
    // so don't do anything here
}

void SybaseConnectionInternal::messageHandler(DBINT msgno, int msgstate, int severity, char* msgtext, char* srvname, char* procname, int line)
{

    Q_UNUSED(msgstate);
    Q_UNUSED(severity);
    Q_UNUSED(srvname);
    Q_UNUSED(procname);
    Q_UNUSED(line);

    res = msgno;
    errmsg = QString::fromLatin1(msgtext);
    //sybaseDebug() << "Message Handler" << res << errmsg;
}

bool SybaseConnectionInternal::db_connect(const KDbConnectionData& data)
{
    if (dbinit() == FAIL)
        return false;

    // set message handler
    dbmsghandle(connectionMessageHandler);

    QByteArray localSocket;
    QString hostName = data.hostName;


    if (data.serverName.isEmpty()) {
        sybaseWarning() << "Can't connect without server name";
        return false;
    }


    // set Error.handlers
    // set message handlers

    LOGINREC* login;

    login = dblogin();
    if (!login) {
        //dbexit();
        return false;
    }

    // umm, copied from pqxx driver.
    if (hostName.isEmpty() || 0 == hostName.compare(QLatin1String("localhost"), Qt::CaseInsensitive)) {
        if (data.useLocalSocketFile) {
            if (data.localSocketFileName.isEmpty()) {
                QStringList sockets;
#ifndef Q_OS_WIN
                sockets.append("/tmp/s.sybase.2638");

                foreach(const QString& socket, sockets) {
                    if (QFile(socket).exists()) {
                        localSocket = socket.toLatin1();
                        break;
                    }
                }
#endif
            } else
                localSocket = QFile::encodeName(data.localSocketFileName);
        } else {
            //we're not using local socket
            hostName = "127.0.0.1";
        }
    }

    QTemporaryFile confFile(QDir::tempPath() + QLatin1String("/kdb_sybase_XXXXXX.conf"));
    confFile.open();

    QTextStream out(&confFile);

    // write global portion
    out << "[global]" << "\n";
    out << " text size = " << 64512 << "\n" ; // Copied from default freetds.conf. is there a more reasonable number?


    // write server portion
    out << '[' << data.serverName << ']' << "\n";
    out << " host = " << hostName << "\n";

    if (data.port == 0)
        out << " port = " << 5000 << "\n"; // default port to be used
    else
        out << " port = " << data.port << "\n";

    out << " tds version = " << 5.0 << "\n";

    // set the file to be read as confFile
    dbsetifile(confFile.fileName().toLatin1().data());

    // set Login parameters
    QByteArray pwd(data.password.isNull() ? QByteArray() : data.password.toLatin1());

    DBSETLUSER(login, data.userName.toLatin1());
    DBSETLPWD(login, pwd);
    DBSETLAPP(login, qApp->applicationName().toLatin1());

    // make the connection
    // Host name assumed to be same as servername
    // where are ports specified ? ( in the interfaces file ? )

    dbProcess = dbopen(login, data.serverName.toLatin1().data());

    dbloginfree(login);

    // Set/ Unset quoted identifier ? ?

    if (dbProcess) {
        // add to map
        SybaseConnectionInternal::dbProcessConnectionMap[dbProcess] = this;

        // set buffering to be true
        // what's a reasonable value of no. of rows to be kept in buffer ?
        // dbsetopt( dbProcess, DBBUFFER, "500", -1 );

        // set quoted identifier to be true
        dbsetopt(dbProcess, DBQUOTEDIDENT, "1", -1);

        return true;
    }

    storeResult();

    //dbexit();
// setError(ERR_DB_SPECIFIC,err);
    return false;
}

bool SybaseConnectionInternal::db_disconnect()
{
    dbclose(dbProcess);
    dbProcess = 0;
    return true;
}

bool SybaseConnectionInternal::useDatabase(const QString &dbName)
{
    if (dbuse(dbProcess, dbName.toLatin1().data()) == SUCCEED) {
        return true;
    }

    return false;
}

bool SybaseConnectionInternal::executeSql(const KDbEscapedString& sql)
{
    // remove queries in buffer if any. flush existing results if any
    dbcancel(dbProcess);
    // put query in command bufffer
    dbcmd(dbProcess, sql.constData());
    if (dbsqlexec(dbProcess) == SUCCEED) {
        while (dbresults(dbProcess) != NO_MORE_RESULTS) {
            /* nop */
        }
        return true;
    }

    // Error.handling

    storeResult();
    return false;
}

QString SybaseConnectionInternal::escapeIdentifier(const QString& str) const
{
    return QString(str).replace("'", "''");
}

//--------------------------------------

SybaseCursorData::SybaseCursorData(KDbConnection* connection)
        : SybaseConnectionInternal(connection)
        , numRows(0)
{
    sybase_owned = false;
}

SybaseCursorData::~SybaseCursorData()
{
}

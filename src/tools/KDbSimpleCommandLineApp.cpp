/* This file is part of the KDE project
   Copyright (C) 2006 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KDbSimpleCommandLineApp.h"

#include <QFileInfo>
#include <QTextStream>

#include <KCmdLineArgs>
#include <KComponentData>

#include "KDbConnectionData.h"
#include "KDbDriverManager.h"
#include "KDb.h"

//! @internal used for KDbSimpleCommandLineApp
class Q_DECL_HIDDEN KDbSimpleCommandLineApp::Private
{
public:
    Private()
            : conn(0) {}
    ~Private() {
        if (conn) {
            conn->disconnect();
            delete(KDbConnection*)conn;
        }
    }

    KDbDriverManager manager;
    KComponentData componentData;
    KDbConnectionData connData;
    QPointer<KDbConnection> conn;
private:
    Q_DISABLE_COPY(Private)
};

//-----------------------------------------

KDbSimpleCommandLineApp::KDbSimpleCommandLineApp(
    int argc, char** argv, const KCmdLineOptions &options,
    const char *programName, const char *version,
    const char *shortDescription, KAboutData::LicenseKey licenseType,
    const char *copyrightStatement, const char *text,
    const char *homePageAddress, const char *bugsEmailAddress)
        : KDbObject()
        , d(new Private())
{
    QFileInfo fi(argv[0]);
    QByteArray appName(fi.baseName().toLatin1());
    KCmdLineArgs::init(argc, argv,
                       new KAboutData(appName, 0, tr(programName),
                                      version, tr(shortDescription), licenseType, tr(copyrightStatement), tr(text),
                                      homePageAddress, bugsEmailAddress));

    d->componentData = KComponentData(appName);

    KCmdLineOptions allOptions;

    // add predefined options
    allOptions.add("drv", KLocalizedString(), KDb::defaultFileBasedDriverId().toUtf8());
    allOptions.add("driver <id>", tr("Database driver ID"));
    allOptions.add("u");
    allOptions.add("user <name>", tr("Database user name"));
    allOptions.add("p");
    allOptions.add("password", tr("Prompt for password"));
    allOptions.add("h");
    allOptions.add("host <name>", tr("Host (server) name"));
    allOptions.add("port <number>", tr("Server's port number"));
    allOptions.add("s");
    allOptions.add("local-socket <filename>", tr("Server's local socket filename"));

    // add user options
    allOptions.add(options);

    KCmdLineArgs::addCmdLineOptions(allOptions);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    d->connData.driverId = args->getOption("driver");
    d->connData.userName = args->getOption("user");
    d->connData.hostName = args->getOption("host");
    d->connData.localSocketFileName = args->getOption("local-socket");
    d->connData.port = args->getOption("port").toInt();
    d->connData.useLocalSocketFile = args->isSet("local-socket");

    if (args->isSet("password")) {
        QString userAtHost = d->connData.userName;
        if (!d->connData.userName.isEmpty())
            userAtHost += '@';
        userAtHost += (d->connData.hostName.isEmpty() ? "localhost" : d->connData.hostName);
        QTextStream cout(stdout, IO_WriteOnly);
        cout << tr("Enter password for %1: ", "Enter password for <user>").arg(userAtHost);
//! @todo make use of pty/tty here! (and care about portability)
        QTextStream cin(stdin, QIODevice::ReadOnly);
        cin >> d->connData.password;
        kdbDebug() << d->connData.password;
    }
}

KDbSimpleCommandLineApp::~KDbSimpleCommandLineApp()
{
    closeDatabase();
    delete d;
}

bool KDbSimpleCommandLineApp::openDatabase(const QString& databaseName)
{
    if (!d->conn) {
        if (d->manager.error()) {
            setError(&d->manager);
            return false;
        }

        //get the driver
        KDbDriver *driver = d->manager.driver(d->connData.driverId);
        if (!driver || d->manager.error()) {
            setError(&d->manager);
            return false;
        }

        if (driver->metaData()->isFileBased())
            d->connData.setFileName(databaseName);

        d->conn = driver->createConnection(&d->connData);
        if (!d->conn || driver->error()) {
            setError(driver);
            return false;
        }
    }
    if (d->conn->isConnected()) {
        // db already opened
        if (d->conn->isDatabaseUsed() && d->conn->currentDatabase() == databaseName) //the same: do nothing
            return true;
        if (!closeDatabase()) // differs: close the first
            return false;
    }
    if (!d->conn->connect()) {
        setError(d->conn);
        delete d->conn;
        d->conn = 0;
        return false;
    }

    if (!d->conn->useDatabase(databaseName)) {
        setError(d->conn);
        delete d->conn;
        d->conn = 0;
        return false;
    }
    return true;
}

bool KDbSimpleCommandLineApp::closeDatabase()
{
    if (!d->conn)
        return true;
    if (!d->conn->disconnect()) {
        setError(d->conn);
        return false;
    }
    return true;
}

const KComponentData &KDbSimpleCommandLineApp::componentData() const
{
    return d->componentData;
}

KDbConnectionData* KDbSimpleCommandLineApp::connectionData() const
{
    return &d->connData;
}

KDbConnection* KDbSimpleCommandLineApp::connection() const
{
    return d->conn;
}

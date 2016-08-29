/* This file is part of the KDE project
   Copyright (C) 2015 Jarosław Staniek <staniek@kde.org>

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

#include "KDbTestUtils.h"

#include <KDbDriverManager>
#include <KDbDriverMetaData>
#include <KDbConnection>
#include <KDbConnectionOptions>

#include <QFile>
#include <QTest>
#include <QMimeDatabase>

#include "../tests/features/tables_test_p.h"

KDbTestUtils::KDbTestUtils()
    : connection(0)
{
}

void KDbTestUtils::testDriverManager()
{
    QCoreApplication::addLibraryPath(KDB_LOCAL_PLUGINS_DIR); // make plugins work without installing them
    QStringList ids = manager.driverIds();
    qDebug() << "DRIVERS:" << ids;
    QVERIFY2(!manager.result().isError(), "Error in driver manager");
    qDebug() << manager.result().message();
    QVERIFY2(!ids.isEmpty(), "No db drivers found");
}

void KDbTestUtils::testDriver(const QString &driverId, bool fileBased, const QStringList &mimeTypes)
{
    // find the metadata
    const KDbDriverMetaData* driverMetaData;
    KDB_VERIFY(manager.resultable(), driverMetaData = manager.driverMetaData(driverId), "Driver metadata not found");
    QCOMPARE(driverMetaData->id(), driverId);
    QCOMPARE(driverMetaData->isFileBased(), fileBased);
    // test the mimetypes
    QStringList foundMimeTypes(driverMetaData->mimeTypes());
    foundMimeTypes.sort();
    QStringList expectedMimeTypes(mimeTypes);
    expectedMimeTypes.sort();
    qDebug() << "mimeTypes:" << mimeTypes;
    QCOMPARE(foundMimeTypes, expectedMimeTypes);
    QVERIFY(!KDb::defaultFileBasedDriverMimeType().isEmpty());
    QMimeDatabase mimeDb;
    foreach(const QString &mimeName, expectedMimeTypes) {
        QVERIFY2(mimeDb.mimeTypeForName(mimeName).isValid(),
                 qPrintable(QString("%1 MIME type not found in the MIME database").arg(mimeName)));
    }
    // find driver for the metadata
    KDB_VERIFY(manager.resultable(), driver = manager.driver(driverId), "Driver not found");
}

void KDbTestUtils::testSqliteDriver()
{
    QStringList mimeTypes;
    mimeTypes << "application/x-kexiproject-sqlite3" << "application/x-sqlite3";
    testDriver("org.kde.kdb.sqlite",
               true, // file-based
               mimeTypes);
    QVERIFY2(mimeTypes.contains(KDb::defaultFileBasedDriverMimeType()), "SQLite's MIME types should include the default file based one");
}

void KDbTestUtils::testConnect(const KDbConnectionData &cdata)
{
    qDebug() << cdata;

    KDbConnectionOptions connOptions;
    QStringList extraSqliteExtensionPaths;
    extraSqliteExtensionPaths << SQLITE_LOCAL_ICU_EXTENSION_PATH;
    connOptions.insert("extraSqliteExtensionPaths", extraSqliteExtensionPaths);

    connection.reset(); // remove previous connection if present
    const int connCount = driver->connections().count();
    connection.reset(driver->createConnection(cdata, connOptions));
    KDB_VERIFY(driver, !connection.isNull(), "Failed to create connection");
    QVERIFY2(cdata.driverId().isEmpty(), "Connection data has filled driver ID");
    QCOMPARE(connection->data().driverId(), driver->metaData()->id());
    QVERIFY2(driver->connections().contains(connection.data()), "Driver does not list created connection");
    QCOMPARE(driver->connections().count(), connCount + 1); // one more

    const KDbUtils::Property extraSqliteExtensionPathsProperty = connection->options()->property("extraSqliteExtensionPaths");
    QVERIFY2(!extraSqliteExtensionPathsProperty.isNull, "extraSqliteExtensionPaths property not found");
    QCOMPARE(extraSqliteExtensionPathsProperty.value.toStringList(), extraSqliteExtensionPaths);

    const KDbUtils::Property readOnlyProperty = connection->options()->property("readOnly");
    QVERIFY2(!readOnlyProperty.isNull, "readOnly property not found");
    QCOMPARE(readOnlyProperty.value.toBool(), connection->options()->isReadOnly());

    //! @todo Add extensive test for a read-only connection

    KDB_VERIFY(connection, connection->connect(), "Failed to connect");
    KDB_VERIFY(connection, connection->isConnected(), "Database not connected after call to connect()");
}

void KDbTestUtils::testUse()
{
    KDB_VERIFY(connection, connection->databaseExists(connection->data().databaseName()), "Database does not exists");
    KDB_VERIFY(connection, connection->useDatabase(), "Failed to use database");
    KDB_VERIFY(connection, connection->isDatabaseUsed(), "Database not used after call to useDatabase()");
}

void KDbTestUtils::testCreate(const QString &dbName)
{
    //open connection
    KDbConnectionData cdata;
    cdata.setDatabaseName(dbName);

    testConnect(cdata);
    QVERIFY(connection);

    //! @todo KDbDriver::metaData
    {
        QScopedPointer<KDbConnection> connGuard(connection.data());

        if (connection->databaseExists(dbName)) {
            KDB_VERIFY(connection, connection->dropDatabase(dbName), "Failed to drop database");
        }
        KDB_VERIFY(connection, !connection->databaseExists(dbName), "Database exists");
        KDB_VERIFY(connection, connection->createDatabase(dbName), "Failed to create db");
        KDB_VERIFY(connection, connection->databaseExists(dbName), "Database does not exists after creation");
        connGuard.take();
    }
}

void KDbTestUtils::testCreateTables()
{
    QVERIFY2(tablesTest_createTables(connection.data()) == 0, "Failed to create test data");
}

void KDbTestUtils::testDisconnectInternal()
{
    if (!connection) {
        return;
    }
    KDB_VERIFY(connection, connection->closeDatabase(), "Failed to close database");
    KDB_VERIFY(connection, !connection->isDatabaseUsed(), "Database still used after closing");
    KDB_VERIFY(connection, connection->closeDatabase(), "Second closeDatabase() call  should not fail");
    KDB_VERIFY(connection, connection->disconnect(), "Failed to disconnect database");
    KDB_VERIFY(connection, !connection->isConnected(), "Database still connected after disconnecting");
    KDB_VERIFY(connection, connection->disconnect(), "Second disconnect() call should not fail");
}

void KDbTestUtils::testDisconnect()
{
    const int connCount = driver->connections().count();
    testDisconnectInternal();
    connection.reset();
    QCOMPARE(driver->connections().count(), connCount - 1); // one less
}

void KDbTestUtils::testDisconnectAndDropDb()
{
    QString dbName(connection.data()->data().databaseName());
    testDisconnectInternal();
    KDB_VERIFY(connection, connection->dropDatabase(dbName), "Failed to drop database");
    connection.reset();
}

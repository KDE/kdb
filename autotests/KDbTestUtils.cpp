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
#include "KDbUtils_p.h"
#include <KDbConnection>
#include <KDbConnectionData>
#include <KDbConnectionOptions>
#include <KDbDriverManager>
#include <KDbDriverManager_p.h>
#include <KDbDriverMetaData>
#include <KDbProperties>

#include <QFile>
#include <QTest>
#include <QMimeDatabase>

#include "../tests/features/tables_test_p.h"

namespace QTest
{
KDBTESTUTILS_EXPORT bool qCompare(const KDbEscapedString &val1, const KDbEscapedString &val2,
                                  const char *actual, const char *expected, const char *file,
                                  int line)
{
    return val1 == val2
        ? compare_helper(true, "COMPARE()", toString(qPrintable(val1.toString())),
                         toString(qPrintable(val2.toString())), actual, expected, file, line)
        : compare_helper(false, "Compared values are not the same",
                         toString(qPrintable(val1.toString())),
                         toString(qPrintable(val2.toString())), actual, expected, file, line);
}

KDBTESTUTILS_EXPORT bool qCompare(const KDbEscapedString &val1, const char *val2,
                                  const char *actual, const char *expected, const char *file,
                                  int line)
{
    return val1 == val2
        ? compare_helper(true, "COMPARE()", toString(qPrintable(val1.toString())),
                         toString(val2), actual, expected, file, line)
        : compare_helper(false, "Compared values are not the same",
                         toString(qPrintable(val1.toString())),
                         toString(val2), actual, expected, file, line);
}

KDBTESTUTILS_EXPORT bool qCompare(const char *val1, const KDbEscapedString &val2,
                                  const char *actual, const char *expected, const char *file,
                                  int line)
{
    return val1 == val2
        ? compare_helper(true, "COMPARE()", toString(val1), toString(qPrintable(val2.toString())),
                         actual, expected, file, line)
        : compare_helper(false, "Compared values are not the same",
                         toString(val1), toString(qPrintable(val2.toString())),
                         actual, expected, file, line);
}

KDBTESTUTILS_EXPORT bool qCompare(const KDbEscapedString &val1, const QString &val2,
                                  const char *actual, const char *expected, const char *file,
                                  int line)
{
    return val1 == KDbEscapedString(val2)
        ? compare_helper(true, "COMPARE()", toString(qPrintable(val1.toString())),
                         toString(val2), actual, expected, file, line)
        : compare_helper(false, "Compared values are not the same",
                         toString(qPrintable(val1.toString())),
                         toString(val2), actual, expected, file, line);
}

KDBTESTUTILS_EXPORT bool qCompare(const QString &val1, const KDbEscapedString &val2,
                                  const char *actual, const char *expected, const char *file,
                                  int line)
{
    return KDbEscapedString(val1) == val2
        ? compare_helper(true, "COMPARE()", toString(val1), toString(qPrintable(val2.toString())),
                         actual, expected, file, line)
        : compare_helper(false, "Compared values are not the same",
                         toString(val1), toString(qPrintable(val2.toString())),
                         actual, expected, file, line);
}
}

KDbTestUtils::KDbTestUtils()
    : connection(nullptr)
{
    QCoreApplication::addLibraryPath(KDB_LOCAL_PLUGINS_DIR); // make plugins work without installing them
}

void KDbTestUtils::testDriverManagerInternal(bool forceEmpty)
{
    DriverManagerInternal::self()->forceEmpty = forceEmpty;
    QStringList ids = manager.driverIds();
    //qDebug() << "DRIVERS:" << ids;
    QVERIFY2(forceEmpty == manager.result().isError(), "Error in driver manager");
    //qDebug() << manager.result().message();
    QVERIFY2(forceEmpty == ids.isEmpty(), "No db drivers found");
    if (forceEmpty) { // no drivers, so try to find one and expect failure
        ids << "org.kde.kdb.sqlite";
    }
    for (const QString &id : qAsConst(ids)) {
        const KDbDriverMetaData* driverMetaData;
        if (forceEmpty) {
            KDB_EXPECT_FAIL(manager.resultable(), driverMetaData = manager.driverMetaData(id),
                            ERR_DRIVERMANAGER, "Driver metadata not found");
            // find driver for the metadata
            KDB_EXPECT_FAIL(manager.resultable(), driver = manager.driver(id),
                            ERR_DRIVERMANAGER, "Driver not found");
        } else {
            KDB_VERIFY(manager.resultable(), driverMetaData = manager.driverMetaData(id),
                       "Driver metadata not found");
            QCOMPARE(driverMetaData->id(), id);
            // find driver for the metadata
            KDB_VERIFY(manager.resultable(), driver = manager.driver(id), "Driver not found");
        }
    }
    DriverManagerInternal::self()->forceEmpty = false; // default state
}

void KDbTestUtils::testDriverManagerInternal()
{
    testDriverManagerInternal(true);
    testDriverManagerInternal(false);
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
    //qDebug() << "mimeTypes:" << mimeTypes;
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

void KDbTestUtils::testSqliteDriverInternal()
{
    QStringList mimeTypes;
    mimeTypes << "application/x-kexiproject-sqlite3" << "application/x-sqlite3";
    testDriver("org.kde.kdb.sqlite",
               true, // file-based
               mimeTypes);
    QVERIFY2(mimeTypes.contains(KDb::defaultFileBasedDriverMimeType()), "SQLite's MIME types should include the default file based one");
}

void KDbTestUtils::testConnectInternal(const KDbConnectionData &cdata)
{
    //qDebug() << cdata;

    if (!driver) {
        //! @todo don't hardcode SQLite here
        KDB_VERIFY(manager.resultable(), driver = manager.driver("org.kde.kdb.sqlite"), "Driver not found");
    }

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
    QVERIFY2(!extraSqliteExtensionPathsProperty.isNull(), "extraSqliteExtensionPaths property not found");
    QCOMPARE(extraSqliteExtensionPathsProperty.value().type(), QVariant::StringList);
    QCOMPARE(extraSqliteExtensionPathsProperty.value().toStringList(), extraSqliteExtensionPaths);

    const KDbUtils::Property readOnlyProperty = connection->options()->property("readOnly");
    QVERIFY2(!readOnlyProperty.isNull(), "readOnly property not found");
    QCOMPARE(readOnlyProperty.value().toBool(), connection->options()->isReadOnly());

    //! @todo Add extensive test for a read-only connection

    KDB_VERIFY(connection, connection->connect(), "Failed to connect");
    KDB_VERIFY(connection, connection->isConnected(), "Database not connected after call to connect()");
}

void KDbTestUtils::testUseInternal()
{
    KDB_VERIFY(connection, connection->databaseExists(connection->data().databaseName()), "Database does not exists");
    KDB_VERIFY(connection, connection->useDatabase(), "Failed to use database");
    KDB_VERIFY(connection, connection->isDatabaseUsed(), "Database not used after call to useDatabase()");
}

void KDbTestUtils::testCreateDbInternal(const QString &dbName)
{
    //open connection
    KDbConnectionData cdata;
    //! @todo don't hardcode SQLite (.kexi) extension here
    QString fullDbName(QDir::fromNativeSeparators(QFile::decodeName(FILES_OUTPUT_DIR "/")
                       + dbName + ".kexi"));
    cdata.setDatabaseName(fullDbName);

    QVERIFY(testConnect(cdata));
    QVERIFY(connection);

    //! @todo KDbDriver::metaData
    {
        QScopedPointer<KDbConnection> connGuard(connection.take());

        if (connGuard->databaseExists(dbName)) {
            KDB_VERIFY(connGuard, connGuard->dropDatabase(fullDbName), "Failed to drop database");
        }
        KDB_VERIFY(connGuard, !connGuard->databaseExists(fullDbName), "Database exists");
        KDB_VERIFY(connGuard, connGuard->createDatabase(fullDbName), "Failed to create db");
        KDB_VERIFY(connGuard, connGuard->databaseExists(fullDbName), "Database does not exists after creation");
        connection.reset(connGuard.take());
    }
}

void KDbTestUtils::testCreateDbWithTablesInternal(const QString &dbName)
{
    QVERIFY(testCreateDb(dbName));
    KDB_VERIFY(connection, connection->useDatabase(), "Failed to use database");
    testCreateTablesInternal();
}

void KDbTestUtils::testPropertiesInternal()
{
    QStringList properties;
    properties << connection->databaseProperties().names();
    QVERIFY(properties.contains("kexidb_major_ver"));
    bool ok;
    QVERIFY(connection->databaseProperties().value("kexidb_major_ver").toInt(&ok) >= 0);
    QVERIFY(ok);
    QVERIFY(properties.contains("kexidb_minor_ver"));
    QVERIFY(connection->databaseProperties().value("kexidb_minor_ver").toInt(&ok) >= 0);
    QVERIFY(ok);
}

void KDbTestUtils::testCreateTablesInternal()
{
    QVERIFY2(tablesTest_createTables(connection.data()) == 0, "Failed to create test data");
}

void KDbTestUtils::testDisconnectPrivate()
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

void KDbTestUtils::testDisconnectInternal()
{
    const int connCount = driver ? driver->connections().count() : 0;
    testDisconnectPrivate();
    QVERIFY(!QTest::currentTestFailed());
    connection.reset();
    QCOMPARE(driver ? driver->connections().count() : -1, connCount - 1); // one less
}

void KDbTestUtils::testDropDbInternal()
{
    QVERIFY(connection->dropDatabase(connection->data().databaseName()));
}

void KDbTestUtils::testDisconnectAndDropDbInternal()
{
    QString dbName(connection.data()->data().databaseName());
    testDisconnectPrivate();
    QVERIFY(!QTest::currentTestFailed());
    KDB_VERIFY(connection, connection->dropDatabase(dbName), "Failed to drop database");
    connection.reset();
}

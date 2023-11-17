/* This file is part of the KDE project
   Copyright (C) 2015-2019 Jarosław Staniek <staniek@kde.org>

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
#include <KDbNativeStatementBuilder>
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

static char *toString(const QStringList &list)
{
    return toString(qPrintable(QStringLiteral("QStringList(%1)").arg(list.join(", "))));
}

KDBTESTUTILS_EXPORT bool qCompare(const QStringList &val1, const QStringList &val2,
                                  const char *actual, const char *expected, const char *file,
                                  int line)
{
    return val1 == val2 ? compare_helper(true, "COMPARE()", toString(val1), toString(val2), actual,
                                         expected, file, line)
                        : compare_helper(false, "Compared values are not the same", toString(val1),
                                         toString(val2), actual, expected, file, line);
}
KDBTESTUTILS_EXPORT bool qCompare(const QByteArray &val1, const char *val2, const char *actual,
                                  const char *expected, const char *file, int line)
{
    return qCompare(val1, QByteArray(val2), actual, expected, file, line);
}
KDBTESTUTILS_EXPORT bool qCompare(const QString &val1, const char *val2, const char *actual,
                                  const char *expected, const char *file, int line)
{
    return qCompare(val1, QString::fromLatin1(val2), actual, expected, file, line);
}
}

class KDbTestUtils::Private {
public:
    Private() {}
    QScopedPointer<KDbNativeStatementBuilder> kdbBuilder;
    QScopedPointer<KDbNativeStatementBuilder> driverBuilder;

    KDbConnection *connection()
    {
        return m_connection.data();
    }

    void setConnection(KDbConnection *conn)
    {
        kdbBuilder.reset(); // dependency will be removed
        m_connection.reset(conn);
    }

    KDbConnection* takeConnection()
    {
        if (!m_connection) {
            return nullptr;
        }
        kdbBuilder.reset(); // dependency may be removed
        return m_connection.take();
    }

private:
    QScopedPointer<KDbConnection> m_connection;
};

KDbTestUtils::KDbTestUtils()
    : d(new Private)
{
    QCoreApplication::addLibraryPath(KDB_LOCAL_PLUGINS_DIR); // make plugins work without installing them
}

KDbTestUtils::~KDbTestUtils()
{
    delete d;
}

KDbConnection* KDbTestUtils::connection()
{
    return d->connection();
}

KDbNativeStatementBuilder* KDbTestUtils::kdbBuilder()
{
    Q_ASSERT(connection());
    if (connection() && !d->kdbBuilder) {
        d->kdbBuilder.reset(new KDbNativeStatementBuilder(connection(), KDb::KDbEscaping));
    }
    return d->kdbBuilder.data();
}

KDbNativeStatementBuilder* KDbTestUtils::driverBuilder()
{
    Q_ASSERT(connection());
    if (connection() && !d->driverBuilder) {
        d->driverBuilder.reset(new KDbNativeStatementBuilder(connection(), KDb::DriverEscaping));
    }
    return d->driverBuilder.data();
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
        ids << "kdb_sqlitedriver";
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

void KDbTestUtils::testDriver(const QString &driverId, bool fileBased,
                              const QStringList &expectedMimeTypes,
                              const QStringList &possiblyInvalidMimeTypes)
{
    // find the metadata
    const KDbDriverMetaData* driverMetaData;
    KDB_VERIFY(manager.resultable(), driverMetaData = manager.driverMetaData(driverId),
               qPrintable(QStringLiteral("Driver metadata not found for id=%1").arg(driverId)));
    QCOMPARE(driverMetaData->id(), driverId);
    QCOMPARE(driverMetaData->isFileBased(), fileBased);
    // test the mimetypes
    const QStringList foundMimeTypes(driverMetaData->mimeTypes());
    QVERIFY2(!KDb::defaultFileBasedDriverMimeType().isEmpty(),
             qPrintable(QStringLiteral("id=%1").arg(driverId)));
    QMimeDatabase mimeDb;
    for (const QString &mimeName : expectedMimeTypes) {
        if (!mimeDb.mimeTypeForName(mimeName).isValid()) {
            const QString msg = QStringLiteral("MIME type %1 not found in the MIME database").arg(mimeName);
            if (possiblyInvalidMimeTypes.contains(mimeName)) {
                qInfo() << qPrintable(msg);
                continue; // ignore
            } else {
                QVERIFY2(mimeDb.mimeTypeForName(mimeName).isValid(), qPrintable(msg));
            }
        }
        const QStringList ids = manager.driverIdsForMimeType(mimeName);
        QVERIFY2(!ids.isEmpty(),
                 qPrintable(QStringLiteral("No drivers found for MIME type=%1").arg(mimeName)));
        QVERIFY2(ids.contains(driverId),
                 qPrintable(QStringLiteral("No driver with id=%1 found for MIME type=%2")
                                .arg(driverId, mimeName)));
    }
    // each found mime type expected?
    for (const QString &mimeName : foundMimeTypes) {
        QVERIFY2(expectedMimeTypes.contains(mimeName),
                 qPrintable(QStringLiteral("Unexpected MIME type=%1 found for driver with id=%2")
                                .arg(mimeName, driverId)));
    }
    // find driver for the metadata
    KDB_VERIFY(manager.resultable(), driver = manager.driver(driverId), "Driver not found");
}

void KDbTestUtils::testSqliteDriverInternal()
{
    const QStringList mimeTypes { "application/x-kexiproject-sqlite3", "application/x-sqlite3",
                                  "application/x-vnd.kde.kexi", "application/vnd.sqlite3" };
    const QStringList possiblyInvalidMimeTypes { "application/vnd.sqlite3" };
    testDriver("kdb_sqlitedriver", true /* file-based */, mimeTypes, possiblyInvalidMimeTypes);
    QVERIFY2(mimeTypes.contains(KDb::defaultFileBasedDriverMimeType()),
             "SQLite's MIME types should include the default file based one");
}

void KDbTestUtils::testConnectInternal(const KDbConnectionData &cdata,
                                       const KDbConnectionOptions &options)
{
    //qDebug() << cdata;

    if (!driver) {
        //! @todo don't hardcode SQLite here
        KDB_VERIFY(manager.resultable(), driver = manager.driver("kdb_sqlitedriver"), "Driver not found");
    }

    KDbConnectionOptions connOptionsOverride(options);
    QStringList extraSqliteExtensionPaths;
    extraSqliteExtensionPaths << SQLITE_LOCAL_ICU_EXTENSION_PATH;
    connOptionsOverride.insert("extraSqliteExtensionPaths", extraSqliteExtensionPaths);

    d->setConnection(nullptr); // remove previous connection if present
    const int connCount = driver->connections().count();
    d->setConnection(driver->createConnection(cdata, connOptionsOverride));
    KDB_VERIFY(driver, connection(), "Failed to create connection");
    QVERIFY2(cdata.driverId().isEmpty(), "Connection data has filled driver ID");
    QCOMPARE(connection()->data().driverId(), driver->metaData()->id());
    QVERIFY2(driver->connections().contains(connection()), "Driver does not list created connection");
    QCOMPARE(driver->connections().count(), connCount + 1); // one more

    const KDbUtils::Property extraSqliteExtensionPathsProperty = connection()->options()->property("extraSqliteExtensionPaths");
    QVERIFY2(!extraSqliteExtensionPathsProperty.isNull(), "extraSqliteExtensionPaths property not found");
    QCOMPARE(extraSqliteExtensionPathsProperty.value().type(), QVariant::StringList);
    QCOMPARE(extraSqliteExtensionPathsProperty.value().toStringList(), extraSqliteExtensionPaths);

    const KDbUtils::Property readOnlyProperty = connection()->options()->property("readOnly");
    QVERIFY2(!readOnlyProperty.isNull(), "readOnly property not found");
    QCOMPARE(readOnlyProperty.value().toBool(), connection()->options()->isReadOnly());

    //! @todo Add extensive test for a read-only connection

    KDB_VERIFY(connection(), connection()->connect(), "Failed to connect");
    KDB_VERIFY(connection(), connection()->isConnected(), "Database not connected after call to connect()");
}

void KDbTestUtils::testUseInternal()
{
    KDB_VERIFY(connection(), connection()->databaseExists(connection()->data().databaseName()), "Database does not exist");
    KDB_VERIFY(connection(), connection()->useDatabase(), "Failed to use database");
    KDB_VERIFY(connection(), connection()->isDatabaseUsed(), "Database not used after call to useDatabase()");
}

void KDbTestUtils::testConnectAndUseInternal(const KDbConnectionData &cdata,
                                             const KDbConnectionOptions &options)
{
    if (!testConnect(cdata, options) || !connection()) {
        qWarning() << driver->result();
        QFAIL("testConnect");
    }
    if (!testUse() || !connection()->isDatabaseUsed()) {
        qWarning() << connection()->result();
        bool result = testDisconnect();
        Q_UNUSED(result);
        QFAIL("testUse");
    }
}

void KDbTestUtils::testConnectAndUseInternal(const QString &path,
                                             const KDbConnectionOptions &options)
{
    KDbConnectionData cdata;
    cdata.setDatabaseName(path);
    testConnectAndUseInternal(cdata, options);
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
    QVERIFY(connection());

    //! @todo KDbDriver::metaData
    {
        QScopedPointer<KDbConnection> connGuard(d->takeConnection());

        if (connGuard->databaseExists(dbName)) {
            KDB_VERIFY(connGuard, connGuard->dropDatabase(fullDbName), "Failed to drop database");
        }
        KDB_VERIFY(connGuard, !connGuard->databaseExists(fullDbName), "Database exists");
        KDB_VERIFY(connGuard, connGuard->createDatabase(fullDbName), "Failed to create db");
        KDB_VERIFY(connGuard, connGuard->databaseExists(fullDbName), "Database does not exist after creation");
        d->setConnection(connGuard.take());
    }
}

void KDbTestUtils::testCreateDbWithTablesInternal(const QString &dbName)
{
    QVERIFY(testCreateDb(dbName));
    KDB_VERIFY(connection(), connection()->useDatabase(), "Failed to use database");
    testCreateTablesInternal();
}

void KDbTestUtils::testPropertiesInternal()
{
    QStringList properties;
    properties << connection()->databaseProperties().names();
    QVERIFY(properties.contains("kexidb_major_ver"));
    bool ok;
    QVERIFY(connection()->databaseProperties().value("kexidb_major_ver").toInt(&ok) >= 0);
    QVERIFY(ok);
    QVERIFY(properties.contains("kexidb_minor_ver"));
    QVERIFY(connection()->databaseProperties().value("kexidb_minor_ver").toInt(&ok) >= 0);
    QVERIFY(ok);
}

void KDbTestUtils::testCreateTablesInternal()
{
    QVERIFY2(tablesTest_createTables(connection()) == 0, "Failed to create test data");
}

void KDbTestUtils::testDisconnectPrivate()
{
    if (!connection()) {
        return;
    }
    KDB_VERIFY(connection(), connection()->closeDatabase(), "Failed to close database");
    KDB_VERIFY(connection(), !connection()->isDatabaseUsed(), "Database still used after closing");
    KDB_VERIFY(connection(), connection()->closeDatabase(), "Second closeDatabase() call  should not fail");
    KDB_VERIFY(connection(), connection()->disconnect(), "Failed to disconnect database");
    KDB_VERIFY(connection(), !connection()->isConnected(), "Database still connected after disconnecting");
    KDB_VERIFY(connection(), connection()->disconnect(), "Second disconnect() call should not fail");
}

void KDbTestUtils::testDisconnectInternal()
{
    const int connCount = driver ? driver->connections().count() : 0;
    testDisconnectPrivate();
    QVERIFY(!QTest::currentTestFailed());
    d->setConnection(nullptr);
    QCOMPARE(driver ? driver->connections().count() : -1, connCount - 1); // one less
}

void KDbTestUtils::testDropDbInternal()
{
    QVERIFY(connection()->dropDatabase(connection()->data().databaseName()));
}

void KDbTestUtils::testDisconnectAndDropDbInternal()
{
    QString dbName(connection()->data().databaseName());
    testDisconnectPrivate();
    QVERIFY(!QTest::currentTestFailed());
    KDB_VERIFY(connection(), connection()->dropDatabase(dbName), "Failed to drop database");
    d->setConnection(nullptr);
}

/* This file is part of the KDE project
   Copyright (C) 2012-2015 Jarosław Staniek <staniek@kde.org>

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

#include "ConnectionTest.h"

#include <KDbDriverManager>
#include <KDbDriverMetaData>

#include <QDir>
#include <QFile>
#include <QTest>

#define TABLETEST_DO_NOT_CREATE_DB
#include "../tests/features/tables_test.h"

QTEST_GUILESS_MAIN(ConnectionTest)

void ConnectionTest::initTestCase()
{
    utils.testDriverManager();
    utils.testSqliteDriver();
}

void ConnectionTest::testConnectionData()
{
    KDbConnectionData cdata;
    QVERIFY(cdata.databaseName().isEmpty());
    QVERIFY(cdata.driverId().isEmpty());
    QVERIFY(cdata.caption().isEmpty());
    QVERIFY(cdata.description().isEmpty());
    QVERIFY(cdata.userName().isEmpty());
    QVERIFY(cdata.hostName().isEmpty());
    QCOMPARE(cdata.port(), 0);
    QVERIFY(cdata.useLocalSocketFile());
    QVERIFY(cdata.localSocketFileName().isEmpty());
    QVERIFY(cdata.password().isEmpty());
    QVERIFY(!cdata.savePassword());
    QCOMPARE(cdata, cdata);
    QVERIFY2(!cdata.isPasswordNeeded(), "Password-needed is not false for empty data");
    QVERIFY(cdata.toUserVisibleString().isEmpty());
    QCOMPARE(cdata, KDbConnectionData());

    QString db = "mydb";
    cdata.setDatabaseName(db);
    QCOMPARE(cdata.databaseName(), db);
    QCOMPARE(db, cdata.toUserVisibleString());
    QCOMPARE(db, cdata.toUserVisibleString(KDbConnectionData::NoUserVisibleStringOption));

    cdata.setDriverId("INVALID.ID");
    QCOMPARE(db, cdata.toUserVisibleString()); // driver ID invalid: still just returns the db name
    QCOMPARE(db, cdata.toUserVisibleString(KDbConnectionData::NoUserVisibleStringOption)); // like above

    KDbDriverManager manager;
    //! @todo more drivers
    if (manager.driver("org.kde.kdb.sqlite")) { // only if mysql is present
        qDebug() << "org.kde.kdb.sqlite driver found, testing...";
        cdata = KDbConnectionData();
        cdata.setDriverId("org.kde.kdb.sqlite");
        QCOMPARE(cdata.toUserVisibleString(), KDbConnection::tr("<file>"));
        cdata.setDatabaseName("my.db");
        QCOMPARE(cdata.toUserVisibleString(), KDbConnection::tr("file: %1").arg("my.db"));
        KDbConnectionData copy(cdata);
        QCOMPARE(cdata, copy);
    }
    if (manager.driver("org.kde.kdb.mysql")) { // only if mysql is present
        qDebug() << "org.kde.kdb.mysql driver found, testing...";
        cdata = KDbConnectionData();
        cdata.setDriverId("org.kde.kdb.mysql");
        QCOMPARE(cdata.toUserVisibleString(), QLatin1String("localhost"));
        QCOMPARE(cdata.toUserVisibleString(KDbConnectionData::NoUserVisibleStringOption),
                 QLatin1String("localhost")); // like above
        cdata.setUserName("joe");
        QCOMPARE(cdata.toUserVisibleString(), QLatin1String("joe@localhost"));
        cdata.setUserName(QString());
        cdata.setHostName("example.com");
        QCOMPARE(cdata.toUserVisibleString(), QLatin1String("example.com"));
        cdata.setUserName("joe");
        QCOMPARE(cdata.toUserVisibleString(), QLatin1String("joe@example.com"));
        QCOMPARE(cdata.toUserVisibleString(KDbConnectionData::NoUserVisibleStringOption),
                 QLatin1String("example.com"));
        cdata.setPort(12345);
        QCOMPARE(cdata.toUserVisibleString(), QLatin1String("joe@example.com:12345"));
        QCOMPARE(cdata.toUserVisibleString(KDbConnectionData::NoUserVisibleStringOption),
                 QLatin1String("example.com:12345"));
        KDbConnectionData copy(cdata);
        QCOMPARE(cdata, copy);
    }
}

void ConnectionTest::testCreateDb()
{
    QVERIFY(utils.driver);

    QString db_name(QDir::fromNativeSeparators(QFile::decodeName(FILES_OUTPUT_DIR "/ConnectionTest.kexi")));

    //open connection
    KDbConnectionData cdata;
    cdata.setDatabaseName(db_name);

    utils.testConnect(cdata);
    QVERIFY(utils.connection);

    //! @todo KDbDriver::metaData
    {
        QScopedPointer<KDbConnection> connGuard(utils.connection.data());
        //! @todo move to KDbTestUtils::testCreate()
        if (utils.connection->databaseExists(db_name)) {
            KDB_VERIFY(utils.connection, utils.connection->dropDatabase(db_name), "Failed to drop database");
        }
        KDB_VERIFY(utils.connection, !utils.connection->databaseExists(db_name), "Database exists");
        KDB_VERIFY(utils.connection, utils.connection->createDatabase(db_name), "Failed to create db");
        KDB_VERIFY(utils.connection, utils.connection->databaseExists(db_name), "Database does not exists after creation");
        utils.testUse();
        QVERIFY2(tablesTest(utils.connection.data()) == 0, "Failed to create test data");
        connGuard.take();
    }

    utils.testDisconnect();
}

void ConnectionTest::cleanupTestCase()
{
}

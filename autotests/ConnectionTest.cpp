/* This file is part of the KDE project
   Copyright (C) 2012-2016 Jarosław Staniek <staniek@kde.org>

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

#include <KDbConnectionData>
#include <KDbDriverManager>
#include <KDbDriverMetaData>

#include <QDir>
#include <QFile>
#include <QTest>

QTEST_GUILESS_MAIN(ConnectionTest)

void ConnectionTest::initTestCase()
{
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
    if (manager.driver("org.kde.kdb.sqlite")) { // only if sqlite is present
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
    QVERIFY(utils.testCreateDb("ConnectionTest"));
    QVERIFY(utils.testUse());
    QVERIFY(utils.testProperties());
    QVERIFY(utils.testCreateTables());
    QVERIFY(utils.testDisconnectAndDropDb());
}

void ConnectionTest::testConnectToNonexistingDb()
{
    QVERIFY(utils.driver);

    //open connection
    KDbConnectionData cdata;
    cdata.setDatabaseName(QLatin1String("/really-non-existing/path/fiuwehf2349f8h23c2jcoeqw"));
    QVERIFY(utils.testConnect(cdata));
    QVERIFY(utils.connection);
    KDB_VERIFY(utils.connection, !utils.connection->databaseExists(utils.connection->data().databaseName()),
                    "Database should not exist");
    KDB_EXPECT_FAIL(utils.connection, utils.connection->useDatabase(),
                    ERR_OBJECT_NOT_FOUND, "Should fail to use database");
    KDB_EXPECT_FAIL(utils.connection, utils.connection->isDatabaseUsed(),
                    ERR_OBJECT_NOT_FOUND, "Database can't be used after call to useDatabase()");
    QVERIFY2(utils.connection->closeDatabase(), "Closing after failed USE should work");
    KDB_VERIFY(utils.connection, utils.connection->disconnect(), "Failed to disconnect database");
    QVERIFY2(!utils.connection->isConnected(), "Should not be connected");
}

void ConnectionTest::cleanupTestCase()
{
}

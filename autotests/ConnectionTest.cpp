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
#include <KDbConnection>

#include <QFile>
#include <QTest>

KDbConnection *conn = 0;

#define TABLETEST_DO_NOT_CREATE_DB
#include "../tests/features/tables_test.h"

QTEST_GUILESS_MAIN(ConnectionTest)

//! Calls @a call and verifies status of @a resultable
//! On error displays the status on debug and does the same as QVERIFY with @a errorMessage
#define KDB_VERIFY(resultable, call, errorMessage) \
    do { \
        bool KDB_VERIFY_ok = (call); \
        if (resultable->result().isError()) { \
            qDebug() << resultable->result(); \
        } \
        if (!QTest::qVerify(KDB_VERIFY_ok && !resultable->result().isError(), # call, (errorMessage), __FILE__, __LINE__)) {\
            return; \
        } \
    } \
    while (false)

void ConnectionTest::initTestCase()
{
    utils.testDriverManager();
    utils.testSqliteDriver();
}

void ConnectionTest::testCreateDb()
{
    QVERIFY(utils.driver);

    QString db_name(QFile::decodeName(FILES_OUTPUT_DIR "/ConnectionTest.kexi"));

    //open connection
    KDbConnectionData cdata;
    cdata.setDatabaseName(db_name);
    KDbConnectionOptions connOptions;
    QStringList extraSqliteExtensionPaths;
    extraSqliteExtensionPaths << SQLITE_ICU_EXTENSION_PATH;
    connOptions.insert("extraSqliteExtensionPaths", extraSqliteExtensionPaths);

    KDB_VERIFY(utils.driver, conn = utils.driver->createConnection(cdata, connOptions), "Failed to create connection");
    QVERIFY2(utils.driver->connections().contains(conn), "Driver does not list created connection");
    QCOMPARE(utils.driver->connections().count(), 1);

    const KDbUtils::Property extraSqliteExtensionPathsProperty = conn->options()->property("extraSqliteExtensionPaths");
    QVERIFY2(!extraSqliteExtensionPathsProperty.isNull, "extraSqliteExtensionPaths property not found");
    QCOMPARE(extraSqliteExtensionPathsProperty.value.toStringList(), extraSqliteExtensionPaths);

    const KDbUtils::Property readOnlyProperty = conn->options()->property("readOnly");
    QVERIFY2(!readOnlyProperty.isNull, "readOnly property not found");
    QCOMPARE(readOnlyProperty.value.toBool(), conn->options()->isReadOnly());
    //! @todo Add extensive test for a read-only connection

    //! @todo KDbDriver::metaData
    {
        QScopedPointer<KDbConnection> connGuard(conn);

        KDB_VERIFY(conn, conn->connect(), "Failed to connect");
        QVERIFY(conn->isConnected());
        if (conn->databaseExists(db_name)) {
            KDB_VERIFY(conn, conn->dropDatabase(db_name), "Failed to drop database");
        }
        KDB_VERIFY(conn, !conn->databaseExists(db_name), "Database still exists after drop");
        KDB_VERIFY(conn, conn->createDatabase(db_name), "Failed to create db");
        KDB_VERIFY(conn, conn->databaseExists(db_name), "Database does not exists after creation");
        KDB_VERIFY(conn, conn->useDatabase(db_name), "Failed to use db");
        KDB_VERIFY(conn, conn->isDatabaseUsed(), "Db isn't marked as used after USE");

        QVERIFY2(tablesTest() == 0, "Failed to create test data");

        KDB_VERIFY(conn, conn->closeDatabase(), "Failed to close database");
        KDB_VERIFY(conn, !conn->isDatabaseUsed(), "Database still used after closing");
        KDB_VERIFY(conn, conn->disconnect(), "Failed to disconnect database");
        KDB_VERIFY(conn, !conn->isConnected(), "Database still connected after disconnecting");
    }
    conn = 0;
    QVERIFY(utils.driver->connections().isEmpty());
}

void ConnectionTest::cleanupTestCase()
{
}

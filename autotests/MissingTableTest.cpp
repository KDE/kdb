/* This file is part of the KDE project
   Copyright (C) 2018 Jarosław Staniek <staniek@kde.org>

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

#include <KDbTestUtils.h>
#include <KDbConnectionData>
#include <KDbConnectionOptions>

class MissingTableTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void init();
    //! Tests if the list of tables skips name for which physical table is missing.
    //! The missingTableTest.kexi file has "persons" table deleted.
    void testListTables();
    void cleanupTestCase();
    void cleanup();

private:
    //! Opens database needed for tests
    bool openDatabase(const QString &path);

    KDbTestUtils m_utils;
};

void MissingTableTest::initTestCase()
{
}

void MissingTableTest::init()
{
    QString dir(QFile::decodeName(FILES_DATA_DIR));
    QVERIFY(openDatabase(dir + "/missingTableTest.kexi"));
}


bool MissingTableTest::openDatabase(const QString &path)
{
    KDbConnectionOptions options;
    options.setReadOnly(true);
    return m_utils.testConnectAndUse(path, options);
}

void MissingTableTest::testListTables()
{
    const bool alsoSystemTables = true;
    bool ok;
    QStringList foundTableNames = m_utils.connection()->tableNames(alsoSystemTables, &ok);
    QVERIFY(ok);

    // call again with ok == nullptr
    QCOMPARE(foundTableNames, m_utils.connection()->tableNames(alsoSystemTables));

    // make sure missing table is not present
    std::sort(foundTableNames.begin(), foundTableNames.end());
    const QStringList expectedTables(
        { "cars", "kexi__db", "kexi__fields", "kexi__objectdata", "kexi__objects" });
    QCOMPARE(foundTableNames, expectedTables);
}

void MissingTableTest::cleanup()
{
    QVERIFY(m_utils.testDisconnect());
}

void MissingTableTest::cleanupTestCase()
{
}

QTEST_GUILESS_MAIN(MissingTableTest)

#include "MissingTableTest.moc"

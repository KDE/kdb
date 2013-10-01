/* This file is part of the KDE project
   Copyright (C) 2012 Jarosław Staniek <staniek@kde.org>

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

#include "TestSqlParser.h"
#include <QtTest/QtTest>

#include <Predicate/DriverManager>

using namespace Predicate;

Q_DECLARE_METATYPE(Predicate::EscapedString)

bool TestSqlParser::openDatabase(const QString &path)
{
    QString driverName("sqlite");
    DriverManager manager;
    qDebug() << manager.driverNames();
    Driver *driver = manager.driver(driverName);
    if (!driver || manager.result().isError()) {
        qDebug() << manager.result();
        return false;
    }

    ConnectionData cdata;
    cdata.setDatabaseName(path);
    m_conn.reset(driver->createConnection(cdata));
    if (!m_conn || driver->result().isError()) {
        qDebug() << driver->result();
        return false;
    }
    if (!m_conn->connect()) {
        qDebug() << m_conn->result();
        return false;
    }
    m_parser.reset(new Parser(m_conn.data()));
#if 0
    if (m_conn->databaseExists(dbName)) {
        if (!m_conn->dropDatabase(dbName)) {
            m_conn->disconnect();
            return false;
        }
        qDebug() << "Database" << dbName << "dropped.";
    }
    if (!m_conn->createDatabase(dbName)) {
        qDebug() << m_conn->result();
        m_conn->disconnect();
        return false;
    }
#endif
    if (!m_conn->useDatabase()) {
        qDebug() << m_conn->result();
        m_conn->disconnect();
        return false;
    }
    return true;
}

void TestSqlParser::initTestCase()
{
}

EscapedString TestSqlParser::parse(const EscapedString& statement, bool *ok)
{
    Parser *parser = m_parser.data();

    *ok = parser->parse(statement);
    if (!*ok) {
        //qDebug() << parser->error();
        return EscapedString();
    }

    QScopedPointer<Predicate::QuerySchema> q(parser->query());
    if (!q) {
        //qDebug() << parser->error();
        *ok = false;
        return EscapedString();
    }
    //qDebug() << *q.data();

    QList<QVariant> params;
    EscapedString sql = m_conn->selectStatement(q.data(), params);
    //qDebug() << sql;
    *ok = true;
    return sql;
}

static void eatComment(QString* string)
{
    if (!string->startsWith("--")) {
        return;
    }
    int i = 0;
    for (; i < string->length() && string->at(i) == '-'; ++i)
        ;
    QString result = string->mid(i).trimmed();
    *string = result;
}

static void eatEndComment(QString* string)
{
    if (!string->endsWith("--")) {
        return;
    }
    int i = string->length() - 1;
    for (; i >= 0 && string->at(i) == '-'; --i)
        ;
    *string = string->left(i+1).trimmed();
}

void TestSqlParser::testParse_data()
{
    QTest::addColumn<QString>("fname");
    QTest::addColumn<int>("lineNum");
    QTest::addColumn<EscapedString>("sql");
    QTest::addColumn<bool>("expectError");

    QString dir(QFile::decodeName(FILES_DATA_DIR));
    QString fname("statements.txt");
    QFile input(dir + QDir::separator() + fname);
    bool ok = input.open(QFile::ReadOnly | QFile::Text);
    QVERIFY2(ok, ("Could not open data file " + input.fileName()).toLatin1());
    QTextStream in(&input);
    QString category;
    QString testName;
    bool expectError = false;
    int lineNum = 1;
    QString dbPath;
    bool clearTestName = false;

    for (; !in.atEnd(); ++lineNum) {
        QString line(in.readLine());
        if (line.startsWith("--")) { // comment
            eatComment(&line);
            eatEndComment(&line);
            if (line.startsWith("TODO:")) {
                continue;
            }
            else if (line.startsWith("CATEGORY: ")) {
                if (clearTestName) {
                    expectError = false;
                    clearTestName = false;
                    testName.clear();
                }
                category = line.mid(QString("CATEGORY: ").length()).trimmed();
                //qDebug() << "CATEGORY:" << category;
            }
            else if (line == "QUIT") {
                break;
            }
            else if (line.startsWith("SQLITEFILE: ")) {
                if (clearTestName) {
                    expectError = false;
                    clearTestName = false;
                    testName.clear();
                }
                ok = dbPath.isEmpty();
                QVERIFY2(ok, QString("Error at line %1: SQLite was file already specified (%2)")
                    .arg(lineNum).arg(dbPath).toLatin1());
                dbPath = line.mid(QString("SQLITEFILE: ").length()).trimmed();
                dbPath = dir + QDir::separator() + dbPath;
                ok = openDatabase(dbPath);
                QVERIFY2(ok, QString("Error at line %1: Could not open SQLite file %2")
                    .arg(lineNum).arg(dbPath).toLatin1());
            }
            else if (line.startsWith("ERROR: ")) {
                if (clearTestName) {
                    clearTestName = false;
                    testName.clear();
                }
                expectError = true;
                testName = line.mid(QString("ERROR: ").length()).trimmed();
            }
            else {
                if (clearTestName) {
                    expectError = false;
                    clearTestName = false;
                    testName.clear();
                }
                if (!testName.isEmpty()) {
                    testName.append(" ");
                }
                testName.append(line);
            }
        }
        else {
            EscapedString sql(line.trimmed());
            clearTestName = true;
            if (sql.isEmpty()) {
                expectError = false;
                continue;
            }
            ok = !dbPath.isEmpty();
            QVERIFY2(ok, QString("Error at line %1: SQLite was file not specified, cannot execute statement")
                .arg(lineNum).toLatin1());

            QTest::newRow(QString("File: %1:%2; Category: \"%3\"; Test: \"%4\"")
                          .arg(fname).arg(lineNum).arg(category).arg(testName).toLatin1()
                          + (expectError ? "; Error expected" :""))
                << fname << lineNum << sql << expectError;
        }
    }
    input.close();
}

void TestSqlParser::testParse()
{
    QFETCH(QString, fname);
    QFETCH(int, lineNum);
    QFETCH(EscapedString, sql);
    QFETCH(bool, expectError);

    QVERIFY2(sql.endsWith(';'), QString("%1:%2: Missing ';' at the end of line").arg(fname).arg(lineNum).toLatin1());
    sql.chop(1);
    //qDebug() << "SQL:" << sql.toString() << expectError;
    bool ok;
    EscapedString result = parse(sql, &ok);
    Parser *parser = m_parser.data();

    if (ok) {
        // sucess, so error cannot be expected
        QVERIFY2(!expectError,
                 (QString("Unexpected success in statement: \"%1\"; Result: %2")
                  .arg(sql.toString()).arg(result.toString()).toLatin1()));
        if (!expectError) {
            qDebug() << "Result:" << result.toString();
        }
    }
    else {
        // failure, so error should be expected
        QVERIFY2(expectError, QString("Statement: \"%1\"; %2")
                 .arg(sql.toString())
                 .arg(Predicate::debugString(parser->error())).toLatin1());
        if (expectError) {
            qDebug() << parser->error();
        }
    }
}

void TestSqlParser::cleanupTestCase()
{
    if (m_conn && m_conn->isConnected()) {
#if 0
        if (!m_conn->dropDatabase()) {
            qDebug() << m_conn->result();
        }
        qDebug() << "Database" << m_conn->data().databaseName() << "dropped.";
#endif
        m_conn->disconnect();
        m_conn.reset();
    }
}

QTEST_MAIN(TestSqlParser)

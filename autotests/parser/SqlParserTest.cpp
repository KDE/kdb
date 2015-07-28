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

#include "SqlParserTest.h"
#include <QtTest>

#include <KDbDriverManager>
#include <KDbNativeStatementBuilder>
#include <KDbToken>

Q_DECLARE_METATYPE(KDbEscapedString)

QTEST_GUILESS_MAIN(SqlParserTest)

void SqlParserTest::initTestCase()
{
    m_utils.testDriverManager();
    m_utils.testSqliteDriver();
}

bool SqlParserTest::openDatabase(const QString &path)
{
    if (!m_utils.driver) {
        return false;
    }

    KDbConnectionData cdata;
    cdata.setDatabaseName(path);
    m_utils.testConnect(cdata);
    if (!m_utils.connection) {
        qDebug() << m_utils.driver->result();
        return false;
    }
    m_parser.reset(new KDbParser(m_utils.connection.data()));
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
    m_utils.testUse();
    if (!m_utils.connection->isDatabaseUsed()) {
        qDebug() << m_utils.connection->result();
        m_utils.testDisconnect();
        return false;
    }
    return true;
}

KDbEscapedString SqlParserTest::parse(const KDbEscapedString& sql, bool *ok)
{
    KDbParser *parser = m_parser.data();

    *ok = parser->parse(sql);
    if (!*ok) {
        //qDebug() << parser->error();
        return KDbEscapedString();
    }

    QScopedPointer<KDbQuerySchema> q(parser->query());
    if (!q) {
        //qDebug() << parser->error();
        *ok = false;
        return KDbEscapedString();
    }
    //qDebug() << *q.data();

    QList<QVariant> params;
    KDbNativeStatementBuilder builder(m_utils.connection.data());
    KDbEscapedString querySql;
    *ok = builder.generateSelectStatement(&querySql, q.data(), params);
    //qDebug() << querySql;
    return querySql;
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

void SqlParserTest::testParse_data()
{
    QTest::addColumn<QString>("fname");
    QTest::addColumn<int>("lineNum");
    QTest::addColumn<KDbEscapedString>("sql");
    QTest::addColumn<bool>("expectError");

    QString dir(QFile::decodeName(FILES_DATA_DIR));
    QString fname("statements.txt");
    QFile input(dir + QDir::separator() + fname);
    bool ok = input.open(QFile::ReadOnly | QFile::Text);
    QVERIFY2(ok, qPrintable(QString("Could not open data file %1").arg(input.fileName())));
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
                QVERIFY2(ok, qPrintable(QString("Error at line %1: SQLite was file already specified (%2)")
                    .arg(lineNum).arg(dbPath)));
                dbPath = line.mid(QString("SQLITEFILE: ").length()).trimmed();
                dbPath = dir + QDir::separator() + dbPath;
                ok = openDatabase(dbPath);
                QVERIFY2(ok, qPrintable(QString("Error at line %1: Could not open SQLite file %2")
                    .arg(lineNum).arg(dbPath)));
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
            KDbEscapedString sql(line.trimmed());
            clearTestName = true;
            if (sql.isEmpty()) {
                expectError = false;
                continue;
            }
            ok = !dbPath.isEmpty();
            QVERIFY2(ok, qPrintable(QString("Error at line %1: SQLite was file not specified, "
                                            "could not execute statement").arg(lineNum)));

            QTest::newRow(qPrintable(QString("File: %1:%2; Category: \"%3\"; Test: \"%4\"%5")
                          .arg(fname).arg(lineNum).arg(category).arg(testName)
                          .arg(expectError ? "; Error expected" :"")))
                << fname << lineNum << sql << expectError;
        }
    }
    input.close();
}

void SqlParserTest::testParse()
{
    QFETCH(QString, fname);
    QFETCH(int, lineNum);
    QFETCH(KDbEscapedString, sql);
    QFETCH(bool, expectError);

    QVERIFY2(sql.endsWith(';'), qPrintable(QString("%1:%2: Missing ';' at the end of line")
                                           .arg(fname).arg(lineNum)));
    sql.chop(1);
    //qDebug() << "SQL:" << sql.toString() << expectError;
    bool ok;
    KDbEscapedString result = parse(sql, &ok);
    KDbParser *parser = m_parser.data();

    if (ok) {
        // sucess, so error cannot be expected
        QVERIFY2(!expectError,
                 (qPrintable(QString("Unexpected success in SQL statement: \"%1\"; Result: %2")
                  .arg(sql.toString()).arg(result.toString()))));
        if (!expectError) {
            qDebug() << "Result:" << result.toString();
        }
    }
    else {
        // failure, so error should be expected
        QVERIFY2(expectError, qPrintable(QString("SQL statement: \"%1\"; %2")
                 .arg(sql.toString())
                 .arg(KDbUtils::debugString(parser->error()))));
        if (expectError) {
            qDebug() << parser->error();
        }
    }
}

void SqlParserTest::testTokens()
{
    KDbToken t = KDbToken::LEFT;
    qDebug() << t << t.toChar() << t.value() << t.isValid();
    t = '+';
    qDebug() << t << t.toChar() << t.value() << t.isValid();
    t = KDbToken();
    qDebug() << t << t.toChar() << t.value() << t.isValid();

    QCOMPARE(KDbToken::SQL_TYPE.value(), 258);
    QCOMPARE(KDbToken::AS.value(), 259);
    QCOMPARE(KDbToken::AS_EMPTY.value(), 260);
    QCOMPARE(KDbToken::ASC.value(), 261);
    QCOMPARE(KDbToken::AUTO_INCREMENT.value(), 262);
    QCOMPARE(KDbToken::BIT.value(), 263);
    QCOMPARE(KDbToken::BITWISE_SHIFT_LEFT.value(), 264);
    QCOMPARE(KDbToken::BITWISE_SHIFT_RIGHT.value(), 265);
    QCOMPARE(KDbToken::BY.value(), 266);
    QCOMPARE(KDbToken::CHARACTER_STRING_LITERAL.value(), 267);
    QCOMPARE(KDbToken::CONCATENATION.value(), 268);
    QCOMPARE(KDbToken::CREATE.value(), 269);
    QCOMPARE(KDbToken::DESC.value(), 270);
    QCOMPARE(KDbToken::DISTINCT.value(), 271);
    QCOMPARE(KDbToken::DOUBLE_QUOTED_STRING.value(), 272);
    QCOMPARE(KDbToken::FROM.value(), 273);
    QCOMPARE(KDbToken::JOIN.value(), 274);
    QCOMPARE(KDbToken::KEY.value(), 275);
    QCOMPARE(KDbToken::LEFT.value(), 276);
    QCOMPARE(KDbToken::LESS_OR_EQUAL.value(), 277);
    QCOMPARE(KDbToken::GREATER_OR_EQUAL.value(), 278);
    QCOMPARE(KDbToken::SQL_NULL.value(), 279);
    QCOMPARE(KDbToken::SQL_IS.value(), 280);
    QCOMPARE(KDbToken::SQL_IS_NULL.value(), 281);
    QCOMPARE(KDbToken::SQL_IS_NOT_NULL.value(), 282);
    QCOMPARE(KDbToken::ORDER.value(), 283);
    QCOMPARE(KDbToken::PRIMARY.value(), 284);
    QCOMPARE(KDbToken::SELECT.value(), 285);
    QCOMPARE(KDbToken::INTEGER_CONST.value(), 286);
    QCOMPARE(KDbToken::REAL_CONST.value(), 287);
    QCOMPARE(KDbToken::RIGHT.value(), 288);
    QCOMPARE(KDbToken::SQL_ON.value(), 289);
    QCOMPARE(KDbToken::DATE_CONST.value(), 290);
    QCOMPARE(KDbToken::DATETIME_CONST.value(), 291);
    QCOMPARE(KDbToken::TIME_CONST.value(), 292);
    QCOMPARE(KDbToken::TABLE.value(), 293);
    QCOMPARE(KDbToken::IDENTIFIER.value(), 294);
    QCOMPARE(KDbToken::IDENTIFIER_DOT_ASTERISK.value(), 295);
    QCOMPARE(KDbToken::QUERY_PARAMETER.value(), 296);
    QCOMPARE(KDbToken::VARCHAR.value(), 297);
    QCOMPARE(KDbToken::WHERE.value(), 298);
    QCOMPARE(KDbToken::SQL.value(), 299);
    QCOMPARE(KDbToken::SQL_TRUE.value(), 300);
    QCOMPARE(KDbToken::SQL_FALSE.value(), 301);
    QCOMPARE(KDbToken::UNION.value(), 302);
    QCOMPARE(KDbToken::SCAN_ERROR.value(), 303);
    QCOMPARE(KDbToken::AND.value(), 304);
    QCOMPARE(KDbToken::BETWEEN.value(), 305);
    QCOMPARE(KDbToken::NOT_BETWEEN.value(), 306);
    QCOMPARE(KDbToken::EXCEPT.value(), 307);
    QCOMPARE(KDbToken::SQL_IN.value(), 308);
    QCOMPARE(KDbToken::INTERSECT.value(), 309);
    QCOMPARE(KDbToken::LIKE.value(), 310);
    QCOMPARE(KDbToken::ILIKE.value(), 311);
    QCOMPARE(KDbToken::NOT_LIKE.value(), 312);
    QCOMPARE(KDbToken::NOT.value(), 313);
    QCOMPARE(KDbToken::NOT_EQUAL.value(), 314);
    QCOMPARE(KDbToken::NOT_EQUAL2.value(), 315);
    QCOMPARE(KDbToken::OR.value(), 316);
    QCOMPARE(KDbToken::SIMILAR_TO.value(), 317);
    QCOMPARE(KDbToken::NOT_SIMILAR_TO.value(), 318);
    QCOMPARE(KDbToken::XOR.value(), 319);
    QCOMPARE(KDbToken::UMINUS.value(), 320);

    //! @todo add extra tokens: BETWEEN_AND, NOT_BETWEEN_AND
}

void SqlParserTest::cleanupTestCase()
{
    m_utils.testDisconnect();
#if 0
        if (!m_conn->dropDatabase()) {
            qDebug() << m_conn->result();
        }
        qDebug() << "Database" << m_conn->data().databaseName() << "dropped.";
#endif
}

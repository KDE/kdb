/* This file is part of the KDE project
   Copyright (C) 2012-2013 Jarosław Staniek <staniek@kde.org>

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

#include "IdentifierTest.h"

#include <KDb>

#include <QTest>

QTEST_GUILESS_MAIN(IdentifierTest)

void IdentifierTest::initTestCase()
{
}

void IdentifierTest::testStringToIdentifier_data()
{
    QTest::addColumn<QString>("string1");
    QTest::addColumn<QString>("string2");

    QTest::newRow("empty") << "" << "";
    QTest::newRow("underscore") << "_" << "_";
    QTest::newRow("whitespace") << " \n   \t" << "";
    QTest::newRow("special chars") << ": \\-abc" << "_abc";
    QTest::newRow("special chars2") << " */$" << "_";
    QTest::newRow("Upper case") << "a A b2" << "a_A_b2";
    QTest::newRow("non-alpha") << "1" << "_1";
    QTest::newRow("non-latin1") << QString::fromUtf8("Ñ©Æ ᑫኂ") << "NcAE_kexi";
    QTest::newRow("umlauts")
            << QString::fromUtf8("Falsches Üben von Xylophonmusik quält jeden größeren Zwerg")
            << "Falsches_Ueben_von_Xylophonmusik_quaelt_jeden_groesseren_Zwerg";
}

void IdentifierTest::testStringToIdentifier()
{
    QFETCH(QString, string1);
    QFETCH(QString, string2);
    QCOMPARE(KDb::stringToIdentifier(string1), string2);
}

void IdentifierTest::testIsIdentifier_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<bool>("result");
    QTest::newRow("empty") << "" << false;
    QTest::newRow("empty") << QString() << false;
    QTest::newRow("zero") << "\0" << false;
    QTest::newRow("space") << " " << false;
    QTest::newRow("number") << "7" << false;
    QTest::newRow("underscore") << "_" << true;
    QTest::newRow("alpha") << "abc_2" << true;
    QTest::newRow("upper") << "Abc_2" << true;
    QTest::newRow("upper2") << "_7" << true;
}

void IdentifierTest::testIsIdentifier()
{
    QFETCH(QString, string);
    QFETCH(bool, result);
    QCOMPARE(KDb::isIdentifier(string), result);
    QCOMPARE(KDb::isIdentifier(string.toLatin1()), result);
}

void IdentifierTest::escapeIdentifier_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<QString>("result"); // quotes not always added
    QTest::addColumn<QString>("resultWithForcedQuotes"); // quotes always added
    QTest::newRow("empty") << "" << QString() << "\"\"";
    QTest::newRow("empty") << QString() << QString() << "\"\"";
    QTest::newRow("\"") << "\"" << "\"\"\"\"" << "\"\"\"\"";
    QTest::newRow("\"-\"") << "\"-\"" << "\"\"\"-\"\"\"" << "\"\"\"-\"\"\"";
    QTest::newRow("\t") << "\t" << "\"\t\"" << "\"\t\"";
    QTest::newRow("id") << "id" << "id" << "\"id\"";
    QTest::newRow("keyword") << "select" << "\"select\"" << "\"select\"";
    QTest::newRow("alpha") << "a b" << "\"a b\"" << "\"a b\"";
}

void IdentifierTest::escapeIdentifier()
{
    QFETCH(QString, string);
    QFETCH(QString, result);
    QFETCH(QString, resultWithForcedQuotes);
    QCOMPARE(KDb::escapeIdentifier(string), result);
    QCOMPARE(KDb::escapeIdentifier(string.toLatin1()), result.toLatin1());
    QCOMPARE(KDb::escapeIdentifierAndAddQuotes(string), resultWithForcedQuotes);
    QCOMPARE(KDb::escapeIdentifierAndAddQuotes(string.toLatin1()), resultWithForcedQuotes.toLatin1());
}

void IdentifierTest::cleanupTestCase()
{
}

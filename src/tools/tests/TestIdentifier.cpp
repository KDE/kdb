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

#include "TestIdentifier.h"
#include <Predicate/Tools/Identifier>
#include <QTest>

void TestIdentifier::initTestCase()
{
}

void TestIdentifier::testStringToIdentifier_data()
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
}

void TestIdentifier::testStringToIdentifier()
{
    QFETCH(QString, string1);
    QFETCH(QString, string2);
    QCOMPARE(Predicate::Utils::stringToIdentifier(string1), string2);
}

void TestIdentifier::testIsIdentifier_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<bool>("result");
    QTest::newRow("empty") << "" << false;
    QTest::newRow("empty") << QString() << false;
    QTest::newRow("empty") << "\0" << false;
    QTest::newRow("empty") << " " << false;
    QTest::newRow("empty") << "7" << false;
    QTest::newRow("empty") << "_" << true;
    QTest::newRow("empty") << "abc_2" << true;
    QTest::newRow("empty") << "Abc_2" << true;
    QTest::newRow("empty") << "_7" << true;
}

void TestIdentifier::testIsIdentifier()
{
    QFETCH(QString, string);
    QFETCH(bool, result);
    QCOMPARE(Predicate::Utils::isIdentifier(string), result);
}

void TestIdentifier::cleanupTestCase()
{
}

QTEST_MAIN(TestIdentifier)

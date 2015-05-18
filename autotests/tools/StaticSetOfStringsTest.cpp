/* This file is part of the KDE project
   Copyright (C) 2011 Adam Pigg <piggz1@gmail.com>

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

#include "StaticSetOfStringsTest.h"
#include <QtTest/QtTest>

QTEST_GUILESS_MAIN(StaticSetOfStringsTest)

const char* StaticSetOfStringsTest::keywords[] = {
    "ABORT",
    "ABSOLUTE",
    "ACCESS",
    "ACTION",
    "ADD",
    "AGGREGATE",
    "ALTER",
    "ANALYSE",
    "ANALYZE",
    "ANY",
    "ARRAY",
    "ASSERTION",
    "ASSIGNMENT",
    "AT",
    "AUTHORIZATION",
    "BACKWARD",
    "BIGINT",
    "BINARY",
    0
};

void StaticSetOfStringsTest::initTestCase()
{
    strings.setStrings(keywords);
}

void StaticSetOfStringsTest::testContains()
{
    QVERIFY(strings.contains("ANY")); //test a random string
    QVERIFY(strings.contains(QString("backward").toUpper().toLocal8Bit()));
    QVERIFY(!strings.contains("BIGIN")); //test a sub-string
    QVERIFY(!strings.contains("XXXXXXXXXX")); //test some garbage
    QVERIFY(!strings.isEmpty());
    QVERIFY(strings.contains("ABORT")); //test start of list
    QVERIFY(strings.contains("BINARY")); //test end of list
}

void StaticSetOfStringsTest::cleanupTestCase()
{
}

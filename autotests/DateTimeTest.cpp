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

#include "DateTimeTest.h"
#include "KDbTestUtils.h"

#include <QtTest>

#include <KDbDateTime>
#include <KDbExpression>
#include "parser/generated/sqlparser.h"
#include "parser/KDbParser_p.h"

QTEST_GUILESS_MAIN(DateTimeTest)

void DateTimeTest::initTestCase()
{
}

void DateTimeTest::testYear()
{
    KDbYear year("2020");
    QVERIFY(year.isValid());
    QCOMPARE(year.toString(), "2020");
    QCOMPARE(year.toQDateValue(), 2020);
    QCOMPARE(year.toIsoValue(), 2020);
    QVariant yearVariant = QVariant::fromValue(year);
    QVERIFY(yearVariant.isValid());
    QCOMPARE(yearVariant.toInt(), 2020);
    QCOMPARE(yearVariant.toString(), "2020");
}

void DateTimeTest::testDate()
{
    KDbYear year("2020");
    KDbDate date(year, "2", "29");
    QVERIFY(date.isValid());
    QCOMPARE(date.toString(), "2020-2-29");
    QDate qtDate(2020, 2, 29);
    QCOMPARE(date.toQDate(), qtDate);
    QVariant dateVariant = QVariant::fromValue(date);
    QVERIFY(dateVariant.isValid());
    QCOMPARE(dateVariant.toDate(), qtDate);
    //! @todo more cases
}

void DateTimeTest::testTime()
{
    KDbTime time("1", "15", "20", "789", KDbTime::Period::Pm);
    QVERIFY(time.isValid());
    QCOMPARE(time.toString(), "1:15:20.789 PM");
    QTime qtTime(13, 15, 20, 789);
    QCOMPARE(time.toQTime(), qtTime);
    QVariant timeVariant = QVariant::fromValue(time);
    QVERIFY(timeVariant.isValid());
    QCOMPARE(timeVariant.toTime(), qtTime);
    //! @todo more cases
}

void DateTimeTest::testDateTime()
{
    KDbYear year("2020");
    KDbTime time("1", "15", "20", "789", KDbTime::Period::Pm);
    KDbDate date(year, "2", "29");

    KDbDateTime dateTime(date, time);
    QVERIFY(dateTime.isValid());
    QCOMPARE(dateTime.toString(), "2020-2-29 1:15:20.789 PM");
    QTime qtTime(13, 15, 20, 789);
    QDate qtDate(2020, 2, 29);
    QDateTime qtDateTime(qtDate, qtTime);
    QCOMPARE(dateTime.toQDateTime(), qtDateTime);
    QVariant dateTimeVariant = QVariant::fromValue(dateTime);
    QVERIFY(dateTimeVariant.isValid());
    QCOMPARE(dateTimeVariant.toDateTime(), qtDateTime);
    //! @todo more cases
}

void DateTimeTest::cleanupTestCase()
{
}

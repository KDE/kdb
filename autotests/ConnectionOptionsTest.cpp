/* This file is part of the KDE project
   Copyright (C) 2016 Jarosław Staniek <staniek@kde.org>

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

#include "ConnectionOptionsTest.h"

#include <KDbConnectionOptions>
#include <QtTest>

QTEST_GUILESS_MAIN(ConnectionOptionsTest)

void ConnectionOptionsTest::initTestCase()
{
}

void ConnectionOptionsTest::testEmpty()
{
    KDbConnectionOptions empty;
    QVERIFY(!empty.isReadOnly());
    QVERIFY(empty.property("fooBar").isNull());
    QVERIFY(empty == KDbConnectionOptions());
    KDbConnectionOptions opt;
    opt.insert("someName", 17);
    QVERIFY(empty != opt);
    opt = KDbConnectionOptions();
    QVERIFY(empty == opt);
    opt.insert("/", "bar"); // no effect, name must be a valid identifier
    QVERIFY(opt.property("/").isNull());
}

void ConnectionOptionsTest::testCopyAndCompare()
{
    KDbConnectionOptions empty;
    QVERIFY(empty == KDbConnectionOptions(empty));
    KDbConnectionOptions opt;
    opt.insert("someName", 17, "Some Name");
    KDbConnectionOptions opt2(opt);
    QVERIFY(opt == opt2);
    opt.remove("nonExisting");
    QVERIFY(opt == opt2);
    opt.remove("someName");
    QVERIFY(opt != opt2);
    opt2.remove("someName");
    QVERIFY(opt == opt2);
    opt.insert("someName2", 171, "Some Name 2");
    opt2.insert("someName2", 171, "Some Name 2");
    QVERIFY(opt == opt2);
    opt2.setCaption("nonExisting", "Nope");
    QVERIFY(opt == opt2);
    // overwrite existing caption
    opt.setCaption("someName2", "Other");
    QVERIFY(opt != opt2);
    opt2.setCaption("someName2", "Other");
    QVERIFY(opt == opt2);
    // can't overwrite existing caption, v2
    opt2.insert("someName2", opt2.property("someName2").value(), "Other");
    QVERIFY(opt == opt2);
}

void ConnectionOptionsTest::testValue()
{
    KDbConnectionOptions opt;
    opt.setValue("foo", 1);
    QVERIFY(opt.property("foo").isNull());
    QVERIFY(opt.property("foo").value().isNull());
    opt.insert("foo", 17);
    QCOMPARE(opt.property("foo").value().type(), QVariant::Int);
    QCOMPARE(opt.property("foo").value().toInt(), 17);
    opt.insert("foo", 18);
    QCOMPARE(opt.property("foo").value().toInt(), 18);
    opt.setValue("foo", 19);
    QCOMPARE(opt.property("foo").value().toInt(), 19);
}

void ConnectionOptionsTest::testReadOnly()
{
    {
        KDbConnectionOptions empty;
        QVERIFY(!empty.isReadOnly());
        QVERIFY(!empty.property("readOnly").isNull());
        QVERIFY(empty.property("readonly").isNull());
        QVERIFY(!empty.property("readOnly").value().toBool());
        empty.setReadOnly(true);
        QVERIFY(empty.isReadOnly());
        QVERIFY(!empty.property("readOnly").isNull());
        QVERIFY(empty.property("readOnly").value().toBool());
    }
    {
        KDbConnectionOptions opt;
        opt.insert("someName", 17);
        QVERIFY(!opt.isReadOnly());
        QVERIFY(!opt.property("readOnly").isNull());
        QVERIFY(!opt.property("readOnly").value().toBool());
        opt.setReadOnly(true);

        KDbConnectionOptions opt2(opt);
        QVERIFY(opt2.isReadOnly());
        QVERIFY(!opt2.property("readOnly").isNull());
        QVERIFY(opt2.property("readOnly").value().toBool());
    }
    {
        // setting readOnly property is special
        KDbConnectionOptions opt;
        opt.insert("readOnly", true, "foo");
        QVERIFY(opt.isReadOnly());
        QVERIFY(opt.property("readOnly").value().toBool());
        opt.insert("readOnly", 1, "foo");
        QVERIFY(opt.isReadOnly());
        QVERIFY(opt.property("readOnly").value().toBool());
    }
    {
        // can't set readOnly property caption
        KDbConnectionOptions opt;
        opt.setCaption("readOnly", "foo");
        QVERIFY(opt.property("readOnly").caption() != "foo");
    }
    {
        // can't remove readOnly property
        KDbConnectionOptions opt;
        opt.remove("readOnly");
        QVERIFY(!opt.property("readOnly").isNull());
    }
}

void ConnectionOptionsTest::cleanupTestCase()
{
}

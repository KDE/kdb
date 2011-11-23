/* This file is part of the KDE project
   Copyright (C) 2011 Jarosław Staniek <staniek@kde.org>

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

#include "TestExpressions.h"
#include <QtTest/QtTest>
#include <qdom.h>

#include <Predicate/Expression>

using namespace Predicate;

void TestExpressions::initTestCase()
{
}

void TestExpressions::testExpression()
{
/* test for dom    
    QDomDocument doc("mydocument");
    QDomElement elem = doc.documentElement();
    QCOMPARE(elem, doc.documentElement());
    QDomElement elem2 = doc.createElement("2");
    QDomElement elem3 = doc.createElement("2");
    QCOMPARE(elem2, elem3);
    elem.appendChild(elem2);
    elem.appendChild(elem3);
    QCOMPARE(elem2, elem3);*/
    
    QVERIFY(Expression() != Expression());
    
    Expression e1;
    Expression e2;
    QVERIFY(e1.isNull());
    QVERIFY(!e1.isBinary());
    QVERIFY(!e1.isConst());
    QVERIFY(!e1.isFunction());
    QVERIFY(!e1.isNArg());
    QVERIFY(!e1.isQueryParameter());
    QVERIFY(!e1.isUnary());
    QVERIFY(!e1.isVariable());
    QCOMPARE(e1.expressionClass(), UnknownExpressionClass);
    QCOMPARE(e1.token(), 0);
    QVERIFY(e1 != Expression());
    QVERIFY(e1 == e1);
    QVERIFY(e1 != e2);
    e1 = e2;
    QVERIFY(e1.isNull());
    QVERIFY(e1 == e2);
}

void TestExpressions::cleanupTestCase()
{
}

QTEST_MAIN(TestExpressions)

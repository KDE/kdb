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
#include <QDomElement>

#include <Predicate/Expression>
#include <parser/SqlParser.h>

using namespace Predicate;

void TestExpressions::initTestCase()
{
}

void TestExpressions::testNullExpression()
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
    QCOMPARE(e1, e2);
}

void TestExpressions::testCloneExpression()
{
    Expression e1;
    Expression e1clone = e1.clone();
    QVERIFY(e1 != e1.clone());
    QVERIFY(e1 != e1clone);
    QVERIFY(e1.clone() != e1clone);
}

Q_DECLARE_METATYPE(Predicate::ExpressionClass)

void TestExpressions::testExpressionClassName_data()
{
    QTest::addColumn<ExpressionClass>("expClass");
    QTest::addColumn<QString>("name");

#define T(n, c) QTest::newRow(n) << c << n
    T("Unknown", UnknownExpressionClass);
    T("Unary", UnaryExpressionClass);
    T("Arithm", ArithmeticExpressionClass);
    T("Logical", LogicalExpressionClass);
    T("Relational", RelationalExpressionClass);
    T("SpecialBinary", SpecialBinaryExpressionClass);
    T("Const", ConstExpressionClass);
    T("Variable", VariableExpressionClass);
    T("Function", FunctionExpressionClass);
    T("Aggregation", AggregationExpressionClass);
    T("TableList", TableListExpressionClass);
    T("QueryParameter", QueryParameterExpressionClass);
#undef T
}

void TestExpressions::testExpressionClassName()
{
    QFETCH(ExpressionClass, expClass);
    QTEST(expressionClassName(expClass), "name");
}

extern const char* tname(int offset);

class TestedExpression : public Expression
{
public:
    TestedExpression(int token) : Expression(new ExpressionData, UnknownExpressionClass, token) {
    }
};

void TestExpressions::testExpressionToken()
{
    Expression e1;
    QCOMPARE(e1.token(), 0);

    for (int i = 0; i < 254; ++i) {
        QCOMPARE(Expression::tokenToDebugString(i),
                isprint(i) ? QString(QLatin1Char(uchar(i))) : QString::number(i));
        QCOMPARE(TestedExpression(i).tokenToString(),
                isprint(i) ? QString(QLatin1Char(uchar(i))) : QString());
    }
    for (int i = 255; i <= __LAST_TOKEN; ++i) {
        QCOMPARE(Expression::tokenToDebugString(i), QLatin1String(tname(i - 255)));
    }
}

void TestExpressions::testNArgExpression()
{
    NArgExpression emptyNarg;
    QVERIFY(emptyNarg.isNArg());
    QVERIFY(emptyNarg.clone().isNArg());
    QVERIFY(emptyNarg.isEmpty());
    QCOMPARE(emptyNarg.argCount(), 0);
    QVERIFY(emptyNarg.arg(-1).isNull());
    QVERIFY(emptyNarg.arg(0).isNull());

    Expression e;
    NArgExpression nNull;
    nNull.append(e);
    QCOMPARE(nNull.argCount(), 0); // n-arg expression should have class, otherwise is null and cannot have children

    NArgExpression nArithm(ArithmeticExpressionClass, '+');
    nArithm.append(e);
    QVERIFY(!nArithm.isEmpty());
    QCOMPARE(nArithm.argCount(), 1);
    QCOMPARE(nArithm.arg(0), e);
}

void TestExpressions::cleanupTestCase()
{
}

QTEST_MAIN(TestExpressions)

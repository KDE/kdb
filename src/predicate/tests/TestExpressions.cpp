/* This file is part of the KDE project
   Copyright (C) 2011-2012 Jarosław Staniek <staniek@kde.org>

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

#include <Predicate/Expression>
#include <parser/SqlParser.h>
#include <parser/Parser_p.h>

using namespace Predicate;

Q_DECLARE_METATYPE(Predicate::ExpressionClass)
Q_DECLARE_METATYPE(Predicate::EscapedString)

void TestExpressions::initTestCase()
{
}

//! compares two expression @a e1 and @a e2 based on strings/debug strings
//! and token strings
template <typename T>
static void compareStrings(const T &e1, const T &e2)
{
    QCOMPARE(e1.toString(), e2.toString());
    QCOMPARE(e1.tokenToDebugString(), e2.tokenToDebugString());
    QCOMPARE(e1.tokenToString(), e2.tokenToString());
    qDebug() << "compareStrings():"
             << e1.toString() << e1.tokenToDebugString() << e1.tokenToString();
}

//! tests clone and copy ctor for @a e1
template <typename T>
static void testCloneExpression(const T &e1)
{
    Expression e1clone = e1.clone();
    QVERIFY(e1 != e1.clone());
    QVERIFY(e1 != e1clone);
    QVERIFY(e1.clone() != e1clone);
    compareStrings(Expression(e1), e1clone);

    const T copied(e1);
    QVERIFY(e1 == copied);
    QVERIFY(e1.clone() != copied);
    compareStrings(e1, copied);
}

void TestExpressions::testNullExpression()
{
    QVERIFY(Expression() != Expression());

    Expression e1;
    Expression e2;
    QVERIFY(e1.isNull());
    QVERIFY(!e1.isBinary());
    QVERIFY(e1.toBinary().isNull());
    QVERIFY(!e1.isConst());
    QVERIFY(e1.toConst().isNull());
    QVERIFY(!e1.isFunction());
    QVERIFY(e1.toFunction().isNull());
    QVERIFY(!e1.isNArg());
    QVERIFY(e1.toNArg().isNull());
    QVERIFY(!e1.isQueryParameter());
    QVERIFY(e1.toQueryParameter().isNull());
    QVERIFY(!e1.isUnary());
    QVERIFY(e1.toUnary().isNull());
    QVERIFY(!e1.isVariable());
    QVERIFY(e1.toVariable().isNull());
    QCOMPARE(e1.expressionClass(), UnknownExpressionClass);
    QCOMPARE(e1.token(), 0);
    QVERIFY(e1 != Expression());
    QVERIFY(e1 == e1);
    QVERIFY(e1 != e2);

    e1 = e2;
    QVERIFY(e1.isNull());
    QCOMPARE(e1, e2);
    QCOMPARE(e1.toString(), EscapedString("NULL"));
    QCOMPARE(e1.tokenToDebugString(), QLatin1String("0"));
    QCOMPARE(e1.tokenToString(), QString());
    compareStrings(e1, e2);

    Expression e3(e2);
    QVERIFY(e3.isNull());
    QCOMPARE(e2, e3);
    compareStrings(e2, e3);
    //ExpressionDebug << "$$$" << e1.toString() << e1.tokenToDebugString() << e1.tokenToString();

    e1 = Expression();
    testCloneExpression(e1);
}

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
    NArgExpression n;
    NArgExpression n2;
    ConstExpression c;
    ConstExpression c1;
    ConstExpression c2;
    ConstExpression c3;

    // -- empty
    NArgExpression emptyNarg;
    QVERIFY(emptyNarg.isNArg());
    QVERIFY(emptyNarg.clone().isNArg());
    QVERIFY(emptyNarg.isEmpty());
    QCOMPARE(emptyNarg.argCount(), 0);
    QVERIFY(emptyNarg.arg(-1).isNull());
    QVERIFY(emptyNarg.arg(0).isNull());

    // -- copy ctor & cloning
    n = NArgExpression(ArithmeticExpressionClass, '+');
    c1 = ConstExpression(INTEGER_CONST, 7);
    c2 = ConstExpression(INTEGER_CONST, 8);
    n.append(c1);
    n.append(c2);
    testCloneExpression(n);

    QCOMPARE(n.tokenToDebugString(), QString("+"));
    QCOMPARE(n.toString(), EscapedString("7, 8"));

    // -- append(Expression), prepend(Expression)
    Expression e;
    NArgExpression nNull;
    nNull.append(e);
    QCOMPARE(nNull.argCount(), 0); // n-arg expression should have class, otherwise is null and cannot have children

    n = NArgExpression(ArithmeticExpressionClass, '+');
    c1 = ConstExpression(INTEGER_CONST, 1);
    n.append(c1);
    QVERIFY(!n.isEmpty());
    QCOMPARE(n.argCount(), 1);
    QCOMPARE(n.arg(0).toConst(), c1);
    QCOMPARE(c1.parent().toNArg(), n);

    n = NArgExpression(ArithmeticExpressionClass, '+');
    n.append(n);
    QCOMPARE(n.argCount(), 0); // append should fail since appending expression
                                // to itself is not allowed
    n.prepend(n);
    QCOMPARE(n.argCount(), 0); // append should fail since prepending expression
                                // to itself is not allowed

    n = NArgExpression(ArithmeticExpressionClass, '+');
    c1 = ConstExpression(INTEGER_CONST, 2);
    n.append(c1);
    n.append(c1); // cannot append the same expression twice
    QCOMPARE(n.argCount(), 1);
    QCOMPARE(n.arg(0).toConst(), c1);

    n = NArgExpression(ArithmeticExpressionClass, '+');
    c1 = ConstExpression(INTEGER_CONST, 3);
    n.prepend(c1);
    n.prepend(c1); // cannot prepend the same expression twice
    QCOMPARE(n.argCount(), 1);
    QCOMPARE(n.arg(0).toConst(), c1);
    n.append(c1); // cannot append/prepend the same expression twice
    QCOMPARE(n.argCount(), 1);
    QCOMPARE(n.arg(0).toConst(), c1);

    n = NArgExpression(ArithmeticExpressionClass, '+');
    n2 = NArgExpression(ArithmeticExpressionClass, '+');
    c1 = ConstExpression(INTEGER_CONST, 4);
    n.append(c1);
    n2.append(c1); // c moves from n to n2
    QVERIFY(n.isEmpty());
    QCOMPARE(n2.argCount(), 1);
    QCOMPARE(c1.parent().toNArg(), n2);
    n.prepend(c1); // c moves from n2 to n
    QCOMPARE(n.argCount(), 1);
    QVERIFY(n2.isEmpty());
    QCOMPARE(c1.parent().toNArg(), n);

    // -- insert(int, Expression)
    n = NArgExpression(ArithmeticExpressionClass, '+');
    c1 = ConstExpression(INTEGER_CONST, 3);
    c2 = ConstExpression(INTEGER_CONST, 4);
    // i must be a valid index position in the list (i.e., 0 <= i < argCount()).
    n.insert(-10, c1);
    QVERIFY(n.isEmpty());
    n.insert(1, c1);
    QVERIFY(n.isEmpty());
    // if i is 0, the expression is prepended to the list of arguments
    n.insert(0, c1);
    QCOMPARE(n.arg(0).toConst(), c1);
    QCOMPARE(n.argCount(), 1);
    QCOMPARE(c1.parent().toNArg(), n);
    n.insert(0, c2);
    QCOMPARE(n.argCount(), 2);
    QCOMPARE(n.arg(0).toConst(), c2);
    QCOMPARE(n.arg(1).toConst(), c1);

    // if i is argCount(), the value is appended to the list of arguments
    n = NArgExpression(ArithmeticExpressionClass, '+');
    n.insert(0, c1);
    n.insert(1, c2);
    QCOMPARE(n.argCount(), 2);
    QCOMPARE(n.arg(0).toConst(), c1);
    QCOMPARE(n.arg(1).toConst(), c2);

    // expression cannot be own child
    n = NArgExpression(ArithmeticExpressionClass, '+');
    n.insert(0, n);
    QVERIFY(n.isEmpty());

    // cannot insert child twice
    n = NArgExpression(ArithmeticExpressionClass, '+');
    n.insert(0, c1);
    n.insert(1, c1);
    QCOMPARE(n.argCount(), 1);

    // -- remove(Expression)
    n = NArgExpression(ArithmeticExpressionClass, '+');
    n.append(c1);
    n.append(c2);
    n.remove(c1); // remove first
    QCOMPARE(n.argCount(), 1);
    QCOMPARE(n.arg(0).toConst(), c2);

    // -- remove(Expression)
    n = NArgExpression(ArithmeticExpressionClass, '+');
    n.prepend(c1);
    n.append(c2);
    c3 = ConstExpression(INTEGER_CONST, 5);
    QVERIFY(!n.remove(c3)); // not found
    QCOMPARE(n.argCount(), 2);
    n.append(c3);
    QCOMPARE(n.argCount(), 3);
    QVERIFY(n.remove(c2)); // remove 2nd of 3, leaves c1 and c3
    QCOMPARE(n.argCount(), 2);
    QCOMPARE(n.arg(0).toConst(), c1);
    QCOMPARE(n.arg(1).toConst(), c3);

    // -- removeAt(int)
    n = NArgExpression(ArithmeticExpressionClass, '+');
    n.prepend(c1);
    n.append(c2);
    n.removeAt(-1); // not found
    QCOMPARE(n.argCount(), 2);
    n.removeAt(3); // not found
    QCOMPARE(n.argCount(), 2);
    n.append(c3);
    n.removeAt(1); // remove 2nd of 3, leaves c1 and c3
    QCOMPARE(n.argCount(), 2);
    QCOMPARE(n.arg(0).toConst(), c1);
    QCOMPARE(n.arg(1).toConst(), c3);
    n.removeAt(0);
    QCOMPARE(n.argCount(), 1);
    n.removeAt(0);
    QCOMPARE(n.argCount(), 0);

    // -- takeAt(int)
    n = NArgExpression(ArithmeticExpressionClass, '+');
    n2 = n;
    c1 = ConstExpression(INTEGER_CONST, 1);
    c2 = ConstExpression(INTEGER_CONST, 2);
    c3 = ConstExpression(INTEGER_CONST, 3);
    n.append(c1);
    n.append(c2);
    n.append(c3);
    n.takeAt(-1); // not found
    QCOMPARE(n.argCount(), 3);
    n.takeAt(3); // not found
    QCOMPARE(n.argCount(), 3);
    e = n.takeAt(1);
    QCOMPARE(e.toConst(), c2); // e is 2nd
    QCOMPARE(n.argCount(), 2); // 1 arg taken
    QCOMPARE(n, n2);

    // -- indexOf(Expression, int)
    n = NArgExpression(ArithmeticExpressionClass, '+');
    c = ConstExpression(INTEGER_CONST, 0);
    c1 = ConstExpression(INTEGER_CONST, 1);
    c2 = ConstExpression(INTEGER_CONST, 2);
    c3 = ConstExpression(INTEGER_CONST, 3);
    n.append(c1);
    n.append(c2);
    n.append(c3);

    QCOMPARE(n.indexOf(c), -1);
    QCOMPARE(n.indexOf(c1), 0);
    QCOMPARE(n.indexOf(c2), 1);
    QCOMPARE(n.indexOf(c3), 2);
    QCOMPARE(n.indexOf(c1, 1), -1);
    QCOMPARE(n.indexOf(c2, 1), 1);

    // -- lastIndexOf(Expression, int)
    QCOMPARE(n.lastIndexOf(c), -1);
    QCOMPARE(n.lastIndexOf(c1), 0);
    QCOMPARE(n.lastIndexOf(c2), 1);
    QCOMPARE(n.lastIndexOf(c3), 2);
    QCOMPARE(n.lastIndexOf(c1, 1), 0);
    QCOMPARE(n.lastIndexOf(c2, 0), -1);
}

void TestExpressions::testValidate()
{
    NArgExpression n;
    ConstExpression c;
    ConstExpression c1;

    QuerySchema *query = 0;
    ParseInfoInternal parseInfo(query);

    c = ConstExpression(SQL_NULL, QVariant());
    QCOMPARE(c.type(), Field::Null);
    QVERIFY(c.validate(&parseInfo));

    n = NArgExpression(ArithmeticExpressionClass, '+');
    c = ConstExpression(INTEGER_CONST, 0);
    c1 = ConstExpression(INTEGER_CONST, 1);
    n.append(c);
    n.append(c1);
    QCOMPARE(n.type(), Field::InvalidType); // N-arg expression is abstract, unspecified type
    QVERIFY(n.validate(&parseInfo));
    testCloneExpression(n);

    // null
    c = ConstExpression(SQL_NULL, QVariant());
    QCOMPARE(c.type(), Field::Null);
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    // integer
    c = ConstExpression(INTEGER_CONST, -0x7f);
    QCOMPARE(c.type(), Field::Byte);
    QCOMPARE(c.value(), QVariant(-0x7f));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    c.setValue(-0x80);
    QCOMPARE(c.type(), Field::ShortInteger); // type has been changed by setValue
    QCOMPARE(c.value(), QVariant(-0x80));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    c = ConstExpression(INTEGER_CONST, -10);
    QCOMPARE(c.type(), Field::Byte);
    QCOMPARE(c.value(), QVariant(-10));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    c = ConstExpression(INTEGER_CONST, 0);
    QCOMPARE(c.type(), Field::Byte);
    QCOMPARE(c.value(), QVariant(0));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    c = ConstExpression(INTEGER_CONST, 20);
    QCOMPARE(c.type(), Field::Byte);
    QCOMPARE(c.value(), QVariant(20));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    c = ConstExpression(INTEGER_CONST, 255);
    QCOMPARE(c.type(), Field::Byte);
    QCOMPARE(c.value(), QVariant(255));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    c = ConstExpression(INTEGER_CONST, -0x80);
    QCOMPARE(c.type(), Field::ShortInteger);
    QCOMPARE(c.value(), QVariant(-0x80));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    c = ConstExpression(INTEGER_CONST, -0x7fff);
    QCOMPARE(c.type(), Field::ShortInteger);
    QCOMPARE(c.value(), QVariant(-0x7fff));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    c = ConstExpression(INTEGER_CONST, 256);
    QCOMPARE(c.type(), Field::ShortInteger);
    QCOMPARE(c.value(), QVariant(256));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    c = ConstExpression(INTEGER_CONST, 0xffff);
    QCOMPARE(c.type(), Field::ShortInteger);
    QCOMPARE(c.value(), QVariant(0xffff));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    c = ConstExpression(INTEGER_CONST, -0x8000);
    QCOMPARE(c.type(), Field::Integer);
    QCOMPARE(c.value(), QVariant(-0x8000));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    c = ConstExpression(INTEGER_CONST, uint(0x10000));
    QCOMPARE(c.type(), Field::Integer);
    QCOMPARE(c.value(), QVariant(0x10000));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    c = ConstExpression(INTEGER_CONST, qlonglong(-0x100000));
    QCOMPARE(c.type(), Field::BigInteger);
    QCOMPARE(c.value(), QVariant(-0x100000));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    c = ConstExpression(INTEGER_CONST, qulonglong(0x1000000));
    QCOMPARE(c.type(), Field::BigInteger);
    QCOMPARE(c.value(), QVariant(0x1000000));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    // string
    int oldMaxLen = Field::defaultMaxLength(); // save
    Field::setDefaultMaxLength(0);
    c = ConstExpression(CHARACTER_STRING_LITERAL, "01234567890");
    QCOMPARE(c.type(), Field::Text);
    QCOMPARE(c.value(), QVariant("01234567890"));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    Field::setDefaultMaxLength(10);
    c = ConstExpression(CHARACTER_STRING_LITERAL, QString());
    QCOMPARE(c.type(), Field::Text);
    QCOMPARE(c.value(), QVariant(QString()));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    c = ConstExpression(CHARACTER_STRING_LITERAL, QVariant());
    QCOMPARE(c.type(), Field::Text);
    QCOMPARE(c.value(), QVariant());
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    c = ConstExpression(CHARACTER_STRING_LITERAL, "01234567890");
    QCOMPARE(c.type(), Field::LongText);
    QCOMPARE(c.value(), QVariant("01234567890"));
    QVERIFY(c.validate(&parseInfo));
    c.setValue("ąćę");
    QCOMPARE(c.value(), QVariant("ąćę"));
    QCOMPARE(c.type(), Field::Text);
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    Field::setDefaultMaxLength(oldMaxLen); // restore

    // bool
    c = ConstExpression(SQL_TRUE, true);
    QCOMPARE(c.type(), Field::Boolean);
    QCOMPARE(c.value(), QVariant(true));
    QVERIFY(c.validate(&parseInfo));
    c.setValue(false);
    QCOMPARE(c.value(), QVariant(false));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    c = ConstExpression(SQL_FALSE, false);
    QCOMPARE(c.type(), Field::Boolean);
    QCOMPARE(c.value(), QVariant(false));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    // real
    c = ConstExpression(REAL_CONST, QVariant());
    QCOMPARE(c.type(), Field::Double);
    QCOMPARE(c.value(), QVariant());
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    c = ConstExpression(REAL_CONST, 3.14159);
    QCOMPARE(c.type(), Field::Double);
    QCOMPARE(c.value(), QVariant(3.14159));
    QVERIFY(c.validate(&parseInfo));
    c.setValue(-18.012);
    QCOMPARE(c.value(), QVariant(-18.012));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    // date
    QDate date(QDate::currentDate());
    c = ConstExpression(DATE_CONST, date);
    QCOMPARE(c.type(), Field::Date);
    QCOMPARE(c.value(), QVariant(date));
    QVERIFY(c.validate(&parseInfo));
    date = date.addDays(17);
    c.setValue(date);
    QCOMPARE(c.value(), QVariant(date));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    // date/time
    QDateTime dateTime(QDateTime::currentDateTime());
    c = ConstExpression(DATETIME_CONST, dateTime);
    QCOMPARE(c.type(), Field::DateTime);
    QCOMPARE(c.value(), QVariant(dateTime));
    QVERIFY(c.validate(&parseInfo));
    dateTime = dateTime.addDays(-17);
    c.setValue(dateTime);
    QCOMPARE(c.value(), QVariant(dateTime));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    // time
    QTime time(QTime::currentTime());
    c = ConstExpression(TIME_CONST, time);
    QCOMPARE(c.type(), Field::Time);
    QCOMPARE(c.value(), QVariant(time));
    QVERIFY(c.validate(&parseInfo));
    time.addSecs(1200);
    c.setValue(time);
    QCOMPARE(c.value(), QVariant(time));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);

    // setValue()
    c = ConstExpression(INTEGER_CONST, 124);
    QCOMPARE(c.value(), QVariant(124));
    c.setValue(299);
    QCOMPARE(c.value(), QVariant(299));
    testCloneExpression(c);
}

void TestExpressions::cleanupTestCase()
{
}

QTEST_MAIN(TestExpressions)

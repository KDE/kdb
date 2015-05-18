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

#include <KDbExpression>
#include "parser/generated/sqlparser.h"
#include "parser/KDbParser_p.h"

Q_DECLARE_METATYPE(KDb::ExpressionClass)
Q_DECLARE_METATYPE(KDbEscapedString)
Q_DECLARE_METATYPE(KDbField::Type)

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
    KDbExpression e1clone = e1.clone();
    QVERIFY(e1 != e1.clone());
    QVERIFY(e1 != e1clone);
    QVERIFY(e1.clone() != e1clone);
    compareStrings(KDbExpression(e1), e1clone);

    const T copied(e1);
    QVERIFY(e1 == copied);
    QVERIFY(e1.clone() != copied);
    compareStrings(e1, copied);
}

void TestExpressions::testNullExpression()
{
    QVERIFY(KDbExpression() != KDbExpression());

    KDbExpression e1;
    KDbExpression e2;
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
    QVERIFY(e1 != KDbExpression());
    QVERIFY(e1 == e1);
    QVERIFY(e1 != e2);

    e1 = e2;
    QVERIFY(e1.isNull());
    QCOMPARE(e1, e2);
    QCOMPARE(e1.toString(), KDbEscapedString("<NULL!>"));
    QCOMPARE(e1.tokenToDebugString(), QLatin1String("0"));
    QCOMPARE(e1.tokenToString(), QString());
    compareStrings(e1, e2);

    KDbExpression e3(e2);
    QVERIFY(e3.isNull());
    QCOMPARE(e2, e3);
    compareStrings(e2, e3);
    //ExpressionDebug << "$$$" << e1.toString() << e1.tokenToDebugString() << e1.tokenToString();

    e1 = KDbExpression();
    testCloneExpression(e1);
}

void TestExpressions::testExpressionClassName_data()
{
    QTest::addColumn<KDb::ExpressionClass>("expClass");
    QTest::addColumn<QString>("name");

#define T(n, c) QTest::newRow(n) << c << n
    T("Unknown", UnknownExpressionClass);
    T("Unary", KDb::UnaryExpression);
    T("Arithm", KDb::ArithmeticExpression);
    T("Logical", KDb::LogicalExpression);
    T("Relational", KDb::RelationalExpression);
    T("SpecialBinary", KDb::SpecialBinaryExpression);
    T("Const", KDb::ConstExpression);
    T("Variable", KDb::VariableExpression);
    T("Function", KDb::FunctionExpression);
    T("Aggregation", KDb::AggregationExpression);
    T("FieldList", FieldListExpressionClass);
    T("TableList", TableListExpressionClass);
    T("QueryParameter", QueryParameterExpressionClass);
#undef T
}

void TestExpressions::testExpressionClassName()
{
    QFETCH(KDb::ExpressionClass, expClass);
    QTEST(expressionClassName(expClass), "name");
}

class TestedExpression : public KDbExpression
{
public:
    TestedExpression(int token) : KDbExpression(new KDbExpressionData, UnknownExpressionClass, token) {
    }
};

void TestExpressions::testExpressionToken()
{
    KDbExpression e1;
    QCOMPARE(e1.token(), 0);

    for (int i = 0; i < 254; ++i) {
        QCOMPARE(KDbExpression::tokenToDebugString(i),
                isprint(i) ? QString(QLatin1Char(uchar(i))) : QString::number(i));
        QCOMPARE(TestedExpression(i).tokenToString(),
                isprint(i) ? QString(QLatin1Char(uchar(i))) : QString());
    }
    for (int i = 255; i <= maxToken(); ++i) {
        QCOMPARE(KDbExpression::tokenToDebugString(i), QLatin1String(tokenName(i)));
    }
}

void TestExpressions::testNArgExpression()
{
    KDbNArgExpression n;
    KDbNArgExpression n2;
    KDbConstExpression c;
    KDbConstExpression c1;
    KDbConstExpression c2;
    KDbConstExpression c3;

    // -- empty
    KDbNArgExpression emptyNarg;
    QVERIFY(emptyNarg.isNArg());
    QVERIFY(emptyNarg.clone().isNArg());
    QVERIFY(emptyNarg.isEmpty());
    QCOMPARE(emptyNarg.argCount(), 0);
    QVERIFY(emptyNarg.arg(-1).isNull());
    QVERIFY(emptyNarg.arg(0).isNull());

    // -- copy ctor & cloning
    n = KDbNArgExpression(KDb::ArithmeticExpression, '+');
    c1 = KDbConstExpression(INTEGER_CONST, 7);
    c2 = KDbConstExpression(INTEGER_CONST, 8);
    n.append(c1);
    n.append(c2);
    testCloneExpression(n);

    // copy on stack
    n = KDbNArgExpression(KDb::ArithmeticExpression, '+');
    {
        KDbConstExpression s1(INTEGER_CONST, 7);
        KDbConstExpression s2(INTEGER_CONST, 8);
        n.append(s1);
        n.append(s2);
        c1 = s1;
        c2 = s2;
    }
    QCOMPARE(n.argCount(), 2);
    QCOMPARE(n.arg(0).toConst(), c1);
    QCOMPARE(n.arg(1).toConst(), c2);

    QCOMPARE(n.tokenToDebugString(), QString("+"));
    QCOMPARE(n.toString(), KDbEscapedString("7, 8"));

    // -- append(KDbExpression), prepend(KDbExpression)
    KDbExpression e;
    KDbNArgExpression nNull;
    nNull.append(e);
    QCOMPARE(nNull.argCount(), 0); // n-arg expression should have class, otherwise is null
                                   // and cannot have children

    n = KDbNArgExpression(KDb::ArithmeticExpression, '+');
    c1 = KDbConstExpression(INTEGER_CONST, 1);
    n.append(c1);
    QVERIFY(!n.isEmpty());
    QCOMPARE(n.argCount(), 1);
    QCOMPARE(n.arg(0).toConst(), c1);
    QCOMPARE(c1.parent().toNArg(), n);

    n = KDbNArgExpression(KDb::ArithmeticExpression, '+');
    n.append(n);
    QCOMPARE(n.argCount(), 0); // append should fail since appending expression
                               // to itself is not allowed
    n.prepend(n);
    QCOMPARE(n.argCount(), 0); // append should fail since prepending expression
                               // to itself is not allowed

    n = KDbNArgExpression(KDb::ArithmeticExpression, '+');
    c1 = KDbConstExpression(INTEGER_CONST, 2);
    n.append(c1);
    n.append(c1); // cannot append the same expression twice
    QCOMPARE(n.argCount(), 1);
    QCOMPARE(n.arg(0).toConst(), c1);

    n = KDbNArgExpression(KDb::ArithmeticExpression, '+');
    c1 = KDbConstExpression(INTEGER_CONST, 3);
    n.prepend(c1);
    n.prepend(c1); // cannot prepend the same expression twice
    QCOMPARE(n.argCount(), 1);
    QCOMPARE(n.arg(0).toConst(), c1);
    n.append(c1); // cannot append/prepend the same expression twice
    QCOMPARE(n.argCount(), 1);
    QCOMPARE(n.arg(0).toConst(), c1);

    n = KDbNArgExpression(KDb::ArithmeticExpression, '+');
    n2 = KDbNArgExpression(KDb::ArithmeticExpression, '+');
    c1 = KDbConstExpression(INTEGER_CONST, 4);
    n.append(c1);
    n2.append(c1); // c moves from n to n2
    QVERIFY(n.isEmpty());
    QCOMPARE(n2.argCount(), 1);
    QCOMPARE(c1.parent().toNArg(), n2);
    n.prepend(c1); // c moves from n2 to n
    QCOMPARE(n.argCount(), 1);
    QVERIFY(n2.isEmpty());
    QCOMPARE(c1.parent().toNArg(), n);

    // -- insert(int, KDbExpression)
    n = KDbNArgExpression(KDb::ArithmeticExpression, '+');
    c1 = KDbConstExpression(INTEGER_CONST, 3);
    c2 = KDbConstExpression(INTEGER_CONST, 4);
    // it must be a valid index position in the list (i.e., 0 <= i < argCount()).
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
    n = KDbNArgExpression(KDb::ArithmeticExpression, '+');
    n.insert(0, c1);
    n.insert(1, c2);
    QCOMPARE(n.argCount(), 2);
    QCOMPARE(n.arg(0).toConst(), c1);
    QCOMPARE(n.arg(1).toConst(), c2);

    // expression cannot be own child
    n = KDbNArgExpression(KDb::ArithmeticExpression, '+');
    n.insert(0, n);
    QVERIFY(n.isEmpty());

    // cannot insert child twice
    n = KDbNArgExpression(KDb::ArithmeticExpression, '+');
    n.insert(0, c1);
    n.insert(1, c1);
    QCOMPARE(n.argCount(), 1);

    // -- remove(KDbExpression)
    n = KDbNArgExpression(KDb::ArithmeticExpression, '+');
    n.append(c1);
    n.append(c2);
    n.remove(c1); // remove first
    QCOMPARE(n.argCount(), 1);
    QCOMPARE(n.arg(0).toConst(), c2);

    // -- remove(KDbExpression)
    n = KDbNArgExpression(KDb::ArithmeticExpression, '+');
    n.prepend(c1);
    n.append(c2);
    c3 = KDbConstExpression(INTEGER_CONST, 5);
    QVERIFY(!n.remove(c3)); // not found
    QCOMPARE(n.argCount(), 2);
    n.append(c3);
    QCOMPARE(n.argCount(), 3);
    QVERIFY(n.remove(c2)); // remove 2nd of 3, leaves c1 and c3
    QCOMPARE(n.argCount(), 2);
    QCOMPARE(n.arg(0).toConst(), c1);
    QCOMPARE(n.arg(1).toConst(), c3);

    // -- removeAt(int)
    n = KDbNArgExpression(KDb::ArithmeticExpression, '+');
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
    n = KDbNArgExpression(KDb::ArithmeticExpression, '+');
    n2 = n;
    c1 = KDbConstExpression(INTEGER_CONST, 1);
    c2 = KDbConstExpression(INTEGER_CONST, 2);
    c3 = KDbConstExpression(INTEGER_CONST, 3);
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

    // -- indexOf(KDbExpression, int)
    n = KDbNArgExpression(KDb::ArithmeticExpression, '+');
    c = KDbConstExpression(INTEGER_CONST, 0);
    c1 = KDbConstExpression(INTEGER_CONST, 1);
    c2 = KDbConstExpression(INTEGER_CONST, 2);
    c3 = KDbConstExpression(INTEGER_CONST, 3);
    n.append(c1);
    n.append(c2);
    n.append(c3);

    QCOMPARE(n.indexOf(c), -1);
    QCOMPARE(n.indexOf(c1), 0);
    QCOMPARE(n.indexOf(c2), 1);
    QCOMPARE(n.indexOf(c3), 2);
    QCOMPARE(n.indexOf(c1, 1), -1);
    QCOMPARE(n.indexOf(c2, 1), 1);

    // -- lastIndexOf(KDbExpression, int)
    QCOMPARE(n.lastIndexOf(c), -1);
    QCOMPARE(n.lastIndexOf(c1), 0);
    QCOMPARE(n.lastIndexOf(c2), 1);
    QCOMPARE(n.lastIndexOf(c3), 2);
    QCOMPARE(n.lastIndexOf(c1, 1), 0);
    QCOMPARE(n.lastIndexOf(c2, 0), -1);
}

void TestExpressions::testUnaryExpression()
{
    KDbUnaryExpression u;
    KDbUnaryExpression u2;
    KDbConstExpression c;
    KDbConstExpression c1;

    // -- empty
    KDbUnaryExpression emptyUnary;
    QVERIFY(emptyUnary.isUnary());
    QVERIFY(emptyUnary.clone().isUnary());
    QVERIFY(emptyUnary.arg().isNull());

    u = KDbUnaryExpression('-', KDbExpression());
    QVERIFY(u.arg().isNull());

    // -- copy ctor & cloning
    c1 = KDbConstExpression(INTEGER_CONST, 7);
    u = KDbUnaryExpression('-', c1);
    testCloneExpression(u);
    QCOMPARE(u.tokenToDebugString(), QString("-"));
    QCOMPARE(u.toString(), KDbEscapedString("-7"));
    QCOMPARE(c1, u.arg().toConst());

    u2 = KDbUnaryExpression('-', u);
    testCloneExpression(u);
    QCOMPARE(u2.tokenToDebugString(), QString("-"));
    QCOMPARE(u2.toString(), KDbEscapedString("--7"));
    QCOMPARE(u, u2.arg().toUnary());

    u = KDbUnaryExpression('(', c1);
    testCloneExpression(u);
    QCOMPARE(u.toString(), KDbEscapedString("(7)"));
    QCOMPARE(c1, u.arg().toConst());

    c1 = KDbConstExpression(SQL_TRUE, true);
    u = KDbUnaryExpression(NOT, c1);
    testCloneExpression(u);
    QCOMPARE(u.toString(), KDbEscapedString("NOT TRUE"));
    QCOMPARE(c1, u.arg().toConst());

    c1 = KDbConstExpression(SQL_NULL, QVariant());
    u = KDbUnaryExpression(SQL_IS_NULL, c1);
    testCloneExpression(u);
    QCOMPARE(u.toString(), KDbEscapedString("NULL IS NULL"));
    QCOMPARE(c1, u.arg().toConst());

    c1 = KDbConstExpression(SQL_NULL, QVariant());
    u = KDbUnaryExpression(SQL_IS_NOT_NULL, c1);
    testCloneExpression(u);
    QCOMPARE(u.toString(), KDbEscapedString("NULL IS NOT NULL"));
    QCOMPARE(c1, u.arg().toConst());

    c1 = KDbConstExpression(INTEGER_CONST, 17);
    u = KDbUnaryExpression(SQL, c1);
    testCloneExpression(u);
    QCOMPARE(u.toString(), KDbEscapedString("{INVALID_OPERATOR#%1} 17").arg(SQL));
    QCOMPARE(c1, u.arg().toConst());

    // -- exchanging arg between two unary expressions
    c = KDbConstExpression(INTEGER_CONST, 17);
    u = KDbUnaryExpression('-', c);
    c1 = KDbConstExpression(INTEGER_CONST, 3);
    u2 = KDbUnaryExpression('+', c1);
    u2.setArg(c); // this should take c arg from u to u2
    QCOMPARE(c, u2.arg().toConst()); // c is now in u2
    QVERIFY(u.arg().isNull()); // u has null arg now

    c = KDbConstExpression(INTEGER_CONST, 17);
    u = KDbUnaryExpression('-', c);
    u2 = KDbUnaryExpression('+', c);
    // u2 takes c arg from u
    QCOMPARE(c, u2.arg().toConst()); // c is now in u2
    QVERIFY(u.arg().isNull()); // u has null arg now

    // -- cycles
    c = KDbConstExpression(INTEGER_CONST, 17);
    u = KDbUnaryExpression('-', c);
    c1 = KDbConstExpression(INTEGER_CONST, 3);
    u2 = KDbUnaryExpression('+', c1);
    u2.setArg(u);
    u.setArg(u2);
    QCOMPARE(u.toString(), KDbEscapedString("-+<CYCLE!>"));
    QCOMPARE(u2.toString(), KDbEscapedString("+-<CYCLE!>"));
}

void TestExpressions::testBinaryExpression()
{
    KDbBinaryExpression b;
    KDbBinaryExpression b2;
    KDbConstExpression c;
    KDbConstExpression c1;

    // -- empty
    KDbBinaryExpression emptyBinary;
    QVERIFY(emptyBinary.isNull());
    QVERIFY(emptyBinary.isBinary());
    QVERIFY(emptyBinary.clone().isBinary());
    QVERIFY(emptyBinary.left().isNull());
    QVERIFY(emptyBinary.right().isNull());

    b = KDbBinaryExpression(KDbExpression(), '-', KDbExpression());
    QVERIFY(b.left().isNull());
    QVERIFY(b.right().isNull());
    QVERIFY(b.isNull()); // it's null because args are null
    qDebug() << b.toString();
    QCOMPARE(b.toString(), KDbEscapedString("<NULL!>"));
    c = KDbConstExpression(INTEGER_CONST, 10);
    b = KDbBinaryExpression(c, '-', KDbExpression());
    QVERIFY(b.left().isNull());
    QVERIFY(b.right().isNull());
    QVERIFY(b.isNull()); // it's null because one arg is null
    qDebug() << b.toString();
    QCOMPARE(b.toString(), KDbEscapedString("<NULL!>"));
    b = KDbBinaryExpression(KDbExpression(), '-', c);
    QVERIFY(b.left().isNull());
    QVERIFY(b.right().isNull());
    QVERIFY(b.isNull()); // it's null because one arg is null
    qDebug() << b.toString();
    QCOMPARE(b.toString(), KDbEscapedString("<NULL!>"));

    // -- copy ctor & cloning
    c = KDbConstExpression(INTEGER_CONST, 3);
    c1 = KDbConstExpression(INTEGER_CONST, 4);
    b = KDbBinaryExpression(c, '/', c1);
    testCloneExpression(b);
    QCOMPARE(b.tokenToDebugString(), QString("/"));
    QCOMPARE(b.toString(), KDbEscapedString("3 / 4"));
    QCOMPARE(c1, b.right().toConst());

    b2 = KDbBinaryExpression(b, '*', b.clone());
    testCloneExpression(b2);
    QCOMPARE(b2.tokenToDebugString(), QString("*"));
    QCOMPARE(b2.toString(), KDbEscapedString("3 / 4 * 3 / 4"));
    QCOMPARE(b, b2.left().toBinary());

    // -- cycles
    // --- ref to parent
    b = KDbBinaryExpression(
            KDbConstExpression(INTEGER_CONST, 1), '+', KDbConstExpression(INTEGER_CONST, 2));
    KDbEscapedString s = b.toString();
    b.setLeft(b); // should not work
    qDebug() << b.toString();
    QCOMPARE(s, b.toString());
    // --- cannot set twice
    c = b.left().toConst();
    b.setLeft(c);
    QCOMPARE(s, b.toString());
    // --- ref to grandparent
    b = KDbBinaryExpression(
            KDbConstExpression(INTEGER_CONST, 1), '+', KDbConstExpression(INTEGER_CONST, 2));
    c = KDbConstExpression(INTEGER_CONST, 10);
    b2 = KDbBinaryExpression(b, '-', c);
    qDebug() << b2.toString();
    QCOMPARE(b2.toString(), KDbEscapedString("1 + 2 - 10"));
    b.setRight(b2);
    qDebug() << b2.toString();
    QCOMPARE(b2.toString(), KDbEscapedString("1 + <CYCLE!> - 10"));

    // -- moving right argument to left should remove right arg
    b = KDbBinaryExpression(
            KDbConstExpression(INTEGER_CONST, 1), '+', KDbConstExpression(INTEGER_CONST, 2));
    c = b.right().toConst();
    b.setLeft(c);
    qDebug() << b.toString();
    QCOMPARE(b.toString(), KDbEscapedString("2 + <NULL!>"));

    // -- moving left argument to right should remove left arg
    b = KDbBinaryExpression(
            KDbConstExpression(INTEGER_CONST, 1), '+', KDbConstExpression(INTEGER_CONST, 2));
    c = b.left().toConst();
    b.setRight(c);
    qDebug() << b.toString();
    QCOMPARE(b.toString(), KDbEscapedString("<NULL!> + 1"));
}

void TestExpressions::testBinaryExpressionCloning_data()
{
    QTest::addColumn<int>("type1");
    QTest::addColumn<QVariant>("const1");
    QTest::addColumn<int>("token");
    QTest::addColumn<int>("type2");
    QTest::addColumn<QVariant>("const2");
    QTest::addColumn<QString>("string");

#define T(type1, const1, token, type2, const2, string) \
        QTest::newRow(KDbExpression::tokenToDebugString(token).toLatin1()) \
            << int(type1) << QVariant(const1) << int(token) \
            << int(type2) << QVariant(const2) << QString(string)

    T(INTEGER_CONST, 3, '/', INTEGER_CONST, 4, "3 / 4");
    T(INTEGER_CONST, 3, BITWISE_SHIFT_RIGHT, INTEGER_CONST, 4, "3 >> 4");
    T(INTEGER_CONST, 3, BITWISE_SHIFT_LEFT, INTEGER_CONST, 4, "3 << 4");
    T(INTEGER_CONST, 3, NOT_EQUAL, INTEGER_CONST, 4, "3 <> 4");
    T(INTEGER_CONST, 3, NOT_EQUAL2, INTEGER_CONST, 4, "3 != 4");
    T(INTEGER_CONST, 3, LESS_OR_EQUAL, INTEGER_CONST, 4, "3 <= 4");
    T(INTEGER_CONST, 3, GREATER_OR_EQUAL, INTEGER_CONST, 4, "3 >= 4");
    T(CHARACTER_STRING_LITERAL, "ABC", LIKE, CHARACTER_STRING_LITERAL, "A%", "'ABC' LIKE 'A%'");
    T(INTEGER_CONST, 3, SQL_IN, INTEGER_CONST, 4, "3 IN 4");
    T(INTEGER_CONST, 3, SIMILAR_TO, INTEGER_CONST, 4, "3 SIMILAR TO 4");
    T(INTEGER_CONST, 3, NOT_SIMILAR_TO, INTEGER_CONST, 4, "3 NOT SIMILAR TO 4");
    T(SQL_TRUE, true, OR, SQL_FALSE, false, "TRUE OR FALSE");
    T(INTEGER_CONST, 3, AND, INTEGER_CONST, 4, "3 AND 4");
    T(INTEGER_CONST, 3, XOR, INTEGER_CONST, 4, "3 XOR 4");
    T(CHARACTER_STRING_LITERAL, "AB", CONCATENATION, CHARACTER_STRING_LITERAL, "CD", "'AB' || 'CD'");
#undef T
}

void TestExpressions::testBinaryExpressionCloning()
{
    QFETCH(int, type1);
    QFETCH(QVariant, const1);
    QFETCH(int, token);
    QFETCH(int, type2);
    QFETCH(QVariant, const2);
    QFETCH(QString, string);

    KDbConstExpression c(type1, const1);
    KDbConstExpression c1(type2, const2);
    KDbBinaryExpression b(c, token, c1);
    testCloneExpression(b);
    QCOMPARE(b.tokenToDebugString(), KDbExpression::tokenToDebugString(token));
    qDebug() << KDbExpression::tokenToDebugString(token) << b.toString();
    QCOMPARE(b.toString(), KDbEscapedString(string));
    QCOMPARE(c, b.left().toConst());
    QCOMPARE(c1, b.right().toConst());
}

void TestExpressions::testConstExpressionValidate()
{
    KDbConstExpression c;
    KDbConstExpression c1;

    KDbQuerySchema *query = 0;
    ParseInfoInternal parseInfo(query);

    c = KDbConstExpression(SQL_NULL, QVariant());
    QCOMPARE(c.type(), KDbField::Null);
    QVERIFY(c.validate(&parseInfo));

    // null
    c = KDbConstExpression(SQL_NULL, QVariant());
    QCOMPARE(c.type(), KDbField::Null);
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    // integer
    c = KDbConstExpression(INTEGER_CONST, -0x7f);
    QCOMPARE(c.type(), KDbField::Byte);
    QCOMPARE(c.value(), QVariant(-0x7f));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    c.setValue(-0x80);
    QCOMPARE(c.type(), KDbField::ShortInteger); // type has been changed by setValue
    QCOMPARE(c.value(), QVariant(-0x80));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    c = KDbConstExpression(INTEGER_CONST, -10);
    QCOMPARE(c.type(), KDbField::Byte);
    QCOMPARE(c.value(), QVariant(-10));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    c = KDbConstExpression(INTEGER_CONST, 0);
    QCOMPARE(c.type(), KDbField::Byte);
    QCOMPARE(c.value(), QVariant(0));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    c = KDbConstExpression(INTEGER_CONST, 20);
    QCOMPARE(c.type(), KDbField::Byte);
    QCOMPARE(c.value(), QVariant(20));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    c = KDbConstExpression(INTEGER_CONST, 255);
    QCOMPARE(c.type(), KDbField::Byte);
    QCOMPARE(c.value(), QVariant(255));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    c = KDbConstExpression(INTEGER_CONST, -0x80);
    QCOMPARE(c.type(), KDbField::ShortInteger);
    QCOMPARE(c.value(), QVariant(-0x80));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    c = KDbConstExpression(INTEGER_CONST, -0x7fff);
    QCOMPARE(c.type(), KDbField::ShortInteger);
    QCOMPARE(c.value(), QVariant(-0x7fff));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    c = KDbConstExpression(INTEGER_CONST, 256);
    QCOMPARE(c.type(), KDbField::ShortInteger);
    QCOMPARE(c.value(), QVariant(256));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    c = KDbConstExpression(INTEGER_CONST, 0xffff);
    QCOMPARE(c.type(), KDbField::ShortInteger);
    QCOMPARE(c.value(), QVariant(0xffff));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    c = KDbConstExpression(INTEGER_CONST, -0x8000);
    QCOMPARE(c.type(), KDbField::Integer);
    QCOMPARE(c.value(), QVariant(-0x8000));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    c = KDbConstExpression(INTEGER_CONST, uint(0x10000));
    QCOMPARE(c.type(), KDbField::Integer);
    QCOMPARE(c.value(), QVariant(0x10000));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    c = KDbConstExpression(INTEGER_CONST, qlonglong(-0x100000));
    QCOMPARE(c.type(), KDbField::BigInteger);
    QCOMPARE(c.value(), QVariant(-0x100000));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    c = KDbConstExpression(INTEGER_CONST, qulonglong(0x1000000));
    QCOMPARE(c.type(), KDbField::BigInteger);
    QCOMPARE(c.value(), QVariant(0x1000000));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    // string
    int oldMaxLen = KDbField::defaultMaxLength(); // save
    KDbField::setDefaultMaxLength(0);
    c = KDbConstExpression(CHARACTER_STRING_LITERAL, "01234567890");
    QCOMPARE(c.type(), KDbField::Text);
    QCOMPARE(c.value(), QVariant("01234567890"));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    KDbField::setDefaultMaxLength(10);
    c = KDbConstExpression(CHARACTER_STRING_LITERAL, QString());
    QCOMPARE(c.type(), KDbField::Text);
    QCOMPARE(c.value(), QVariant(QString()));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    c = KDbConstExpression(CHARACTER_STRING_LITERAL, QVariant());
    QCOMPARE(c.type(), KDbField::Text);
    QCOMPARE(c.value(), QVariant());
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    c = KDbConstExpression(CHARACTER_STRING_LITERAL, "01234567890");
    QCOMPARE(c.type(), KDbField::LongText);
    QCOMPARE(c.value(), QVariant("01234567890"));
    QVERIFY(c.validate(&parseInfo));
    qDebug() << c;
    c.setValue("ąćę");
    QCOMPARE(c.value(), QVariant("ąćę"));
    QCOMPARE(c.type(), KDbField::Text);
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    KDbField::setDefaultMaxLength(oldMaxLen); // restore

    // bool
    c = KDbConstExpression(SQL_TRUE, true);
    QCOMPARE(c.type(), KDbField::Boolean);
    QCOMPARE(c.value(), QVariant(true));
    QVERIFY(c.validate(&parseInfo));
    qDebug() << c;
    c.setValue(false);
    QCOMPARE(c.value(), QVariant(false));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    c = KDbConstExpression(SQL_FALSE, false);
    QCOMPARE(c.type(), KDbField::Boolean);
    QCOMPARE(c.value(), QVariant(false));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    // real
    c = KDbConstExpression(REAL_CONST, QVariant());
    QCOMPARE(c.type(), KDbField::Double);
    QCOMPARE(c.value(), QVariant());
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    c = KDbConstExpression(REAL_CONST, 3.14159);
    QCOMPARE(c.type(), KDbField::Double);
    QCOMPARE(c.value(), QVariant(3.14159));
    QVERIFY(c.validate(&parseInfo));
    qDebug() << c;
    c.setValue(-18.012);
    QCOMPARE(c.value(), QVariant(-18.012));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    // date
    QDate date(QDate::currentDate());
    c = KDbConstExpression(DATE_CONST, date);
    QCOMPARE(c.type(), KDbField::Date);
    QCOMPARE(c.value(), QVariant(date));
    QVERIFY(c.validate(&parseInfo));
    qDebug() << c;
    date = date.addDays(17);
    c.setValue(date);
    QCOMPARE(c.value(), QVariant(date));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    // date/time
    QDateTime dateTime(QDateTime::currentDateTime());
    c = KDbConstExpression(DATETIME_CONST, dateTime);
    QCOMPARE(c.type(), KDbField::DateTime);
    QCOMPARE(c.value(), QVariant(dateTime));
    QVERIFY(c.validate(&parseInfo));
    qDebug() << c;
    dateTime = dateTime.addDays(-17);
    c.setValue(dateTime);
    QCOMPARE(c.value(), QVariant(dateTime));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    // time
    QTime time(QTime::currentTime());
    c = KDbConstExpression(TIME_CONST, time);
    QCOMPARE(c.type(), KDbField::Time);
    QCOMPARE(c.value(), QVariant(time));
    qDebug() << c;
    QVERIFY(c.validate(&parseInfo));
    time.addSecs(1200);
    c.setValue(time);
    QCOMPARE(c.value(), QVariant(time));
    QVERIFY(c.validate(&parseInfo));
    testCloneExpression(c);
    qDebug() << c;

    // setValue()
    c = KDbConstExpression(INTEGER_CONST, 124);
    QCOMPARE(c.value(), QVariant(124));
    c.setValue(299);
    QCOMPARE(c.value(), QVariant(299));
    testCloneExpression(c);
    qDebug() << c;
}

void TestExpressions::testUnaryExpressionValidate()
{
    KDbConstExpression c;
    KDbConstExpression c1;
    KDbUnaryExpression u;
    KDbUnaryExpression u2;
    KDbQuerySchema *query = 0;
    ParseInfoInternal parseInfo(query);

    // cycles detected by validate()
    c = KDbConstExpression(INTEGER_CONST, 17);
    u = KDbUnaryExpression('-', c);
    c1 = KDbConstExpression(INTEGER_CONST, 3);
    u2 = KDbUnaryExpression('+', c1);
    u2.setArg(u);
    u.setArg(u2);
    QVERIFY(!u.validate(&parseInfo));
    qDebug() << c << u << c1 << u2;
}

void TestExpressions::testNArgExpressionValidate()
{
    KDbNArgExpression n;
    KDbConstExpression c;
    KDbConstExpression c1;

    KDbQuerySchema *query = 0;
    ParseInfoInternal parseInfo(query);

    c = KDbConstExpression(SQL_NULL, QVariant());
    QCOMPARE(c.type(), KDbField::Null);
    QVERIFY(c.validate(&parseInfo));

    n = KDbNArgExpression(KDb::ArithmeticExpression, '+');
    c = KDbConstExpression(INTEGER_CONST, 0);
    c1 = KDbConstExpression(INTEGER_CONST, 1);
    n.append(c);
    n.append(c1);
    QCOMPARE(n.type(), KDbField::InvalidType); // N-arg expression is abstract, unspecified type
    QVERIFY(n.validate(&parseInfo));
    testCloneExpression(n);
    qDebug() << c << c1 << n;
}

void TestExpressions::testBinaryExpressionValidate_data()
{
    QTest::addColumn<int>("type1");
    QTest::addColumn<QVariant>("const1");
    QTest::addColumn<int>("token");
    QTest::addColumn<int>("type2");
    QTest::addColumn<QVariant>("const2");
    QTest::addColumn<KDbField::Type>("type3");

    KDbQuerySchema *query = 0;
    ParseInfoInternal parseInfo(query);

    // invalid
    KDbConstExpression c(INTEGER_CONST, 7);
    KDbBinaryExpression b(c, '+', KDbExpression());
    QCOMPARE(b.type(), KDbField::InvalidType);
    QVERIFY(!b.validate(&parseInfo)); // unknown class
    testCloneExpression(b);
    qDebug() << b;

    b = KDbBinaryExpression(KDbExpression(), '/', KDbExpression());
    QCOMPARE(b.type(), KDbField::InvalidType);
    QVERIFY(!b.validate(&parseInfo)); // unknown class
    testCloneExpression(b);
    qDebug() << b;

    // invalid left or right
    KDbBinaryExpression b2(b, '*', c.clone());
    QCOMPARE(b2.type(), KDbField::InvalidType);
    QVERIFY(!b2.validate(&parseInfo)); // unknown class
    testCloneExpression(b2);
    qDebug() << b2;
    KDbBinaryExpression b3(c.clone(), '*', b);
    QCOMPARE(b3.type(), KDbField::InvalidType);
    QVERIFY(!b3.validate(&parseInfo)); // unknown class
    testCloneExpression(b3);
    qDebug() << b3;

#define T1(type1, const1, token, type2, const2, type3) \
        QTest::newRow( \
            QByteArray::number(__LINE__) + ": " + KDbExpression::tokenToDebugString(type1).toLatin1() + " " \
             + QVariant(const1).toString().toLatin1() + " " \
             + KDbExpression::tokenToDebugString(token).toLatin1() + " " \
             + KDbExpression::tokenToDebugString(type2).toLatin1() + " " \
             + QVariant(const2).toString().toLatin1()) \
            << int(type1) << QVariant(const1) \
            << int(token) << int(type2) << QVariant(const2) \
            << type3
// tests both f(x, y) and f(y, x)
#define T(type1, const1, token, type2, const2, type3) \
        T1(type1, const1, token, type2, const2, type3); \
        T1(type2, const2, token, type1, const1, type3)

    // null
    T(SQL_NULL, QVariant(), '+', INTEGER_CONST, 7, KDbField::Null);
    // NULL OR bool == bool
    T(SQL_NULL, QVariant(), OR, SQL_TRUE, true, KDbField::Boolean);
    T(SQL_NULL, QVariant(), OR, CHARACTER_STRING_LITERAL, "xyz", KDbField::InvalidType);
    // integer
    // -- KDb::ArithmeticExpression only: resulting type is Integer or more
    //    see explanation for KDb::maximumForIntegerTypes()
    T(INTEGER_CONST, 50, '+', INTEGER_CONST, 20, KDbField::Integer);
    T(INTEGER_CONST, 50, '-', INTEGER_CONST, 20, KDbField::Integer);
    T(INTEGER_CONST, 50, '*', INTEGER_CONST, 20, KDbField::Integer);
    T(INTEGER_CONST, 50, '/', INTEGER_CONST, 20, KDbField::Integer);
    T(INTEGER_CONST, 50, '&', INTEGER_CONST, 20, KDbField::Integer);
    T(INTEGER_CONST, 50, '|', INTEGER_CONST, 20, KDbField::Integer);
    T(INTEGER_CONST, 50, '%', INTEGER_CONST, 20, KDbField::Integer);
    T(INTEGER_CONST, 50, BITWISE_SHIFT_RIGHT, INTEGER_CONST, 20, KDbField::Integer);
    T(INTEGER_CONST, 50, BITWISE_SHIFT_LEFT, INTEGER_CONST, 20, KDbField::Integer);
    T(INTEGER_CONST, 300, '+', INTEGER_CONST, 20, KDbField::Integer);
    T(INTEGER_CONST, 300, '+', INTEGER_CONST, 300, KDbField::Integer);
    T(INTEGER_CONST, 300, '+', INTEGER_CONST, 300, KDbField::Integer);
    T(INTEGER_CONST, 50, '+', INTEGER_CONST, qulonglong(INT_MAX), KDbField::BigInteger);
    T(INTEGER_CONST, INT_MAX, '+', INTEGER_CONST, qulonglong(INT_MAX), KDbField::BigInteger);

    T(INTEGER_CONST, 50, '<', INTEGER_CONST, 20, KDbField::Boolean);
    T(INTEGER_CONST, 50, '=', INTEGER_CONST, 20, KDbField::Boolean);
    T(INTEGER_CONST, 50, '>', INTEGER_CONST, 20, KDbField::Boolean);
    T(INTEGER_CONST, 50, '<', INTEGER_CONST, INT_MAX, KDbField::Boolean);
    T(INTEGER_CONST, 50, '<', INTEGER_CONST, qulonglong(INT_MAX), KDbField::Boolean);
    T(INTEGER_CONST, qulonglong(INT_MAX), '<', INTEGER_CONST, INT_MAX, KDbField::Boolean);
    T(INTEGER_CONST, 300, LESS_OR_EQUAL, INTEGER_CONST, 20, KDbField::Boolean);
    T(INTEGER_CONST, 300, GREATER_OR_EQUAL, INTEGER_CONST, 300, KDbField::Boolean);
    T(INTEGER_CONST, 300, '>', INTEGER_CONST, 300, KDbField::Boolean);

    T(INTEGER_CONST, 300, OR, INTEGER_CONST, 20, KDbField::InvalidType);
    T(INTEGER_CONST, 300, AND, INTEGER_CONST, 20, KDbField::InvalidType);
    T(INTEGER_CONST, 300, XOR, INTEGER_CONST, 20, KDbField::InvalidType);
    T(INTEGER_CONST, 300, OR, SQL_NULL, QVariant(), KDbField::InvalidType);
    // real
    T(REAL_CONST, 0.5, '+', REAL_CONST, -9.4, KDbField::Double);
    T(REAL_CONST, 0.5, '-', REAL_CONST, -9.4, KDbField::Double);
    T(REAL_CONST, 0.5, '*', REAL_CONST, -9.4, KDbField::Double);
    T(REAL_CONST, 0.5, '/', REAL_CONST, -9.4, KDbField::Double);
    T(REAL_CONST, 0.5, '&', REAL_CONST, -9.4, KDbField::Integer);
    T(REAL_CONST, 0.5, '&', INTEGER_CONST, 9, KDbField::Byte);
    T(REAL_CONST, 0.5, '&', INTEGER_CONST, 1000, KDbField::ShortInteger);
    T(REAL_CONST, 0.5, '&', INTEGER_CONST, qulonglong(INT_MAX), KDbField::BigInteger);
    T(REAL_CONST, 0.5, '%', REAL_CONST, -9.4, KDbField::Double);
    T(REAL_CONST, 0.5, BITWISE_SHIFT_RIGHT, REAL_CONST, 9.4, KDbField::Integer);
    T(REAL_CONST, 0.5, BITWISE_SHIFT_LEFT, REAL_CONST, 9.4, KDbField::Integer);
    T(REAL_CONST, 0.5, '+', INTEGER_CONST, 300, KDbField::Double);
    T(REAL_CONST, 0.5, '-', INTEGER_CONST, 300, KDbField::Double);
    T(REAL_CONST, 0.5, '/', INTEGER_CONST, 300, KDbField::Double);
    T(REAL_CONST, 0.5, '-', SQL_NULL, QVariant(), KDbField::Null);

    T(REAL_CONST, 0.5, '>', REAL_CONST, -9.4, KDbField::Boolean);
    T(REAL_CONST, 0.5, '>', INTEGER_CONST, 300, KDbField::Boolean);
    T(REAL_CONST, 0.5, '=', INTEGER_CONST, 300, KDbField::Boolean);
    T(REAL_CONST, 0.5, '<', INTEGER_CONST, qulonglong(INT_MAX), KDbField::Boolean);
    T(REAL_CONST, 0.5, LESS_OR_EQUAL, INTEGER_CONST, 300, KDbField::Boolean);
    T(REAL_CONST, 0.5, GREATER_OR_EQUAL, INTEGER_CONST, 300, KDbField::Boolean);
    T(REAL_CONST, 0.5, '>', SQL_NULL, QVariant(), KDbField::Null);

    T(REAL_CONST, 30.2, OR, REAL_CONST, 20, KDbField::InvalidType);
    T(REAL_CONST, 30.2, AND, REAL_CONST, 20, KDbField::InvalidType);
    T(REAL_CONST, 30.2, XOR, REAL_CONST, 20, KDbField::InvalidType);
    // string
    T(CHARACTER_STRING_LITERAL, "ab", CONCATENATION, CHARACTER_STRING_LITERAL, "cd", KDbField::Text);
    T(SQL_NULL, QVariant(), CONCATENATION, CHARACTER_STRING_LITERAL, "cd", KDbField::Null);
    T(INTEGER_CONST, 50, CONCATENATION, INTEGER_CONST, 20, KDbField::InvalidType);
    T(CHARACTER_STRING_LITERAL, "ab", CONCATENATION, INTEGER_CONST, 20, KDbField::InvalidType);
    T(CHARACTER_STRING_LITERAL, "ab", GREATER_OR_EQUAL, CHARACTER_STRING_LITERAL, "cd", KDbField::Boolean);
    T(CHARACTER_STRING_LITERAL, "ab", '<', INTEGER_CONST, 3, KDbField::InvalidType);
    T(CHARACTER_STRING_LITERAL, "ab", '+', CHARACTER_STRING_LITERAL, "cd", KDbField::InvalidType);
    T(CHARACTER_STRING_LITERAL, "A", OR, REAL_CONST, 20, KDbField::InvalidType);
    T(CHARACTER_STRING_LITERAL, "A", AND, REAL_CONST, 20, KDbField::InvalidType);
    T(CHARACTER_STRING_LITERAL, "A", XOR, REAL_CONST, 20, KDbField::InvalidType);
    // bool
    T(SQL_TRUE, true, '<', SQL_FALSE, false, KDbField::Boolean);
    T(SQL_TRUE, true, '=', SQL_FALSE, false, KDbField::Boolean);
    T(SQL_TRUE, true, '+', SQL_FALSE, false, KDbField::InvalidType);
    T(SQL_TRUE, true, '<', INTEGER_CONST, 20, KDbField::Boolean);
    T(SQL_TRUE, true, '<', REAL_CONST, -10.1, KDbField::Boolean);
    T(SQL_TRUE, true, '-', SQL_NULL, QVariant(), KDbField::Null);
    T(SQL_TRUE, true, '<', SQL_NULL, QVariant(), KDbField::Null);
    T(SQL_TRUE, true, OR, SQL_FALSE, false, KDbField::Boolean);
    T(SQL_TRUE, true, AND, SQL_FALSE, false, KDbField::Boolean);
    T(SQL_TRUE, true, XOR, SQL_FALSE, false, KDbField::Boolean);
    // date/time
    T(DATE_CONST, QDate(2001, 1, 2), '=', DATE_CONST, QDate(2002, 1, 2), KDbField::Boolean);
    T(DATETIME_CONST, QDateTime(QDate(2001, 1, 2), QTime(1, 2, 3)), LESS_OR_EQUAL, DATE_CONST, QDateTime::currentDateTime(), KDbField::Boolean);
    T(TIME_CONST, QTime(1, 2, 3), '<', DATE_CONST, QTime::currentTime(), KDbField::Boolean);
    T(DATE_CONST, QDate(2001, 1, 2), '=', INTEGER_CONST, 17, KDbField::InvalidType);
    T(DATE_CONST, QDate(2001, 1, 2), '=', SQL_NULL, QVariant(), KDbField::Null);
    T(DATE_CONST, QDate(2001, 1, 2), OR, SQL_FALSE, false, KDbField::InvalidType);
    T(DATE_CONST, QDate(2001, 1, 2), AND, SQL_FALSE, false, KDbField::InvalidType);
    T(DATE_CONST, QDate(2001, 1, 2), XOR, SQL_FALSE, false, KDbField::InvalidType);
#undef T
#undef T1
}

void TestExpressions::testBinaryExpressionValidate()
{
    QFETCH(int, type1);
    QFETCH(QVariant, const1);
    QFETCH(int, token);
    QFETCH(int, type2);
    QFETCH(QVariant, const2);
    QFETCH(KDbField::Type, type3);

    KDbQuerySchema *query = 0;
    ParseInfoInternal parseInfo(query);

    KDbConstExpression c(type1, const1);
    KDbConstExpression c1(type2, const2);
    KDbBinaryExpression b(c, token, c1);
    qDebug() << b.type();
    qDebug() << type3;
    QCOMPARE(b.type(), type3);
    QVERIFY(b.validate(&parseInfo) == (type3 != KDbField::InvalidType));
    testCloneExpression(b);
}

void TestExpressions::cleanupTestCase()
{
}

QTEST_MAIN(TestExpressions)

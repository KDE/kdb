/* This file is part of the KDE project
   Copyright (C) 2011-2015 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_EXPRESSIONSTEST_H
#define KDB_EXPRESSIONSTEST_H

#include <QObject>
#include <KDbUtils>

class ExpressionsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testNullExpression();
    void testExpressionClassName_data();
    void testExpressionClassName();
    void testExpressionToken();
    void testNArgExpression();
    void testUnaryExpression();
    void testBinaryExpression();
    void testBinaryExpressionCloning_data();
    void testBinaryExpressionCloning();
    void testFunctionExpression();

    void testConstExpressionValidate();
    void testUnaryExpressionValidate();
    void testNArgExpressionValidate();
    void testBinaryExpressionValidate_data();
    void testBinaryExpressionValidate();
    void testFunctionExpressionValidate();

    void cleanupTestCase();
};

#endif

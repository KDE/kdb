/* This file is part of the KDE project
   Copyright (C) 2018 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDBORDERBYCOLUMNTEST_H
#define KDBORDERBYCOLUMNTEST_H

#include "KDbTestUtils.h"

class OrderByColumnTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    //! Test ORDER BY data for "SELECT 'foo'" query
    void testSelect1Query();

    //! Test ORDER BY data for "SELECT * FROM cars ORDER BY 2"
    void testOrderByIndex();

    //! Test ORDER BY data for "SELECT * FROM cars ORDER BY model"
    void testOrderByColumnName();

    void cleanupTestCase();

private:
    KDbTestUtils utils;
};

#endif

/* This file is part of the KDE project
   Copyright (C) 2015 Jarosław Staniek <staniek@kde.org>

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

#include "KDbTestUtils.h"

#include <KDbDriverManager>
#include <KDbDriverMetaData>
#include <KDbConnection>

#include <QFile>
#include <QTest>

void KDbTestUtils::testDriverManager()
{
    QStringList names = manager.driverIds();
    qDebug() << "DRIVERS:" << names;
    QVERIFY2(!manager.result().isError(), "Error in driver manager");
    qDebug() << manager.result().message();
    QVERIFY2(!names.isEmpty(), "No db drivers found");
}

void KDbTestUtils::testSqliteDriver()
{
    QString drv_id = "org.kde.kdb.sqlite";
    const KDbDriverMetaData* driverMetaData = manager.driverMetaData(drv_id);
    QVERIFY2(driverMetaData, "Driver metadata not found");
    QCOMPARE(driverMetaData->id(), drv_id);
    QVERIFY(driverMetaData->isFileBased());
    driver = manager.driver(drv_id);
    QVERIFY2(driver, "Error in driver manager after KDbDriverManager::driver() call");
    QVERIFY2(!manager.result().isError(), "Error in driver manager after KDbDriverManager::driver() call");
}

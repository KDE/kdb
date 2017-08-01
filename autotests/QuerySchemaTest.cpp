/* This file is part of the KDE project
   Copyright (C) 2017 Jarosław Staniek <staniek@kde.org>

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

#include "QuerySchemaTest.h"

#include <KDb>
#include <KDbConnectionData>
#include <KDbQueryAsterisk>
#include <KDbQuerySchema>
#include <KDbVersionInfo>

#include <QRegularExpression>
#include <QTest>

QTEST_GUILESS_MAIN(QuerySchemaTest)

void QuerySchemaTest::initTestCase()
{
}

void QuerySchemaTest::testCaching()
{
    QVERIFY(utils.testCreateDbWithTables("QuerySchemaTest"));
    KDbQuerySchema query;
    KDbTableSchema *carsTable = utils.connection->tableSchema("cars");
    QVERIFY(carsTable);
    query.addTable(carsTable);
    KDbField *idField = carsTable->field("id");
    QVERIFY(idField);
    // "SELECT id, cars.* from cars"
    query.addField(idField);
    query.addAsterisk(new KDbQueryAsterisk(&query, *carsTable));
    QCOMPARE(query.fieldCount(), 2);
    const KDbQueryColumnInfo::Vector expandedAll1 = query.fieldsExpanded(utils.connection.data());
    QCOMPARE(expandedAll1.count(), 4);
    const KDbQueryColumnInfo::Vector expandedUnique1
        = query.fieldsExpanded(utils.connection.data(), KDbQuerySchema::FieldsExpandedMode::Unique);
    QCOMPARE(expandedUnique1.count(), 3);
    // remove the asterisk -> "SELECT id from cars"
    query.removeField(query.field(1));
    QCOMPARE(query.fieldCount(), 1);
    const KDbQueryColumnInfo::Vector expandedAll2 = query.fieldsExpanded(utils.connection.data());
    QCOMPARE(expandedAll2.count(), 1);
    const KDbQueryColumnInfo::Vector expandedUnique2
        = query.fieldsExpanded(utils.connection.data(), KDbQuerySchema::FieldsExpandedMode::Unique);
    QCOMPARE(expandedUnique2.count(), 1);
}

void QuerySchemaTest::cleanupTestCase()
{
}

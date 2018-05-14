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

#include "OrderByColumnTest.h"

#include <KDbExpression>
#include <KDbOrderByColumn>
#include <KDbQueryAsterisk>
#include <KDbQuerySchema>
#include <KDbNativeStatementBuilder>

#include <QTest>

QTEST_GUILESS_MAIN(OrderByColumnTest)

void OrderByColumnTest::initTestCase()
{
}

void OrderByColumnTest::testSelect1Query()
{
    QVERIFY(utils.testCreateDbWithTables("OrderByColumnTest"));
    KDbQuerySchema query;
    KDbField *oneField = new KDbField;
    oneField->setExpression(KDbConstExpression(KDbToken::CHARACTER_STRING_LITERAL, "foo"));
    query.addField(oneField);
    KDbOrderByColumnList* orderBy = query.orderByColumnList();
    QVERIFY(orderBy);
    QVERIFY(orderBy->isEmpty());
    QCOMPARE(orderBy->count(), 0);
    orderBy->appendField(oneField);
    KDbConnection *conn = utils.connection.data();

    // automatic alias "expr1"
    KDbEscapedString sql;
    QVERIFY(utils.kdbBuilder()->generateSelectStatement(&sql, &query));
    QCOMPARE(sql, "SELECT 'foo' AS expr1 ORDER BY expr1");
    QVERIFY(!orderBy->isEmpty());
    QCOMPARE(orderBy->count(), 1);
    const int indexOfField = query.indexOf(*oneField);
    QCOMPARE(indexOfField, 0);
    const QString alias(query.columnAlias(indexOfField));
    QVERIFY(!alias.isEmpty());
    KDbOrderByColumn *orderByColumn = orderBy->value(indexOfField);
    QVERIFY(orderByColumn);
    QVERIFY(!orderByColumn->column());
    QCOMPARE(orderByColumn->field(), oneField);
    QVERIFY(!orderBy->value(orderBy->count() + 10));
    KDbEscapedString orderBySqlOldApi = orderBy->toSqlString(true, conn, KDb::KDbEscaping);
    QCOMPARE(orderBySqlOldApi, ""); // alias is not used
    KDbEscapedString orderBySql = orderBy->toSqlString(true, conn, &query, KDb::KDbEscaping);
    QCOMPARE(orderBySql, alias); // alias is used to point to the column "'foo'"

    // change alias to something other than valid ID
    QVERIFY(query.setColumnAlias(indexOfField, "0"));
    QVERIFY(utils.kdbBuilder()->generateSelectStatement(&sql, &query));
    QCOMPARE(sql, "SELECT 'foo' AS \"0\" ORDER BY \"0\"");
    orderBySqlOldApi = orderBy->toSqlString(true, conn, KDb::KDbEscaping);
    QCOMPARE(orderBySqlOldApi, ""); // alias is not used
    orderBySql = orderBy->toSqlString(true, conn, &query, KDb::KDbEscaping);
    QCOMPARE(orderBySql, "\"0\""); // alias is used to point to the column "'foo'"
}

void OrderByColumnTest::testOrderByIndex()
{
    QVERIFY(utils.testCreateDbWithTables("OrderByColumnTest"));
    KDbQuerySchema query;
    KDbTableSchema *carsTable = utils.connection->tableSchema("cars");
    QVERIFY(carsTable);
    query.addTable(carsTable);
    query.addAsterisk(new KDbQueryAsterisk(&query));
    KDbOrderByColumnList* orderBy = query.orderByColumnList();
    KDbConnection *conn = utils.connection.data();

    // "SELECT * FROM cars ORDER BY model ASC, owner DESC"
    QVERIFY(query.orderByColumnList()->isEmpty());
    QVERIFY(orderBy->appendColumn(conn, &query,
                                  KDbOrderByColumn::SortOrder::Ascending, 2));
    QVERIFY(orderBy->appendColumn(conn, &query,
                                  KDbOrderByColumn::SortOrder::Descending, 1));
    KDbEscapedString sql;
    QVERIFY(utils.kdbBuilder()->generateSelectStatement(&sql, &query));
    QCOMPARE(sql, "SELECT cars.* FROM cars ORDER BY 3, 2 DESC");

    QVERIFY2(!orderBy->appendColumn(conn, &query,
                                    KDbOrderByColumn::SortOrder::Ascending, 3),
             "appendField for null");
}

void OrderByColumnTest::testOrderByColumnName()
{
    QVERIFY(utils.testCreateDbWithTables("OrderByColumnTest"));
    KDbQuerySchema query;
    KDbTableSchema *carsTable = utils.connection->tableSchema("cars");
    QVERIFY(carsTable);
    query.addTable(carsTable);
    query.addAsterisk(new KDbQueryAsterisk(&query));

    // "SELECT * FROM cars ORDER BY model, owner"
    QVERIFY(query.orderByColumnList());
    QVERIFY(query.orderByColumnList()->isEmpty());
    QCOMPARE(query.orderByColumnList()->count(), 0);

    KDbOrderByColumnList* orderBy = query.orderByColumnList();
    QVERIFY(orderBy);
    QVERIFY(orderBy->isEmpty());
    KDbField *modelField = carsTable->field("model");
    QVERIFY(modelField);
    KDbField *ownerField = carsTable->field("owner");
    QVERIFY(ownerField);
    orderBy->appendField(modelField);
    orderBy->appendField(ownerField);
    KDbConnection *conn = utils.connection.data();
    KDbEscapedString orderBySql = orderBy->toSqlString(true, conn, &query, KDb::KDbEscaping);
    QCOMPARE(orderBySql, "cars.model, cars.owner");

    KDbEscapedString sql;
    QVERIFY(utils.kdbBuilder()->generateSelectStatement(&sql, &query));
    QCOMPARE(sql, "SELECT cars.* FROM cars ORDER BY model, owner");
    QVERIFY(utils.driverBuilder()->generateSelectStatement(&sql, &query));
    QCOMPARE(sql, "SELECT [cars].* FROM [cars] ORDER BY [model] COLLATE '', [owner]");

    // "SELECT * FROM cars ORDER BY model ASC, owner DESC"
    orderBy->clear();
    QVERIFY(query.orderByColumnList()->isEmpty());
    orderBy->appendField(modelField, KDbOrderByColumn::SortOrder::Ascending);
    orderBy->appendField(ownerField, KDbOrderByColumn::SortOrder::Descending);
    QVERIFY(utils.kdbBuilder()->generateSelectStatement(&sql, &query));
    const char validSelect1[] = "SELECT cars.* FROM cars ORDER BY model, owner DESC";
    QCOMPARE(sql, validSelect1);
    QVERIFY(utils.driverBuilder()->generateSelectStatement(&sql, &query));
    const char validDriverSelect1[] = "SELECT [cars].* FROM [cars] ORDER BY [model] COLLATE '', [owner] DESC";
    QCOMPARE(sql, validDriverSelect1);

    // The same query, adding null field
    orderBy->clear();
    QVERIFY(query.orderByColumnList()->isEmpty());
    orderBy->appendField(nullptr);
    QVERIFY2(query.orderByColumnList()->isEmpty(), "Adding null fields should not affect OREDR BY");
    orderBy->appendField(modelField, KDbOrderByColumn::SortOrder::Ascending);
    orderBy->appendField(ownerField, KDbOrderByColumn::SortOrder::Descending);
    orderBy->appendField(nullptr);
    QVERIFY(utils.kdbBuilder()->generateSelectStatement(&sql, &query));
    QCOMPARE(sql, validSelect1);
    QVERIFY(utils.driverBuilder()->generateSelectStatement(&sql, &query));
    QCOMPARE(sql, validDriverSelect1);

    // The same query, overload
    orderBy->clear();
    QVERIFY(query.orderByColumnList()->isEmpty());
    QVERIFY(orderBy->appendFields(conn, &query,
                                  "model", KDbOrderByColumn::SortOrder::Ascending,
                                  "owner", KDbOrderByColumn::SortOrder::Descending));
    QVERIFY(utils.kdbBuilder()->generateSelectStatement(&sql, &query));
    QCOMPARE(sql, validSelect1);
    QVERIFY(utils.driverBuilder()->generateSelectStatement(&sql, &query));
    QCOMPARE(sql, validDriverSelect1);

    // The same query, overload
    orderBy->clear();
    QVERIFY(query.orderByColumnList()->isEmpty());
    QVERIFY(orderBy->appendField(conn, &query, "model", KDbOrderByColumn::SortOrder::Ascending));
    QVERIFY(orderBy->appendField(conn, &query, "owner", KDbOrderByColumn::SortOrder::Descending));
    QVERIFY(utils.kdbBuilder()->generateSelectStatement(&sql, &query));
    QCOMPARE(sql, validSelect1);
    QVERIFY(utils.driverBuilder()->generateSelectStatement(&sql, &query));
    QCOMPARE(sql, validDriverSelect1);

    QCOMPARE(orderBy->count(), 2);
    QVERIFY2(!orderBy->appendField(conn, &query, ""), "appendField for null");
    QCOMPARE(orderBy->count(), 2);

    // The same query, overload
    orderBy->clear();
    QCOMPARE(orderBy->count(), 0);
    QVERIFY(query.orderByColumnList()->isEmpty());
    KDbQueryColumnInfo::Vector columns = query.fieldsExpanded(conn);
    KDbQueryColumnInfo *ownerColumnInfo = columns.value(1);
    QVERIFY(ownerColumnInfo);
    KDbQueryColumnInfo *modelColumnInfo = columns.value(2);
    QVERIFY(modelColumnInfo);
    orderBy->appendColumn(modelColumnInfo, KDbOrderByColumn::SortOrder::Ascending);
    orderBy->appendColumn(ownerColumnInfo, KDbOrderByColumn::SortOrder::Descending);
    QCOMPARE(orderBy->count(), 2);
    QVERIFY(utils.kdbBuilder()->generateSelectStatement(&sql, &query));
    QCOMPARE(sql, validSelect1);
    QVERIFY(utils.driverBuilder()->generateSelectStatement(&sql, &query));
    QCOMPARE(sql, validDriverSelect1);
}

//! @todo Test KDbQuerySchema::setOrderByColumnList
//! @todo Test more KDbOrderByColumnList and KDbOrderByColumn

void OrderByColumnTest::cleanupTestCase()
{
}

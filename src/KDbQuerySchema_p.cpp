/* This file is part of the KDE project
   Copyright (C) 2003-2016 Jarosław Staniek <staniek@kde.org>

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

#include "KDbQuerySchema_p.h"
#include "KDbOrderByColumn.h"

KDbQuerySchema::Private::Private(KDbQuerySchema* q, Private* copy)
        : query(q)
        , masterTable(nullptr)
        , fakeRecordIdField(nullptr)
        , fakeRecordIdCol(nullptr)
        , conn(nullptr)
        , maxIndexWithAlias(-1)
        , visibility(64)
        , fieldsExpanded(nullptr)
        , visibleFieldsExpanded(nullptr)
        , internalFields(nullptr)
        , fieldsExpandedWithInternalAndRecordId(nullptr)
        , visibleFieldsExpandedWithInternalAndRecordId(nullptr)
        , fieldsExpandedWithInternal(nullptr)
        , visibleFieldsExpandedWithInternal(nullptr)
        , orderByColumnList(nullptr)
        , autoincFields(nullptr)
        , columnsOrder(nullptr)
        , columnsOrderWithoutAsterisks(nullptr)
        , columnsOrderExpanded(nullptr)
        , pkeyFieldsOrder(nullptr)
        , pkeyFieldCount(0)
        , tablesBoundToColumns(64, -1) // will be resized if needed
        , ownedVisibleColumns(nullptr)
        , regenerateExprAliases(false)
{
    visibility.fill(false);
    if (copy) {
        // deep copy
        *this = *copy;
        // <clear, so computeFieldsExpanded() will re-create it>
        fieldsExpanded = nullptr;
        visibleFieldsExpanded = nullptr;
        internalFields = nullptr;
        columnsOrder = nullptr;
        columnsOrderWithoutAsterisks = nullptr;
        columnsOrderExpanded = nullptr;
        orderByColumnList = nullptr;
        autoincFields = nullptr;
        autoIncrementSqlFieldsList.clear();
        columnInfosByNameExpanded.clear();
        columnInfosByName.clear();
        ownedVisibleColumns = nullptr;
        fieldsExpandedWithInternalAndRecordId = nullptr;
        visibleFieldsExpandedWithInternalAndRecordId = nullptr;
        fieldsExpandedWithInternal = nullptr;
        visibleFieldsExpandedWithInternal = nullptr;
        pkeyFieldsOrder = nullptr;
        fakeRecordIdCol = nullptr;
        fakeRecordIdField = nullptr;
        conn = nullptr;
        ownedVisibleColumns = nullptr;
        // </clear, so computeFieldsExpanded() will re-create it>
        if (!copy->whereExpr.isNull()) {
            whereExpr = copy->whereExpr.clone();
        }
        // "*this = *copy" causes copying pointers; pull of them without destroying,
        // will be deep-copied in the KDbQuerySchema ctor.
        asterisks.setAutoDelete(false);
        asterisks.clear();
        asterisks.setAutoDelete(true);
    }
    else {
        orderByColumnList = new KDbOrderByColumnList;
    }
}

KDbQuerySchema::Private::~Private()
{
    delete orderByColumnList;
    delete autoincFields;
    delete columnsOrder;
    delete columnsOrderWithoutAsterisks;
    delete columnsOrderExpanded;
    delete pkeyFieldsOrder;
    delete fakeRecordIdCol;
    delete fakeRecordIdField;
    delete ownedVisibleColumns;
    if (fieldsExpanded) {
        qDeleteAll(*fieldsExpanded);
        delete fieldsExpanded;
    }
    if (internalFields) {
        qDeleteAll(*internalFields);
        delete internalFields;
    }
    delete fieldsExpandedWithInternalAndRecordId;
    delete visibleFieldsExpandedWithInternalAndRecordId;
    delete fieldsExpandedWithInternal;
    delete visibleFieldsExpandedWithInternal;
}

//static
KDbQuerySchema* KDbQuerySchema::Private::createQuery(KDbConnection *conn)
{
    return new KDbQuerySchema(conn);
}

void KDbQuerySchema::Private::clear()
{
    columnAliases.clear();
    tableAliases.clear();
    asterisks.clear();
    relations.clear();
    masterTable = nullptr;
    tables.clear();
    clearCachedData();
    delete pkeyFieldsOrder;
    pkeyFieldsOrder = nullptr;
    visibility.fill(false);
    tablesBoundToColumns = QVector<int>(64, -1); // will be resized if needed
    tablePositionsForAliases.clear();
    columnPositionsForAliases.clear();
}

void KDbQuerySchema::Private::clearCachedData()
{
    if (orderByColumnList) {
        orderByColumnList->clear();
    }
    if (fieldsExpanded) {
        delete columnsOrder;
        columnsOrder = nullptr;
        delete columnsOrderWithoutAsterisks;
        columnsOrderWithoutAsterisks = nullptr;
        delete columnsOrderExpanded;
        columnsOrderExpanded = nullptr;
        delete autoincFields;
        autoincFields = nullptr;
        autoIncrementSqlFieldsList.clear();
        columnInfosByNameExpanded.clear();
        columnInfosByName.clear();
        delete ownedVisibleColumns;
        ownedVisibleColumns = nullptr;
        qDeleteAll(*fieldsExpanded);
        delete fieldsExpanded;
        fieldsExpanded = nullptr;
        delete visibleFieldsExpanded; // NO qDeleteAll, items not owned
        visibleFieldsExpanded = nullptr;
        if (internalFields) {
            qDeleteAll(*internalFields);
            delete internalFields;
            internalFields = nullptr;
        }
        delete fieldsExpandedWithInternalAndRecordId;
        fieldsExpandedWithInternalAndRecordId = nullptr;
        delete visibleFieldsExpandedWithInternalAndRecordId;
        visibleFieldsExpandedWithInternalAndRecordId = nullptr;
        delete fieldsExpandedWithInternal;
        fieldsExpandedWithInternal = nullptr;
        delete visibleFieldsExpandedWithInternal;
        visibleFieldsExpandedWithInternal = nullptr;
    }
}

void KDbQuerySchema::Private::setColumnAlias(int position, const QString& alias)
{
    if (alias.isEmpty()) {
        columnAliases.remove(position);
        maxIndexWithAlias = -1;
    } else {
        setColumnAliasInternal(position, alias);
    }
}

void KDbQuerySchema::Private::tryRegenerateExprAliases()
{
    if (!regenerateExprAliases)
        return;
    //regenerate missing aliases for experessions
    int colNum = 0; //used to generate a name
    QString columnAlias;
    int p = -1;
    foreach(KDbField* f, *query->fields()) {
        p++;
        if (f->isExpression() && columnAliases.value(p).isEmpty()) {
            //missing
            do { //find 1st unused
                colNum++;
                columnAlias = tr("expr%1", "short for 'expression' word, it will expand "
                                           "to 'expr1', 'expr2', etc. Please use ONLY latin "
                                           "letters and DON'T use '.'").arg(colNum);
                columnAlias = KDb::stringToIdentifier(columnAlias); // sanity fix, translators make mistakes!
            } while (-1 != tablePositionForAlias(columnAlias));

            setColumnAliasInternal(p, columnAlias);
        }
    }
    regenerateExprAliases = false;
}

void KDbQuerySchema::Private::setColumnAliasInternal(int position, const QString& alias)
{
    columnAliases.insert(position, alias.toLower());
    columnPositionsForAliases.insert(alias.toLower(), position);
    maxIndexWithAlias = qMax(maxIndexWithAlias, position);
}

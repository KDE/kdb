/* This file is part of the KDE project
   Copyright (C) 2003-2014 Jarosław Staniek <staniek@kde.org>

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

KDbQuerySchema::Private::Private(KDbQuerySchema* q, Private* copy)
        : query(q)
        , masterTable(0)
        , fakeRecordIdField(0)
        , fakeRecordIdCol(0)
        , conn(0)
        , maxIndexWithAlias(-1)
        , visibility(64)
        , fieldsExpanded(0)
        , internalFields(0)
        , fieldsExpandedWithInternalAndRecordId(0)
        , fieldsExpandedWithInternal(0)
        , orderByColumnList(0)
        , autoincFields(0)
        , columnsOrder(0)
        , columnsOrderWithoutAsterisks(0)
        , columnsOrderExpanded(0)
        , pkeyFieldsOrder(0)
        , pkeyFieldCount(0)
        , tablesBoundToColumns(64, -1) // will be resized if needed
        , ownedVisibleColumns(0)
        , regenerateExprAliases(false)
{
    visibility.fill(false);
    if (copy) {
        // deep copy
        *this = *copy;
        // <clear, so computeFieldsExpanded() will re-create it>
        fieldsExpanded = 0;
        internalFields = 0;
        columnsOrder = 0;
        columnsOrderWithoutAsterisks = 0;
        columnsOrderExpanded = 0;
        orderByColumnList = 0;
        autoincFields = 0;
        autoIncrementSQLFieldsList.clear();
        columnInfosByNameExpanded.clear();
        columnInfosByName.clear();
        ownedVisibleColumns = 0;
        fieldsExpandedWithInternalAndRecordId = 0;
        fieldsExpandedWithInternal = 0;
        pkeyFieldsOrder = 0;
        fakeRecordIdCol = 0;
        fakeRecordIdField = 0;
        conn = 0;
        ownedVisibleColumns = 0;
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
    delete fieldsExpandedWithInternal;
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
    masterTable = 0;
    tables.clear();
    clearCachedData();
    delete pkeyFieldsOrder;
    pkeyFieldsOrder = 0;
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
        columnsOrder = 0;
        delete columnsOrderWithoutAsterisks;
        columnsOrderWithoutAsterisks = 0;
        delete columnsOrderExpanded;
        columnsOrderExpanded = 0;
        delete autoincFields;
        autoincFields = 0;
        autoIncrementSQLFieldsList.clear();
        columnInfosByNameExpanded.clear();
        columnInfosByName.clear();
        delete ownedVisibleColumns;
        ownedVisibleColumns = 0;
        qDeleteAll(*fieldsExpanded);
        delete fieldsExpanded;
        fieldsExpanded = 0;
        if (internalFields) {
            qDeleteAll(*internalFields);
            delete internalFields;
            internalFields = 0;
        }
    }
}

void KDbQuerySchema::Private::setColumnAlias(uint position, const QString& alias)
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
    uint colNum = 0; //used to generate a name
    QString columnAlias;
    uint p = -1;
    foreach(KDbField* f, *query->fields()) {
        p++;
        if (f->isExpression() && columnAliases.value(p).isEmpty()) {
            //missing
            do { //find 1st unused
                colNum++;
                columnAlias = QObject::tr("expr", "short for 'expression' word, e.g. 'expr' (only latin letters, please, no '.')")
                              + QString::number(colNum);
            } while (-1 != tablePositionForAlias(columnAlias));

            setColumnAliasInternal(p, columnAlias);
        }
    }
    regenerateExprAliases = false;
}

void KDbQuerySchema::Private::setColumnAliasInternal(uint position, const QString& alias)
{
    columnAliases.insert(position, alias.toLower());
    columnPositionsForAliases.insert(alias.toLower(), position);
    maxIndexWithAlias = qMax(maxIndexWithAlias, (int)position);
}

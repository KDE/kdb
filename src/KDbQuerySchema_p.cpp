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
#include "KDbConnection.h"
#include "KDbConnection_p.h"
#include "KDbOrderByColumn.h"
#include "kdb_debug.h"

KDbQuerySchemaPrivate::KDbQuerySchemaPrivate(KDbQuerySchema* q, KDbQuerySchemaPrivate* copy)
        : query(q)
        , masterTable(nullptr)
        , fakeRecordIdField(nullptr)
        , fakeRecordIdCol(nullptr)
        , maxIndexWithAlias(-1)
        , visibility(64)
        , orderByColumnList(nullptr)
        , autoincFields(nullptr)
        , pkeyFieldsOrder(nullptr)
        , pkeyFieldCount(0)
        , tablesBoundToColumns(64, -1) // will be resized if needed
        , regenerateExprAliases(false)
{
    visibility.fill(false);
    if (copy) {
        // deep copy
        *this = *copy;
        // <clear, so computeFieldsExpanded() will re-create it>
        orderByColumnList = nullptr;
        autoincFields = nullptr;
        autoIncrementSqlFieldsList.clear();
        pkeyFieldsOrder = nullptr;
        fakeRecordIdCol = nullptr;
        fakeRecordIdField = nullptr;
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

KDbQuerySchemaPrivate::~KDbQuerySchemaPrivate()
{
    if (recentConnection) {
        recentConnection->d->insertFieldsExpanded(query, nullptr);
    }
    delete orderByColumnList;
    delete autoincFields;
    delete pkeyFieldsOrder;
    delete fakeRecordIdCol;
    delete fakeRecordIdField;
}

void KDbQuerySchemaPrivate::clear()
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

void KDbQuerySchemaPrivate::clearCachedData()
{
    if (orderByColumnList) {
        orderByColumnList->clear();
    }
    if (recentConnection) {
        recentConnection->d->insertFieldsExpanded(query, nullptr);
    }
    delete autoincFields;
    autoincFields = nullptr;
    autoIncrementSqlFieldsList.clear();
}

bool KDbQuerySchemaPrivate::setColumnAlias(int position, const QString& alias)
{
    if (alias.isEmpty()) {
        columnAliases.remove(position);
        maxIndexWithAlias = -1;
        return true;
    }
    return setColumnAliasInternal(position, alias);
}

void KDbQuerySchemaPrivate::tryRegenerateExprAliases()
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

            (void)setColumnAliasInternal(p, columnAlias);
        }
    }
    regenerateExprAliases = false;
}

bool KDbQuerySchemaPrivate::setColumnAliasInternal(int position, const QString& alias)
{
    const int currentPos = columnPositionsForAliases.value(alias.toLower(), -1);
    if (currentPos == position) {
        return true; // already set
    }
    if (currentPos == -1) {
        columnAliases.insert(position, alias.toLower());
        columnPositionsForAliases.insert(alias.toLower(), position);
        maxIndexWithAlias = qMax(maxIndexWithAlias, position);
        return true;
    }
    kdbWarning() << "Alias" << alias << "for already set for column" << currentPos
                 << ", cannot set to a new column. Remove old alias first.";
    return false;
}

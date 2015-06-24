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

#ifndef KDB_QUERYSCHEMA_P_H
#define KDB_QUERYSCHEMA_P_H

#include "KDbQuerySchema.h"
#include "KDbDriver.h"
#include "KDbDriverManager_p.h"
#include "KDbConnection.h"
#include "KDbExpression.h"
#include "KDb.h"
#include "KDbLookupFieldSchema.h"

#include <QBitArray>
#include <QWeakPointer>

//! @internal
class KDbQuerySchema::Private
{
public:
    explicit Private(KDbQuerySchema* q, Private* copy = 0);

    ~Private();

    //! @return a new query that's associated with @a conn. Used internally, e.g. by the parser.
    //! Uses an internal KDbQuerySchema(KDbConnection*) ctor.
    static KDbQuerySchema* createQuery(KDbConnection *conn);

    void clear();

    void clearCachedData();

    void setColumnAlias(uint position, const QString& alias);

    inline void setTableAlias(uint position, const QString& alias) {
        tableAliases.insert(position, alias.toLower());
        tablePositionsForAliases.insert(alias.toLower(), position);
    }

    inline int columnAliasesCount() {
        tryRegenerateExprAliases();
        return columnAliases.count();
    }

    inline QString columnAlias(uint position) {
        tryRegenerateExprAliases();
        return columnAliases.value(position);
    }

    inline bool hasColumnAlias(uint position) {
        tryRegenerateExprAliases();
        return columnAliases.contains(position);
    }

    inline void removeTablePositionForAlias(const QString& alias) {
        tablePositionsForAliases.remove(alias.toLower());
    }

    inline int tablePositionForAlias(const QString& alias) const {
        return tablePositionsForAliases.value(alias.toLower(), -1);
    }

    inline int columnPositionForAlias(const QString& alias) const {
        return columnPositionsForAliases.value(alias.toLower(), -1);
    }

    KDbQuerySchema *query;

    /*! Master table of the query. (may be NULL)
      Any data modifications can be performed if we know master table.
      If null, query's records cannot be modified. */
    KDbTableSchema *masterTable;

    /*! List of tables used in this query */
    QList<KDbTableSchema*> tables;

    KDbField *fakeRecordIdField; //! used to mark a place for record Id
    KDbQueryColumnInfo *fakeRecordIdCol; //! used to mark a place for record Id

    //! Connection on which this query operates
    //! @todo use equivalent of QPointer<KDbConnection>
    KDbConnection* conn;

protected:
    void tryRegenerateExprAliases();

    void setColumnAliasInternal(uint position, const QString& alias);

    /*! Used to mapping columns to its aliases for this query */
    QHash<int, QString> columnAliases;

    /*! Collects table positions for aliases: used in tablePositionForAlias(). */
    QHash<QString, int> tablePositionsForAliases;

    /*! Collects column positions for aliases: used in columnPositionForAlias(). */
    QHash<QString, int> columnPositionsForAliases;

public:
    /*! Used to mapping tables to its aliases for this query */
    QHash<int, QString> tableAliases;

    /*! Helper used with aliases */
    int maxIndexWithAlias;

    /*! Helper used with tableAliases */
    int maxIndexWithTableAlias;

    /*! Used to store visibility flag for every field */
    QBitArray visibility;

    /*! List of asterisks defined for this query  */
    KDbField::List asterisks;

    /*! Temporary field vector for using in fieldsExpanded() */
    KDbQueryColumnInfo::Vector *fieldsExpanded;

    /*! Temporary field vector containing internal fields used for lookup columns. */
    KDbQueryColumnInfo::Vector *internalFields;

    /*! Temporary, used to cache sum of expanded fields and internal fields (+record Id) used for lookup columns.
     Contains not auto-deleted items.*/
    KDbQueryColumnInfo::Vector *fieldsExpandedWithInternalAndRecordId;

    /*! Temporary, used to cache sum of expanded fields and internal fields used for lookup columns.
     Contains not auto-deleted items.*/
    KDbQueryColumnInfo::Vector *fieldsExpandedWithInternal;

    /*! A list of fields for ORDER BY section. @see KDbQuerySchema::orderByColumnList(). */
    KDbOrderByColumnList* orderByColumnList;

    /*! A cache for autoIncrementFields(). */
    KDbQueryColumnInfo::List *autoincFields;

    /*! A cache for autoIncrementSQLFieldsList(). */
    KDbEscapedString autoIncrementSQLFieldsList;
    QWeakPointer<const KDbDriver> lastUsedDriverForAutoIncrementSQLFieldsList;

    /*! A hash for fast lookup of query columns' order (unexpanded version). */
    QHash<KDbQueryColumnInfo*, int> *columnsOrder;

    /*! A hash for fast lookup of query columns' order (unexpanded version without asterisks). */
    QHash<KDbQueryColumnInfo*, int> *columnsOrderWithoutAsterisks;

    /*! A hash for fast lookup of query columns' order.
     This is exactly opposite information compared to vector returned
     by fieldsExpanded() */
    QHash<KDbQueryColumnInfo*, int> *columnsOrderExpanded;

    /*! order of PKEY fields (e.g. for updateRecord() ) */
    QVector<int> *pkeyFieldsOrder;

    /*! number of PKEY fields within the query */
    uint pkeyFieldCount;

    /*! Forced (predefined) raw SQL statement */
    KDbEscapedString sql;

    /*! Relationships defined for this query. */
    QList<KDbRelationship*> relations;

    /*! Information about columns bound to tables.
     Used if table is used in FROM section more than once
     (using table aliases).

     This list is updated by insertField(uint position, KDbField *field,
     int bindToTable, bool visible), using bindToTable parameter.

     Example: for this statement:
     SELECT t1.a, othertable.x, t2.b FROM table t1, table t2, othertable;
     tablesBoundToColumns list looks like this:
     [ 0, -1, 1 ]
     - first column is bound to table 0 "t1"
     - second coulmn is not specially bound (othertable.x isn't ambiguous)
     - third column is bound to table 1 "t2"
    */
    QVector<int> tablesBoundToColumns;

    /*! WHERE expression */
    KDbExpression whereExpr;

    QHash<QString, KDbQueryColumnInfo*> columnInfosByNameExpanded;

    QHash<QString, KDbQueryColumnInfo*> columnInfosByName; //!< Same as columnInfosByNameExpanded but asterisks are skipped

    //! field schemas created for multiple joined columns like a||' '||b||' '||c
    KDbField::List *ownedVisibleColumns;

    /*! Set by insertField(): true, if aliases for expression columns should
     be generated on next columnAlias() call. */
    bool regenerateExprAliases;
};

#endif

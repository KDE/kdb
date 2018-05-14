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

#ifndef KDB_QUERYSCHEMA_P_H
#define KDB_QUERYSCHEMA_P_H

#include "KDbDriver.h"
#include "KDbExpression.h"
#include "KDbQueryColumnInfo.h"
#include "KDbQuerySchema.h"

#include <QBitArray>
#include <QWeakPointer>

class KDbConnection;

class Q_DECL_HIDDEN KDbQueryColumnInfo::Private
{
public:
    Private(KDbField *f, const QString& a, bool v, KDbQueryColumnInfo *foreign)
        : field(f)
        , alias(a)
        , visible(v)
        , indexForVisibleLookupValue(-1)
        , foreignColumn(foreign)
    {
    }

    KDbConnection *connection = nullptr; //!< Used to relate KDbQueryColumnInfo with query. @since 3.2
    const KDbQuerySchema *querySchema = nullptr; //!< Used to relate KDbQueryColumnInfo with query. @since 3.2
    KDbField *field;
    QString alias;

    //! @c true if this column is visible to the user (and its data is fetched by the engine)
    bool visible;

    /*! Index of column with visible lookup value within the 'fields expanded' vector.
     @see KDbQueryColumnInfo::indexForVisibleLookupValue() */
    int indexForVisibleLookupValue;

    //! Non-nullptr if this column is a visible column for @a foreignColumn
    KDbQueryColumnInfo *foreignColumn;
};

class KDbQuerySchemaPrivate
{
    Q_DECLARE_TR_FUNCTIONS(KDbQuerySchema)
public:
    explicit KDbQuerySchemaPrivate(KDbQuerySchema* q, KDbQuerySchemaPrivate* copy = nullptr);

    ~KDbQuerySchemaPrivate();

    //! @return a new query that's associated with @a conn. Used internally, e.g. by the parser.
    //! Uses an internal KDbQuerySchema(KDbConnection*) ctor.
    static KDbQuerySchema* createQuery(KDbConnection *conn);

    void clear();

    void clearCachedData();

    bool setColumnAlias(int position, const QString& alias);

    inline bool setTableAlias(int position, const QString& alias) {
        if (tablePositionsForAliases.contains(alias.toLower())) {
            return false;
        }
        tableAliases.insert(position, alias.toLower());
        tablePositionsForAliases.insert(alias.toLower(), position);
        return true;
    }

    inline int columnAliasesCount() {
        tryRegenerateExprAliases();
        return columnAliases.count();
    }

    inline QString columnAlias(int position) {
        tryRegenerateExprAliases();
        return columnAliases.value(position);
    }

    inline bool hasColumnAlias(int position) {
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

    //! Accessor for buildSelectQuery()
    static void setWhereExpressionInternal(KDbQuerySchema *query, const KDbExpression &expr)
    {
        query->d->whereExpr = expr;
    }

    KDbQuerySchema *query;

    /*! Master table of the query. Can be @c nullptr.
      Any data modifications can be performed if we know master table.
      If null, query's records cannot be modified. */
    KDbTableSchema *masterTable;

    /*! List of tables used in this query */
    QList<KDbTableSchema*> tables;

    KDbField *fakeRecordIdField; //! used to mark a place for record Id
    KDbQueryColumnInfo *fakeRecordIdCol; //! used to mark a place for record Id

protected:
    void tryRegenerateExprAliases();

    bool setColumnAliasInternal(int position, const QString& alias);

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

    /*! Used to store visibility flag for every field */
    QBitArray visibility;

    /*! List of asterisks defined for this query  */
    KDbField::List asterisks;

    /*! A list of fields for ORDER BY section. @see KDbQuerySchema::orderByColumnList(). */
    KDbOrderByColumnList* orderByColumnList;

    /*! A cache for autoIncrementFields(). */
    KDbQueryColumnInfo::List *autoincFields;

    /*! A cache for autoIncrementSqlFieldsList(). */
    KDbEscapedString autoIncrementSqlFieldsList;
    QWeakPointer<const KDbDriver> lastUsedDriverForAutoIncrementSQLFieldsList;

    /*! order of PKEY fields (e.g. for updateRecord() ) */
    QVector<int> *pkeyFieldsOrder;

    /*! number of PKEY fields within the query */
    int pkeyFieldCount;

    /*! Forced (predefined) raw SQL statement */
    KDbEscapedString sql;

    /*! Relationships defined for this query. */
    QList<KDbRelationship*> relations;

    /*! Information about columns bound to tables.
     Used if table is used in FROM section more than once
     (using table aliases).

     This list is updated by insertField(int position, KDbField *field,
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

    /*! Set by insertField(): true, if aliases for expression columns should
     be generated on next columnAlias() call. */
    bool regenerateExprAliases;

    //! Points to connection recently used for caching
    //! @todo use equivalent of QPointer<KDbConnection>
    KDbConnection *recentConnection = nullptr;

    //! Owned fields created by KDbQuerySchema::addExpressionInternal()
    KDbField::List ownedExpressionFields;
};

//! Information about expanded fields for a single query schema, used for caching
class KDbQuerySchemaFieldsExpanded
{
public:
    inline KDbQuerySchemaFieldsExpanded()
    {
    }

    inline ~KDbQuerySchemaFieldsExpanded()
    {
        qDeleteAll(fieldsExpanded);
        qDeleteAll(internalFields);
    }

    /*! Temporary field vector for using in fieldsExpanded() */
    KDbQueryColumnInfo::Vector fieldsExpanded;

    /*! Like fieldsExpanded but only visible column infos; infos are not owned. */
    KDbQueryColumnInfo::Vector visibleFieldsExpanded;

    /*! Temporary field vector containing internal fields used for lookup columns. */
    KDbQueryColumnInfo::Vector internalFields;

    /*! Temporary, used to cache sum of expanded fields and internal fields (+record Id) used for lookup columns.
     Contains not auto-deleted items.*/
    KDbQueryColumnInfo::Vector fieldsExpandedWithInternalAndRecordId;

    /*! Like fieldsExpandedWithInternalAndRecordId but only contains visible column infos; infos are not owned.*/
    KDbQueryColumnInfo::Vector visibleFieldsExpandedWithInternalAndRecordId;

    /*! Temporary, used to cache sum of expanded fields and internal fields used for lookup columns.
     Contains not auto-deleted items.*/
    KDbQueryColumnInfo::Vector fieldsExpandedWithInternal;

    /*! Like fieldsExpandedWithInternal but only contains visible column infos; infos are not owned.*/
    KDbQueryColumnInfo::Vector visibleFieldsExpandedWithInternal;

    /*! A hash for fast lookup of query columns' order (unexpanded version). */
    QHash<KDbQueryColumnInfo*, int> columnsOrder;

    /*! A hash for fast lookup of query columns' order (unexpanded version without asterisks). */
    QHash<KDbQueryColumnInfo*, int> columnsOrderWithoutAsterisks;

    /*! A hash for fast lookup of query columns' order.
     This is exactly opposite information compared to vector returned
     by fieldsExpanded() */
    QHash<KDbQueryColumnInfo*, int> columnsOrderExpanded;

    QHash<QString, KDbQueryColumnInfo*> columnInfosByNameExpanded;

    QHash<QString, KDbQueryColumnInfo*> columnInfosByName; //!< Same as columnInfosByNameExpanded but asterisks are skipped

    //! Fields created for multiple joined columns like a||' '||b||' '||c
    KDbField::List ownedVisibleFields;
};

/**
 * Return identifier string @a name escaped using @a conn connection and type @a escapingType
 *
 * @a conn is only used for KDb::DriverEscaping type. If @a conn is missing for this type,
 * identifier is escaped using double quotes (").
 */
QString escapeIdentifier(const QString& name, KDbConnection *conn,
                         KDb::IdentifierEscapingType escapingType);

#endif

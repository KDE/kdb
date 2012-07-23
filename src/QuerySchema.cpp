/* This file is part of the KDE project
   Copyright (C) 2003-2012 Jarosław Staniek <staniek@kde.org>

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

#include "QuerySchema.h"
#include "Driver.h"
#include "DriverManager_p.h"
#include "Connection.h"
#include "Expression.h"
#include "parser/SqlParser.h"
#include "Utils.h"
#include "LookupFieldSchema.h"

#include <assert.h>

#include <QBitArray>
#include <QWeakPointer>
#include <QtDebug>


using namespace Predicate;

QueryColumnInfo::QueryColumnInfo(Field *f, const QString& _alias, bool _visible,
                                 QueryColumnInfo *foreignColumn)
        : field(f), alias(_alias), visible(_visible), m_indexForVisibleLookupValue(-1)
        , m_foreignColumn(foreignColumn)
{
}

QueryColumnInfo::~QueryColumnInfo()
{
}

QString QueryColumnInfo::aliasOrName() const
{
    return alias.isEmpty() ? field->name() : alias;
}

QDebug operator<<(QDebug dbg, const QueryColumnInfo& info)
{
    dbg.nospace()
        << (info.field->table() ? (info.field->table()->name() + QLatin1Char('.')) : QString())
        << *info.field
        << (info.alias.isEmpty() ? QString()
            : (QLatin1String(" AS ") + info.alias))
            + (info.visible ? QString() : QLatin1String(" [INVISIBLE]"));
    return dbg.space();
}

//=======================================
namespace Predicate
{
//! @internal
class QuerySchemaPrivate
{
public:
    QuerySchemaPrivate(QuerySchema* q, QuerySchemaPrivate* copy = 0)
            : query(q)
            , masterTable(0)
            , fakeRecordIdField(0)
            , fakeRecordIdCol(0)
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
            , pkeyFieldsCount(0)
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
            ownedVisibleColumns = 0;
            // </clear, so computeFieldsExpanded() will re-create it>
            if (!copy->whereExpr.isNull()) {
                whereExpr = copy->whereExpr.clone();
            }
            // "*this = *copy" causes copying pointers; pull of them without destroying,
            // will be deep-copied in the QuerySchema ctor.
            asterisks.setAutoDelete(false);
            asterisks.clear();
            asterisks.setAutoDelete(true);
        }
        else {
            orderByColumnList = new OrderByColumnList;
        }
    }
    ~QuerySchemaPrivate() {
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

    void clear() {
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

    void clearCachedData() {
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

    inline void setColumnAlias(uint position, const QString& alias) {
        if (alias.isEmpty()) {
            columnAliases.remove(position);
            maxIndexWithAlias = -1;
        } else {
            setColumnAliasInternal(position, alias);
        }
    }

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

    QuerySchema *query;

    /*! Master table of the query. (may be NULL)
      Any data modifications can be performed if we know master table.
      If null, query's records cannot be modified. */
    TableSchema *masterTable;

    /*! List of tables used in this query */
    TableSchema::List tables;

    Field *fakeRecordIdField; //! used to mark a place for record Id
    QueryColumnInfo *fakeRecordIdCol; //! used to mark a place for record Id

protected:
    void tryRegenerateExprAliases() {
        if (!regenerateExprAliases)
            return;
        //regenerate missing aliases for experessions
        uint colNum = 0; //used to generate a name
        QString columnAlias;
        uint p = -1;
        foreach(Field* f, *query->fields()) {
            p++;
            if (f->isExpression() && columnAliases.value(p).isEmpty()) {
                //missing
                do { //find 1st unused
                    colNum++;
                    columnAlias = QObject::tr("expr", "short for 'expression' word, e.g. 'expr' (only latin letters, please)")
                                  + QString::number(colNum);
                } while (-1 != tablePositionForAlias(columnAlias));

                setColumnAliasInternal(p, columnAlias);
            }
        }
        regenerateExprAliases = false;
    }

    void setColumnAliasInternal(uint position, const QString& alias) {
        columnAliases.insert(position, alias.toLower());
        columnPositionsForAliases.insert(alias.toLower(), position);
        maxIndexWithAlias = qMax(maxIndexWithAlias, (int)position);
    }

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
    Field::List asterisks;

    /*! Temporary field vector for using in fieldsExpanded() */
    QueryColumnInfo::Vector *fieldsExpanded;

    /*! Temporary field vector containing internal fields used for lookup columns. */
    QueryColumnInfo::Vector *internalFields;

    /*! Temporary, used to cache sum of expanded fields and internal fields (+record Id) used for lookup columns.
     Contains not auto-deleted items.*/
    QueryColumnInfo::Vector *fieldsExpandedWithInternalAndRecordId;

    /*! Temporary, used to cache sum of expanded fields and internal fields used for lookup columns.
     Contains not auto-deleted items.*/
    QueryColumnInfo::Vector *fieldsExpandedWithInternal;

    /*! A list of fields for ORDER BY section. @see QuerySchema::orderByColumnList(). */
    OrderByColumnList* orderByColumnList;

    /*! A cache for autoIncrementFields(). */
    QueryColumnInfo::List *autoincFields;

    /*! A cache for autoIncrementSQLFieldsList(). */
    EscapedString autoIncrementSQLFieldsList;
    QWeakPointer<const Driver> lastUsedDriverForAutoIncrementSQLFieldsList;

    /*! A hash for fast lookup of query columns' order (unexpanded version). */
    QHash<QueryColumnInfo*, int> *columnsOrder;

    /*! A hash for fast lookup of query columns' order (unexpanded version without asterisks). */
    QHash<QueryColumnInfo*, int> *columnsOrderWithoutAsterisks;

    /*! A hash for fast lookup of query columns' order.
     This is exactly opposite information compared to vector returned
     by fieldsExpanded() */
    QHash<QueryColumnInfo*, int> *columnsOrderExpanded;

//  QValueList<bool> detailedVisibility;

    /*! order of PKEY fields (e.g. for updateRecord() ) */
    QVector<int> *pkeyFieldsOrder;

    /*! number of PKEY fields within the query */
    uint pkeyFieldsCount;

    /*! forced (predefined) statement */
    EscapedString statement;

    /*! Relationships defined for this query. */
    Relationship::List relations;

    /*! Information about columns bound to tables.
     Used if table is used in FROM section more than once
     (using table aliases).

     This list is updated by insertField(uint position, Field *field,
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
    Expression whereExpr;

    QHash<QString, QueryColumnInfo*> columnInfosByNameExpanded;

    QHash<QString, QueryColumnInfo*> columnInfosByName; //!< Same as columnInfosByNameExpanded but asterisks are skipped

    //! field schemas created for multiple joined columns like a||' '||b||' '||c
    Field::List *ownedVisibleColumns;

    /*! Set by insertField(): true, if aliases for expression columns should
     be generated on next columnAlias() call. */
    bool regenerateExprAliases;
};
}

//=======================================

OrderByColumn::OrderByColumn()
        : m_column(0)
        , m_pos(-1)
        , m_field(0)
        , m_ascending(true)
{
}

OrderByColumn::OrderByColumn(QueryColumnInfo& column, bool ascending, int pos)
        : m_column(&column)
        , m_pos(pos)
        , m_field(0)
        , m_ascending(ascending)
{
}

OrderByColumn::OrderByColumn(Field& field, bool ascending)
        : m_column(0)
        , m_pos(-1)
        , m_field(&field)
        , m_ascending(ascending)
{
}

OrderByColumn* OrderByColumn::copy(QuerySchema* fromQuery, QuerySchema* toQuery) const
{
    if (m_field) {
        return new OrderByColumn(*m_field, m_ascending);
    }
    if (m_column) {
        QueryColumnInfo* columnInfo;
        if (fromQuery && toQuery) {
            int columnIndex = fromQuery->columnsOrder().value(m_column);
            if (columnIndex < 0) {
                PreDbg << "OrderByColumn::copy(): Index not found for column" << *m_column;
                return 0;
            }
            columnInfo = toQuery->expandedOrInternalField(columnIndex);
            if (!columnInfo) {
                PreDbg << "OrderByColumn::copy(): Column info not found at index"
                       << columnIndex << "in toQuery";
                return 0;
            }
        }
        else {
            columnInfo = m_column;
        }
        return new OrderByColumn(*columnInfo, m_ascending, m_pos);
    }
    Q_ASSERT(m_field || m_column);
    return 0;
}

OrderByColumn::~OrderByColumn()
{
}

QDebug operator<<(QDebug dbg, const OrderByColumn& order)
{
    const QLatin1String orderString(order.ascending() ? "ASCENDING" : "DESCENDING");
    if (order.column()) {
        if (order.position() > -1) {
            dbg.nospace() << QString::fromLatin1("COLUMN_AT_POSITION_%1(").arg(order.position() + 1);
            dbg.space() << *order.column() << ',';
            dbg.space() << orderString << ')';
            return dbg.space();
        }
        else {
            dbg.nospace() << "COLUMN(" << *order.column() << ',';
            dbg.space() << orderString << ')';
            return dbg.space();
        }
    }
    if (order.field()) {
        dbg.nospace() << "FIELD(" << *order.field() << ',';
        dbg.space() << orderString << ')';
        return dbg.space();
    }
    dbg.nospace() << "NONE";
    return dbg.space();
}

static QString escapeIdentifier(const QString& name, Connection *conn,
                                Predicate::EscapingType escapingType)
{
    switch (escapingType) {
    case DriverEscaping:
        if (conn)
            return conn->escapeIdentifier(name);
        break;
    case PredicateEscaping:
        return Predicate::escapeIdentifier(name);
    }
    return QLatin1Char('"') + name + QLatin1Char('"');
}

EscapedString OrderByColumn::toSQLString(bool includeTableName,
                                   Connection *conn, Predicate::EscapingType escapingType) const
{
    const QByteArray orderString(m_ascending ? "" : " DESC");
    EscapedString fieldName, tableName, collationString;
    if (m_column) {
        if (m_pos > -1)
            return EscapedString::number(m_pos + 1) + orderString;
        else {
            if (includeTableName && m_column->alias.isEmpty()) {
                tableName = EscapedString(escapeIdentifier(m_column->field->table()->name(), conn, escapingType));
                tableName += '.';
            }
            fieldName = EscapedString(escapeIdentifier(m_column->aliasOrName(), conn, escapingType));
        }
        if (m_column->field->isTextType()) {
            collationString = conn->driver()->collationSQL();
        }
    }
    else {
        if (m_field && includeTableName) {
            tableName = EscapedString(escapeIdentifier(m_field->table()->name(), conn, escapingType));
            tableName += '.';
        }
        fieldName = EscapedString(escapeIdentifier(
            m_field ? m_field->name() : QLatin1String("??")/*error*/, conn, escapingType));
        if (m_field && m_field->isTextType()) {
            collationString = conn->driver()->collationSQL();
        }
    }
    return tableName + fieldName + collationString + orderString;
}

//=======================================

OrderByColumnList::OrderByColumnList()
        : OrderByColumnListBase()
{
}

OrderByColumnList::OrderByColumnList(const OrderByColumnList& other,
                                     QuerySchema* fromQuery, QuerySchema* toQuery)
        : OrderByColumnListBase()
{
    for (QList<OrderByColumn*>::ConstIterator it(other.constBegin()); it != other.constEnd(); ++it) {
        OrderByColumn* order = (*it)->copy(fromQuery, toQuery);
        if (order) {
            append(order);
        }
    }
}

bool OrderByColumnList::appendFields(QuerySchema& querySchema,
                                     const QString& field1, bool ascending1,
                                     const QString& field2, bool ascending2,
                                     const QString& field3, bool ascending3,
                                     const QString& field4, bool ascending4,
                                     const QString& field5, bool ascending5)
{
    uint numAdded = 0;
#define ADD_COL(fieldName, ascending) \
    if (ok && !fieldName.isEmpty()) { \
        if (!appendField( querySchema, fieldName, ascending )) \
            ok = false; \
        else \
            numAdded++; \
    }
    bool ok = true;
    ADD_COL(field1, ascending1);
    ADD_COL(field2, ascending2);
    ADD_COL(field3, ascending3);
    ADD_COL(field4, ascending4);
    ADD_COL(field5, ascending5);
#undef ADD_COL
    if (ok)
        return true;
    for (uint i = 0; i < numAdded; i++)
        removeLast();
    return false;
}

OrderByColumnList::~OrderByColumnList()
{
    qDeleteAll(begin(), end());
}

void OrderByColumnList::appendColumn(QueryColumnInfo& columnInfo, bool ascending)
{
    append(new OrderByColumn(columnInfo, ascending));
}

bool OrderByColumnList::appendColumn(QuerySchema& querySchema, bool ascending, int pos)
{
    QueryColumnInfo::Vector fieldsExpanded(querySchema.fieldsExpanded());
    QueryColumnInfo* ci = (pos >= (int)fieldsExpanded.size()) ? 0 : fieldsExpanded[pos];
    if (!ci)
        return false;
    append(new OrderByColumn(*ci, ascending, pos));
    return true;
}

void OrderByColumnList::appendField(Field& field, bool ascending)
{
    append(new OrderByColumn(field, ascending));
}

bool OrderByColumnList::appendField(QuerySchema& querySchema,
                                    const QString& fieldName, bool ascending)
{
    QueryColumnInfo *columnInfo = querySchema.columnInfo(fieldName);
    if (columnInfo) {
        append(new OrderByColumn(*columnInfo, ascending));
        return true;
    }
    Field *field = querySchema.findTableField(fieldName);
    if (field) {
        append(new OrderByColumn(*field, ascending));
        return true;
    }
    PreWarn << "no such field" << fieldName;
    return false;
}

QDebug operator<<(QDebug dbg, const OrderByColumnList& list)
{
    if (list.isEmpty()) {
        dbg.nospace() << "NONE";
        return dbg.space();
    }
    bool first = true;
    for (QList<OrderByColumn*>::ConstIterator it(list.constBegin()); it != list.constEnd(); ++it) {
        if (first)
            first = false;
        else
            dbg.nospace() << '\n';
        dbg.nospace() << *(*it);
    }
    return dbg.space();
}

EscapedString OrderByColumnList::toSQLString(bool includeTableNames, Connection *conn,
                                       Predicate::EscapingType escapingType) const
{
    EscapedString string;
    for (QList<OrderByColumn*>::ConstIterator it(constBegin()); it != constEnd(); ++it) {
        if (!string.isEmpty())
            string += ", ";
        string += (*it)->toSQLString(includeTableNames, conn, escapingType);
    }
    return string;
}

void OrderByColumnList::clear()
{
    qDeleteAll(begin(), end());
    OrderByColumnListBase::clear();
}

//=======================================

QuerySchema::QuerySchema()
        : FieldList(false)//fields are not owned by QuerySchema object
        , Object(Predicate::QueryObjectType)
        , d(new QuerySchemaPrivate(this))
{
    init();
}

QuerySchema::QuerySchema(TableSchema& tableSchema)
        : FieldList(false)
        , Object(Predicate::QueryObjectType)
        , d(new QuerySchemaPrivate(this))
{
    d->masterTable = &tableSchema;
    init();
    /*if (!d->masterTable) {
      PreWarn << "!d->masterTable";
      m_name.clear();
      return;
    }*/
    addTable(d->masterTable);
    //defaults:
    //inherit name from a table
    setName(d->masterTable->name());
    //inherit caption from a table
    setCaption(d->masterTable->caption());

//replaced by explicit field list: //add all fields of the table as asterisk:
//replaced by explicit field list: addField( new QueryAsterisk(this) );

    // add explicit field list to avoid problems (e.g. with fields added outside of the app):
    foreach(Field* f, *d->masterTable->fields()) {
        addField(f);
    }
}

QuerySchema::QuerySchema(const QuerySchema& querySchema)
        : FieldList(querySchema, false /* !deepCopyFields */)
        , Object(querySchema)
        , d(new QuerySchemaPrivate(this, querySchema.d))
{
    //only deep copy query asterisks
    foreach(Field* f, *querySchema.fields()) {
        Field *copiedField;
        if (dynamic_cast<QueryAsterisk*>(f)) {
            copiedField = f->copy();
            if (static_cast<const Predicate::FieldList *>(f->m_parent) == &querySchema) {
                copiedField->m_parent = this;
            }
        }
        else {
            copiedField = f;
        }
        addField(copiedField);
    }
    // this deep copy must be after the 'd' initialization because fieldsExpanded() is used there
    d->orderByColumnList = new OrderByColumnList(*querySchema.d->orderByColumnList,
                                                 const_cast<QuerySchema*>(&querySchema), this);
}

QuerySchema::~QuerySchema()
{
    delete d;
}

void QuerySchema::init()
{
//m_fields_by_name.setAutoDelete( true ); //because we're using QueryColumnInfoEntry objects
}

void QuerySchema::clear()
{
    FieldList::clear();
    Object::clear();
    d->clear();
}

#warning TODO move visible to overload
FieldList& QuerySchema::insertField(uint position, Field *field, bool visible)
{
    return insertField(position, field, -1/*don't bind*/, visible);
}

/*virtual*/
FieldList& QuerySchema::insertField(uint position, Field *field)
{
    return insertField(position, field, -1/*don't bind*/, true);
}

#warning TODO move visible to overload
FieldList& QuerySchema::insertField(uint position, Field *field,
                                    int bindToTable, bool visible)
{
    if (!field) {
        PreWarn << "!field";
        return *this;
    }

    if (position > (uint)m_fields.count()) {
        PreWarn << "position" << position << "out of range";
        return *this;
    }
    if (!field->isQueryAsterisk() && !field->isExpression() && !field->table()) {
        PreWarn << "field" << field->name() << "must contain table information!";
        return *this;
    }
    if ((int)fieldCount() >= d->visibility.size()) {
        d->visibility.resize(d->visibility.size()*2);
        d->tablesBoundToColumns.resize(d->tablesBoundToColumns.size()*2);
    }
    d->clearCachedData();
    FieldList::insertField(position, field);
    if (field->isQueryAsterisk()) {
        d->asterisks.append(field);
        //if this is single-table asterisk,
        //add a table to list if doesn't exist there:
        if (field->table() && !d->tables.contains(field->table()))
            d->tables.append(field->table());
    } else if (field->table()) {
        //add a table to list if doesn't exist there:
        if (!d->tables.contains(field->table()))
            d->tables.append(field->table());
    }
// //visible by default
// setFieldVisible(field, true);
// d->visibility.setBit(fieldCount()-1, visible);
    //update visibility
    //--move bits to make a place for a new one
    for (uint i = fieldCount() - 1; i > position; i--)
        d->visibility.setBit(i, d->visibility.testBit(i - 1));
    d->visibility.setBit(position, visible);

    //bind to table
    if (bindToTable < -1 && bindToTable > (int)d->tables.count()) {
        PreWarn << "bindToTable" << bindToTable << "out of range";
        bindToTable = -1;
    }
    //--move items to make a place for a new one
    for (uint i = fieldCount() - 1; i > position; i--)
        d->tablesBoundToColumns[i] = d->tablesBoundToColumns[i-1];
    d->tablesBoundToColumns[position] = bindToTable;

    PreDbg << "bound to table" << bindToTable;
    if (bindToTable == -1)
        PreDbg << " <NOT SPECIFIED>";
    else
        PreDbg << " name=" << d->tables.at(bindToTable)->name()
        << " alias=" << tableAlias(bindToTable);
    QString s;
    for (uint i = 0; i < fieldCount();i++)
        s += (QString::number(d->tablesBoundToColumns[i]) + QLatin1Char(' '));
    PreDbg << "tablesBoundToColumns == [" << s << "]";

    if (field->isExpression())
        d->regenerateExprAliases = true;

    return *this;
}

int QuerySchema::tableBoundToColumn(uint columnPosition) const
{
    int res = d->tablesBoundToColumns.value(columnPosition, -99);
    if (res == -99) {
        PreWarn << "columnPosition" << columnPosition << "out of range";
        return -1;
    }
    return res;
}

#warning TODO move visible to overload
Predicate::FieldList& QuerySchema::addField(Predicate::Field* field, bool visible)
{
    return insertField(m_fields.count(), field, visible);
}

#warning TODO move visible to overload
Predicate::FieldList& QuerySchema::addField(Predicate::Field* field, int bindToTable,
        bool visible)
{
    return insertField(m_fields.count(), field, bindToTable, visible);
}

void QuerySchema::removeField(Predicate::Field *field)
{
    if (!field)
        return;
    d->clearCachedData();
    if (field->isQueryAsterisk()) {
        d->asterisks.removeAt(d->asterisks.indexOf(field));   //this will destroy this asterisk
    }
//! @todo should we also remove table for this field or asterisk?
    FieldList::removeField(field);
}

#warning TODO move visible to overload
FieldList& QuerySchema::addExpression(const Expression& expr, bool visible)
{
    return addField(new Field(this, expr), visible);
}

bool QuerySchema::isColumnVisible(uint position) const
{
    return (position < fieldCount()) ? d->visibility.testBit(position) : false;
}

void QuerySchema::setColumnVisible(uint position, bool v)
{
    if (position < fieldCount())
        d->visibility.setBit(position, v);
}

#warning TODO move visible to overload
FieldList& QuerySchema::addAsterisk(QueryAsterisk *asterisk, bool visible)
{
    if (!asterisk)
        return *this;
    //make unique name
    asterisk->setName((asterisk->table() ? (asterisk->table()->name() + QLatin1String(".*"))
                                         : QString::fromLatin1("*"))
                       + QString::number(asterisks()->count()));
    return addField(asterisk, visible);
}

Connection* QuerySchema::connection() const
{
    TableSchema *mt = masterTable();
    return mt ? mt->connection() : 0;
}

QDebug operator<<(QDebug dbg, const QuerySchema& query)
{
    //fields
    TableSchema *mt = query.masterTable();
    dbg.nospace() << "QUERY";
    dbg.space() << static_cast<const Object&>(query) << '\n';
    dbg.nospace() << " - MASTERTABLE=" << (mt ? mt->name() : QLatin1String("<NULL>"))
        << "\n - COLUMNS:\n";
    if (query.fieldCount() > 0)
        dbg.nospace() << static_cast<const FieldList&>(query) << '\n';
    else
        dbg.nospace() << "<NONE>\n";

    if (query.fieldCount() == 0)
        dbg.nospace() << " - NO FIELDS\n";
    else
        dbg.nospace() << " - FIELDS EXPANDED (";

    uint fieldsExpandedCount = 0;
    bool first;
    if (query.fieldCount() > 0) {
        const QueryColumnInfo::Vector fe(query.fieldsExpanded());
        fieldsExpandedCount = fe.size();
        dbg.nospace() << fieldsExpandedCount << "):\n";
        first = true;
        for (uint i = 0; i < fieldsExpandedCount; i++) {
            QueryColumnInfo *ci = fe[i];
            if (first)
                first = false;
            else
                dbg.nospace() << ",\n";
            dbg.nospace() << *ci;
        }
        dbg.nospace() << '\n';
    }

    //it's safer to delete fieldsExpanded for now
    // (debugString() could be called before all fields are added)
//causes a crash d->clearCachedData();

    //bindings
    dbg.nospace() << " - BINDINGS:\n";
    bool bindingsExist = false;
    for (uint i = 0; i < query.fieldCount(); i++) {
        const int tablePos = query.tableBoundToColumn(i);
        if (tablePos >= 0) {
            const QString tAlias(query.tableAlias(tablePos));
            if (!tAlias.isEmpty()) {
                bindingsExist = true;
                dbg.space() << "FIELD";
                dbg.space() << static_cast<const FieldList&>(query).field(i)->name();
                dbg.space() << "USES ALIAS";
                dbg.space() << tAlias;
                dbg.space() << "OF TABLE";
                dbg.space() << query.tables()->at(tablePos)->name() << '\n';
            }
        }
    }
    if (!bindingsExist) {
        dbg.nospace() << "<NONE>\n";
    }

    //tables
    dbg.nospace() << " - TABLES:\n";
    first = true;
    foreach(TableSchema *table, *query.tables()) {
        if (first)
            first = false;
        else
            dbg.nospace() << ",";
        dbg.space() << table->name();
    }
    if (query.tables()->isEmpty())
        dbg.nospace() << "<NONE>";

    //aliases
    dbg.nospace() << "\n - COLUMN ALIASES:\n";
    if (query.columnAliasesCount() == 0) {
        dbg.nospace() << "<NONE>\n";
    }
    else {
        int i = -1;
        foreach(Field *f, *query.fields()) {
            i++;
            const QString alias(query.columnAlias(i));
            if (!alias.isEmpty()) {
                dbg.nospace() << QString::fromLatin1("FIELD #%1:").arg(i);
                dbg.space() << (f->name().isEmpty()
                    ? QLatin1String("<NONAME>") : f->name()) << " -> " << alias << '\n';
            }
        }
    }

    dbg.nospace() << " - TABLE ALIASES:\n";
    if (query.tableAliasesCount() == 0) {
        dbg.nospace() << "<NONE>\n";
    }
    else {
        int i = -1;
        foreach(TableSchema* table, *query.tables()) {
            i++;
            const QString alias(query.tableAlias(i));
            if (!alias.isEmpty()) {
                dbg.nospace() << QString::fromLatin1("table #%1:").arg(i);
                dbg.space() << (table->name().isEmpty()
                    ? QLatin1String("<NONAME>") : table->name()) << " -> " << alias << '\n';
            }
        }
    }
    if (!query.whereExpression().isNull()) {
        dbg.nospace() << " - WHERE EXPRESSION:\n" << query.whereExpression() << '\n';
    }
    if (!query.orderByColumnList().isEmpty()) {
        dbg.space() << QString::fromLatin1(" - ORDER BY (%1):\n").arg(query.orderByColumnList().count());
        dbg.nospace() << query.orderByColumnList();
    }
    return dbg.nospace();
}

TableSchema* QuerySchema::masterTable() const
{
    if (d->masterTable)
        return d->masterTable;
    if (d->tables.isEmpty())
        return 0;

    //try to find master table if there's only one table (with possible aliasses)
    QString tableNameLower;
    int num = -1;
    foreach(TableSchema *table, d->tables) {
        num++;
        if (!tableNameLower.isEmpty() && table->name().toLower() != tableNameLower) {
            //two or more different tables
            return 0;
        }
        tableNameLower = tableAlias(num);
    }
    return d->tables.first();
}

void QuerySchema::setMasterTable(TableSchema *table)
{
    if (table)
        d->masterTable = table;
}

TableSchema::List* QuerySchema::tables() const
{
    return &d->tables;
}

void QuerySchema::addTable(TableSchema *table, const QString& alias)
{
    PreDbg << (void *)table << "alias=" << alias;
    if (!table)
        return;

    //only append table if:
    //-it has alias
    //-it has no alias but there is no such table on the list
    if (alias.isEmpty() && d->tables.contains(table)) {
        const QString tableNameLower(table->name().toLower());
        const QString aliasLower(alias.toLower());
        int num = -1;
        foreach(TableSchema *table, d->tables) {
            num++;
            if (table->name().toLower() == tableNameLower) {
                const QString tAlias = tableAlias(num);
                if (tAlias == aliasLower) {
                    PreWarn << "table with" << tAlias << "alias already added!";
                    return;
                }
            }
        }
    }

    d->tables.append(table);

    if (!alias.isEmpty())
        setTableAlias(d->tables.count() - 1, alias);
}

void QuerySchema::removeTable(TableSchema *table)
{
    if (!table)
        return;
    if (d->masterTable == table)
        d->masterTable = 0;
    d->tables.removeAt(d->tables.indexOf(table));
//! @todo remove fields!
}

TableSchema* QuerySchema::table(const QString& tableName) const
{
//! @todo maybe use tables_byname?
    foreach(TableSchema *table, d->tables) {
        if (table->name().toLower() == tableName.toLower())
            return table;
    }
    return 0;
}

bool QuerySchema::contains(TableSchema *table) const
{
    return d->tables.contains(table);
}

Field* QuerySchema::findTableField(const QString &tableOrTableAndFieldName) const
{
    QString tableName, fieldName;
    if (!Predicate::splitToTableAndFieldParts(tableOrTableAndFieldName,
                                           tableName, fieldName, Predicate::SetFieldNameIfNoTableName)) {
        return 0;
    }
    if (tableName.isEmpty()) {
        foreach(TableSchema *table, d->tables) {
            if (table->field(fieldName))
                return table->field(fieldName);
        }
        return 0;
    }
    TableSchema *tableSchema = table(tableName);
    if (!tableSchema)
        return 0;
    return tableSchema->field(fieldName);
}

int QuerySchema::columnAliasesCount() const
{
    return d->columnAliasesCount();
}

QString QuerySchema::columnAlias(uint position) const
{
    return d->columnAlias(position);
}

bool QuerySchema::hasColumnAlias(uint position) const
{
    return d->hasColumnAlias(position);
}

void QuerySchema::setColumnAlias(uint position, const QString& alias)
{
    if (position >= (uint)m_fields.count()) {
        PreWarn << "position"  << position << "out of range!";
        return;
    }
    const QString fixedAlias(alias.trimmed());
    Field *f = FieldList::field(position);
    if (f->captionOrName().isEmpty() && fixedAlias.isEmpty()) {
        PreWarn << "position" << position << "could not remove alias when no name is specified for expression column!";
        return;
    }
    d->setColumnAlias(position, fixedAlias);
}

int QuerySchema::tableAliasesCount() const
{
    return d->tableAliases.count();
}

QString QuerySchema::tableAlias(uint position) const
{
    return d->tableAliases.value(position);
}

int QuerySchema::tablePositionForAlias(const QString& name) const
{
    return d->tablePositionForAlias(name);
}

int QuerySchema::tablePosition(const QString& tableName) const
{
    int num = -1;
    QString tableNameLower(tableName.toLower());
    foreach(TableSchema* table, d->tables) {
        num++;
        if (table->name().toLower() == tableNameLower)
            return num;
    }
    return -1;
}

QList<int> QuerySchema::tablePositions(const QString& tableName) const
{
    QList<int> result;
    QString tableNameLower(tableName.toLower());
    int num = -1;
    foreach(TableSchema* table, d->tables) {
        num++;
        if (table->name().toLower() == tableNameLower) {
            result += num;
        }
    }
    return result;
}

bool QuerySchema::hasTableAlias(uint position) const
{
    return d->tableAliases.contains(position);
}

int QuerySchema::columnPositionForAlias(const QString& name) const
{
    return d->columnPositionForAlias(name);
}

void QuerySchema::setTableAlias(uint position, const QString& alias)
{
    if (position >= (uint)d->tables.count()) {
        PreWarn << "position"  << position << "out of range!";
        return;
    }
    const QString fixedAlias(alias.trimmed());
    if (fixedAlias.isEmpty()) {
        const QString oldAlias(d->tableAliases.take(position));
        if (!oldAlias.isEmpty()) {
            d->removeTablePositionForAlias(oldAlias);
        }
//   d->maxIndexWithTableAlias = -1;
    } else {
        d->setTableAlias(position, fixedAlias);
//  d->maxIndexWithTableAlias = qMax( d->maxIndexWithTableAlias, (int)index );
    }
}

Relationship::List* QuerySchema::relationships() const
{
    return &d->relations;
}

Field::List* QuerySchema::asterisks() const
{
    return &d->asterisks;
}

EscapedString QuerySchema::statement() const
{
    return d->statement;
}

void QuerySchema::setStatement(const EscapedString& statement)
{
    d->statement = statement;
}

Field* QuerySchema::field(const QString& name) const
{
    return field(name, true);
}

Field* QuerySchema::field(const QString& identifier, bool expanded) const
{
    QueryColumnInfo *ci = columnInfo(identifier, expanded);
    return ci ? ci->field : 0;
}

QueryColumnInfo* QuerySchema::columnInfo(const QString& identifier, bool expanded) const
{
    computeFieldsExpanded();
    return expanded ? d->columnInfosByNameExpanded.value(identifier)
            : d->columnInfosByName.value(identifier);
}

QueryColumnInfo::Vector QuerySchema::fieldsExpanded(FieldsExpandedOptions options) const
{
    computeFieldsExpanded();
    if (options == WithInternalFields || options == WithInternalFieldsAndRecordId) {
        //a ref to a proper pointer (as we cache the vector for two cases)
        QueryColumnInfo::Vector*& tmpFieldsExpandedWithInternal =
            (options == WithInternalFields) ? d->fieldsExpandedWithInternal : d->fieldsExpandedWithInternalAndRecordId;
        //special case
        if (!tmpFieldsExpandedWithInternal) {
            //glue expanded and internal fields and cache it
            const uint size = d->fieldsExpanded->count()
                              + (d->internalFields ? d->internalFields->count() : 0)
                              + ((options == WithInternalFieldsAndRecordId) ? 1 : 0) /*ROWID*/;
            tmpFieldsExpandedWithInternal = new QueryColumnInfo::Vector(size);
            const uint fieldsExpandedVectorSize = d->fieldsExpanded->size();
            for (uint i = 0; i < fieldsExpandedVectorSize; i++) {
                (*tmpFieldsExpandedWithInternal)[i] = d->fieldsExpanded->at(i);
            }
            const uint internalFieldsCount = d->internalFields ? d->internalFields->size() : 0;
            if (internalFieldsCount > 0) {
                for (uint i = 0; i < internalFieldsCount; i++) {
                    (*tmpFieldsExpandedWithInternal)[fieldsExpandedVectorSize + i] = d->internalFields->at(i);
                }
            }
            if (options == WithInternalFieldsAndRecordId) {
                if (!d->fakeRecordIdField) {
                    d->fakeRecordIdField = new Field(QLatin1String("rowID"), Field::BigInteger);
                    d->fakeRecordIdCol = new QueryColumnInfo(d->fakeRecordIdField, QString(), true);
                }
                (*tmpFieldsExpandedWithInternal)[fieldsExpandedVectorSize + internalFieldsCount] = d->fakeRecordIdCol;
            }
        }
        return *tmpFieldsExpandedWithInternal;
    }

    if (options == Default)
        return *d->fieldsExpanded;

    //options == Unique:
    QSet<QString> columnsAlreadyFound;
    const uint fieldsExpandedCount(d->fieldsExpanded->count());
    QueryColumnInfo::Vector result(fieldsExpandedCount);   //initial size is set
// QMapConstIterator<QueryColumnInfo*, bool> columnsAlreadyFoundIt;
    //compute unique list
    uint uniqueListCount = 0;
    for (uint i = 0; i < fieldsExpandedCount; i++) {
        QueryColumnInfo *ci = d->fieldsExpanded->at(i);
//  columnsAlreadyFoundIt = columnsAlreadyFound.find(ci);
//  uint foundColumnIndex = -1;
        if (!columnsAlreadyFound.contains(ci->aliasOrName())) {// columnsAlreadyFoundIt==columnsAlreadyFound.constEnd())
            columnsAlreadyFound.insert(ci->aliasOrName());
            result[uniqueListCount++] = ci;
        }
    }
    result.resize(uniqueListCount); //update result size
    return result;
}

QueryColumnInfo::Vector QuerySchema::internalFields() const
{
    computeFieldsExpanded();
    return d->internalFields ? *d->internalFields : QueryColumnInfo::Vector();
}

QueryColumnInfo* QuerySchema::expandedOrInternalField(uint index) const
{
    return fieldsExpanded(WithInternalFields).value(index);
}

inline QString lookupColumnKey(Field *foreignField, Field* field)
{
    QString res;
    if (field->table()) // can be 0 for anonymous fields built as joined multiple visible columns
        res = field->table()->name() + QLatin1Char('.');
    return res + field->name() + QLatin1Char('_') + foreignField->table()->name()
               + QLatin1Char('.') + foreignField->name();
}

void QuerySchema::computeFieldsExpanded() const
{
    if (d->fieldsExpanded)
        return;

    if (!d->columnsOrder) {
        d->columnsOrder = new QHash<QueryColumnInfo*, int>();
        d->columnsOrderWithoutAsterisks = new QHash<QueryColumnInfo*, int>();
    } else {
        d->columnsOrder->clear();
        d->columnsOrderWithoutAsterisks->clear();
    }
    if (d->ownedVisibleColumns)
        d->ownedVisibleColumns->clear();

    //collect all fields in a list (not a vector yet, because we do not know its size)
    QueryColumnInfo::List list; //temporary
    QueryColumnInfo::List lookup_list; //temporary, for collecting additional fields related to lookup fields
    QHash<QueryColumnInfo*, bool> columnInfosOutsideAsterisks; //helper for filling d->columnInfosByName
    int i = 0;
    uint numberOfColumnsWithMultipleVisibleFields = 0; //used to find an unique name for anonymous field
    int fieldPosition = -1;
    foreach(Field *f, m_fields) {
        fieldPosition++;
        if (f->isQueryAsterisk()) {
            if (static_cast<QueryAsterisk*>(f)->isSingleTableAsterisk()) {
                const Field::List *ast_fields = static_cast<QueryAsterisk*>(f)->table()->fields();
                foreach(Field *ast_f, *ast_fields) {
//     d->detailedVisibility += isFieldVisible(fieldPosition);
                    QueryColumnInfo *ci = new QueryColumnInfo(ast_f, QString()/*no field for asterisk!*/,
                            isColumnVisible(fieldPosition));
                    list.append(ci);
                    PreDbg << "caching (unexpanded) columns order:" << *ci << "at position" << fieldPosition;
                    d->columnsOrder->insert(ci, fieldPosition);
//     list.append(ast_f);
                }
            } else {//all-tables asterisk: iterate through table list
                foreach(TableSchema *table, d->tables) {
                    //add all fields from this table
                    const Field::List *tab_fields = table->fields();
                    foreach(Field *tab_f, *tab_fields) {
//! @todo (js): perhaps not all fields should be appended here
//      d->detailedVisibility += isFieldVisible(fieldPosition);
//      list.append(tab_f);
                        QueryColumnInfo *ci = new QueryColumnInfo(tab_f, QString()/*no field for asterisk!*/,
                                isColumnVisible(fieldPosition));
                        list.append(ci);
                        PreDbg << "caching (unexpanded) columns order:" << *ci << "at position" << fieldPosition;
                        d->columnsOrder->insert(ci, fieldPosition);
                    }
                }
            }
        } else {
            //a single field
//   d->detailedVisibility += isFieldVisible(fieldPosition);
            QueryColumnInfo *ci = new QueryColumnInfo(f, columnAlias(fieldPosition), isColumnVisible(fieldPosition));
            list.append(ci);
            columnInfosOutsideAsterisks.insert(ci, true);
            PreDbg << "caching (unexpanded) column's order:" << *ci << "at position" << fieldPosition;
            d->columnsOrder->insert(ci, fieldPosition);
            d->columnsOrderWithoutAsterisks->insert(ci, fieldPosition);

            //handle lookup field schema
            LookupFieldSchema *lookupFieldSchema = f->table() ? f->table()->lookupFieldSchema(*f) : 0;
            if (!lookupFieldSchema || lookupFieldSchema->boundColumn() < 0)
                continue;
            // Lookup field schema found:
            // Now we also need to fetch "visible" value from the lookup table, not only the value of binding.
            // -> build LEFT OUTER JOIN clause for this purpose (LEFT, not INNER because the binding can be broken)
            // "LEFT OUTER JOIN lookupTable ON thisTable.thisField=lookupTable.boundField"
            LookupFieldSchema::RecordSource recordSource = lookupFieldSchema->recordSource();
            if (recordSource.type() == LookupFieldSchema::RecordSource::Table) {
                TableSchema *lookupTable = connection()->tableSchema(recordSource.name());
                FieldList* visibleColumns = 0;
                Field *boundField = 0;
                if (lookupTable
                        && (uint)lookupFieldSchema->boundColumn() < lookupTable->fieldCount()
                        && (visibleColumns = lookupTable->subList(lookupFieldSchema->visibleColumns()))
                        && (boundField = lookupTable->field(lookupFieldSchema->boundColumn()))) {
                    Field *visibleColumn = 0;
                    // for single visible column, just add it as-is
                    if (visibleColumns->fieldCount() == 1) {
                        visibleColumn = visibleColumns->fields()->first();
                    } else {
                        // for multiple visible columns, build an expression column
                        // (the expression object will be owned by column info)
                        visibleColumn = new Field();
                        visibleColumn->setName(
                            QString::fromLatin1("[multiple_visible_fields_%1]")
                            .arg(++numberOfColumnsWithMultipleVisibleFields));
                        visibleColumn->setExpression(
                            ConstExpression(CHARACTER_STRING_LITERAL, QVariant()/*not important*/));
                        if (!d->ownedVisibleColumns) {
                            d->ownedVisibleColumns = new Field::List();
//Qt 4       d->ownedVisibleColumns->setAutoDelete(true);
                        }
                        d->ownedVisibleColumns->append(visibleColumn);   // remember to delete later
                    }

                    lookup_list.append(
                        new QueryColumnInfo(visibleColumn, QString(), true/*visible*/, ci/*foreign*/));
                    /*
                              //add visibleField to the list of SELECTed fields if it is not yes present there
                              if (!findTableField( visibleField->table()->name()+"."+visibleField->name() )) {
                                if (!table( visibleField->table()->name() )) {
                                }
                                if (!sql.isEmpty())
                                  sql += QString::fromLatin1(", ");
                                sql += (escapeIdentifier(visibleField->table()->name(), drvEscaping) + "."
                                  + escapeIdentifier(visibleField->name(), drvEscaping));
                              }*/
                }
                delete visibleColumns;
            } else if (recordSource.type() == LookupFieldSchema::RecordSource::Query) {
                QuerySchema *lookupQuery = connection()->querySchema(recordSource.name());
                if (!lookupQuery)
                    continue;
                const QueryColumnInfo::Vector lookupQueryFieldsExpanded(lookupQuery->fieldsExpanded());
                if (lookupFieldSchema->boundColumn() >= lookupQueryFieldsExpanded.count())
                    continue;
                QueryColumnInfo *boundColumnInfo = 0;
                if (!(boundColumnInfo = lookupQueryFieldsExpanded.value(lookupFieldSchema->boundColumn())))
                    continue;
                Field *boundField = boundColumnInfo->field;
                if (!boundField)
                    continue;
                const QList<uint> visibleColumns(lookupFieldSchema->visibleColumns());
                bool ok = true;
                // all indices in visibleColumns should be in [0..lookupQueryFieldsExpanded.size()-1]
                foreach(uint visibleColumn, visibleColumns) {
                    if (visibleColumn >= (uint)lookupQueryFieldsExpanded.count()) {
                        ok = false;
                        break;
                    }
                }
                if (!ok)
                    continue;
                Field *visibleColumn = 0;
                // for single visible column, just add it as-is
                if (visibleColumns.count() == 1) {
                    visibleColumn = lookupQueryFieldsExpanded.value(visibleColumns.first())->field;
                } else {
                    // for multiple visible columns, build an expression column
                    // (the expression object will be owned by column info)
                    visibleColumn = new Field();
                    visibleColumn->setName(
                        QString::fromLatin1("[multiple_visible_fields_%1]")
                        .arg(++numberOfColumnsWithMultipleVisibleFields));
                    visibleColumn->setExpression(
                        ConstExpression(CHARACTER_STRING_LITERAL, QVariant()/*not important*/));
                    if (!d->ownedVisibleColumns) {
                        d->ownedVisibleColumns = new Field::List();
//Qt 4      d->ownedVisibleColumns->setAutoDelete(true);
                    }
                    d->ownedVisibleColumns->append(visibleColumn);   // remember to delete later
                }

                lookup_list.append(
                    new QueryColumnInfo(visibleColumn, QString(), true/*visible*/, ci/*foreign*/));
                /*
                        //add visibleField to the list of SELECTed fields if it is not yes present there
                        if (!findTableField( visibleField->table()->name()+"."+visibleField->name() )) {
                          if (!table( visibleField->table()->name() )) {
                          }
                          if (!sql.isEmpty())
                            sql += QString::fromLatin1(", ");
                          sql += (escapeIdentifier(visibleField->table()->name(), drvEscaping) + "."
                            + escapeIdentifier(visibleField->name(), drvEscaping));
                        }*/
            }
        }
    }
    //prepare clean vector for expanded list, and a map for order information
    if (!d->fieldsExpanded) {
        d->fieldsExpanded = new QueryColumnInfo::Vector(list.count());  // Field::Vector( list.count() );
//Qt 4  d->fieldsExpanded->setAutoDelete(true);
        d->columnsOrderExpanded = new QHash<QueryColumnInfo*, int>();
    } else {//for future:
        qDeleteAll(*d->fieldsExpanded);
        d->fieldsExpanded->clear();
        d->fieldsExpanded->resize(list.count());
        d->columnsOrderExpanded->clear();
    }

    /*fill (based on prepared 'list' and 'lookup_list'):
     -the vector
     -the map
     -"fields by name" dictionary
    */
    d->columnInfosByName.clear();
    d->columnInfosByNameExpanded.clear();
    i = -1;
    foreach(QueryColumnInfo* ci, list) {
        i++;
        (*d->fieldsExpanded)[i] = ci;
        d->columnsOrderExpanded->insert(ci, i);
        //remember field by name/alias/table.name if there's no such string yet in d->columnInfosByNameExpanded
        if (!ci->alias.isEmpty()) {
            //store alias and table.alias
            if (!d->columnInfosByNameExpanded.contains(ci->alias)) {
                d->columnInfosByNameExpanded.insert(ci->alias, ci);
            }
            QString tableAndAlias(ci->alias);
            if (ci->field->table())
                tableAndAlias.prepend(ci->field->table()->name() + QLatin1Char('.'));
            if (!d->columnInfosByNameExpanded.contains(tableAndAlias)) {
                d->columnInfosByNameExpanded.insert(tableAndAlias, ci);
            }
            //the same for "unexpanded" list
            if (columnInfosOutsideAsterisks.contains(ci)) {
                if (!d->columnInfosByName.contains(ci->alias)) {
                    d->columnInfosByName.insert(ci->alias, ci);
                }
                if (!d->columnInfosByName.contains(tableAndAlias)) {
                    d->columnInfosByName.insert(tableAndAlias, ci);
                }
            }
        } else {
            //no alias: store name and table.name
            if (!d->columnInfosByNameExpanded.contains(ci->field->name())) {
                d->columnInfosByNameExpanded.insert(ci->field->name(), ci);
            }
            QString tableAndName(ci->field->name());
            if (ci->field->table())
                tableAndName.prepend(ci->field->table()->name() + QLatin1Char('.'));
            if (!d->columnInfosByNameExpanded.contains(tableAndName)) {
                d->columnInfosByNameExpanded.insert(tableAndName, ci);
            }
            //the same for "unexpanded" list
            if (columnInfosOutsideAsterisks.contains(ci)) {
                if (!d->columnInfosByName.contains(ci->field->name())) {
                    d->columnInfosByName.insert(ci->field->name(), ci);
                }
                if (!d->columnInfosByName.contains(tableAndName)) {
                    d->columnInfosByName.insert(tableAndName, ci);
                }
            }
        }
    }

    //remove duplicates for lookup fields
    QHash<QString, uint> lookup_dict; //used to fight duplicates and to update QueryColumnInfo::indexForVisibleLookupValue()
    // (a mapping from table.name string to uint* lookupFieldIndex
    i = 0;
    for (QMutableListIterator<QueryColumnInfo*> it(lookup_list); it.hasNext();) {
        QueryColumnInfo* ci = it.next();
        const QString key(lookupColumnKey(ci->foreignColumn()->field, ci->field));
        if ( /* not needed   columnInfo( tableAndFieldName ) || */
            lookup_dict.contains(key)) {
            // this table.field is already fetched by this query
            it.remove();
            delete ci;
        } else {
            lookup_dict.insert(key, i);
            i++;
        }
    }

    //create internal expanded list with lookup fields
    if (d->internalFields) {
        qDeleteAll(*d->internalFields);
        d->internalFields->clear();
        d->internalFields->resize(lookup_list.count());
    }
    delete d->fieldsExpandedWithInternal; //clear cache
    delete d->fieldsExpandedWithInternalAndRecordId; //clear cache
    d->fieldsExpandedWithInternal = 0;
    d->fieldsExpandedWithInternalAndRecordId = 0;
    if (!lookup_list.isEmpty() && !d->internalFields) {//create on demand
        d->internalFields = new QueryColumnInfo::Vector(lookup_list.count());
//Qt 4  d->internalFields->setAutoDelete(true);
    }
    i = -1;
    foreach(QueryColumnInfo *ci, lookup_list) {
        i++;
        //add it to the internal list
        (*d->internalFields)[i] = ci;
        d->columnsOrderExpanded->insert(ci, list.count() + i);
    }

    //update QueryColumnInfo::indexForVisibleLookupValue() cache for columns
    numberOfColumnsWithMultipleVisibleFields = 0;
    for (i = 0; i < (int)d->fieldsExpanded->size(); i++) {
        QueryColumnInfo* ci = d->fieldsExpanded->at(i);
//! @todo QuerySchema itself will also support lookup fields...
        LookupFieldSchema *lookupFieldSchema
        = ci->field->table() ? ci->field->table()->lookupFieldSchema(*ci->field) : 0;
        if (!lookupFieldSchema || lookupFieldSchema->boundColumn() < 0)
            continue;
        const LookupFieldSchema::RecordSource recordSource = lookupFieldSchema->recordSource();
        if (recordSource.type() == LookupFieldSchema::RecordSource::Table) {
            TableSchema *lookupTable = connection()->tableSchema(recordSource.name());
            FieldList* visibleColumns = 0;
            if (lookupTable
                    && (uint)lookupFieldSchema->boundColumn() < lookupTable->fieldCount()
                    && (visibleColumns = lookupTable->subList(lookupFieldSchema->visibleColumns()))) {
                Field *visibleColumn = 0;
                // for single visible column, just add it as-is
                if (visibleColumns->fieldCount() == 1) {
                    visibleColumn = visibleColumns->fields()->first();
                    const QString key(lookupColumnKey(ci->field, visibleColumn));
                    int index = lookup_dict.value(key, -99);
                    if (index != -99)
                        ci->setIndexForVisibleLookupValue(d->fieldsExpanded->size() + index);
                } else {
                    const QString key(QString::fromLatin1("[multiple_visible_fields_%1]_%2.%3")
                                      .arg(++numberOfColumnsWithMultipleVisibleFields)
                                      .arg(ci->field->table()->name()).arg(ci->field->name()));
                    int index = lookup_dict.value(key, -99);
                    if (index != -99)
                        ci->setIndexForVisibleLookupValue(d->fieldsExpanded->size() + index);
                }
            }
            delete visibleColumns;
        } else if (recordSource.type() == LookupFieldSchema::RecordSource::Query) {
            QuerySchema *lookupQuery = connection()->querySchema(recordSource.name());
            if (!lookupQuery)
                continue;
            const QueryColumnInfo::Vector lookupQueryFieldsExpanded(lookupQuery->fieldsExpanded());
            if (lookupFieldSchema->boundColumn() >= lookupQueryFieldsExpanded.count())
                continue;
            QueryColumnInfo *boundColumnInfo = 0;
            if (!(boundColumnInfo = lookupQueryFieldsExpanded.value(lookupFieldSchema->boundColumn())))
                continue;
            Field *boundField = boundColumnInfo->field;
            if (!boundField)
                continue;
            const QList<uint> visibleColumns(lookupFieldSchema->visibleColumns());
            // for single visible column, just add it as-is
            if (visibleColumns.count() == 1) {
                if (lookupQueryFieldsExpanded.count() > (int)visibleColumns.first()) { // sanity check
                    Field *visibleColumn = lookupQueryFieldsExpanded.at(visibleColumns.first())->field;
                    const QString key(lookupColumnKey(ci->field, visibleColumn));
                    int index = lookup_dict.value(key, -99);
                    if (index != -99)
                        ci->setIndexForVisibleLookupValue(d->fieldsExpanded->size() + index);
                }
            } else {
                const QString key(QString::fromLatin1("[multiple_visible_fields_%1]_%2.%3")
                                  .arg(++numberOfColumnsWithMultipleVisibleFields)
                                  .arg(ci->field->table()->name()).arg(ci->field->name()));
                int index = lookup_dict.value(key, -99);
                if (index != -99)
                    ci->setIndexForVisibleLookupValue(d->fieldsExpanded->size() + index);
            }
        } else {
            PreWarn << "unsupported record source type" << recordSource.typeName();
        }
    }
}

QHash<QueryColumnInfo*, int> QuerySchema::columnsOrder(ColumnsOrderOptions options) const
{
    if (!d->columnsOrder)
        computeFieldsExpanded();
    if (options == UnexpandedList)
        return *d->columnsOrder;
    else if (options == UnexpandedListWithoutAsterisks)
        return *d->columnsOrderWithoutAsterisks;
    return *d->columnsOrderExpanded;
}

QVector<int> QuerySchema::pkeyFieldsOrder() const
{
    if (d->pkeyFieldsOrder)
        return *d->pkeyFieldsOrder;

    TableSchema *tbl = masterTable();
    if (!tbl || !tbl->primaryKey())
        return QVector<int>();

    //get order of PKEY fields (e.g. for records updating or inserting )
    IndexSchema *pkey = tbl->primaryKey();
    PreDbg << *pkey;
    d->pkeyFieldsOrder = new QVector<int>(pkey->fieldCount(), -1);

    const uint fCount = fieldsExpanded().count();
    d->pkeyFieldsCount = 0;
    for (uint i = 0; i < fCount; i++) {
        QueryColumnInfo *fi = d->fieldsExpanded->at(i);
        const int fieldIndex = fi->field->table() == tbl ? pkey->indexOf(*fi->field) : -1;
        if (fieldIndex != -1/* field found in PK */
                && d->pkeyFieldsOrder->at(fieldIndex) == -1 /* first time */) {
            PreDbg << "FIELD" << fi->field->name() << "IS IN PKEY AT POSITION #" << fieldIndex;
//   (*d->pkeyFieldsOrder)[j]=i;
            (*d->pkeyFieldsOrder)[fieldIndex] = i;
            d->pkeyFieldsCount++;
//   j++;
        }
    }
    PreDbg << d->pkeyFieldsCount
    << " OUT OF " << pkey->fieldCount() << " PKEY'S FIELDS FOUND IN QUERY " << name();
    return *d->pkeyFieldsOrder;
}

uint QuerySchema::pkeyFieldsCount()
{
    (void)pkeyFieldsOrder(); /* rebuild information */
    return d->pkeyFieldsCount;
}

Relationship* QuerySchema::addRelationship(Field *field1, Field *field2)
{
//@todo: find existing global db relationships
    Relationship *r = new Relationship(this, field1, field2);
    if (r->isEmpty()) {
        delete r;
        return 0;
    }

    d->relations.append(r);
    return r;
}

QueryColumnInfo::List* QuerySchema::autoIncrementFields() const
{
    if (!d->autoincFields) {
        d->autoincFields = new QueryColumnInfo::List();
    }
    TableSchema *mt = masterTable();
    if (!mt) {
        PreWarn << "no master table!";
        return d->autoincFields;
    }
    if (d->autoincFields->isEmpty()) {//no cache
        QueryColumnInfo::Vector fexp = fieldsExpanded();
        for (int i = 0; i < (int)fexp.count(); i++) {
            QueryColumnInfo *fi = fexp[i];
            if (fi->field->table() == mt && fi->field->isAutoIncrement()) {
                d->autoincFields->append(fi);
            }
        }
    }
    return d->autoincFields;
}

// static
EscapedString QuerySchema::sqlColumnsList(const QueryColumnInfo::List& infolist, Connection *conn,
                                    Predicate::EscapingType escapingType)
{
    EscapedString result;
    result.reserve(256);
    bool start = true;
    foreach(QueryColumnInfo* ci, infolist) {
        if (!start)
            result += ",";
        else
            start = false;
        result += escapeIdentifier(ci->field->name(), conn, escapingType);
    }
    return result;
}

EscapedString QuerySchema::autoIncrementSQLFieldsList(Connection *conn) const
{
    QWeakPointer<const Driver> driverWeakPointer
            = DriverManagerInternal::self()->driverWeakPointer(*conn->driver());
    if (   d->lastUsedDriverForAutoIncrementSQLFieldsList != driverWeakPointer
        || d->autoIncrementSQLFieldsList.isEmpty())
    {
        d->autoIncrementSQLFieldsList = QuerySchema::sqlColumnsList(*autoIncrementFields(), conn);
        d->lastUsedDriverForAutoIncrementSQLFieldsList = driverWeakPointer;
    }
    return d->autoIncrementSQLFieldsList;
}

void QuerySchema::setWhereExpression(const Expression& expr)
{
    d->whereExpr = expr.clone();
}

void QuerySchema::addToWhereExpression(Predicate::Field *field,
                                       const QVariant& value, int relation)
{
    int token;
    if (value.isNull())
        token = SQL_NULL;
    else if (field->isIntegerType()) {
        token = INTEGER_CONST;
    } else if (field->isFPNumericType()) {
        token = REAL_CONST;
    } else {
        token = CHARACTER_STRING_LITERAL;
//! @todo date, time
    }

    BinaryExpression newExpr(
        ConstExpression(token, value),
        relation,
        VariableExpression((field->table() ? (field->table()->name() + QLatin1Char('.')) : QString()) + field->name())
    );
    if (d->whereExpr.isNull()) {
        d->whereExpr = newExpr;
    }
    else {
        d->whereExpr = BinaryExpression(
            d->whereExpr,
            AND,
            newExpr
        );
    }
}

/*
void QuerySchema::addToWhereExpression(Predicate::Field *field, const QVariant& value)
    switch (value.type()) {
    case Int: case UInt: case Bool: case LongLong: case ULongLong:
      token = INTEGER_CONST;
      break;
    case Double:
      token = REAL_CONST;
      break;
    default:
      token = CHARACTER_STRING_LITERAL;
    }
//! @todo date, time

*/

Expression QuerySchema::whereExpression() const
{
    return d->whereExpr;
}

void QuerySchema::setOrderByColumnList(const OrderByColumnList& list)
{
    delete d->orderByColumnList;
    d->orderByColumnList = new OrderByColumnList(list, 0, 0);
// all field names should be found, exit otherwise ..........?
}

OrderByColumnList& QuerySchema::orderByColumnList() const
{
    return *d->orderByColumnList;
}

QuerySchemaParameterList QuerySchema::parameters() const
{
    if (whereExpression().isNull())
        return QuerySchemaParameterList();
    QuerySchemaParameterList params;
    whereExpression().getQueryParameters(params);
    return params;
}

/*
  new field1, Field *field2
  if (!field1 || !field2) {
    PreWarn << "!masterField || !detailsField";
    return;
  }
  if (field1->isQueryAsterisk() || field2->isQueryAsterisk()) {
    PreWarn << "relationship's fields cannot be asterisks";
    return;
  }
  if (!hasField(field1) && !hasField(field2)) {
    PreWarn << "fields do not belong to this query";
    return;
  }
  if (field1->table() == field2->table()) {
    PreWarn << "fields cannot belong to the same table";
    return;
  }
//@todo: check more things: -types
//@todo: find existing global db relationships

  Field *masterField = 0, *detailsField = 0;
  IndexSchema *masterIndex = 0, *detailsIndex = 0;
  if (field1->isPrimaryKey() && field2->isPrimaryKey()) {
    //2 primary keys
    masterField = field1;
    masterIndex = masterField->table()->primaryKey();
    detailsField = field2;
    detailsIndex = masterField->table()->primaryKey();
  }
  else if (field1->isPrimaryKey()) {
    masterField = field1;
    masterIndex = masterField->table()->primaryKey();
    detailsField = field2;
//@todo: check if it already exists
    detailsIndex = new IndexSchema(detailsField->table());
    detailsIndex->addField(detailsField);
    detailsIndex->setForeigKey(true);
  //  detailsField->setForeignKey(true);
  }
  else if (field2->isPrimaryKey()) {
    detailsField = field1;
    masterField = field2;
    masterIndex = masterField->table()->primaryKey();
//@todo
  }

  if (!masterIndex || !detailsIndex)
    return; //failed

  Relationship *rel = new Relationship(masterIndex, detailsIndex);

  d->relations.append( rel );
}*/

//---------------------------------------------------

QueryAsterisk::QueryAsterisk(QuerySchema *query, TableSchema *table)
        : Field()
        , m_table(table)
{
    assert(query);
    m_parent = query;
    setType(Field::Asterisk);
}

QueryAsterisk::QueryAsterisk(const QueryAsterisk &asterisk)
        : Field(asterisk)
        , m_table(asterisk.table())
{
}

QueryAsterisk::~QueryAsterisk()
{
}

Field* QueryAsterisk::copy() const
{
    return new QueryAsterisk(*this);
}

void QueryAsterisk::setTable(TableSchema *table)
{
    PreDbg;
    m_table = table;
}

QDebug operator<<(QDebug dbg, const QueryAsterisk& asterisk)
{
    if (asterisk.isAllTableAsterisk()) {
        dbg.nospace() << "ALL-TABLES ASTERISK (*) ON TABLES(";
        bool first = true;
        foreach(TableSchema *table, *asterisk.query()->tables()) {
            if (first)
                first = false;
            else
                dbg.nospace() << ',';
            dbg.space() << table->name();
        }
        dbg.space() << ')';
    } else {
        dbg.nospace() << "SINGLE-TABLE ASTERISK (" << asterisk.table()->name() << ".*)";
    }
    return dbg.space();
}

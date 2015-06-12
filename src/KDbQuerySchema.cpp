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

#include "KDbQuerySchema.h"
#include "KDbDriver.h"
#include "KDbDriverManager_p.h"
#include "KDbConnection.h"
#include "KDbExpression.h"
#include "generated/sqlparser.h"
#include "KDb.h"
#include "KDbLookupFieldSchema.h"
#include "KDbParser_p.h"

#include <assert.h>

#include <QBitArray>
#include <QWeakPointer>
#include "kdb_debug.h"

//! @internal
class KDbQuerySchema::Private
{
public:
    explicit Private(KDbQuerySchema* q, Private* copy = 0)
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
    ~Private() {
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

    KDbQuerySchema *query;

    /*! Master table of the query. (may be NULL)
      Any data modifications can be performed if we know master table.
      If null, query's records cannot be modified. */
    KDbTableSchema *masterTable;

    /*! List of tables used in this query */
    QList<KDbTableSchema*> tables;

    KDbField *fakeRecordIdField; //! used to mark a place for record Id
    KDbQueryColumnInfo *fakeRecordIdCol; //! used to mark a place for record Id

protected:
    void tryRegenerateExprAliases() {
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

//=======================================

KDbOrderByColumn::KDbOrderByColumn()
        : m_column(0)
        , m_pos(-1)
        , m_field(0)
        , m_ascending(true)
{
}

KDbOrderByColumn::KDbOrderByColumn(KDbQueryColumnInfo& column, bool ascending, int pos)
        : m_column(&column)
        , m_pos(pos)
        , m_field(0)
        , m_ascending(ascending)
{
}

KDbOrderByColumn::KDbOrderByColumn(KDbField& field, bool ascending)
        : m_column(0)
        , m_pos(-1)
        , m_field(&field)
        , m_ascending(ascending)
{
}

KDbOrderByColumn* KDbOrderByColumn::copy(KDbQuerySchema* fromQuery, KDbQuerySchema* toQuery) const
{
    if (m_field) {
        return new KDbOrderByColumn(*m_field, m_ascending);
    }
    if (m_column) {
        KDbQueryColumnInfo* columnInfo;
        if (fromQuery && toQuery) {
            int columnIndex = fromQuery->columnsOrder().value(m_column);
            if (columnIndex < 0) {
                kdbDebug() << "KDbOrderByColumn::copy(): Index not found for column" << *m_column;
                return 0;
            }
            columnInfo = toQuery->expandedOrInternalField(columnIndex);
            if (!columnInfo) {
                kdbDebug() << "KDbOrderByColumn::copy(): Column info not found at index"
                       << columnIndex << "in toQuery";
                return 0;
            }
        }
        else {
            columnInfo = m_column;
        }
        return new KDbOrderByColumn(*columnInfo, m_ascending, m_pos);
    }
    Q_ASSERT(m_field || m_column);
    return 0;
}

KDbOrderByColumn::~KDbOrderByColumn()
{
}

KDbQueryColumnInfo* KDbOrderByColumn::column() const
{
    return m_column;
}

int KDbOrderByColumn::position() const
{
    return m_pos;
}

KDbField* KDbOrderByColumn::field() const
{
    return m_field;
}

bool KDbOrderByColumn::ascending() const
{
    return m_ascending;
}

bool KDbOrderByColumn::operator== (const KDbOrderByColumn& col) const
{
    return m_column == col.m_column && m_field == col.m_field
           && m_ascending == col.m_ascending;
}

QDebug operator<<(QDebug dbg, const KDbOrderByColumn& order)
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

static QString escapeIdentifier(const QString& name, KDbConnection *conn,
                                KDb::IdentifierEscapingType escapingType)
{
    switch (escapingType) {
    case KDb::DriverEscaping:
        if (conn)
            return conn->escapeIdentifier(name);
        break;
    case KDb::KDbEscaping:
        return KDb::escapeIdentifier(name);
    }
    return QLatin1Char('"') + name + QLatin1Char('"');
}

KDbEscapedString KDbOrderByColumn::toSQLString(bool includeTableName,
                                   KDbConnection *conn, KDb::IdentifierEscapingType escapingType) const
{
    const QByteArray orderString(m_ascending ? "" : " DESC");
    KDbEscapedString fieldName, tableName, collationString;
    if (m_column) {
        if (m_pos > -1)
            return KDbEscapedString::number(m_pos + 1) + orderString;
        else {
            if (includeTableName && m_column->alias.isEmpty()) {
                tableName = KDbEscapedString(escapeIdentifier(m_column->field->table()->name(), conn, escapingType));
                tableName += '.';
            }
            fieldName = KDbEscapedString(escapeIdentifier(m_column->aliasOrName(), conn, escapingType));
        }
        if (m_column->field->isTextType()) {
            collationString = conn->driver()->collationSQL();
        }
    }
    else {
        if (m_field && includeTableName) {
            tableName = KDbEscapedString(escapeIdentifier(m_field->table()->name(), conn, escapingType));
            tableName += '.';
        }
        fieldName = KDbEscapedString(escapeIdentifier(
            m_field ? m_field->name() : QLatin1String("??")/*error*/, conn, escapingType));
        if (m_field && m_field->isTextType()) {
            collationString = conn->driver()->collationSQL();
        }
    }
    return tableName + fieldName + collationString + orderString;
}

//=======================================

KDbOrderByColumnList::KDbOrderByColumnList()
        : QList<KDbOrderByColumn*>()
{
}

KDbOrderByColumnList::KDbOrderByColumnList(const KDbOrderByColumnList& other,
                                     KDbQuerySchema* fromQuery, KDbQuerySchema* toQuery)
        : QList<KDbOrderByColumn*>()
{
    for (QList<KDbOrderByColumn*>::ConstIterator it(other.constBegin()); it != other.constEnd(); ++it) {
        KDbOrderByColumn* order = (*it)->copy(fromQuery, toQuery);
        if (order) {
            append(order);
        }
    }
}

bool KDbOrderByColumnList::appendFields(KDbQuerySchema& querySchema,
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

KDbOrderByColumnList::~KDbOrderByColumnList()
{
    qDeleteAll(begin(), end());
}

void KDbOrderByColumnList::appendColumn(KDbQueryColumnInfo& columnInfo, bool ascending)
{
    append(new KDbOrderByColumn(columnInfo, ascending));
}

bool KDbOrderByColumnList::appendColumn(KDbQuerySchema& querySchema, bool ascending, int pos)
{
    KDbQueryColumnInfo::Vector fieldsExpanded(querySchema.fieldsExpanded());
    if (pos < 0 || pos >= fieldsExpanded.size()) {
        return false;
    }
    KDbQueryColumnInfo* ci = fieldsExpanded[pos];
    append(new KDbOrderByColumn(*ci, ascending, pos));
    return true;
}

void KDbOrderByColumnList::appendField(KDbField& field, bool ascending)
{
    append(new KDbOrderByColumn(field, ascending));
}

bool KDbOrderByColumnList::appendField(KDbQuerySchema& querySchema,
                                    const QString& fieldName, bool ascending)
{
    KDbQueryColumnInfo *columnInfo = querySchema.columnInfo(fieldName);
    if (columnInfo) {
        append(new KDbOrderByColumn(*columnInfo, ascending));
        return true;
    }
    KDbField *field = querySchema.findTableField(fieldName);
    if (field) {
        append(new KDbOrderByColumn(*field, ascending));
        return true;
    }
    kdbWarning() << "no such field" << fieldName;
    return false;
}

QDebug operator<<(QDebug dbg, const KDbOrderByColumnList& list)
{
    if (list.isEmpty()) {
        dbg.nospace() << "NONE";
        return dbg.space();
    }
    bool first = true;
    for (QList<KDbOrderByColumn*>::ConstIterator it(list.constBegin()); it != list.constEnd(); ++it) {
        if (first)
            first = false;
        else
            dbg.nospace() << '\n';
        dbg.nospace() << *(*it);
    }
    return dbg.space();
}

KDbEscapedString KDbOrderByColumnList::toSQLString(bool includeTableNames, KDbConnection *conn,
                                       KDb::IdentifierEscapingType escapingType) const
{
    KDbEscapedString string;
    for (QList<KDbOrderByColumn*>::ConstIterator it(constBegin()); it != constEnd(); ++it) {
        if (!string.isEmpty())
            string += ", ";
        string += (*it)->toSQLString(includeTableNames, conn, escapingType);
    }
    return string;
}

void KDbOrderByColumnList::clear()
{
    qDeleteAll(begin(), end());
    QList<KDbOrderByColumn*>::clear();
}

//=======================================

KDbQuerySchema::KDbQuerySchema()
        : KDbFieldList(false)//fields are not owned by KDbQuerySchema object
        , KDbObject(KDb::QueryObjectType)
        , d(new Private(this))
{
    init();
}

KDbQuerySchema::KDbQuerySchema(KDbTableSchema *tableSchema)
        : KDbFieldList(false)
        , KDbObject(KDb::QueryObjectType)
        , d(new Private(this))
{
    Q_ASSERT(tableSchema);
    d->masterTable = tableSchema;
    init();
    /*if (!d->masterTable) {
      kdbWarning() << "!d->masterTable";
      m_name.clear();
      return;
    }*/
    addTable(d->masterTable);
    //defaults:
    //inherit name from a table
    setName(d->masterTable->name());
    //inherit caption from a table
    setCaption(d->masterTable->caption());

    // add explicit field list to avoid problems (e.g. with fields added outside of the app):
    foreach(KDbField* f, *d->masterTable->fields()) {
        addField(f);
    }
}

KDbQuerySchema::KDbQuerySchema(const KDbQuerySchema& querySchema)
        : KDbFieldList(querySchema, false /* !deepCopyFields */)
        , KDbObject(querySchema)
        , d(new Private(this, querySchema.d))
{
    //only deep copy query asterisks
    foreach(KDbField* f, *querySchema.fields()) {
        KDbField *copiedField;
        if (dynamic_cast<KDbQueryAsterisk*>(f)) {
            copiedField = f->copy();
            if (static_cast<const KDbFieldList *>(f->m_parent) == &querySchema) {
                copiedField->m_parent = this;
            }
        }
        else {
            copiedField = f;
        }
        addField(copiedField);
    }
    // this deep copy must be after the 'd' initialization because fieldsExpanded() is used there
    d->orderByColumnList = new KDbOrderByColumnList(*querySchema.d->orderByColumnList,
                                                 const_cast<KDbQuerySchema*>(&querySchema), this);
}

KDbQuerySchema::~KDbQuerySchema()
{
    delete d;
}

void KDbQuerySchema::init()
{
//m_fields_by_name.setAutoDelete( true ); //because we're using QueryColumnInfoEntry objects
}

void KDbQuerySchema::clear()
{
    KDbFieldList::clear();
    KDbObject::clear();
    d->clear();
}

//! @todo IMPORTANT: move visible to overload
KDbFieldList& KDbQuerySchema::insertField(uint position, KDbField *field, bool visible)
{
    return insertField(position, field, -1/*don't bind*/, visible);
}

/*virtual*/
KDbFieldList& KDbQuerySchema::insertField(uint position, KDbField *field)
{
    return insertField(position, field, -1/*don't bind*/, true);
}

//! @todo IMPORTANT: move visible to overload
KDbFieldList& KDbQuerySchema::insertField(uint position, KDbField *field,
                                    int bindToTable, bool visible)
{
    if (!field) {
        kdbWarning() << "!field";
        return *this;
    }

    if (position > (uint)m_fields.count()) {
        kdbWarning() << "position" << position << "out of range";
        return *this;
    }
    if (!field->isQueryAsterisk() && !field->isExpression() && !field->table()) {
        kdbWarning() << "field" << field->name() << "must contain table information!";
        return *this;
    }
    if ((int)fieldCount() >= d->visibility.size()) {
        d->visibility.resize(d->visibility.size()*2);
        d->tablesBoundToColumns.resize(d->tablesBoundToColumns.size()*2);
    }
    d->clearCachedData();
    KDbFieldList::insertField(position, field);
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
    //update visibility
    //--move bits to make a place for a new one
    for (uint i = fieldCount() - 1; i > position; i--)
        d->visibility.setBit(i, d->visibility.testBit(i - 1));
    d->visibility.setBit(position, visible);

    //bind to table
    if (bindToTable < -1 || bindToTable > int(d->tables.count())) {
        kdbWarning() << "bindToTable" << bindToTable << "out of range";
        bindToTable = -1;
    }
    //--move items to make a place for a new one
    for (uint i = fieldCount() - 1; i > position; i--)
        d->tablesBoundToColumns[i] = d->tablesBoundToColumns[i-1];
    d->tablesBoundToColumns[position] = bindToTable;

    kdbDebug() << "bound to table" << bindToTable;
    if (bindToTable == -1)
        kdbDebug() << " <NOT SPECIFIED>";
    else
        kdbDebug() << " name=" << d->tables.at(bindToTable)->name()
        << " alias=" << tableAlias(bindToTable);
    QString s;
    for (uint i = 0; i < fieldCount();i++)
        s += (QString::number(d->tablesBoundToColumns[i]) + QLatin1Char(' '));
    kdbDebug() << "tablesBoundToColumns == [" << s << "]";

    if (field->isExpression())
        d->regenerateExprAliases = true;

    return *this;
}

int KDbQuerySchema::tableBoundToColumn(uint columnPosition) const
{
    int res = d->tablesBoundToColumns.value(columnPosition, -99);
    if (res == -99) {
        kdbWarning() << "columnPosition" << columnPosition << "out of range";
        return -1;
    }
    return res;
}

//! @todo IMPORTANT: move visible to overload
KDbFieldList& KDbQuerySchema::addField(KDbField* field, bool visible)
{
    return insertField(m_fields.count(), field, visible);
}

//! @todo IMPORTANT: move visible to overload
KDbFieldList& KDbQuerySchema::addField(KDbField* field, int bindToTable,
        bool visible)
{
    return insertField(m_fields.count(), field, bindToTable, visible);
}

bool KDbQuerySchema::removeField(KDbField *field)
{
    int indexOfAsterisk = -1;
    if (field->isQueryAsterisk()) {
        indexOfAsterisk = d->asterisks.indexOf(field);
    }
    if (!KDbFieldList::removeField(field)) {
        return false;
    }
    d->clearCachedData();
    if (indexOfAsterisk >= 0) {
        //kdbDebug() << "d->asterisks.removeAt:" << field;
        //field->debug();
        d->asterisks.removeAt(indexOfAsterisk); //this will destroy this asterisk
    }
//! @todo should we also remove table for this field or asterisk?
    return true;
}

//! @todo IMPORTANT: move visible to overload
KDbFieldList& KDbQuerySchema::addExpression(const KDbExpression& expr, bool visible)
{
    return addField(new KDbField(this, expr), visible);
}

bool KDbQuerySchema::isColumnVisible(uint position) const
{
    return (position < fieldCount()) ? d->visibility.testBit(position) : false;
}

void KDbQuerySchema::setColumnVisible(uint position, bool v)
{
    if (position < fieldCount())
        d->visibility.setBit(position, v);
}

//! @todo IMPORTANT: move visible to overload
KDbFieldList& KDbQuerySchema::addAsterisk(KDbQueryAsterisk *asterisk, bool visible)
{
    if (!asterisk)
        return *this;
    //make unique name
    asterisk->setName((asterisk->table() ? (asterisk->table()->name() + QLatin1String(".*"))
                                         : QString(QLatin1Char('*')))
                       + QString::number(asterisks()->count()));
    return addField(asterisk, visible);
}

KDbConnection* KDbQuerySchema::connection() const
{
    KDbTableSchema *mt = masterTable();
    return mt ? mt->connection() : 0;
}

QDebug operator<<(QDebug dbg, const KDbQuerySchema& query)
{
    //fields
    KDbTableSchema *mt = query.masterTable();
    dbg.nospace() << "QUERY";
    dbg.space() << static_cast<const KDbObject&>(query) << '\n';
    dbg.nospace() << " - MASTERTABLE=" << (mt ? mt->name() : QLatin1String("<NULL>"))
        << "\n - COLUMNS:\n";
    if (query.fieldCount() > 0)
        dbg.nospace() << static_cast<const KDbFieldList&>(query) << '\n';
    else
        dbg.nospace() << "<NONE>\n";

    if (query.fieldCount() == 0)
        dbg.nospace() << " - NO FIELDS\n";
    else
        dbg.nospace() << " - FIELDS EXPANDED (";

    uint fieldsExpandedCount = 0;
    bool first;
    if (query.fieldCount() > 0) {
        const KDbQueryColumnInfo::Vector fe(query.fieldsExpanded());
        fieldsExpandedCount = fe.size();
        dbg.nospace() << fieldsExpandedCount << "):\n";
        first = true;
        for (uint i = 0; i < fieldsExpandedCount; i++) {
            KDbQueryColumnInfo *ci = fe[i];
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
                dbg.space() << static_cast<const KDbFieldList&>(query).field(i)->name();
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
    foreach(KDbTableSchema *table, *query.tables()) {
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
        foreach(KDbField *f, *query.fields()) {
            i++;
            const QString alias(query.columnAlias(i));
            if (!alias.isEmpty()) {
                dbg.nospace() << QString::fromLatin1("FIELD #%1:").arg(i);
                dbg.space() << (f->name().isEmpty()
                    ? QLatin1String("<NONAME>") : f->name()) << " -> " << alias << '\n';
            }
        }
    }

    dbg.nospace() << "\n- TABLE ALIASES:\n";
    if (query.tableAliasesCount() == 0) {
        dbg.nospace() << "<NONE>\n";
    }
    else {
        int i = -1;
        foreach(KDbTableSchema* table, *query.tables()) {
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
    if (!query.orderByColumnList()->isEmpty()) {
        dbg.space() << QString::fromLatin1(" - ORDER BY (%1):\n").arg(query.orderByColumnList()->count());
        dbg.nospace() << *query.orderByColumnList();
    }
    return dbg.nospace();
}

KDbTableSchema* KDbQuerySchema::masterTable() const
{
    if (d->masterTable)
        return d->masterTable;
    if (d->tables.isEmpty())
        return 0;

    //try to find master table if there's only one table (with possible aliasses)
    QString tableNameLower;
    int num = -1;
    foreach(KDbTableSchema *table, d->tables) {
        num++;
        if (!tableNameLower.isEmpty() && table->name().toLower() != tableNameLower) {
            //two or more different tables
            return 0;
        }
        tableNameLower = tableAlias(num);
    }
    return d->tables.first();
}

void KDbQuerySchema::setMasterTable(KDbTableSchema *table)
{
    if (table)
        d->masterTable = table;
}

QList<KDbTableSchema*>* KDbQuerySchema::tables() const
{
    return &d->tables;
}

void KDbQuerySchema::addTable(KDbTableSchema *table, const QString& alias)
{
    kdbDebug() << (void *)table << "alias=" << alias;
    if (!table)
        return;

    // only append table if: it has alias or it has no alias but there is no such table on the list
    if (alias.isEmpty() && d->tables.contains(table)) {
        int num = -1;
        foreach(KDbTableSchema *t, d->tables) {
            num++;
            if (0 == t->name().compare(table->name(), Qt::CaseInsensitive)) {
                if (tableAlias(num).isEmpty()) {
                    kdbDebug() << "table" << table->name() << "without alias already added";
                    return;
                }
            }
        }
    }
    d->tables.append(table);
    if (!alias.isEmpty())
        setTableAlias(d->tables.count() - 1, alias);
}

void KDbQuerySchema::removeTable(KDbTableSchema *table)
{
    if (!table)
        return;
    if (d->masterTable == table)
        d->masterTable = 0;
    d->tables.removeAt(d->tables.indexOf(table));
//! @todo remove fields!
}

KDbTableSchema* KDbQuerySchema::table(const QString& tableName) const
{
//! @todo maybe use tables_byname?
    foreach(KDbTableSchema *table, d->tables) {
        if (0 == table->name().compare(tableName, Qt::CaseInsensitive)) {
            return table;
        }
    }
    return 0;
}

bool KDbQuerySchema::contains(KDbTableSchema *table) const
{
    return d->tables.contains(table);
}

KDbField* KDbQuerySchema::findTableField(const QString &tableOrTableAndFieldName) const
{
    QString tableName, fieldName;
    if (!KDb::splitToTableAndFieldParts(tableOrTableAndFieldName,
                                        &tableName, &fieldName,
                                        KDb::SetFieldNameIfNoTableName)) {
        return 0;
    }
    if (tableName.isEmpty()) {
        foreach(KDbTableSchema *table, d->tables) {
            if (table->field(fieldName))
                return table->field(fieldName);
        }
        return 0;
    }
    KDbTableSchema *tableSchema = table(tableName);
    if (!tableSchema)
        return 0;
    return tableSchema->field(fieldName);
}

int KDbQuerySchema::columnAliasesCount() const
{
    return d->columnAliasesCount();
}

QString KDbQuerySchema::columnAlias(uint position) const
{
    return d->columnAlias(position);
}

bool KDbQuerySchema::hasColumnAlias(uint position) const
{
    return d->hasColumnAlias(position);
}

void KDbQuerySchema::setColumnAlias(uint position, const QString& alias)
{
    if (position >= (uint)m_fields.count()) {
        kdbWarning() << "position"  << position << "out of range!";
        return;
    }
    const QString fixedAlias(alias.trimmed());
    KDbField *f = KDbFieldList::field(position);
    if (f->captionOrName().isEmpty() && fixedAlias.isEmpty()) {
        kdbWarning() << "position" << position << "could not remove alias when no name is specified for expression column!";
        return;
    }
    d->setColumnAlias(position, fixedAlias);
}

int KDbQuerySchema::tableAliasesCount() const
{
    return d->tableAliases.count();
}

QString KDbQuerySchema::tableAlias(uint position) const
{
    return d->tableAliases.value(position);
}

QString KDbQuerySchema::tableAlias(const QString& tableName) const
{
    const int pos = tablePosition(tableName);
    if (pos == -1) {
        return QString();
    }
    return d->tableAliases.value(pos);
}

QString KDbQuerySchema::tableAliasOrName(const QString& tableName) const
{
    const int pos = tablePosition(tableName);
    if (pos == -1) {
        return QString();
    }
    return KDb::iifNotEmpty(d->tableAliases.value(pos), tableName);
}

int KDbQuerySchema::tablePositionForAlias(const QString& name) const
{
    return d->tablePositionForAlias(name);
}

int KDbQuerySchema::tablePosition(const QString& tableName) const
{
    int num = -1;
    foreach(KDbTableSchema* table, d->tables) {
        num++;
        if (0 == table->name().compare(tableName, Qt::CaseInsensitive)) {
            return num;
        }
    }
    return -1;
}

QList<int> KDbQuerySchema::tablePositions(const QString& tableName) const
{
    QList<int> result;
    int num = -1;
    foreach(KDbTableSchema* table, d->tables) {
        num++;
        if (0 == table->name().compare(tableName, Qt::CaseInsensitive)) {
            result += num;
        }
    }
    return result;
}

bool KDbQuerySchema::hasTableAlias(uint position) const
{
    return d->tableAliases.contains(position);
}

int KDbQuerySchema::columnPositionForAlias(const QString& name) const
{
    return d->columnPositionForAlias(name);
}

void KDbQuerySchema::setTableAlias(uint position, const QString& alias)
{
    if (position >= (uint)d->tables.count()) {
        kdbWarning() << "position"  << position << "out of range!";
        return;
    }
    const QString fixedAlias(alias.trimmed());
    if (fixedAlias.isEmpty()) {
        const QString oldAlias(d->tableAliases.take(position));
        if (!oldAlias.isEmpty()) {
            d->removeTablePositionForAlias(oldAlias);
        }
    } else {
        d->setTableAlias(position, fixedAlias);
    }
}

QList<KDbRelationship*>* KDbQuerySchema::relationships() const
{
    return &d->relations;
}

KDbField::List* KDbQuerySchema::asterisks() const
{
    return &d->asterisks;
}

KDbEscapedString KDbQuerySchema::statement() const
{
    return d->sql;
}

void KDbQuerySchema::setStatement(const KDbEscapedString& sql)
{
    d->sql = sql;
}

KDbField* KDbQuerySchema::field(const QString& name) const
{
    return field(name, true);
}

KDbField* KDbQuerySchema::field(const QString& identifier, bool expanded) const
{
    KDbQueryColumnInfo *ci = columnInfo(identifier, expanded);
    return ci ? ci->field : 0;
}

KDbQueryColumnInfo* KDbQuerySchema::columnInfo(const QString& identifier, bool expanded) const
{
    computeFieldsExpanded();
    return expanded ? d->columnInfosByNameExpanded.value(identifier)
            : d->columnInfosByName.value(identifier);
}

KDbQueryColumnInfo::Vector KDbQuerySchema::fieldsExpanded(FieldsExpandedOptions options) const
{
    computeFieldsExpanded();
    if (options == WithInternalFields || options == WithInternalFieldsAndRecordId) {
        //a ref to a proper pointer (as we cache the vector for two cases)
        KDbQueryColumnInfo::Vector*& tmpFieldsExpandedWithInternal =
            (options == WithInternalFields) ? d->fieldsExpandedWithInternal : d->fieldsExpandedWithInternalAndRecordId;
        //special case
        if (!tmpFieldsExpandedWithInternal) {
            //glue expanded and internal fields and cache it
            const uint size = d->fieldsExpanded->count()
                              + (d->internalFields ? d->internalFields->count() : 0)
                              + ((options == WithInternalFieldsAndRecordId) ? 1 : 0) /*ROWID*/;
            tmpFieldsExpandedWithInternal = new KDbQueryColumnInfo::Vector(size);
            const uint fieldsExpandedVectorSize = d->fieldsExpanded->size();
            for (uint i = 0; i < fieldsExpandedVectorSize; i++) {
                (*tmpFieldsExpandedWithInternal)[i] = d->fieldsExpanded->at(i);
            }
            const uint internalFieldCount = d->internalFields ? d->internalFields->size() : 0;
            if (internalFieldCount > 0) {
                for (uint i = 0; i < internalFieldCount; i++) {
                    (*tmpFieldsExpandedWithInternal)[fieldsExpandedVectorSize + i] = d->internalFields->at(i);
                }
            }
            if (options == WithInternalFieldsAndRecordId) {
                if (!d->fakeRecordIdField) {
                    d->fakeRecordIdField = new KDbField(QLatin1String("rowID"), KDbField::BigInteger);
                    d->fakeRecordIdCol = new KDbQueryColumnInfo(d->fakeRecordIdField, QString(), true);
                }
                (*tmpFieldsExpandedWithInternal)[fieldsExpandedVectorSize + internalFieldCount] = d->fakeRecordIdCol;
            }
        }
        return *tmpFieldsExpandedWithInternal;
    }

    if (options == Default)
        return *d->fieldsExpanded;

    //options == Unique:
    QSet<QString> columnsAlreadyFound;
    const uint fieldsExpandedCount(d->fieldsExpanded->count());
    KDbQueryColumnInfo::Vector result(fieldsExpandedCount);   //initial size is set
    //compute unique list
    uint uniqueListCount = 0;
    for (uint i = 0; i < fieldsExpandedCount; i++) {
        KDbQueryColumnInfo *ci = d->fieldsExpanded->at(i);
        if (!columnsAlreadyFound.contains(ci->aliasOrName())) {
            columnsAlreadyFound.insert(ci->aliasOrName());
            result[uniqueListCount++] = ci;
        }
    }
    result.resize(uniqueListCount); //update result size
    return result;
}

KDbQueryColumnInfo::Vector KDbQuerySchema::internalFields() const
{
    computeFieldsExpanded();
    return d->internalFields ? *d->internalFields : KDbQueryColumnInfo::Vector();
}

KDbQueryColumnInfo* KDbQuerySchema::expandedOrInternalField(uint index) const
{
    return fieldsExpanded(WithInternalFields).value(index);
}

inline QString lookupColumnKey(KDbField *foreignField, KDbField* field)
{
    QString res;
    if (field->table()) // can be 0 for anonymous fields built as joined multiple visible columns
        res = field->table()->name() + QLatin1Char('.');
    return res + field->name() + QLatin1Char('_') + foreignField->table()->name()
               + QLatin1Char('.') + foreignField->name();
}

void KDbQuerySchema::computeFieldsExpanded() const
{
    if (d->fieldsExpanded)
        return;

    if (!d->columnsOrder) {
        d->columnsOrder = new QHash<KDbQueryColumnInfo*, int>();
        d->columnsOrderWithoutAsterisks = new QHash<KDbQueryColumnInfo*, int>();
    } else {
        d->columnsOrder->clear();
        d->columnsOrderWithoutAsterisks->clear();
    }
    if (d->ownedVisibleColumns)
        d->ownedVisibleColumns->clear();

    //collect all fields in a list (not a vector yet, because we do not know its size)
    KDbQueryColumnInfo::List list; //temporary
    KDbQueryColumnInfo::List lookup_list; //temporary, for collecting additional fields related to lookup fields
    QHash<KDbQueryColumnInfo*, bool> columnInfosOutsideAsterisks; //helper for filling d->columnInfosByName
    int i = 0;
    uint numberOfColumnsWithMultipleVisibleFields = 0; //used to find an unique name for anonymous field
    int fieldPosition = -1;
    foreach(KDbField *f, m_fields) {
        fieldPosition++;
        if (f->isQueryAsterisk()) {
            if (static_cast<KDbQueryAsterisk*>(f)->isSingleTableAsterisk()) {
                const KDbField::List *ast_fields = static_cast<KDbQueryAsterisk*>(f)->table()->fields();
                foreach(KDbField *ast_f, *ast_fields) {
                    KDbQueryColumnInfo *ci = new KDbQueryColumnInfo(ast_f, QString()/*no field for asterisk!*/,
                            isColumnVisible(fieldPosition));
                    list.append(ci);
                    kdbDebug() << "caching (unexpanded) columns order:" << *ci << "at position" << fieldPosition;
                    d->columnsOrder->insert(ci, fieldPosition);
                }
            } else {//all-tables asterisk: iterate through table list
                foreach(KDbTableSchema *table, d->tables) {
                    //add all fields from this table
                    const KDbField::List *tab_fields = table->fields();
                    foreach(KDbField *tab_f, *tab_fields) {
//! @todo (js): perhaps not all fields should be appended here
//      d->detailedVisibility += isFieldVisible(fieldPosition);
//      list.append(tab_f);
                        KDbQueryColumnInfo *ci = new KDbQueryColumnInfo(tab_f, QString()/*no field for asterisk!*/,
                                isColumnVisible(fieldPosition));
                        list.append(ci);
                        kdbDebug() << "caching (unexpanded) columns order:" << *ci << "at position" << fieldPosition;
                        d->columnsOrder->insert(ci, fieldPosition);
                    }
                }
            }
        } else {
            //a single field
            KDbQueryColumnInfo *ci = new KDbQueryColumnInfo(f, columnAlias(fieldPosition), isColumnVisible(fieldPosition));
            list.append(ci);
            columnInfosOutsideAsterisks.insert(ci, true);
            kdbDebug() << "caching (unexpanded) column's order:" << *ci << "at position" << fieldPosition;
            d->columnsOrder->insert(ci, fieldPosition);
            d->columnsOrderWithoutAsterisks->insert(ci, fieldPosition);

            //handle lookup field schema
            KDbLookupFieldSchema *lookupFieldSchema = f->table() ? f->table()->lookupFieldSchema(*f) : 0;
            if (!lookupFieldSchema || lookupFieldSchema->boundColumn() < 0)
                continue;
            // Lookup field schema found:
            // Now we also need to fetch "visible" value from the lookup table, not only the value of binding.
            // -> build LEFT OUTER JOIN clause for this purpose (LEFT, not INNER because the binding can be broken)
            // "LEFT OUTER JOIN lookupTable ON thisTable.thisField=lookupTable.boundField"
            KDbLookupFieldSchema::RecordSource recordSource = lookupFieldSchema->recordSource();
            if (recordSource.type() == KDbLookupFieldSchema::RecordSource::Table) {
                KDbTableSchema *lookupTable = connection()->tableSchema(recordSource.name());
                KDbFieldList* visibleColumns = 0;
                KDbField *boundField = 0;
                if (lookupTable
                        && (uint)lookupFieldSchema->boundColumn() < lookupTable->fieldCount()
                        && (visibleColumns = lookupTable->subList(lookupFieldSchema->visibleColumns()))
                        && (boundField = lookupTable->field(lookupFieldSchema->boundColumn()))) {
                    KDbField *visibleColumn = 0;
                    // for single visible column, just add it as-is
                    if (visibleColumns->fieldCount() == 1) {
                        visibleColumn = visibleColumns->fields()->first();
                    } else {
                        // for multiple visible columns, build an expression column
                        // (the expression object will be owned by column info)
                        visibleColumn = new KDbField();
                        visibleColumn->setName(
                            QString::fromLatin1("[multiple_visible_fields_%1]")
                            .arg(++numberOfColumnsWithMultipleVisibleFields));
                        visibleColumn->setExpression(
                            KDbConstExpression(CHARACTER_STRING_LITERAL, QVariant()/*not important*/));
                        if (!d->ownedVisibleColumns) {
                            d->ownedVisibleColumns = new KDbField::List();
                        }
                        d->ownedVisibleColumns->append(visibleColumn);   // remember to delete later
                    }

                    lookup_list.append(
                        new KDbQueryColumnInfo(visibleColumn, QString(), true/*visible*/, ci/*foreign*/));
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
            } else if (recordSource.type() == KDbLookupFieldSchema::RecordSource::Query) {
                KDbQuerySchema *lookupQuery = connection()->querySchema(recordSource.name());
                if (!lookupQuery)
                    continue;
                const KDbQueryColumnInfo::Vector lookupQueryFieldsExpanded(lookupQuery->fieldsExpanded());
                if (lookupFieldSchema->boundColumn() >= lookupQueryFieldsExpanded.count())
                    continue;
                KDbQueryColumnInfo *boundColumnInfo = 0;
                if (!(boundColumnInfo = lookupQueryFieldsExpanded.value(lookupFieldSchema->boundColumn())))
                    continue;
                KDbField *boundField = boundColumnInfo->field;
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
                KDbField *visibleColumn = 0;
                // for single visible column, just add it as-is
                if (visibleColumns.count() == 1) {
                    visibleColumn = lookupQueryFieldsExpanded.value(visibleColumns.first())->field;
                } else {
                    // for multiple visible columns, build an expression column
                    // (the expression object will be owned by column info)
                    visibleColumn = new KDbField();
                    visibleColumn->setName(
                        QString::fromLatin1("[multiple_visible_fields_%1]")
                        .arg(++numberOfColumnsWithMultipleVisibleFields));
                    visibleColumn->setExpression(
                        KDbConstExpression(CHARACTER_STRING_LITERAL, QVariant()/*not important*/));
                    if (!d->ownedVisibleColumns) {
                        d->ownedVisibleColumns = new KDbField::List();
                    }
                    d->ownedVisibleColumns->append(visibleColumn);   // remember to delete later
                }

                lookup_list.append(
                    new KDbQueryColumnInfo(visibleColumn, QString(), true/*visible*/, ci/*foreign*/));
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
        d->fieldsExpanded = new KDbQueryColumnInfo::Vector(list.count());
        d->columnsOrderExpanded = new QHash<KDbQueryColumnInfo*, int>();
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
    foreach(KDbQueryColumnInfo* ci, list) {
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
    QHash<QString, uint> lookup_dict; //used to fight duplicates and to update KDbQueryColumnInfo::indexForVisibleLookupValue()
    // (a mapping from table.name string to uint* lookupFieldIndex
    i = 0;
    for (QMutableListIterator<KDbQueryColumnInfo*> it(lookup_list); it.hasNext();) {
        KDbQueryColumnInfo* ci = it.next();
        const QString key(lookupColumnKey(ci->foreignColumn()->field, ci->field));
        if (lookup_dict.contains(key)) {
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
        d->internalFields = new KDbQueryColumnInfo::Vector(lookup_list.count());
    }
    i = -1;
    foreach(KDbQueryColumnInfo *ci, lookup_list) {
        i++;
        //add it to the internal list
        (*d->internalFields)[i] = ci;
        d->columnsOrderExpanded->insert(ci, list.count() + i);
    }

    //update KDbQueryColumnInfo::indexForVisibleLookupValue() cache for columns
    numberOfColumnsWithMultipleVisibleFields = 0;
    for (i = 0; i < (int)d->fieldsExpanded->size(); i++) {
        KDbQueryColumnInfo* ci = d->fieldsExpanded->at(i);
//! @todo KDbQuerySchema itself will also support lookup fields...
        KDbLookupFieldSchema *lookupFieldSchema
        = ci->field->table() ? ci->field->table()->lookupFieldSchema(*ci->field) : 0;
        if (!lookupFieldSchema || lookupFieldSchema->boundColumn() < 0)
            continue;
        const KDbLookupFieldSchema::RecordSource recordSource = lookupFieldSchema->recordSource();
        if (recordSource.type() == KDbLookupFieldSchema::RecordSource::Table) {
            KDbTableSchema *lookupTable = connection()->tableSchema(recordSource.name());
            KDbFieldList* visibleColumns = 0;
            if (lookupTable
                    && (uint)lookupFieldSchema->boundColumn() < lookupTable->fieldCount()
                    && (visibleColumns = lookupTable->subList(lookupFieldSchema->visibleColumns()))) {
                // for single visible column, just add it as-is
                if (visibleColumns->fieldCount() == 1) {
                    KDbField *visibleColumn = visibleColumns->fields()->first();
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
        } else if (recordSource.type() == KDbLookupFieldSchema::RecordSource::Query) {
            KDbQuerySchema *lookupQuery = connection()->querySchema(recordSource.name());
            if (!lookupQuery)
                continue;
            const KDbQueryColumnInfo::Vector lookupQueryFieldsExpanded(lookupQuery->fieldsExpanded());
            if (lookupFieldSchema->boundColumn() >= lookupQueryFieldsExpanded.count())
                continue;
            KDbQueryColumnInfo *boundColumnInfo = 0;
            if (!(boundColumnInfo = lookupQueryFieldsExpanded.value(lookupFieldSchema->boundColumn())))
                continue;
            KDbField *boundField = boundColumnInfo->field;
            if (!boundField)
                continue;
            const QList<uint> visibleColumns(lookupFieldSchema->visibleColumns());
            // for single visible column, just add it as-is
            if (visibleColumns.count() == 1) {
                if (lookupQueryFieldsExpanded.count() > (int)visibleColumns.first()) { // sanity check
                    KDbField *visibleColumn = lookupQueryFieldsExpanded.at(visibleColumns.first())->field;
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
            kdbWarning() << "unsupported record source type" << recordSource.typeName();
        }
    }
}

QHash<KDbQueryColumnInfo*, int> KDbQuerySchema::columnsOrder(ColumnsOrderOptions options) const
{
    if (!d->columnsOrder)
        computeFieldsExpanded();
    if (options == UnexpandedList)
        return *d->columnsOrder;
    else if (options == UnexpandedListWithoutAsterisks)
        return *d->columnsOrderWithoutAsterisks;
    return *d->columnsOrderExpanded;
}

QVector<int> KDbQuerySchema::pkeyFieldsOrder() const
{
    if (d->pkeyFieldsOrder)
        return *d->pkeyFieldsOrder;

    KDbTableSchema *tbl = masterTable();
    if (!tbl || !tbl->primaryKey())
        return QVector<int>();

    //get order of PKEY fields (e.g. for records updating or inserting )
    KDbIndexSchema *pkey = tbl->primaryKey();
    kdbDebug() << *pkey;
    d->pkeyFieldsOrder = new QVector<int>(pkey->fieldCount(), -1);

    const uint fCount = fieldsExpanded().count();
    d->pkeyFieldCount = 0;
    for (uint i = 0; i < fCount; i++) {
        KDbQueryColumnInfo *fi = d->fieldsExpanded->at(i);
        const int fieldIndex = fi->field->table() == tbl ? pkey->indexOf(*fi->field) : -1;
        if (fieldIndex != -1/* field found in PK */
                && d->pkeyFieldsOrder->at(fieldIndex) == -1 /* first time */) {
            kdbDebug() << "FIELD" << fi->field->name() << "IS IN PKEY AT POSITION #" << fieldIndex;
            (*d->pkeyFieldsOrder)[fieldIndex] = i;
            d->pkeyFieldCount++;
        }
    }
    kdbDebug() << d->pkeyFieldCount
    << " OUT OF " << pkey->fieldCount() << " PKEY'S FIELDS FOUND IN QUERY " << name();
    return *d->pkeyFieldsOrder;
}

uint KDbQuerySchema::pkeyFieldCount()
{
    (void)pkeyFieldsOrder(); /* rebuild information */
    return d->pkeyFieldCount;
}

KDbRelationship* KDbQuerySchema::addRelationship(KDbField *field1, KDbField *field2)
{
//@todo: find existing global db relationships
    KDbRelationship *r = new KDbRelationship(this, field1, field2);
    if (r->isEmpty()) {
        delete r;
        return 0;
    }

    d->relations.append(r);
    return r;
}

KDbQueryColumnInfo::List* KDbQuerySchema::autoIncrementFields() const
{
    if (!d->autoincFields) {
        d->autoincFields = new KDbQueryColumnInfo::List();
    }
    KDbTableSchema *mt = masterTable();
    if (!mt) {
        kdbWarning() << "no master table!";
        return d->autoincFields;
    }
    if (d->autoincFields->isEmpty()) {//no cache
        KDbQueryColumnInfo::Vector fexp = fieldsExpanded();
        for (int i = 0; i < (int)fexp.count(); i++) {
            KDbQueryColumnInfo *fi = fexp[i];
            if (fi->field->table() == mt && fi->field->isAutoIncrement()) {
                d->autoincFields->append(fi);
            }
        }
    }
    return d->autoincFields;
}

// static
KDbEscapedString KDbQuerySchema::sqlColumnsList(const KDbQueryColumnInfo::List& infolist, KDbConnection *conn,
                                    KDb::IdentifierEscapingType escapingType)
{
    KDbEscapedString result;
    result.reserve(256);
    bool start = true;
    foreach(KDbQueryColumnInfo* ci, infolist) {
        if (!start)
            result += ",";
        else
            start = false;
        result += escapeIdentifier(ci->field->name(), conn, escapingType);
    }
    return result;
}

KDbEscapedString KDbQuerySchema::autoIncrementSQLFieldsList(KDbConnection *conn) const
{
//    QWeakPointer<const KDbDriver> driverWeakPointer
//            = DriverManagerInternal::self()->driverWeakPointer(*conn->driver());
    if (   /*d->lastUsedDriverForAutoIncrementSQLFieldsList != driverWeakPointer
        ||*/ d->autoIncrementSQLFieldsList.isEmpty())
    {
        d->autoIncrementSQLFieldsList = KDbQuerySchema::sqlColumnsList(*autoIncrementFields(), conn);
        //d->lastUsedDriverForAutoIncrementSQLFieldsList = driverWeakPointer;
    }
    return d->autoIncrementSQLFieldsList;
}

void KDbQuerySchema::setWhereExpression(const KDbExpression& expr)
{
    d->whereExpr = expr.clone();
}

void KDbQuerySchema::addToWhereExpression(KDbField *field,
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

    KDbBinaryExpression newExpr(
        KDbConstExpression(token, value),
        relation,
        KDbVariableExpression((field->table() ? (field->table()->name() + QLatin1Char('.')) : QString()) + field->name())
    );
    if (d->whereExpr.isNull()) {
        d->whereExpr = newExpr;
    }
    else {
        d->whereExpr = KDbBinaryExpression(
            d->whereExpr,
            AND,
            newExpr
        );
    }
}

/*
void KDbQuerySchema::addToWhereExpression(KDbField *field, const QVariant& value)
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

KDbExpression KDbQuerySchema::whereExpression() const
{
    return d->whereExpr;
}

void KDbQuerySchema::setOrderByColumnList(const KDbOrderByColumnList& list)
{
    delete d->orderByColumnList;
    d->orderByColumnList = new KDbOrderByColumnList(list, 0, 0);
// all field names should be found, exit otherwise ..........?
}

KDbOrderByColumnList* KDbQuerySchema::orderByColumnList()
{
    return d->orderByColumnList;
}

const KDbOrderByColumnList* KDbQuerySchema::orderByColumnList() const
{
    return d->orderByColumnList;
}

QList<KDbQuerySchemaParameter> KDbQuerySchema::parameters() const
{
    if (whereExpression().isNull())
        return QList<KDbQuerySchemaParameter>();
    QList<KDbQuerySchemaParameter> params;
    whereExpression().getQueryParameters(params);
    return params;
}

static void setResult(const KDbParseInfoInternal &parseInfo,
                      QString *errorMessage, QString *errorDescription)
{
    if (errorMessage) {
        *errorMessage = parseInfo.errorMessage();
    }
    if (errorDescription) {
        *errorDescription = parseInfo.errorDescription();
    }
}

bool KDbQuerySchema::validate(QString *errorMessage, QString *errorDescription)
{
    KDbParseInfoInternal parseInfo(this);
    foreach(KDbField* f, *fields()) {
        if (f->isExpression()) {
            if (!f->expression().validate(&parseInfo)) {
                setResult(parseInfo, errorMessage, errorDescription);
                return false;
            }
        }
    }
    if (!whereExpression().validate(&parseInfo)) {
        setResult(parseInfo, errorMessage, errorDescription);
        return false;
    }
    return true;
}

//---------------------------------------------------

KDbQueryAsterisk::KDbQueryAsterisk(KDbQuerySchema *query, KDbTableSchema *table)
        : KDbField()
        , m_table(table)
{
    assert(query);
    m_parent = query;
    setType(KDbField::Asterisk);
}

KDbQueryAsterisk::KDbQueryAsterisk(const KDbQueryAsterisk &asterisk)
        : KDbField(asterisk)
        , m_table(asterisk.table())
{
}

KDbQueryAsterisk::~KDbQueryAsterisk()
{
}

KDbQuerySchema *KDbQueryAsterisk::query() const
{
    return static_cast<KDbQuerySchema*>(m_parent);
}

KDbTableSchema* KDbQueryAsterisk::table() const
{
    return m_table;
}

KDbField* KDbQueryAsterisk::copy() const
{
    return new KDbQueryAsterisk(*this);
}

void KDbQueryAsterisk::setTable(KDbTableSchema *table)
{
    m_table = table;
}

bool KDbQueryAsterisk::isSingleTableAsterisk() const
{
    return m_table;
}

bool KDbQueryAsterisk::isAllTableAsterisk() const
{
    return !m_table;
}

QDebug operator<<(QDebug dbg, const KDbQueryAsterisk& asterisk)
{
    if (asterisk.isAllTableAsterisk()) {
        dbg.nospace() << "ALL-TABLES ASTERISK (*) ON TABLES(";
        bool first = true;
        foreach(KDbTableSchema *table, *asterisk.query()->tables()) {
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

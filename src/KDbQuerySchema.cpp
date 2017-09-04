/* This file is part of the KDE project
   Copyright (C) 2003-2017 Jarosław Staniek <staniek@kde.org>

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
#include "KDbQuerySchema_p.h"
#include "KDbQueryAsterisk.h"
#include "KDbConnection.h"
#include "KDbConnection_p.h"
#include "kdb_debug.h"
#include "KDbLookupFieldSchema.h"
#include "KDbOrderByColumn.h"
#include "KDbParser_p.h"
#include "KDbQuerySchemaParameter.h"
#include "KDbRelationship.h"

QString escapeIdentifier(const QString& name, KDbConnection *conn,
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

KDbQuerySchema::KDbQuerySchema()
        : KDbFieldList(false)//fields are not owned by KDbQuerySchema object
        , KDbObject(KDb::QueryObjectType)
        , d(new KDbQuerySchemaPrivate(this))
{
}

KDbQuerySchema::KDbQuerySchema(KDbTableSchema *tableSchema)
        : KDbFieldList(false)//fields are not owned by KDbQuerySchema object
        , KDbObject(KDb::QueryObjectType)
        , d(new KDbQuerySchemaPrivate(this))
{
    if (tableSchema) {
        d->masterTable = tableSchema;
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
}

KDbQuerySchema::KDbQuerySchema(const KDbQuerySchema& querySchema, KDbConnection *conn)
        : KDbFieldList(querySchema, false /* !deepCopyFields */)
        , KDbObject(querySchema)
        , d(new KDbQuerySchemaPrivate(this, querySchema.d))
{
    //only deep copy query asterisks
    foreach(KDbField* f, *querySchema.fields()) {
        KDbField *copiedField;
        if (dynamic_cast<KDbQueryAsterisk*>(f)) {
            copiedField = f->copy();
            if (static_cast<const KDbFieldList *>(f->parent()) == &querySchema) {
                copiedField->setParent(this);
            }
        }
        else {
            copiedField = f;
        }
        addField(copiedField);
    }
    // this deep copy must be after the 'd' initialization because fieldsExpanded() is used there
    d->orderByColumnList = new KDbOrderByColumnList(*querySchema.d->orderByColumnList, conn,
                                                    const_cast<KDbQuerySchema*>(&querySchema), this);
}

KDbQuerySchema::~KDbQuerySchema()
{
    delete d;
}

void KDbQuerySchema::clear()
{
    KDbFieldList::clear();
    KDbObject::clear();
    d->clear();
}

/*virtual*/
bool KDbQuerySchema::insertField(int position, KDbField *field)
{
    return insertFieldInternal(position, field, -1/*don't bind*/, true);
}

bool KDbQuerySchema::insertInvisibleField(int position, KDbField *field)
{
    return insertFieldInternal(position, field, -1/*don't bind*/, false);
}

bool KDbQuerySchema::insertField(int position, KDbField *field, int bindToTable)
{
    return insertFieldInternal(position, field, bindToTable, true);
}

bool KDbQuerySchema::insertInvisibleField(int position, KDbField *field, int bindToTable)
{
    return insertFieldInternal(position, field, bindToTable, false);
}

bool KDbQuerySchema::insertFieldInternal(int position, KDbField *field,
                                         int bindToTable, bool visible)
{
    if (!field) {
        kdbWarning() << "!field";
        return false;
    }

    if (position > fieldCount()) {
        kdbWarning() << "position" << position << "out of range";
        return false;
    }
    if (!field->isQueryAsterisk() && !field->isExpression() && !field->table()) {
        kdbWarning() << "field" << field->name() << "must contain table information!";
        return false;
    }
    if (fieldCount() >= d->visibility.size()) {
        d->visibility.resize(d->visibility.size()*2);
        d->tablesBoundToColumns.resize(d->tablesBoundToColumns.size()*2);
    }
    d->clearCachedData();
    if (!KDbFieldList::insertField(position, field)) {
        return false;
    }
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
    for (int i = fieldCount() - 1; i > position; i--)
        d->visibility.setBit(i, d->visibility.testBit(i - 1));
    d->visibility.setBit(position, visible);

    //bind to table
    if (bindToTable < -1 || bindToTable > d->tables.count()) {
        kdbWarning() << "bindToTable" << bindToTable << "out of range";
        bindToTable = -1;
    }
    //--move items to make a place for a new one
    for (int i = fieldCount() - 1; i > position; i--)
        d->tablesBoundToColumns[i] = d->tablesBoundToColumns[i-1];
    d->tablesBoundToColumns[position] = bindToTable;

    kdbDebug() << "bound to table" << bindToTable;
    if (bindToTable == -1)
        kdbDebug() << " <NOT SPECIFIED>";
    else
        kdbDebug() << " name=" << d->tables.at(bindToTable)->name()
        << " alias=" << tableAlias(bindToTable);
    QString s;
    for (int i = 0; i < fieldCount();i++)
        s += (QString::number(d->tablesBoundToColumns[i]) + QLatin1Char(' '));
    kdbDebug() << "tablesBoundToColumns == [" << s << "]";

    if (field->isExpression())
        d->regenerateExprAliases = true;

    return true;
}

int KDbQuerySchema::tableBoundToColumn(int columnPosition) const
{
    int res = d->tablesBoundToColumns.value(columnPosition, -99);
    if (res == -99) {
        kdbWarning() << "columnPosition" << columnPosition << "out of range";
        return -1;
    }
    return res;
}

bool KDbQuerySchema::addField(KDbField* field)
{
    return insertField(fieldCount(), field);
}

bool KDbQuerySchema::addField(KDbField* field, int bindToTable)
{
    return insertField(fieldCount(), field, bindToTable);
}

bool KDbQuerySchema::addInvisibleField(KDbField* field)
{
    return insertInvisibleField(fieldCount(), field);
}

bool KDbQuerySchema::addInvisibleField(KDbField* field, int bindToTable)
{
    return insertInvisibleField(fieldCount(), field, bindToTable);
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

bool KDbQuerySchema::addExpressionInternal(const KDbExpression& expr, bool visible)
{
    KDbField *field = new KDbField(this, expr);
    bool ok;
    if (visible) {
        ok = addField(field);
    } else {
        ok = addInvisibleField(field);
    }
    if (!ok) {
        delete field;
    }
    return ok;
}

bool KDbQuerySchema::addExpression(const KDbExpression& expr)
{
    return addExpressionInternal(expr, true);
}

bool KDbQuerySchema::addInvisibleExpression(const KDbExpression& expr)
{
    return addExpressionInternal(expr, false);
}

bool KDbQuerySchema::isColumnVisible(int position) const
{
    return (position < fieldCount()) ? d->visibility.testBit(position) : false;
}

void KDbQuerySchema::setColumnVisible(int position, bool visible)
{
    if (position < fieldCount())
        d->visibility.setBit(position, visible);
}

bool KDbQuerySchema::addAsteriskInternal(KDbQueryAsterisk *asterisk, bool visible)
{
    if (!asterisk) {
        return false;
    }
    //make unique name
    asterisk->setName((asterisk->table() ? (asterisk->table()->name() + QLatin1String(".*"))
                                         : QString(QLatin1Char('*')))
                       + QString::number(asterisks()->count()));
    return visible ? addField(asterisk) : addInvisibleField(asterisk);
}

bool KDbQuerySchema::addAsterisk(KDbQueryAsterisk *asterisk)
{
    return addAsteriskInternal(asterisk, true);
}

bool KDbQuerySchema::addInvisibleAsterisk(KDbQueryAsterisk *asterisk)
{
    return addAsteriskInternal(asterisk, false);
}

QDebug operator<<(QDebug dbg, const KDbConnectionAndQuerySchema &connectionAndSchema)
{
    KDbConnection* conn = std::get<0>(connectionAndSchema);
    const KDbQuerySchema& query = std::get<1>(connectionAndSchema);
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

    int fieldsExpandedCount = 0;
    bool first;
    if (query.fieldCount() > 0) {
        const KDbQueryColumnInfo::Vector fe(query.fieldsExpanded(conn));
        fieldsExpandedCount = fe.size();
        dbg.nospace() << fieldsExpandedCount << "):\n";
        first = true;
        for (int i = 0; i < fieldsExpandedCount; i++) {
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
    for (int i = 0; i < query.fieldCount(); i++) {
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
        return nullptr;

    //try to find master table if there's only one table (with possible aliasses)
    QString tableNameLower;
    int num = -1;
    foreach(KDbTableSchema *table, d->tables) {
        num++;
        if (!tableNameLower.isEmpty() && table->name().toLower() != tableNameLower) {
            //two or more different tables
            return nullptr;
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
        d->masterTable = nullptr;
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
    return nullptr;
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
        return nullptr;
    }
    if (tableName.isEmpty()) {
        foreach(KDbTableSchema *table, d->tables) {
            if (table->field(fieldName))
                return table->field(fieldName);
        }
        return nullptr;
    }
    KDbTableSchema *tableSchema = table(tableName);
    if (!tableSchema)
        return nullptr;
    return tableSchema->field(fieldName);
}

int KDbQuerySchema::columnAliasesCount() const
{
    return d->columnAliasesCount();
}

QString KDbQuerySchema::columnAlias(int position) const
{
    return d->columnAlias(position);
}

bool KDbQuerySchema::hasColumnAlias(int position) const
{
    return d->hasColumnAlias(position);
}

void KDbQuerySchema::setColumnAlias(int position, const QString& alias)
{
    if (position >= fieldCount()) {
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

QString KDbQuerySchema::tableAlias(int position) const
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

bool KDbQuerySchema::hasTableAlias(int position) const
{
    return d->tableAliases.contains(position);
}

int KDbQuerySchema::columnPositionForAlias(const QString& name) const
{
    return d->columnPositionForAlias(name);
}

void KDbQuerySchema::setTableAlias(int position, const QString& alias)
{
    if (position >= d->tables.count()) {
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

const KDbField* KDbQuerySchema::field(KDbConnection *conn, const QString& identifier,
                                      ExpandMode mode) const
{
    KDbQueryColumnInfo *ci = columnInfo(conn, identifier, mode);
    return ci ? ci->field() : nullptr;
}

KDbField* KDbQuerySchema::field(KDbConnection *conn, const QString& identifier, ExpandMode mode)
{
    return const_cast<KDbField *>(
        static_cast<const KDbQuerySchema *>(this)->field(conn, identifier, mode));
}

KDbField* KDbQuerySchema::field(int id)
{
    return KDbFieldList::field(id);
}

const KDbField* KDbQuerySchema::field(int id) const
{
    return KDbFieldList::field(id);
}

KDbQueryColumnInfo *KDbQuerySchema::columnInfo(KDbConnection *conn, const QString &identifier,
                                               ExpandMode mode) const
{
    const KDbQuerySchemaFieldsExpanded *cache = computeFieldsExpanded(conn);
    return mode == ExpandMode::Expanded ? cache->columnInfosByNameExpanded.value(identifier)
                                        : cache->columnInfosByName.value(identifier);
}

KDbQueryColumnInfo::Vector KDbQuerySchema::fieldsExpandedInternal(
        KDbConnection *conn, FieldsExpandedMode mode, bool onlyVisible) const
{
    if (!conn) {
        kdbWarning() << "Connection required";
        return KDbQueryColumnInfo::Vector();
    }
    KDbQuerySchemaFieldsExpanded *cache = computeFieldsExpanded(conn);
    const KDbQueryColumnInfo::Vector *realFieldsExpanded
        = onlyVisible ? &cache->visibleFieldsExpanded : &cache->fieldsExpanded;
    if (mode == FieldsExpandedMode::WithInternalFields
        || mode == FieldsExpandedMode::WithInternalFieldsAndRecordId)
    {
        //a ref to a proper pointer (as we cache the vector for two cases)
        KDbQueryColumnInfo::Vector& tmpFieldsExpandedWithInternal =
            (mode == FieldsExpandedMode::WithInternalFields) ?
                (onlyVisible ? cache->visibleFieldsExpandedWithInternal : cache->fieldsExpandedWithInternal)
              : (onlyVisible ? cache->visibleFieldsExpandedWithInternalAndRecordId : cache->fieldsExpandedWithInternalAndRecordId);
        //special case
        if (tmpFieldsExpandedWithInternal.isEmpty()) {
            //glue expanded and internal fields and cache it
            const int internalFieldCount = cache->internalFields.size();
            const int fieldsExpandedVectorSize = realFieldsExpanded->size();
            const int size = fieldsExpandedVectorSize + internalFieldCount
                + ((mode == FieldsExpandedMode::WithInternalFieldsAndRecordId) ? 1 : 0) /*ROWID*/;
            tmpFieldsExpandedWithInternal.resize(size);
            for (int i = 0; i < fieldsExpandedVectorSize; ++i) {
                tmpFieldsExpandedWithInternal[i] = realFieldsExpanded->at(i);
            }
            if (internalFieldCount > 0) {
                for (int i = 0; i < internalFieldCount; ++i) {
                    KDbQueryColumnInfo *info = cache->internalFields[i];
                    tmpFieldsExpandedWithInternal[fieldsExpandedVectorSize + i] = info;
                }
            }
            if (mode == FieldsExpandedMode::WithInternalFieldsAndRecordId) {
                if (!d->fakeRecordIdField) {
                    d->fakeRecordIdField = new KDbField(QLatin1String("rowID"), KDbField::BigInteger);
                    d->fakeRecordIdCol = new KDbQueryColumnInfo(d->fakeRecordIdField, QString(), true);
                }
                tmpFieldsExpandedWithInternal[fieldsExpandedVectorSize + internalFieldCount] = d->fakeRecordIdCol;
            }
        }
        return tmpFieldsExpandedWithInternal;
    }

    if (mode == FieldsExpandedMode::Default) {
        return *realFieldsExpanded;
    }

    //mode == Unique:
    QSet<QString> columnsAlreadyFound;
    const int fieldsExpandedCount(realFieldsExpanded->count());
    KDbQueryColumnInfo::Vector result(fieldsExpandedCount);   //initial size is set
    //compute unique list
    int uniqueListCount = 0;
    for (int i = 0; i < fieldsExpandedCount; i++) {
        KDbQueryColumnInfo *ci = realFieldsExpanded->at(i);
        if (!columnsAlreadyFound.contains(ci->aliasOrName())) {
            columnsAlreadyFound.insert(ci->aliasOrName());
            result[uniqueListCount++] = ci;
        }
    }
    result.resize(uniqueListCount); //update result size
    return result;
}

KDbQueryColumnInfo::Vector KDbQuerySchema::internalFields(KDbConnection *conn) const
{
    KDbQuerySchemaFieldsExpanded *cache = computeFieldsExpanded(conn);
    return cache->internalFields;
}

KDbQueryColumnInfo* KDbQuerySchema::expandedOrInternalField(KDbConnection *conn, int index) const
{
    return fieldsExpanded(conn, FieldsExpandedMode::WithInternalFields).value(index);
}

inline static QString lookupColumnKey(KDbField *foreignField, KDbField* field)
{
    QString res;
    if (field->table()) // can be 0 for anonymous fields built as joined multiple visible columns
        res = field->table()->name() + QLatin1Char('.');
    return res + field->name() + QLatin1Char('_') + foreignField->table()->name()
               + QLatin1Char('.') + foreignField->name();
}

KDbQuerySchemaFieldsExpanded *KDbQuerySchema::computeFieldsExpanded(KDbConnection *conn) const
{
    KDbQuerySchemaFieldsExpanded *cache = conn->d->fieldsExpanded(this);
    if (cache) {
        return cache;
    }
    cache = new KDbQuerySchemaFieldsExpanded;
    QScopedPointer<KDbQuerySchemaFieldsExpanded> guard(cache);

    //collect all fields in a list (not a vector yet, because we do not know its size)
    KDbQueryColumnInfo::List list; //temporary
    KDbQueryColumnInfo::List lookup_list; //temporary, for collecting additional fields related to lookup fields
    QHash<KDbQueryColumnInfo*, bool> columnInfosOutsideAsterisks; //helper for filling d->columnInfosByName
    int i = 0;
    int numberOfColumnsWithMultipleVisibleFields = 0; //used to find an unique name for anonymous field
    int fieldPosition = -1;
    for (KDbField *f : *fields()) {
        fieldPosition++;
        if (f->isQueryAsterisk()) {
            if (static_cast<KDbQueryAsterisk*>(f)->isSingleTableAsterisk()) {
                const KDbField::List *ast_fields = static_cast<KDbQueryAsterisk*>(f)->table()->fields();
                foreach(KDbField *ast_f, *ast_fields) {
                    KDbQueryColumnInfo *ci = new KDbQueryColumnInfo(ast_f, QString()/*no field for asterisk!*/,
                            isColumnVisible(fieldPosition));
                    list.append(ci);
                    kdbDebug() << "caching (unexpanded) columns order:" << *ci << "at position" << fieldPosition;
                    cache->columnsOrder.insert(ci, fieldPosition);
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
                        cache->columnsOrder.insert(ci, fieldPosition);
                    }
                }
            }
        } else {
            //a single field
            KDbQueryColumnInfo *ci = new KDbQueryColumnInfo(f, columnAlias(fieldPosition), isColumnVisible(fieldPosition));
            list.append(ci);
            columnInfosOutsideAsterisks.insert(ci, true);
            kdbDebug() << "caching (unexpanded) column's order:" << *ci << "at position" << fieldPosition;
            cache->columnsOrder.insert(ci, fieldPosition);
            cache->columnsOrderWithoutAsterisks.insert(ci, fieldPosition);

            //handle lookup field schema
            KDbLookupFieldSchema *lookupFieldSchema = f->table() ? f->table()->lookupFieldSchema(*f) : nullptr;
            if (!lookupFieldSchema || lookupFieldSchema->boundColumn() < 0)
                continue;
            // Lookup field schema found:
            // Now we also need to fetch "visible" value from the lookup table, not only the value of binding.
            // -> build LEFT OUTER JOIN clause for this purpose (LEFT, not INNER because the binding can be broken)
            // "LEFT OUTER JOIN lookupTable ON thisTable.thisField=lookupTable.boundField"
            KDbLookupFieldSchemaRecordSource recordSource = lookupFieldSchema->recordSource();
            if (recordSource.type() == KDbLookupFieldSchemaRecordSource::Type::Table) {
                KDbTableSchema *lookupTable = conn->tableSchema(recordSource.name());
                KDbFieldList* visibleColumns = nullptr;
                KDbField *boundField = nullptr;
                if (lookupTable
                        && lookupFieldSchema->boundColumn() < lookupTable->fieldCount()
                        && (visibleColumns = lookupTable->subList(lookupFieldSchema->visibleColumns()))
                        && (boundField = lookupTable->field(lookupFieldSchema->boundColumn()))) {
                    KDbField *visibleColumn = nullptr;
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
                            KDbConstExpression(KDbToken::CHARACTER_STRING_LITERAL, QVariant()/*not important*/));
                        cache->ownedVisibleColumns.append(visibleColumn);   // remember to delete later
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
            } else if (recordSource.type() == KDbLookupFieldSchemaRecordSource::Type::Query) {
                KDbQuerySchema *lookupQuery = conn->querySchema(recordSource.name());
                if (!lookupQuery)
                    continue;
                const KDbQueryColumnInfo::Vector lookupQueryFieldsExpanded(
                    lookupQuery->fieldsExpanded(conn));
                if (lookupFieldSchema->boundColumn() >= lookupQueryFieldsExpanded.count())
                    continue;
                KDbQueryColumnInfo *boundColumnInfo = nullptr;
                if (!(boundColumnInfo = lookupQueryFieldsExpanded.value(lookupFieldSchema->boundColumn())))
                    continue;
                KDbField *boundField = boundColumnInfo->field();
                if (!boundField)
                    continue;
                const QList<int> visibleColumns(lookupFieldSchema->visibleColumns());
                bool ok = true;
                // all indices in visibleColumns should be in [0..lookupQueryFieldsExpanded.size()-1]
                foreach(int visibleColumn, visibleColumns) {
                    if (visibleColumn >= lookupQueryFieldsExpanded.count()) {
                        ok = false;
                        break;
                    }
                }
                if (!ok)
                    continue;
                KDbField *visibleColumn = nullptr;
                // for single visible column, just add it as-is
                if (visibleColumns.count() == 1) {
                    visibleColumn = lookupQueryFieldsExpanded.value(visibleColumns.first())->field();
                } else {
                    // for multiple visible columns, build an expression column
                    // (the expression object will be owned by column info)
                    visibleColumn = new KDbField();
                    visibleColumn->setName(
                        QString::fromLatin1("[multiple_visible_fields_%1]")
                        .arg(++numberOfColumnsWithMultipleVisibleFields));
                    visibleColumn->setExpression(
                        KDbConstExpression(KDbToken::CHARACTER_STRING_LITERAL, QVariant()/*not important*/));
                    cache->ownedVisibleColumns.append(visibleColumn);   // remember to delete later
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
    cache->fieldsExpanded.resize(list.count());
    cache->visibleFieldsExpanded.resize(list.count());

    /*fill (based on prepared 'list' and 'lookup_list'):
     -the vector
     -the map
     -"fields by name" dictionary
    */
    i = -1;
    int visibleIndex = -1;
    foreach(KDbQueryColumnInfo* ci, list) {
        i++;
        cache->fieldsExpanded[i] = ci;
        if (ci->isVisible()) {
            ++visibleIndex;
            cache->visibleFieldsExpanded[visibleIndex] = ci;
        }
        cache->columnsOrderExpanded.insert(ci, i);
        //remember field by name/alias/table.name if there's no such string yet in d->columnInfosByNameExpanded
        if (!ci->alias().isEmpty()) {
            //store alias and table.alias
            if (!cache->columnInfosByNameExpanded.contains(ci->alias())) {
                cache->columnInfosByNameExpanded.insert(ci->alias(), ci);
            }
            QString tableAndAlias(ci->alias());
            if (ci->field()->table())
                tableAndAlias.prepend(ci->field()->table()->name() + QLatin1Char('.'));
            if (!cache->columnInfosByNameExpanded.contains(tableAndAlias)) {
                cache->columnInfosByNameExpanded.insert(tableAndAlias, ci);
            }
            //the same for "unexpanded" list
            if (columnInfosOutsideAsterisks.contains(ci)) {
                if (!cache->columnInfosByName.contains(ci->alias())) {
                    cache->columnInfosByName.insert(ci->alias(), ci);
                }
                if (!cache->columnInfosByName.contains(tableAndAlias)) {
                    cache->columnInfosByName.insert(tableAndAlias, ci);
                }
            }
        } else {
            //no alias: store name and table.name
            if (!cache->columnInfosByNameExpanded.contains(ci->field()->name())) {
                cache->columnInfosByNameExpanded.insert(ci->field()->name(), ci);
            }
            QString tableAndName(ci->field()->name());
            if (ci->field()->table())
                tableAndName.prepend(ci->field()->table()->name() + QLatin1Char('.'));
            if (!cache->columnInfosByNameExpanded.contains(tableAndName)) {
                cache->columnInfosByNameExpanded.insert(tableAndName, ci);
            }
            //the same for "unexpanded" list
            if (columnInfosOutsideAsterisks.contains(ci)) {
                if (!cache->columnInfosByName.contains(ci->field()->name())) {
                    cache->columnInfosByName.insert(ci->field()->name(), ci);
                }
                if (!cache->columnInfosByName.contains(tableAndName)) {
                    cache->columnInfosByName.insert(tableAndName, ci);
                }
            }
        }
    }
    cache->visibleFieldsExpanded.resize(visibleIndex + 1);

    //remove duplicates for lookup fields
    QHash<QString, int> lookup_dict; //used to fight duplicates and to update KDbQueryColumnInfo::indexForVisibleLookupValue()
    // (a mapping from table.name string to int* lookupFieldIndex
    i = 0;
    for (QMutableListIterator<KDbQueryColumnInfo*> it(lookup_list); it.hasNext();) {
        KDbQueryColumnInfo* ci = it.next();
        const QString key(lookupColumnKey(ci->foreignColumn()->field(), ci->field()));
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
    cache->internalFields.resize(lookup_list.count());
    i = -1;
    foreach(KDbQueryColumnInfo *ci, lookup_list) {
        i++;
        //add it to the internal list
        cache->internalFields[i] = ci;
        cache->columnsOrderExpanded.insert(ci, list.count() + i);
    }

    //update KDbQueryColumnInfo::indexForVisibleLookupValue() cache for columns
    numberOfColumnsWithMultipleVisibleFields = 0;
    for (i = 0; i < cache->fieldsExpanded.size(); i++) {
        KDbQueryColumnInfo* ci = cache->fieldsExpanded[i];
//! @todo KDbQuerySchema itself will also support lookup fields...
        KDbLookupFieldSchema *lookupFieldSchema
            = ci->field()->table() ? ci->field()->table()->lookupFieldSchema(*ci->field()) : nullptr;
        if (!lookupFieldSchema || lookupFieldSchema->boundColumn() < 0)
            continue;
        const KDbLookupFieldSchemaRecordSource recordSource = lookupFieldSchema->recordSource();
        if (recordSource.type() == KDbLookupFieldSchemaRecordSource::Type::Table) {
            KDbTableSchema *lookupTable = conn->tableSchema(recordSource.name());
            KDbFieldList* visibleColumns = nullptr;
            if (lookupTable
                    && lookupFieldSchema->boundColumn() < lookupTable->fieldCount()
                    && (visibleColumns = lookupTable->subList(lookupFieldSchema->visibleColumns()))) {
                // for single visible column, just add it as-is
                if (visibleColumns->fieldCount() == 1) {
                    KDbField *visibleColumn = visibleColumns->fields()->first();
                    const QString key(lookupColumnKey(ci->field(), visibleColumn));
                    int index = lookup_dict.value(key, -99);
                    if (index != -99)
                        ci->setIndexForVisibleLookupValue(cache->fieldsExpanded.size() + index);
                } else {
                    const QString key(QString::fromLatin1("[multiple_visible_fields_%1]_%2.%3")
                                      .arg(++numberOfColumnsWithMultipleVisibleFields)
                                      .arg(ci->field()->table()->name(), ci->field()->name()));
                    int index = lookup_dict.value(key, -99);
                    if (index != -99)
                        ci->setIndexForVisibleLookupValue(cache->fieldsExpanded.size() + index);
                }
            }
            delete visibleColumns;
        } else if (recordSource.type() == KDbLookupFieldSchemaRecordSource::Type::Query) {
            KDbQuerySchema *lookupQuery = conn->querySchema(recordSource.name());
            if (!lookupQuery)
                continue;
            const KDbQueryColumnInfo::Vector lookupQueryFieldsExpanded(
                lookupQuery->fieldsExpanded(conn));
            if (lookupFieldSchema->boundColumn() >= lookupQueryFieldsExpanded.count())
                continue;
            KDbQueryColumnInfo *boundColumnInfo = nullptr;
            if (!(boundColumnInfo = lookupQueryFieldsExpanded.value(lookupFieldSchema->boundColumn())))
                continue;
            KDbField *boundField = boundColumnInfo->field();
            if (!boundField)
                continue;
            const QList<int> visibleColumns(lookupFieldSchema->visibleColumns());
            // for single visible column, just add it as-is
            if (visibleColumns.count() == 1) {
                if (lookupQueryFieldsExpanded.count() > visibleColumns.first()) { // sanity check
                    KDbField *visibleColumn = lookupQueryFieldsExpanded.at(visibleColumns.first())->field();
                    const QString key(lookupColumnKey(ci->field(), visibleColumn));
                    int index = lookup_dict.value(key, -99);
                    if (index != -99)
                        ci->setIndexForVisibleLookupValue(cache->fieldsExpanded.size() + index);
                }
            } else {
                const QString key(QString::fromLatin1("[multiple_visible_fields_%1]_%2.%3")
                                  .arg(++numberOfColumnsWithMultipleVisibleFields)
                                  .arg(ci->field()->table()->name(), ci->field()->name()));
                int index = lookup_dict.value(key, -99);
                if (index != -99)
                    ci->setIndexForVisibleLookupValue(cache->fieldsExpanded.size() + index);
            }
        } else {
            kdbWarning() << "unsupported record source type" << recordSource.typeName();
        }
    }
    if (d->recentConnection != conn) {
        if (d->recentConnection) {
            // connection changed: remove old cache
            d->recentConnection->d->insertFieldsExpanded(this, nullptr);
        }
        d->recentConnection = conn;
    }
    conn->d->insertFieldsExpanded(this, guard.take());
    return cache;
}

QHash<KDbQueryColumnInfo*, int> KDbQuerySchema::columnsOrder(KDbConnection *conn,
                                                             ColumnsOrderMode mode) const
{
    KDbQuerySchemaFieldsExpanded *cache = computeFieldsExpanded(conn);
    if (mode == ColumnsOrderMode::UnexpandedList) {
        return cache->columnsOrder;
    } else if (mode == ColumnsOrderMode::UnexpandedListWithoutAsterisks) {
        return cache->columnsOrderWithoutAsterisks;
    }
    return cache->columnsOrderExpanded;
}

QVector<int> KDbQuerySchema::pkeyFieldsOrder(KDbConnection *conn) const
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

    d->pkeyFieldCount = 0;
    const KDbQueryColumnInfo::Vector fieldsExpanded(this->fieldsExpanded(conn));
    const int fCount = fieldsExpanded.count();
    for (int i = 0; i < fCount; i++) {
        const KDbQueryColumnInfo *fi = fieldsExpanded[i];
        const int fieldIndex = fi->field()->table() == tbl ? pkey->indexOf(*fi->field()) : -1;
        if (fieldIndex != -1/* field found in PK */
                && d->pkeyFieldsOrder->at(fieldIndex) == -1 /* first time */) {
            kdbDebug() << "FIELD" << fi->field()->name() << "IS IN PKEY AT POSITION #" << fieldIndex;
            (*d->pkeyFieldsOrder)[fieldIndex] = i;
            d->pkeyFieldCount++;
        }
    }
    kdbDebug() << d->pkeyFieldCount
    << " OUT OF " << pkey->fieldCount() << " PKEY'S FIELDS FOUND IN QUERY " << name();
    return *d->pkeyFieldsOrder;
}

int KDbQuerySchema::pkeyFieldCount(KDbConnection *conn)
{
    (void)pkeyFieldsOrder(conn); /* rebuild information */
    return d->pkeyFieldCount;
}

KDbRelationship* KDbQuerySchema::addRelationship(KDbField *field1, KDbField *field2)
{
//@todo: find existing global db relationships
    KDbRelationship *r = new KDbRelationship(this, field1, field2);
    if (r->isEmpty()) {
        delete r;
        return nullptr;
    }

    d->relations.append(r);
    return r;
}

KDbQueryColumnInfo::List* KDbQuerySchema::autoIncrementFields(KDbConnection *conn) const
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
        const KDbQueryColumnInfo::Vector fieldsExpanded(this->fieldsExpanded(conn));
        for (int i = 0; i < fieldsExpanded.count(); i++) {
            KDbQueryColumnInfo *ci = fieldsExpanded[i];
            if (ci->field()->table() == mt && ci->field()->isAutoIncrement()) {
                d->autoincFields->append(ci);
            }
        }
    }
    return d->autoincFields;
}

// static
KDbEscapedString KDbQuerySchema::sqlColumnsList(const KDbQueryColumnInfo::List &infolist,
                                                KDbConnection *conn,
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
        result += escapeIdentifier(ci->field()->name(), conn, escapingType);
    }
    return result;
}

KDbEscapedString KDbQuerySchema::autoIncrementSqlFieldsList(KDbConnection *conn) const
{
//    QWeakPointer<const KDbDriver> driverWeakPointer
//            = DriverManagerInternal::self()->driverWeakPointer(*conn->driver());
    if (   /*d->lastUsedDriverForAutoIncrementSQLFieldsList != driverWeakPointer
        ||*/ d->autoIncrementSqlFieldsList.isEmpty())
    {
        d->autoIncrementSqlFieldsList = KDbQuerySchema::sqlColumnsList(*autoIncrementFields(conn), conn);
        //d->lastUsedDriverForAutoIncrementSQLFieldsList = driverWeakPointer;
    }
    return d->autoIncrementSqlFieldsList;
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

bool KDbQuerySchema::setWhereExpression(const KDbExpression &expr, QString *errorMessage,
                                        QString *errorDescription)
{
    KDbExpression newWhereExpr = expr.clone();
    KDbParseInfoInternal parseInfo(this);
    QString tempErrorMessage;
    QString tempErrorDescription;
    QString *errorMessagePointer = errorMessage ? errorMessage : &tempErrorMessage;
    QString *errorDescriptionPointer
        = errorDescription ? errorDescription : &tempErrorDescription;
    if (!newWhereExpr.validate(&parseInfo)) {
        setResult(parseInfo, errorMessagePointer, errorDescription);
        kdbWarning() << "message=" << *errorMessagePointer
                     << "description=" << *errorDescriptionPointer;
        kdbWarning() << newWhereExpr;
        d->whereExpr = KDbExpression();
        return false;
    }
    errorMessagePointer->clear();
    errorDescriptionPointer->clear();
    KDbQuerySchemaPrivate::setWhereExpressionInternal(this, newWhereExpr);
    return true;
}

bool KDbQuerySchema::addToWhereExpression(KDbField *field, const QVariant &value,
                                          KDbToken relation, QString *errorMessage,
                                          QString *errorDescription)
{
    KDbToken token;
    if (value.isNull()) {
        token = KDbToken::SQL_NULL;
    } else {
        const KDbField::Type type = field->type(); // cache: evaluating type of expressions can be expensive
        if (KDbField::isIntegerType(type)) {
            token = KDbToken::INTEGER_CONST;
        } else if (KDbField::isFPNumericType(type)) {
            token = KDbToken::REAL_CONST;
        } else {
            token = KDbToken::CHARACTER_STRING_LITERAL;
        }
//! @todo date, time
    }

    KDbBinaryExpression newExpr(
        KDbConstExpression(token, value),
        relation,
        KDbVariableExpression((field->table() ? (field->table()->name() + QLatin1Char('.')) : QString()) + field->name())
    );
    const KDbExpression origWhereExpr = d->whereExpr;
    if (!d->whereExpr.isNull()) {
        newExpr = KDbBinaryExpression(
            d->whereExpr,
            KDbToken::AND,
            newExpr
        );
    }
    const bool result = setWhereExpression(newExpr, errorMessage, errorDescription);
    if (!result) { // revert, setWhereExpression() cleared it
        d->whereExpr = origWhereExpr;
    }
    return result;
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
    d->orderByColumnList = new KDbOrderByColumnList(list, nullptr, nullptr, nullptr);
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

QList<KDbQuerySchemaParameter> KDbQuerySchema::parameters(KDbConnection *conn) const
{
    QList<KDbQuerySchemaParameter> params;
    const KDbQueryColumnInfo::Vector fieldsExpanded(this->fieldsExpanded(conn));
    for (int i = 0; i < fieldsExpanded.count(); ++i) {
        KDbQueryColumnInfo *ci = fieldsExpanded[i];
        if (!ci->field()->expression().isNull()) {
            ci->field()->expression().getQueryParameters(&params);
        }
    }
    KDbExpression where = whereExpression();
    if (!where.isNull()) {
        where.getQueryParameters(&params);
    }
    return params;
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

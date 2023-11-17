/* This file is part of the KDE project
   Copyright (C) 2003-2018 Jarosław Staniek <staniek@kde.org>

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

#include "KDbOrderByColumn.h"
#include "KDbQuerySchema.h"
#include "KDbQuerySchema_p.h"
#include "KDbConnection.h"
#include "kdb_debug.h"

class Q_DECL_HIDDEN KDbOrderByColumn::Private
{
public:
    Private()
        : columnIndex(-1)
        , pos(-1)
        , field(nullptr)
        , order(KDbOrderByColumn::SortOrder::Ascending)
    {
    }
    Private(const Private &other) {
        copy(other);
    }
#define KDbOrderByColumnPrivateArgs(o) std::tie(o.querySchema, o.connection, o.columnIndex, o.pos, o.field, o.order)
    Private(KDbQueryColumnInfo* aColumn, int aPos, KDbField* aField, KDbOrderByColumn::SortOrder aOrder)
    {
        const KDbQuerySchema *foundQuerySchema = nullptr;
        KDbConnection *foundConnection = nullptr;
        int foundColumnIndex = -1;
        if (aColumn) {
            foundQuerySchema =aColumn->querySchema();
            foundConnection = aColumn->connection();
            const KDbQueryColumnInfo::Vector fieldsExpanded = foundQuerySchema->fieldsExpanded(
                    foundConnection, KDbQuerySchema::FieldsExpandedMode::WithInternalFields);
            foundColumnIndex = fieldsExpanded.indexOf(aColumn);
            if (foundColumnIndex < 0) {
                kdbWarning() << "Column not found in query:" << *aColumn;
            }
        }
        KDbOrderByColumnPrivateArgs((*this))
            = std::tie(foundQuerySchema, foundConnection, foundColumnIndex, aPos, aField, aOrder);
    }
    void copy(const Private &other) {
        KDbOrderByColumnPrivateArgs((*this)) = KDbOrderByColumnPrivateArgs(other);
    }
    bool operator==(const Private &other) const {
        return KDbOrderByColumnPrivateArgs((*this)) == KDbOrderByColumnPrivateArgs(other);
    }

    //! Query schema that owns the KDbQueryColumnInfo and thus also this KDbOrderByColumn object.
    //! Cached for performance, can be cached since lifetime of the KDbOrderByColumn object depends
    //! on the query. @c nullptr if columnIndex is not provided. @since 3.2
    const KDbQuerySchema *querySchema = nullptr;

    //! Connection used to compute expanded fields. Like querySchema, connection is cached for
    //! performance and can be cached since lifetime of the KDbOrderByColumn object depends on the
    //! connection. @c nullptr if columnIndex is not provided. @since 3.2
    KDbConnection *connection = nullptr;

    //! Index of column to sort, -1 if field is present. @since 3.2
    int columnIndex;

    //! Value that indicates that column to sort (columnIndex) has been specified by providing its
    //! position, not name. For example, using "SELECT a, b FROM T ORDER BY 2".
    //! Value of -1 means that the column to sort has been specified by providing its name (or alias).
    //! For example "SELECT a, b FROM T ORDER BY b". -1 is the default.
    int pos;

    //! Used only in case when the second constructor is used.
    KDbField* field;

    //! Sort order
    KDbOrderByColumn::SortOrder order;
};

//----

KDbOrderByColumn::KDbOrderByColumn()
    : d(new Private)
{
}

KDbOrderByColumn::KDbOrderByColumn(KDbQueryColumnInfo* column, SortOrder order, int pos)
    : d(new Private(column, pos, nullptr, order))
{
}

KDbOrderByColumn::KDbOrderByColumn(KDbField* field, SortOrder order)
    : d(new Private(nullptr, -1, field, order))
{
}

KDbOrderByColumn::KDbOrderByColumn(const KDbOrderByColumn &other)
    : d(new Private(*other.d))
{
}

KDbOrderByColumn::~KDbOrderByColumn()
{
    delete d;
}

KDbOrderByColumn *KDbOrderByColumn::copy(KDbConnection *conn, KDbQuerySchema *fromQuery,
                                         KDbQuerySchema *toQuery) const
{
    if (d->field) {
        return new KDbOrderByColumn(d->field, d->order);
    }
    if (d->columnIndex >= 0) {
        KDbQueryColumnInfo* columnInfo;
        if (fromQuery && toQuery) {
            columnInfo = toQuery->expandedOrInternalField(conn, d->columnIndex);
            if (!columnInfo) {
                kdbWarning() << "Column info not found at index" << d->columnIndex << "in toQuery";
                return nullptr;
            }
        }
        else {
            columnInfo = column();
        }
        return new KDbOrderByColumn(columnInfo, d->order, d->pos);
    }
    return nullptr;
}

KDbQueryColumnInfo* KDbOrderByColumn::column() const
{
    if (d->columnIndex < 0 || !d->querySchema || !d->connection) {
        return nullptr;
    }
    return d->querySchema->expandedOrInternalField(d->connection, d->columnIndex);
}

int KDbOrderByColumn::position() const
{
    return d->pos;
}

KDbField* KDbOrderByColumn::field() const
{
    return d->field;
}

KDbOrderByColumn::SortOrder KDbOrderByColumn::sortOrder() const
{
    return d->order;
}

KDbOrderByColumn& KDbOrderByColumn::operator=(const KDbOrderByColumn &other)
{
    if (this != &other) {
        *d = *other.d;
    }
    return *this;
}

bool KDbOrderByColumn::operator==(const KDbOrderByColumn& col) const
{
    return *d == *col.d;
}

QDebug operator<<(QDebug dbg, const KDbOrderByColumn& order)
{
    const QLatin1String orderString(
        order.sortOrder() == KDbOrderByColumn::SortOrder::Ascending ? "ASCENDING" : "DESCENDING");
    if (order.column()) {
        if (order.position() > -1) {
            dbg.nospace() << qPrintable(QString::fromLatin1("COLUMN_AT_POSITION_%1(").arg(order.position() + 1))
                          << *order.column() << ','
                          << qPrintable(orderString) << ')';
            return dbg.space();
        }
        else {
            dbg.nospace() << "COLUMN(" << *order.column() << ',';
            dbg.nospace() << qPrintable(orderString) << ')';
            return dbg.space();
        }
    }
    if (order.field()) {
        dbg.nospace() << "FIELD(" << *order.field() << ',';
        dbg.nospace() << qPrintable(orderString) << ')';
        return dbg.space();
    }
    dbg.nospace() << "NONE";
    return dbg.space();
}

KDbEscapedString KDbOrderByColumn::toSqlString(bool includeTableName,
                                               KDbConnection *conn,
                                               KDbQuerySchema *query,
                                               KDb::IdentifierEscapingType escapingType) const
{
    const QByteArray orderString(d->order == KDbOrderByColumn::SortOrder::Ascending ? "" : " DESC");
    KDbEscapedString fieldName, tableName, collationString;
    KDbQueryColumnInfo *col = column();
    if (col) {
        if (d->pos > -1)
            return KDbEscapedString::number(d->pos + 1) + orderString;
        else {
            if (includeTableName && col->field()->table() && col->alias().isEmpty()) {
                tableName = KDbEscapedString(escapeIdentifier(col->field()->table()->name(), conn, escapingType));
                tableName += '.';
            }
            fieldName = KDbEscapedString(escapeIdentifier(col->aliasOrName(), conn, escapingType));
        }
        if (conn && col->field()->isTextType() && escapingType == KDb::DriverEscaping) {
            collationString = conn->driver()->collationSql();
        }
    }
    else {
        QString aliasOrName;
        if (includeTableName && d->field && d->field->table()) {
            tableName = KDbEscapedString(escapeIdentifier(d->field->table()->name(), conn, escapingType));
            tableName += '.';
        } else if (d->field && conn && query) {
            if (d->field->isExpression()) {
                const int indexOfField = query->indexOf(*d->field);
                aliasOrName = query->columnAlias(indexOfField);
                if (aliasOrName.isEmpty()) {
                    kdbWarning() << "This field does not belong to specified query:" << *d->field
                                 << Qt::endl << "cannot find alias";
                    aliasOrName = QLatin1String("?unknown_field?");
                }
            } else {
                KDbQueryColumnInfo *ci = query->columnInfo(conn, d->field->name());
                if (ci) {
                    aliasOrName = ci->aliasOrName();
                }
            }
        }
        if (aliasOrName.isEmpty()) {
            // The field is not present on the SELECT list but is still correct,
            // e.g. SELECT id FROM cars ORDER BY owner
            aliasOrName = d->field ? d->field->name() : QLatin1String("?missing_field?")/*error*/;
        }
        fieldName = KDbEscapedString(escapeIdentifier(aliasOrName, conn, escapingType));
        if (conn && d->field && d->field->isTextType() && escapingType == KDb::DriverEscaping) {
            collationString = conn->driver()->collationSql();
        }
    }
    return tableName + fieldName + collationString + orderString;
}

KDbEscapedString KDbOrderByColumn::toSqlString(bool includeTableName,
                                               KDbConnection *conn,
                                               KDb::IdentifierEscapingType escapingType) const
{
    return toSqlString(includeTableName, conn, nullptr, escapingType);
}

//=======================================

class Q_DECL_HIDDEN KDbOrderByColumnList::Private
{
public:
    Private() {
    }
    ~Private() {
        qDeleteAll(data);
    }
    QList<KDbOrderByColumn*> data;
};

KDbOrderByColumnList::KDbOrderByColumnList()
        : d(new Private)
{
}

KDbOrderByColumnList::KDbOrderByColumnList(const KDbOrderByColumnList& other, KDbConnection *conn,
                                           KDbQuerySchema* fromQuery, KDbQuerySchema* toQuery)
        : KDbOrderByColumnList()
{
    for (QList<KDbOrderByColumn *>::ConstIterator it(other.constBegin()); it != other.constEnd();
         ++it)
    {
        KDbOrderByColumn* order = (*it)->copy(conn, fromQuery, toQuery);
        if (order) {
            d->data.append(order);
        }
    }
}

KDbOrderByColumnList::~KDbOrderByColumnList()
{
    delete d;
}

bool KDbOrderByColumnList::operator==(const KDbOrderByColumnList &other) const
{
    return d->data == other.d->data;
}

const KDbOrderByColumn* KDbOrderByColumnList::value(int index) const
{
    return d->data.value(index);
}

KDbOrderByColumn* KDbOrderByColumnList::value(int index)
{
    return d->data.value(index);
}

bool KDbOrderByColumnList::appendFields(KDbConnection *conn, KDbQuerySchema* querySchema,
                                        const QString& field1, KDbOrderByColumn::SortOrder order1,
                                        const QString& field2, KDbOrderByColumn::SortOrder order2,
                                        const QString& field3, KDbOrderByColumn::SortOrder order3,
                                        const QString& field4, KDbOrderByColumn::SortOrder order4,
                                        const QString& field5, KDbOrderByColumn::SortOrder order5)
{
    if (!querySchema) {
        return false;
    }
    int numAdded = 0;
#define ADD_COL(fieldName, order) \
    if (ok && !fieldName.isEmpty()) { \
        if (!appendField(conn, querySchema, fieldName, order)) \
            ok = false; \
        else \
            numAdded++; \
    }
    bool ok = true;
    ADD_COL(field1, order1)
    ADD_COL(field2, order2)
    ADD_COL(field3, order3)
    ADD_COL(field4, order4)
    ADD_COL(field5, order5)
#undef ADD_COL
    if (ok) {
        return true;
    }
    for (int i = 0; i < numAdded; i++) {
        d->data.removeLast();
    }
    return false;
}

void KDbOrderByColumnList::appendColumn(KDbQueryColumnInfo* columnInfo,
                                        KDbOrderByColumn::SortOrder order)
{
    if (columnInfo) {
        d->data.append(new KDbOrderByColumn(columnInfo, order));
    }
}

bool KDbOrderByColumnList::appendColumn(KDbConnection *conn, KDbQuerySchema* querySchema,
                                        KDbOrderByColumn::SortOrder order, int pos)
{
    if (!querySchema) {
        return false;
    }
    const KDbQueryColumnInfo::Vector fieldsExpanded(querySchema->fieldsExpanded(conn));
    if (pos < 0 || pos >= fieldsExpanded.size()) {
        return false;
    }
    KDbQueryColumnInfo* ci = fieldsExpanded[pos];
    d->data.append(new KDbOrderByColumn(ci, order, pos));
    return true;
}

void KDbOrderByColumnList::appendField(KDbField* field, KDbOrderByColumn::SortOrder order)
{
    if (field) {
        d->data.append(new KDbOrderByColumn(field, order));
    }
}

bool KDbOrderByColumnList::appendField(KDbConnection *conn, KDbQuerySchema* querySchema,
                                       const QString& fieldName, KDbOrderByColumn::SortOrder order)
{
    if (!querySchema) {
        return false;
    }
    KDbQueryColumnInfo *columnInfo = querySchema->columnInfo(conn, fieldName);
    if (columnInfo) {
        d->data.append(new KDbOrderByColumn(columnInfo, order));
        return true;
    }
    KDbField *field = querySchema->findTableField(fieldName);
    if (field) {
        d->data.append(new KDbOrderByColumn(field, order));
        return true;
    }
    kdbWarning() << "no such field" << fieldName;
    return false;
}

bool KDbOrderByColumnList::isEmpty() const
{
    return d->data.isEmpty();
}

int KDbOrderByColumnList::count() const
{
    return d->data.count();
}

QList<KDbOrderByColumn*>::Iterator KDbOrderByColumnList::begin()
{
    return d->data.begin();
}

QList<KDbOrderByColumn*>::Iterator KDbOrderByColumnList::end()
{
    return d->data.end();
}

QList<KDbOrderByColumn*>::ConstIterator KDbOrderByColumnList::constBegin() const
{
    return d->data.constBegin();
}

QList<KDbOrderByColumn*>::ConstIterator KDbOrderByColumnList::constEnd() const
{
    return d->data.constEnd();
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

KDbEscapedString KDbOrderByColumnList::toSqlString(bool includeTableNames, KDbConnection *conn,
                                                   KDbQuerySchema *query,
                                                   KDb::IdentifierEscapingType escapingType) const
{
    KDbEscapedString string;
    for (QList<KDbOrderByColumn*>::ConstIterator it(constBegin()); it != constEnd(); ++it) {
        if (!string.isEmpty())
            string += ", ";
        string += (*it)->toSqlString(includeTableNames, conn, query, escapingType);
    }
    return string;
}

KDbEscapedString KDbOrderByColumnList::toSqlString(bool includeTableNames, KDbConnection *conn,
                                                   KDb::IdentifierEscapingType escapingType) const
{
    return toSqlString(includeTableNames, conn, nullptr, escapingType);
}

void KDbOrderByColumnList::clear()
{
    qDeleteAll(d->data);
    d->data.clear();
}

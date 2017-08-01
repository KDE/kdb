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

#include "KDbOrderByColumn.h"
#include "KDbQuerySchema.h"
#include "KDbQuerySchema_p.h"
#include "KDbConnection.h"
#include "kdb_debug.h"

class Q_DECL_HIDDEN KDbOrderByColumn::Private
{
public:
    Private()
        : column(nullptr)
        , pos(-1)
        , field(nullptr)
        , order(KDbOrderByColumn::SortOrder::Ascending)
    {
    }
    Private(const Private &other) {
        copy(other);
    }
#define KDbOrderByColumnPrivateArgs(o) std::tie(o.column, o.pos, o.field, o.order)
    Private(KDbQueryColumnInfo* aColumn, int aPos, KDbField* aField, KDbOrderByColumn::SortOrder aOrder) {
        KDbOrderByColumnPrivateArgs((*this)) = std::tie(aColumn, aPos, aField, aOrder);
    }
    void copy(const Private &other) {
        KDbOrderByColumnPrivateArgs((*this)) = KDbOrderByColumnPrivateArgs(other);
    }
    bool operator==(const Private &other) const {
        return KDbOrderByColumnPrivateArgs((*this)) == KDbOrderByColumnPrivateArgs(other);
    }

    //! Column to sort, @c nullptr if field is non-0.
    KDbQueryColumnInfo* column;
    //! A helper for d->column that allows to know that sorting column
    //! was defined by providing its position. -1 by default.
    //! e.g. SELECT a, b FROM T ORDER BY 2
    int pos;
    //! Used only in case when the second contructor is used.
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
    if (d->column) {
        KDbQueryColumnInfo* columnInfo;
        if (fromQuery && toQuery) {
            int columnIndex = fromQuery->columnsOrder(conn).value(d->column);
            if (columnIndex < 0) {
                kdbWarning() << "Index not found for column" << *d->column;
                return nullptr;
            }
            columnInfo = toQuery->expandedOrInternalField(conn, columnIndex);
            if (!columnInfo) {
                kdbWarning() << "Column info not found at index" << columnIndex << "in toQuery";
                return nullptr;
            }
        }
        else {
            columnInfo = d->column;
        }
        return new KDbOrderByColumn(columnInfo, d->order, d->pos);
    }
    return nullptr;
}

KDbQueryColumnInfo* KDbOrderByColumn::column() const
{
    return d->column;
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

KDbEscapedString KDbOrderByColumn::toSqlString(bool includeTableName,
                                               KDbConnection *conn,
                                               KDb::IdentifierEscapingType escapingType) const
{
    const QByteArray orderString(d->order == KDbOrderByColumn::SortOrder::Ascending ? "" : " DESC");
    KDbEscapedString fieldName, tableName, collationString;
    if (d->column) {
        if (d->pos > -1)
            return KDbEscapedString::number(d->pos + 1) + orderString;
        else {
            if (includeTableName && d->column->alias().isEmpty()) {
                tableName = KDbEscapedString(escapeIdentifier(d->column->field()->table()->name(), conn, escapingType));
                tableName += '.';
            }
            fieldName = KDbEscapedString(escapeIdentifier(d->column->aliasOrName(), conn, escapingType));
        }
        if (d->column->field()->isTextType()) {
            collationString = conn->driver()->collationSql();
        }
    }
    else {
        if (d->field && includeTableName) {
            tableName = KDbEscapedString(escapeIdentifier(d->field->table()->name(), conn, escapingType));
            tableName += '.';
        }
        fieldName = KDbEscapedString(escapeIdentifier(
            d->field ? d->field->name() : QLatin1String("??")/*error*/, conn, escapingType));
        if (d->field && d->field->isTextType()) {
            collationString = conn->driver()->collationSql();
        }
    }
    return tableName + fieldName + collationString + orderString;
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
    KDbQueryColumnInfo::Vector fieldsExpanded(querySchema->fieldsExpanded(conn));
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
                                       KDb::IdentifierEscapingType escapingType) const
{
    KDbEscapedString string;
    for (QList<KDbOrderByColumn*>::ConstIterator it(constBegin()); it != constEnd(); ++it) {
        if (!string.isEmpty())
            string += ", ";
        string += (*it)->toSqlString(includeTableNames, conn, escapingType);
    }
    return string;
}

void KDbOrderByColumnList::clear()
{
    qDeleteAll(d->data);
    d->data.clear();
}

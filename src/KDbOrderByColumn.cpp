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
    Q_ASSERT(column);
}

KDbOrderByColumn::KDbOrderByColumn(KDbField* field, SortOrder order)
    : d(new Private(nullptr, -1, field, order))
{
    Q_ASSERT(field);
}

KDbOrderByColumn::KDbOrderByColumn(const KDbOrderByColumn &other)
    : d(new Private(*other.d))
{
}

KDbOrderByColumn::~KDbOrderByColumn()
{
    delete d;
}

KDbOrderByColumn* KDbOrderByColumn::copy(KDbQuerySchema* fromQuery, KDbQuerySchema* toQuery) const
{
    if (d->field) {
        return new KDbOrderByColumn(d->field, d->order);
    }
    if (d->column) {
        KDbQueryColumnInfo* columnInfo;
        if (fromQuery && toQuery) {
            int columnIndex = fromQuery->columnsOrder().value(d->column);
            if (columnIndex < 0) {
                kdbWarning() << "Index not found for column" << *d->column;
                return nullptr;
            }
            columnInfo = toQuery->expandedOrInternalField(columnIndex);
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
    Q_ASSERT(d->field || d->column);
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

KDbEscapedString KDbOrderByColumn::toSQLString(bool includeTableName,
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
            collationString = conn->driver()->collationSQL();
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

bool KDbOrderByColumnList::appendFields(KDbQuerySchema* querySchema,
                                        const QString& field1, KDbOrderByColumn::SortOrder order1,
                                        const QString& field2, KDbOrderByColumn::SortOrder order2,
                                        const QString& field3, KDbOrderByColumn::SortOrder order3,
                                        const QString& field4, KDbOrderByColumn::SortOrder order4,
                                        const QString& field5, KDbOrderByColumn::SortOrder order5)
{
    Q_ASSERT(querySchema);
    int numAdded = 0;
#define ADD_COL(fieldName, order) \
    if (ok && !fieldName.isEmpty()) { \
        if (!appendField( querySchema, fieldName, order )) \
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
        removeLast();
    }
    return false;
}

KDbOrderByColumnList::~KDbOrderByColumnList()
{
    qDeleteAll(begin(), end());
}

void KDbOrderByColumnList::appendColumn(KDbQueryColumnInfo* columnInfo,
                                        KDbOrderByColumn::SortOrder order)
{
    Q_ASSERT(columnInfo);
    append(new KDbOrderByColumn(columnInfo, order));
}

bool KDbOrderByColumnList::appendColumn(KDbQuerySchema* querySchema,
                                        KDbOrderByColumn::SortOrder order, int pos)
{
    Q_ASSERT(querySchema);
    KDbQueryColumnInfo::Vector fieldsExpanded(querySchema->fieldsExpanded());
    if (pos < 0 || pos >= fieldsExpanded.size()) {
        return false;
    }
    KDbQueryColumnInfo* ci = fieldsExpanded[pos];
    append(new KDbOrderByColumn(ci, order, pos));
    return true;
}

void KDbOrderByColumnList::appendField(KDbField* field, KDbOrderByColumn::SortOrder order)
{
    Q_ASSERT(field);
    append(new KDbOrderByColumn(field, order));
}

bool KDbOrderByColumnList::appendField(KDbQuerySchema* querySchema,
                                       const QString& fieldName, KDbOrderByColumn::SortOrder order)
{
    Q_ASSERT(querySchema);
    KDbQueryColumnInfo *columnInfo = querySchema->columnInfo(fieldName);
    if (columnInfo) {
        append(new KDbOrderByColumn(columnInfo, order));
        return true;
    }
    KDbField *field = querySchema->findTableField(fieldName);
    if (field) {
        append(new KDbOrderByColumn(field, order));
        return true;
    }
    kdbWarning() << "no such field" << fieldName;
    return false;
}

bool KDbOrderByColumnList::isEmpty() const
{
    return QList<KDbOrderByColumn*>::isEmpty();
}

int KDbOrderByColumnList::count() const
{
    return QList<KDbOrderByColumn*>::count();
}

KDbOrderByColumnList::iterator KDbOrderByColumnList::begin()
{
    return QList<KDbOrderByColumn*>::begin();
}

KDbOrderByColumnList::iterator KDbOrderByColumnList::end()
{
    return QList<KDbOrderByColumn*>::end();
}

KDbOrderByColumnList::const_iterator KDbOrderByColumnList::constBegin() const
{
    return QList<KDbOrderByColumn*>::constBegin();
}

KDbOrderByColumnList::const_iterator KDbOrderByColumnList::constEnd() const
{
    return QList<KDbOrderByColumn*>::constEnd();
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

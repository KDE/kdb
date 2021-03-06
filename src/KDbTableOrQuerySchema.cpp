/* This file is part of the KDE project
   Copyright (C) 2004-2015 Jarosław Staniek <staniek@kde.org>
   Copyright (c) 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1997 Matthias Kalle Dalheimer <kalle@kde.org>

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

#include "KDbTableOrQuerySchema.h"
#include "KDbConnection.h"
#include "KDbQuerySchema.h"
#include "kdb_debug.h"

class Q_DECL_HIDDEN KDbTableOrQuerySchema::Private
{
public:
    Private() {}
    //! The name is kept here because m_table and m_table can be 0
    //! and we still want name() and acptionOrName() work.
    QByteArray name;

    KDbTableSchema* table;

    KDbQuerySchema* query;
private:
    Q_DISABLE_COPY(Private)
};

KDbTableOrQuerySchema::KDbTableOrQuerySchema(KDbConnection *conn, const QByteArray& name)
        : d(new Private)
{
    d->name = name;
    d->table = conn->tableSchema(QLatin1String(name));
    d->query = d->table ? nullptr : conn->querySchema(QLatin1String(name));
    if (!d->table && !d->query) {
        kdbWarning() << "tableOrQuery is neither table nor query!";
    }
}

KDbTableOrQuerySchema::KDbTableOrQuerySchema(KDbConnection *conn, const QByteArray& name, Type type)
        : d(new Private)
{
    d->name = name;
    d->table = type == Type::Table ? conn->tableSchema(QLatin1String(name)) : nullptr;
    d->query = type == Type::Query ? conn->querySchema(QLatin1String(name)) : nullptr;
    if (type == Type::Table && !d->table) {
        kdbWarning() << "no table specified!";
    }
    if (type == Type::Query && !d->query) {
        kdbWarning() << "no query specified!";
    }
}

KDbTableOrQuerySchema::KDbTableOrQuerySchema(KDbFieldList *tableOrQuery)
    : d(new Private)
{
    d->table = dynamic_cast<KDbTableSchema*>(tableOrQuery);
    d->query = dynamic_cast<KDbQuerySchema*>(tableOrQuery);
    if (!d->table && !d->query) {
        kdbWarning() << "tableOrQuery is neither table nor query!";
    }
}

KDbTableOrQuerySchema::KDbTableOrQuerySchema(KDbConnection *conn, int id)
    : d(new Private)
{
    d->table = conn->tableSchema(id);
    d->query = d->table ? nullptr : conn->querySchema(id);
    if (!d->table && !d->query) {
        kdbWarning() << "no table or query found for id==" << id;
    }
}

KDbTableOrQuerySchema::KDbTableOrQuerySchema(KDbTableSchema* table)
    : d(new Private)
{
    d->table = table;
    d->query = nullptr;
    if (!d->table) {
        kdbWarning() << "no table specified!";
    }
}

KDbTableOrQuerySchema::KDbTableOrQuerySchema(KDbQuerySchema* query)
    : d(new Private)
{
    d->table = nullptr;
    d->query = query;
    if (!d->query) {
        kdbWarning() << "no query specified!";
    }
}

KDbTableOrQuerySchema::~KDbTableOrQuerySchema()
{
    delete d;
}

int KDbTableOrQuerySchema::fieldCount(KDbConnection *conn) const
{
    if (d->table)
        return d->table->fieldCount();
    if (d->query && conn)
        return d->query->fieldsExpanded(conn).size();
    return -1;
}

const KDbQueryColumnInfo::Vector KDbTableOrQuerySchema::columns(KDbConnection * conn, ColumnsMode mode)
{
    if (d->table) {
        return d->table->query()->fieldsExpanded(conn, mode == ColumnsMode::Unique
                                                     ? KDbQuerySchema::FieldsExpandedMode::Unique
                                                     : KDbQuerySchema::FieldsExpandedMode::Default);
    }
    if (d->query) {
        return d->query->fieldsExpanded(conn, mode == ColumnsMode::Unique
                                            ? KDbQuerySchema::FieldsExpandedMode::Unique
                                            : KDbQuerySchema::FieldsExpandedMode::Default);
    }
    kdbWarning() << "no query or table specified!";
    return KDbQueryColumnInfo::Vector();
}

QByteArray KDbTableOrQuerySchema::name() const
{
    if (d->table)
        return d->table->name().toLatin1();
    if (d->query)
        return d->query->name().toLatin1();
    return d->name;
}

QString KDbTableOrQuerySchema::captionOrName() const
{
    KDbObject *object = d->table ? static_cast<KDbObject *>(d->table) : static_cast<KDbObject *>(d->query);
    if (!object)
        return QLatin1String(d->name);
    return object->caption().isEmpty() ? object->name() : object->caption();
}

KDbField* KDbTableOrQuerySchema::field(const QString& name)
{
    if (d->table)
        return d->table->field(name);
    if (d->query)
        return d->query->field(name);

    return nullptr;
}

KDbQueryColumnInfo* KDbTableOrQuerySchema::columnInfo(KDbConnection *conn, const QString& name)
{
    if (d->table)
        return d->table->query()->columnInfo(conn, name);

    if (d->query)
        return d->query->columnInfo(conn, name);

    return nullptr;
}

//! Sends information about table or query schema @a schema to debug output @a dbg.
QDebug operator<<(QDebug dbg, const KDbConnectionAndSchema &connectionAndSchema)
{
    if (std::get<1>(connectionAndSchema).table()) {
        dbg.nospace() << *std::get<1>(connectionAndSchema).table();
    } else if (std::get<1>(connectionAndSchema).query()) {
        dbg.nospace() << KDbConnectionAndQuerySchema(std::get<0>(connectionAndSchema),
                                                     *std::get<1>(connectionAndSchema).query());
    }
    return dbg.space();
}

KDbQuerySchema* KDbTableOrQuerySchema::query() const
{
    return d->query;
}

KDbTableSchema* KDbTableOrQuerySchema::table() const
{
    return d->table;
}

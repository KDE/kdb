/* This file is part of the KDE project
   Copyright (C) 2005-2010 Jarosław Staniek <staniek@kde.org>

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

#include "PreparedStatement.h"
#include "Interfaces/PreparedStatementInterface.h"
#include "TableSchema.h"

#include <QtDebug>

using namespace Predicate;

PreparedStatement::Data::~Data()
{
    delete iface;
    delete whereFields;
}

PreparedStatement::~PreparedStatement()
{
}

bool PreparedStatement::execute(const PreparedStatementParameters& parameters)
{
    if (d->dirty) {
        QByteArray s;
        if (!generateStatementString(&s)) // sets d->fieldsForParameters too
            return false;
//! @todo error message?
        if (s.isEmpty())
            return false;
        if (!d->iface->prepare(s))
            return false;
        d->dirty = false;
    }
    return d->iface->execute(d->type, *d->fieldsForParameters, parameters);
}

bool PreparedStatement::generateStatementString(QByteArray* s)
{
    s->reserve(1024);
    switch (d->type) {
    case SelectStatement:
        return generateSelectStatementString(s);
    case InsertStatement:
        return generateInsertStatementString(s);
    default:;
    }
    PreFatal << "Unsupported type" << d->type;
    return false;
}

bool PreparedStatement::generateSelectStatementString(QByteArray* s)
{
//! @todo only tables and trivial queries supported for select...
    *s = "SELECT ";
    bool first = true;
    foreach(Field *f, *d->fields.fields()) {
        if (first)
            first = false;
        else
            s->append(", ");
        s->append(f->name().toUtf8());
    }
    // create WHERE
    first = true;
    delete d->whereFields;
    d->whereFields = new Field::List();
    foreach(const QString& whereItem, d->whereFieldNames) {
        if (first) {
            s->append(" WHERE ");
            first = false;
        }
        else
            s->append(" AND ");
        Field *f = d->fields.field(whereItem);
        if (!f) {
            PreWarn << "field" << whereItem << "not found, aborting";
            s->clear();
            return false;
        }
        d->whereFields->append(f);
        s->append(whereItem.toUtf8() + "=?");
    }
    d->fieldsForParameters = d->whereFields;
    return true;
}

bool PreparedStatement::generateInsertStatementString(QByteArray* s)
{
    //! @todo only tables supported for insert; what about views?
    TableSchema *table = d->fields.isEmpty() ? 0 : d->fields.field(0)->table();
    if (!table)
        return false; //err

    QByteArray namesList;
    bool first = true;
    //we are using a selection of fields only
    const bool allTableFieldsUsed = dynamic_cast<TableSchema*>(&d->fields);
    foreach(Field* f, *d->fields.fields()) {
        if (first) {
            s->append("?");
            if (!allTableFieldsUsed)
                namesList = f->name().toUtf8();
            first = false;
        } else {
            s->append(",?");
            if (!allTableFieldsUsed)
                namesList.append(QByteArray(", ") + f->name().toUtf8());
        }
    }
    s->append(")");
    s->prepend(QByteArray("INSERT INTO ") + table->name().toUtf8()
              + (allTableFieldsUsed ? "" : (" (" + namesList + ")"))
              + " VALUES (");
    d->fieldsForParameters = d->fields.fields();
    return true;
}

/*bool PreparedStatement::insert()
{
  const bool res = m_conn->drv_prepareStatement(this);
  const bool res = m_conn->drv_insertRecord(this);
  clearArguments();
  return res;
}

bool PreparedStatement::select()
{
  const bool res = m_conn->drv_bindArgumentForPreparedStatement(this, m_args.count()-1);
}*/

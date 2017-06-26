/* This file is part of the KDE project
   Copyright (C) 2005-2016 Jarosław Staniek <staniek@kde.org>

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

#include "KDbPreparedStatement.h"
#include "KDbPreparedStatementInterface.h"
#include "KDbSqlResult.h"
#include "KDbTableSchema.h"
#include "kdb_debug.h"

KDbPreparedStatement::Data::Data()
    : Data(InvalidStatement, nullptr, nullptr, QStringList())
{
}

KDbPreparedStatement::Data::Data(Type _type, KDbPreparedStatementInterface* _iface,
                                 KDbFieldList* _fields,
     const QStringList& _whereFieldNames)
    : type(_type), fields(_fields), whereFieldNames(_whereFieldNames)
    , fieldsForParameters(nullptr), whereFields(nullptr), dirty(true), iface(_iface)
    , lastInsertRecordId(std::numeric_limits<quint64>::max())
{
}

KDbPreparedStatement::Data::~Data()
{
    delete iface;
    delete whereFields;
}

KDbPreparedStatement::KDbPreparedStatement()
    : d( new Data() )
{
}

KDbPreparedStatement::KDbPreparedStatement(KDbPreparedStatementInterface* iface,
                                           Type type, KDbFieldList* fields,
                                           const QStringList& whereFieldNames)
    : d( new Data(type, iface, fields, whereFieldNames) )
{
}

KDbPreparedStatement::~KDbPreparedStatement()
{
}

bool KDbPreparedStatement::execute(const KDbPreparedStatementParameters& parameters)
{
    if (d->dirty) {
        KDbEscapedString s;
        if (!generateStatementString(&s)) { // sets d->fieldsForParameters too
            m_result.setCode(ERR_OTHER);
            return false;
        }
//! @todo error message?
        if (s.isEmpty()) {
            m_result.setCode(ERR_OTHER);
            return false;
        }
        if (!d->iface->prepare(s)) {
            m_result.setCode(ERR_OTHER);
            return false;
        }
        d->dirty = false;
    }
    QSharedPointer<KDbSqlResult> result
        = d->iface->execute(d->type, *d->fieldsForParameters, d->fields, parameters);
    if (!result) {
        return false;
    }
    d->lastInsertRecordId = result->lastInsertRecordId();
    return true;
}

bool KDbPreparedStatement::generateStatementString(KDbEscapedString * s)
{
    s->reserve(1024);
    switch (d->type) {
    case SelectStatement:
        return generateSelectStatementString(s);
    case InsertStatement:
        return generateInsertStatementString(s);
    default:;
    }
    kdbCritical() << "Unsupported type" << d->type;
    return false;
}

bool KDbPreparedStatement::generateSelectStatementString(KDbEscapedString * s)
{
//! @todo only tables and trivial queries supported for select...
    *s = "SELECT ";
    bool first = true;
    foreach(KDbField *f, *d->fields->fields()) {
        if (first)
            first = false;
        else
            s->append(", ");
        s->append(f->name());
    }
    // create WHERE
    first = true;
    delete d->whereFields;
    d->whereFields = new KDbField::List();
    foreach(const QString& whereItem, d->whereFieldNames) {
        if (first) {
            s->append(" WHERE ");
            first = false;
        }
        else
            s->append(" AND ");
        KDbField *f = d->fields->field(whereItem);
        if (!f) {
            kdbWarning() << "field" << whereItem << "not found, aborting";
            s->clear();
            return false;
        }
        d->whereFields->append(f);
        s->append(whereItem.toUtf8() + "=?");
    }
    d->fieldsForParameters = d->whereFields;
    return true;
}

bool KDbPreparedStatement::generateInsertStatementString(KDbEscapedString * s)
{
    //! @todo only tables supported for insert; what about views?
    KDbTableSchema *table = d->fields->isEmpty() ? nullptr : d->fields->field(0)->table();
    if (!table)
        return false; //err

    KDbEscapedString namesList;
    bool first = true;
    //we are using a selection of fields only
    const bool allTableFieldsUsed = dynamic_cast<KDbTableSchema*>(d->fields);
    foreach(const KDbField* f, *d->fields->fields()) {
        if (first) {
            s->append("?");
            if (!allTableFieldsUsed)
                namesList = KDbEscapedString(f->name());
            first = false;
        } else {
            s->append(",?");
            if (!allTableFieldsUsed) {
                namesList.append(", ");
                namesList.append(f->name());
            }
        }
    }
    s->append(")");
    s->prepend(KDbEscapedString("INSERT INTO ") + table->name()
               + (allTableFieldsUsed ? KDbEscapedString() : (KDbEscapedString(" (") + namesList + ')'))
               + " VALUES (");
    d->fieldsForParameters = d->fields->fields();
    return true;
}

bool KDbPreparedStatement::isValid() const
{
    return d->type != InvalidStatement;
}

KDbPreparedStatement::Type KDbPreparedStatement::type() const
{
    return d->type;
}

void KDbPreparedStatement::setType(KDbPreparedStatement::Type type)
{
    d->type = type;
    d->dirty = true;
}

const KDbFieldList* KDbPreparedStatement::fields() const
{
    return d->fields;
}

void KDbPreparedStatement::setFields(KDbFieldList* fields)
{
    if (fields) {
        d->fields = fields;
        d->dirty = true;
    }
}

QStringList KDbPreparedStatement::whereFieldNames() const
{
    return d->whereFieldNames;
}

void KDbPreparedStatement::setWhereFieldNames(const QStringList& whereFieldNames)
{
    d->whereFieldNames = whereFieldNames;
    d->dirty = true;
}

quint64 KDbPreparedStatement::lastInsertRecordId() const
{
    return d->lastInsertRecordId;
}

/*bool KDbPreparedStatement::insert()
{
  const bool res = m_conn->drv_prepareStatement(this);
  const bool res = m_conn->drv_insertRecord(this);
  clearArguments();
  return res;
}

bool KDbPreparedStatement::select()
{
  const bool res = m_conn->drv_bindArgumentForPreparedStatement(this, m_args.count()-1);
}*/

/* This file is part of the KDE project
   Copyright (C) 2003-2011 Jarosław Staniek <staniek@kde.org>

   Based on nexp.cpp : Parser module of Python-like language
   (C) 2001 Jarosław Staniek, MIMUW (www.mimuw.edu.pl)

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

#include <Predicate/Expression>
#include "Expression_p.h"
#include <Predicate/Utils>
#include <Predicate/QuerySchema>
#include "parser/SqlParser.h"
#include "parser/Parser_p.h"
#include <Predicate/Tools/Static>

#include <ctype.h>

using namespace Predicate;

VariableExpressionData::VariableExpressionData()
 : ExpressionData()
 , field(0)
 , tablePositionForField(-1)
 , tableForQueryAsterisk(0)
{
    qDebug() << "VariableExpressionData" << ref;
}

VariableExpressionData::VariableExpressionData(const QString& aName)
 : ExpressionData()
 , name(aName)
 , field(0)
 , tablePositionForField(-1)
 , tableForQueryAsterisk(0)
{
   qDebug() << "VariableExpressionData" << ref;
}

VariableExpressionData::~VariableExpressionData()
{
    qDebug() << "~VariableExpressionData" << ref;
}

VariableExpressionData* VariableExpressionData::clone()
{
    qDebug() << "VariableExpressionData::clone" << *this;
    return new VariableExpressionData(*this);
}

QDebug VariableExpressionData::debug(QDebug dbg) const
{
    dbg.nospace() << "VariableExp(" << name
        << QString::fromLatin1(",type=%1)")
          .arg(field ? Driver::defaultSQLTypeName(type())
                     : QLatin1String("FIELD NOT DEFINED YET"));
    return dbg.space();
}

EscapedString VariableExpressionData::toString(QuerySchemaParameterValueListIterator* params) const
{
    Q_UNUSED(params);
    return EscapedString(name);
}

void VariableExpressionData::getQueryParameters(QuerySchemaParameterList& params)
{
    Q_UNUSED(params);
}

//! We're assuming it's called after VariableExpr::validate()
Field::Type VariableExpressionData::type() const
{
    if (field)
        return field->type();

    //BTW, asterisks are not stored in VariableExpr outside of parser, so ok.
    return Field::InvalidType;
}

#define IMPL_ERROR(errmsg) parseInfo.errMsg = "Implementation error"; parseInfo.errDescr = errmsg

bool VariableExpressionData::validate(ParseInfo& parseInfo)
{
    if (!ExpressionData::validate(parseInfo))
        return false;
    field = 0;
    tablePositionForField = -1;
    tableForQueryAsterisk = 0;

    /* taken from parser's addColumn(): */
    PreDbg << "checking variable name: " << name;
    int dotPos = name.indexOf('.');
    QString tableName, fieldName;
//! @todo shall we also support db name?
    if (dotPos > 0) {
        tableName = name.left(dotPos);
        fieldName = name.mid(dotPos + 1);
    }
    if (tableName.isEmpty()) {//fieldname only
        fieldName = name;
        if (fieldName == "*") {
//   querySchema->addAsterisk( new QueryAsterisk(querySchema) );
            return true;
        }

        //find first table that has this field
        Field *firstField = 0;
        foreach(TableSchema *table, *parseInfo.querySchema->tables()) {
            Field *f = table->field(fieldName);
            if (f) {
                if (!firstField) {
                    firstField = f;
                } else if (f->table() != firstField->table()) {
                    //ambiguous field name
                    parseInfo.errMsg = QObject::tr("Ambiguous field name");
                    parseInfo.errDescr = QObject::tr("Both table \"%1\" and \"%2\" have defined \"%3\" field. "
                                              "Use \"<tableName>.%4\" notation to specify table name.")
                                              .arg(firstField->table()->name(), f->table()->name(),
                                              fieldName, fieldName);
                    return false;
                }
            }
        }
        if (!firstField) {
            parseInfo.errMsg = QObject::tr("Field not found");
            parseInfo.errDescr = QObject::tr("Table containing \"%1\" field not found").arg(fieldName);
            return false;
        }
        //ok
        field = firstField; //store
//  querySchema->addField(firstField);
        return true;
    }

    //table.fieldname or tableAlias.fieldname
    tableName = tableName.toLower();
    TableSchema *ts = parseInfo.querySchema->table(tableName);
    if (ts) {//table.fieldname
        //check if "table" is covered by an alias
        const QList<int> tPositions = parseInfo.querySchema->tablePositions(tableName);
        QByteArray tableAlias;
        bool covered = true;
        foreach(int position, tPositions) {
            tableAlias = parseInfo.querySchema->tableAlias(position);
            if (tableAlias.isEmpty() || tableAlias.toLower() == tableName.toLatin1()) {
                covered = false; //uncovered
                break;
            }
            PreDbg << " --" << "covered by " << tableAlias << " alias";
        }
        if (covered) {
            parseInfo.errMsg = QObject::tr("Could not access the table directly using its name");
            parseInfo.errDescr = QObject::tr("Table \"%1\" is covered by aliases. Instead of \"%2\", "
                                      "you can write \"%3\"").arg(tableName, tableName + "." + fieldName, tableAlias + "." + QString(fieldName));
            return false;
        }
    }

    int tablePosition = -1;
    if (!ts) {//try to find tableAlias
        tablePosition = parseInfo.querySchema->tablePositionForAlias(tableName.toLatin1());
        if (tablePosition >= 0) {
            ts = parseInfo.querySchema->tables()->at(tablePosition);
            if (ts) {
//    PreDbg << " --it's a tableAlias.name";
            }
        }
    }

    if (!ts) {
        parseInfo.errMsg = QObject::tr("Table not found");
        parseInfo.errDescr = QObject::tr("Unknown table \"%1\"").arg(tableName);
        return false;
    }

    if (!parseInfo.repeatedTablesAndAliases.contains(tableName)) {  //for sanity
        IMPL_ERROR(tableName + "." + fieldName + ", !positionsList ");
        return false;
    }
    const QList<int> positionsList(parseInfo.repeatedTablesAndAliases.value(tableName));

    //it's a table.*
    if (fieldName == "*") {
        if (positionsList.count() > 1) {
            parseInfo.errMsg = QObject::tr("Ambiguous \"%1.*\" expression").arg(tableName);
            parseInfo.errDescr = QObject::tr("More than one \"%1\" table or alias defined").arg(tableName);
            return false;
        }
        tableForQueryAsterisk = ts;
//   querySchema->addAsterisk( new QueryAsterisk(querySchema, ts) );
        return true;
    }

// PreDbg << " --it's a table.name";
    Field *realField = ts->field(fieldName);
    if (!realField) {
        parseInfo.errMsg = QObject::tr("Field not found");
        parseInfo.errDescr = QObject::tr("Table \"%1\" has no \"%2\" field").arg(tableName, fieldName);
        return false;
    }

    // check if table or alias is used twice and both have the same column
    // (so the column is ambiguous)
    int numberOfTheSameFields = 0;
    foreach(int position, positionsList) {
        TableSchema *otherTS = parseInfo.querySchema->tables()->at(position);
        if (otherTS->field(fieldName))
            numberOfTheSameFields++;
        if (numberOfTheSameFields > 1) {
            parseInfo.errMsg = QObject::tr("Ambiguous \"%1.%2\" expression").arg(tableName, fieldName);
            parseInfo.errDescr = QObject::tr("More than one \"%1\" table or alias defined containing \"%2\" field")
                                      .arg(tableName, fieldName);
            return false;
        }
    }
    field = realField; //store
    tablePositionForField = tablePosition;
//    querySchema->addField(realField, tablePosition);

    return true;
}

//=========================================

VariableExpression::VariableExpression()
 : Expression(new VariableExpressionData)
{
    qDebug() << "VariableExpression() ctor" << *this;
}

VariableExpression::VariableExpression(const QString& name)
        : Expression(new VariableExpressionData(name),
              VariableExpressionClass, 0/*undefined*/)
{
}

VariableExpression::VariableExpression(const VariableExpression& expr)
        : Expression(expr)
{
}

VariableExpression::VariableExpression(ExpressionData* data)
    : Expression(data)
{
    qDebug() << "VariableExpression ctor (ExpressionData*)" << *this;
}

VariableExpression::~VariableExpression()
{
}

QString VariableExpression::name() const
{
    return d->convert<VariableExpressionData>()->name;
}

Field *VariableExpression::field() const
{
    return d->convert<VariableExpressionData>()->field;
}

int VariableExpression::tablePositionForField() const
{
    return d->convert<VariableExpressionData>()->tablePositionForField;
}

TableSchema *VariableExpression::tableForQueryAsterisk() const
{
    return d->convert<VariableExpressionData>()->tableForQueryAsterisk;
}

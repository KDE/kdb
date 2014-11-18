/* This file is part of the KDE project
   Copyright (C) 2003-2012 Jarosław Staniek <staniek@kde.org>

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
#include <Predicate/Utils>
#include <Predicate/QuerySchema>
#include <Predicate/Tools/Static>
#include "parser/Parser_p.h"

#include <ctype.h>

using namespace Predicate;

VariableExpressionData::VariableExpressionData()
 : ExpressionData()
 , field(0)
 , tablePositionForField(-1)
 , tableForQueryAsterisk(0)
{
    ExpressionDebug << "VariableExpressionData" << ref;
}

VariableExpressionData::VariableExpressionData(const QString& aName)
 : ExpressionData()
 , name(aName)
 , field(0)
 , tablePositionForField(-1)
 , tableForQueryAsterisk(0)
{
   ExpressionDebug << "VariableExpressionData" << ref;
}

VariableExpressionData::~VariableExpressionData()
{
    ExpressionDebug << "~VariableExpressionData" << ref;
}

VariableExpressionData* VariableExpressionData::clone()
{
    ExpressionDebug << "VariableExpressionData::clone" << *this;
    return new VariableExpressionData(*this);
}

void VariableExpressionData::debugInternal(QDebug dbg, CallStack* callStack) const
{
    Q_UNUSED(callStack);
    dbg.nospace() << "VariableExp(" << name
        << QString::fromLatin1(",type=%1)")
          .arg(field ? Driver::defaultSQLTypeName(type())
                     : QLatin1String("FIELD NOT DEFINED YET"));
}

EscapedString VariableExpressionData::toStringInternal(QuerySchemaParameterValueListIterator* params,
                                                       CallStack* callStack) const
{
    Q_UNUSED(params);
    Q_UNUSED(callStack);
    return EscapedString(name);
}

void VariableExpressionData::getQueryParameters(QuerySchemaParameterList& params)
{
    Q_UNUSED(params);
}

//! We're assuming it's called after VariableExpr::validate()
Field::Type VariableExpressionData::typeInternal(CallStack* callStack) const
{
    Q_UNUSED(callStack);
    if (field)
        return field->type();

    //BTW, asterisks are not stored in VariableExpr outside of parser, so ok.
    return Field::InvalidType;
}

static void validateImplError(ParseInfo *parseInfo_, const QString &errmsg)
{
    ParseInfoInternal *parseInfo = static_cast<ParseInfoInternal*>(parseInfo_);
    parseInfo->setErrorMessage(QLatin1String("Implementation error"));
    parseInfo->setErrorDescription(errmsg);
}

bool VariableExpressionData::validateInternal(ParseInfo *parseInfo_, CallStack* callStack)
{
    Q_UNUSED(callStack);
    ParseInfoInternal *parseInfo = static_cast<ParseInfoInternal*>(parseInfo_);
    field = 0;
    tablePositionForField = -1;
    tableForQueryAsterisk = 0;

    /* taken from parser's addColumn(): */
    PreDbg << "checking variable name: " << name;
    QString tableName, fieldName;
    if (!Predicate::splitToTableAndFieldParts(name, &tableName, &fieldName,
                                              Predicate::SetFieldNameIfNoTableName))
    {
        return false;
    }
    //! @todo shall we also support db name?
    if (tableName.isEmpty()) {//fieldname only
        if (fieldName == QLatin1String("*")) {
            return true;
        }

        //find first table that has this field
        Field *firstField = 0;
        foreach(TableSchema *table, *parseInfo->querySchema()->tables()) {
            Field *f = table->field(fieldName);
            if (f) {
                if (!firstField) {
                    firstField = f;
                } else if (f->table() != firstField->table()) {
                    //ambiguous field name
                    parseInfo->setErrorMessage(
                        QObject::tr("Ambiguous field name"));
                    parseInfo->setErrorDescription(
                        QObject::tr("Both table \"%1\" and \"%2\" have defined \"%3\" field. "
                                    "Use \"<tableName>.%4\" notation to specify table name.")
                                   .arg(firstField->table()->name(), f->table()->name(),
                                        fieldName, fieldName));
                    return false;
                }
            }
        }
        if (!firstField) {
            parseInfo->setErrorMessage(
                QObject::tr("Field not found"));
            parseInfo->setErrorDescription(
                QObject::tr("Table containing \"%1\" field not found").arg(fieldName));
            return false;
        }
        //ok
        field = firstField; //store
        return true;
    }

    //table.fieldname or tableAlias.fieldname
    TableSchema *ts = parseInfo->querySchema()->table(tableName);
    int tablePosition = -1;
    if (ts) {//table.fieldname
        //check if "table" is covered by an alias
        const QList<int> tPositions = parseInfo->querySchema()->tablePositions(tableName);
        QString tableAlias;
        bool covered = true;
        foreach(int position, tPositions) {
            tableAlias = parseInfo->querySchema()->tableAlias(position);
            if (tableAlias.isEmpty() || tableAlias.toLower() == tableName) {
                covered = false; //uncovered
                break;
            }
            PreDbg << " --" << "covered by " << tableAlias << " alias";
        }
        if (covered) {
            parseInfo->setErrorMessage(
                QObject::tr("Could not access the table directly using its name"));
            parseInfo->setErrorDescription(
                QObject::tr("Table \"%1\" is covered by aliases. Instead of \"%2\", "
                            "you can write \"%3\"")
                         .arg(tableName,
                              tableName + QLatin1Char('.') + fieldName,
                              tableAlias + QLatin1Char('.') + fieldName));
            return false;
        }
        if (!tPositions.isEmpty()) {
            tablePosition = tPositions.first();
        }
    }
    else {//try to find tableAlias
        tablePosition = parseInfo->querySchema()->tablePositionForAlias(tableName);
        if (tablePosition >= 0) {
            ts = parseInfo->querySchema()->tables()->at(tablePosition);
            if (ts) {
//    PreDbg << " --it's a tableAlias.name";
            }
        }
    }

    if (!ts) {
        parseInfo->setErrorMessage(
            QObject::tr("Table not found"));
        parseInfo->setErrorDescription(
            QObject::tr("Unknown table \"%1\"").arg(tableName));
        return false;
    }

    if (parseInfo->tablesAndAliasesForName(tableName).isEmpty()) {  //for sanity
        validateImplError(parseInfo,
            QString::fromLatin1("%1.%2, !positionsList ").arg(tableName, fieldName));
        return false;
    }
    const QList<int> positionsList(parseInfo->tablesAndAliasesForName(tableName));

    //it's a table.*
    if (fieldName == QLatin1String("*")) {
        if (positionsList.count() > 1) {
            parseInfo->setErrorMessage(
                QObject::tr("Ambiguous \"%1.*\" expression").arg(tableName));
            parseInfo->setErrorDescription(
                QObject::tr("More than one \"%1\" table or alias defined").arg(tableName));
            return false;
        }
        tableForQueryAsterisk = ts;
        return true;
    }

// PreDbg << " --it's a table.name";
    Field *realField = ts->field(fieldName);
    if (!realField) {
        parseInfo->setErrorMessage(QObject::tr("Field not found"));
        parseInfo->setErrorDescription(
            QObject::tr("Table \"%1\" has no \"%2\" field").arg(tableName, fieldName));
        return false;
    }

    // check if table or alias is used twice and both have the same column
    // (so the column is ambiguous)
    int numberOfTheSameFields = 0;
    foreach(int position, positionsList) {
        TableSchema *otherTS = parseInfo->querySchema()->tables()->at(position);
        if (otherTS->field(fieldName))
            numberOfTheSameFields++;
        if (numberOfTheSameFields > 1) {
            parseInfo->setErrorMessage(
                QObject::tr("Ambiguous \"%1.%2\" expression").arg(tableName, fieldName));
            parseInfo->setErrorDescription(
                QObject::tr("More than one \"%1\" table or alias defined containing \"%2\" field")
                            .arg(tableName, fieldName));
            return false;
        }
    }
    field = realField; //store
    tablePositionForField = tablePosition;
    return true;
}

//=========================================

VariableExpression::VariableExpression()
 : Expression(new VariableExpressionData)
{
    ExpressionDebug << "VariableExpression() ctor" << *this;
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
    ExpressionDebug << "VariableExpression ctor (ExpressionData*)" << *this;
}

VariableExpression::VariableExpression(const ExplicitlySharedExpressionDataPointer &ptr)
    : Expression(ptr)
{
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

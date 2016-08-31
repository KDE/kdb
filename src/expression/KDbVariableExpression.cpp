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

#include "KDbExpression.h"
#include "KDb.h"
#include "KDbDriver.h"
#include "KDbQuerySchema.h"
#include "KDbParser_p.h"
#include "kdb_debug.h"

KDbVariableExpressionData::KDbVariableExpressionData()
 : KDbExpressionData()
 , field(0)
 , tablePositionForField(-1)
 , tableForQueryAsterisk(0)
{
    ExpressionDebug << "VariableExpressionData" << ref;
}

KDbVariableExpressionData::KDbVariableExpressionData(const QString& aName)
 : KDbExpressionData()
 , name(aName)
 , field(0)
 , tablePositionForField(-1)
 , tableForQueryAsterisk(0)
{
   ExpressionDebug << "VariableExpressionData" << ref;
}

KDbVariableExpressionData::~KDbVariableExpressionData()
{
    ExpressionDebug << "~VariableExpressionData" << ref;
}

KDbVariableExpressionData* KDbVariableExpressionData::clone()
{
    ExpressionDebug << "VariableExpressionData::clone" << *this;
    return new KDbVariableExpressionData(*this);
}

void KDbVariableExpressionData::debugInternal(QDebug dbg, KDb::ExpressionCallStack* callStack) const
{
    Q_UNUSED(callStack);
    dbg.nospace() << qPrintable(QString::fromLatin1("VariableExp(\"%1\",type=%2)")
        .arg(name).arg(field ? KDbDriver::defaultSQLTypeName(type())
                             : QLatin1String("FIELD_NOT_DEFINED_YET")));
}

KDbEscapedString KDbVariableExpressionData::toStringInternal(
                                        const KDbDriver *driver,
                                        KDbQuerySchemaParameterValueListIterator* params,
                                        KDb::ExpressionCallStack* callStack) const
{
    Q_UNUSED(driver);
    Q_UNUSED(params);
    Q_UNUSED(callStack);
    return KDbEscapedString(name);
}

void KDbVariableExpressionData::getQueryParameters(QList<KDbQuerySchemaParameter>* params)
{
    Q_UNUSED(params);
}

//! We're assuming it's called after VariableExpr::validate()
KDbField::Type KDbVariableExpressionData::typeInternal(KDb::ExpressionCallStack* callStack) const
{
    Q_UNUSED(callStack);
    if (field)
        return field->type();

    //BTW, asterisks are not stored in VariableExpr outside of parser, so ok.
    return KDbField::InvalidType;
}

static void validateImplError(KDbParseInfo *parseInfo_, const QString &errmsg)
{
    KDbParseInfoInternal *parseInfo = static_cast<KDbParseInfoInternal*>(parseInfo_);
    parseInfo->setErrorMessage(QLatin1String("Implementation error"));
    parseInfo->setErrorDescription(errmsg);
}

bool KDbVariableExpressionData::validateInternal(KDbParseInfo *parseInfo_, KDb::ExpressionCallStack* callStack)
{
    Q_UNUSED(callStack);
    KDbParseInfoInternal *parseInfo = static_cast<KDbParseInfoInternal*>(parseInfo_);
    field = 0;
    tablePositionForField = -1;
    tableForQueryAsterisk = 0;

    /* taken from parser's addColumn(): */
    kdbDebug() << "checking variable name: " << name;
    QString tableName, fieldName;
    if (!KDb::splitToTableAndFieldParts(name, &tableName, &fieldName,
                                        KDb::SetFieldNameIfNoTableName))
    {
        return false;
    }
    //! @todo shall we also support db name?
    if (tableName.isEmpty()) {//fieldname only
        if (fieldName == QLatin1String("*")) {
            return true;
        }

        //find first table that has this field
        KDbField *firstField = 0;
        foreach(KDbTableSchema *table, *parseInfo->querySchema()->tables()) {
            KDbField *f = table->field(fieldName);
            if (f) {
                if (!firstField) {
                    firstField = f;
                } else if (f->table() != firstField->table()) {
                    //ambiguous field name
                    parseInfo->setErrorMessage(KDbExpression::tr("Ambiguous field name"));
                    parseInfo->setErrorDescription(
                        KDbExpression::tr("Both table \"%1\" and \"%2\" have defined \"%3\" field. "
                                          "Use \"<tableName>.%4\" notation to specify table name.")
                                          .arg(firstField->table()->name(), f->table()->name(),
                                               fieldName, fieldName));
                    return false;
                }
            }
        }
        if (!firstField) {
            parseInfo->setErrorMessage(KDbExpression::tr("Field not found"));
            parseInfo->setErrorDescription(
                KDbExpression::tr("Table containing \"%1\" field not found.").arg(fieldName));
            return false;
        }
        //ok
        field = firstField; //store
        return true;
    }

    //table.fieldname or tableAlias.fieldname
    KDbTableSchema *ts = parseInfo->querySchema()->table(tableName);
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
            kdbDebug() << " --" << "covered by " << tableAlias << " alias";
        }
        if (covered) {
            parseInfo->setErrorMessage(KDbExpression::tr("Could not access the table directly using its name"));
            parseInfo->setErrorDescription(
                KDbExpression::tr("Table name \"%1\" is covered by aliases. Instead of \"%2\", "
                                  "you can write \"%3\".")
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
//    kdbDebug() << " --it's a tableAlias.name";
            }
        }
    }

    if (!ts) {
        parseInfo->setErrorMessage(KDbExpression::tr("Table not found"));
        parseInfo->setErrorDescription(KDbExpression::tr("Unknown table \"%1\".").arg(tableName));
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
            parseInfo->setErrorMessage(KDbExpression::tr("Ambiguous \"%1.*\" expression").arg(tableName));
            parseInfo->setErrorDescription(KDbExpression::tr("More than one \"%1\" table or alias defined.").arg(tableName));
            return false;
        }
        tableForQueryAsterisk = ts;
        return true;
    }

// kdbDebug() << " --it's a table.name";
    KDbField *realField = ts->field(fieldName);
    if (!realField) {
        parseInfo->setErrorMessage(KDbExpression::tr("Field not found"));
        parseInfo->setErrorDescription(
            KDbExpression::tr("Table \"%1\" has no \"%2\" field.").arg(tableName, fieldName));
        return false;
    }

    // check if table or alias is used twice and both have the same column
    // (so the column is ambiguous)
    if (positionsList.count() > 1) {
        parseInfo->setErrorMessage(KDbExpression::tr("Ambiguous \"%1.%2\" expression").arg(tableName, fieldName));
        parseInfo->setErrorDescription(
            KDbExpression::tr("More than one \"%1\" table or alias defined containing \"%2\" field.")
                              .arg(tableName, fieldName));
        return false;
    }
    field = realField; //store
    tablePositionForField = tablePosition;
    return true;
}

//=========================================

KDbVariableExpression::KDbVariableExpression()
 : KDbExpression(new KDbVariableExpressionData)
{
    ExpressionDebug << "KDbVariableExpression() ctor" << *this;
}

KDbVariableExpression::KDbVariableExpression(const QString& name)
        : KDbExpression(new KDbVariableExpressionData(name),
              KDb::VariableExpression, KDbToken()/*undefined*/)
{
}

KDbVariableExpression::KDbVariableExpression(const KDbVariableExpression& expr)
        : KDbExpression(expr)
{
}

KDbVariableExpression::KDbVariableExpression(KDbExpressionData* data)
    : KDbExpression(data)
{
    ExpressionDebug << "KDbVariableExpression ctor (KDbExpressionData*)" << *this;
}

KDbVariableExpression::KDbVariableExpression(const ExplicitlySharedExpressionDataPointer &ptr)
    : KDbExpression(ptr)
{
}

KDbVariableExpression::~KDbVariableExpression()
{
}

QString KDbVariableExpression::name() const
{
    return d->convert<KDbVariableExpressionData>()->name;
}

KDbField *KDbVariableExpression::field() const
{
    return d->convert<KDbVariableExpressionData>()->field;
}

int KDbVariableExpression::tablePositionForField() const
{
    return d->convert<KDbVariableExpressionData>()->tablePositionForField;
}

KDbTableSchema *KDbVariableExpression::tableForQueryAsterisk() const
{
    return d->convert<KDbVariableExpressionData>()->tableForQueryAsterisk;
}

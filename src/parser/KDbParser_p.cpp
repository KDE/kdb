/* This file is part of the KDE project
   Copyright (C) 2004-2012 Jarosław Staniek <staniek@kde.org>

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

#include "KDbParser_p.h"
#include "generated/sqlparser.h"

#include <QRegExp>
#include <QMutableListIterator>

#include <assert.h>

KDbParser *globalParser = 0;
KDbField *globalField = 0;
QList<KDbField*> fieldList;
int globalCurrentPos = 0;
QByteArray globalToken;

extern int yylex_destroy(void);

//-------------------------------------

KDbParser::Private::Private()
        : initialized(false)
{
    clear();
    table = 0;
    select = 0;
    db = 0;
}

KDbParser::Private::~Private()
{
    delete select;
    delete table;
}

void KDbParser::Private::clear()
{
    operation = KDbParser::OP_None;
    error = KDbParserError();
}

//-------------------------------------

KDbParseInfoInternal::KDbParseInfoInternal(KDbQuerySchema *query)
: KDbParseInfo(query)
{
}

KDbParseInfoInternal::~KDbParseInfoInternal()
{
}

void KDbParseInfoInternal::appendPositionForTableOrAliasName(const QString &tableOrAliasName, int pos)
{
    QList<int> *list = d->repeatedTablesAndAliases.value(tableOrAliasName);
    if (!list) {
        list = new QList<int>();
        d->repeatedTablesAndAliases.insert(tableOrAliasName, list);
    }
    list->append(pos);
}

void KDbParseInfoInternal::setErrorMessage(const QString &message)
{
    d->errorMessage = message;
}

void KDbParseInfoInternal::setErrorDescription(const QString &description)
{
    d->errorDescription = description;
}

//-------------------------------------

extern int yyparse();
extern void tokenize(const char *data);

void yyerror(const char *str)
{
    KDbDbg << "error: " << str;
    KDbDbg << "at character " << globalCurrentPos << " near tooken " << globalToken;
    globalParser->setOperation(KDbParser::OP_Error);

    const bool otherError = (qstrnicmp(str, "other error", 11) == 0);
    const bool syntaxError = qstrnicmp(str, "syntax error", 12) == 0;
    if ((   globalParser->error().type().isEmpty() && (str == 0 || strlen(str) == 0 || syntaxError))
        || otherError)
    {
        KDbDbg << globalParser->statement();
        QString ptrline(globalCurrentPos, QLatin1Char(' '));

        ptrline += QLatin1String("^");

        KDbDbg << ptrline;

#if 0
        //lexer may add error messages
        QString lexerErr = globalParser->error().message();

        QString errtypestr = QLatin1String(str);
        if (lexerErr.isEmpty()) {
            if (errtypestr.startsWith(QString::fromLatin1("parse error, expecting `IDENTIFIER'"))) {
                lexerErr = QObject::tr("identifier was expected");
            }
        }
#endif

        //! @todo exact invalid expression can be selected in the editor, based on KDbParseInfo data
        if (!otherError) {
            const bool isKDbSQLKeyword = KDb::isKDbSQLKeyword(globalToken);
            if (isKDbSQLKeyword || syntaxError) {
                if (isKDbSQLKeyword) {
                    globalParser->setError(KDbParserError(QObject::tr("Syntax Error"),
                                                          QObject::tr("\"%1\" is a reserved keyword.").arg(QLatin1String(globalToken)),
                                                          globalToken, globalCurrentPos));
                } else {
                    globalParser->setError(KDbParserError(QObject::tr("Syntax Error"),
                                                          QObject::tr("Syntax error."),
                                                          globalToken, globalCurrentPos));
                }
            } else {
                globalParser->setError(KDbParserError(QObject::tr("Error"),
                                                      QObject::tr("Error near \"%1\".").arg(QLatin1String(globalToken)),
                                                      globalToken, globalCurrentPos));
            }
        }
    }
}

void setError(const QString& errName, const QString& errDesc)
{
    globalParser->setError(KDbParserError(errName, errDesc, globalToken, globalCurrentPos));
    yyerror(errName.toLatin1().constData());
}

void setError(const QString& errDesc)
{
    setError(QObject::tr("Other error"), errDesc);
}

/* this is better than assert() */
#define IMPL_ERROR(errmsg) setError(QObject::tr("Implementation error"), QLatin1String(errmsg))

bool parseData(KDbParser *p, const char *data)
{
    /* todo: make this REENTRANT */
    globalParser = p;
    globalParser->clear();
    globalField = 0;
    fieldList.clear();

    if (!data) {
        KDbParserError err(QObject::tr("Error"), QObject::tr("No query statement specified."),
                           globalToken, globalCurrentPos);
        globalParser->setError(err);
        yyerror("");
        globalParser = 0;
        return false;
    }

    tokenize(data);
    if (!globalParser->error().type().isEmpty()) {
        globalParser = 0;
        return false;
    }
    yyparse();

    bool ok = true;
    if (globalParser->operation() == KDbParser::OP_Select) {
        KDbDbg << "parseData(): ok";
//   KDbDbg << "parseData(): " << tableDict.count() << " loaded tables";
        /*   KDbTableSchema *ts;
              for(QDictIterator<KDbTableSchema> it(tableDict); KDbTableSchema *s = tableList.first(); s; s = tableList.next())
              {
                KDbDbg << " " << s->name();
              }*/
    } else {
        ok = false;
    }
    yylex_destroy();
    globalParser = 0;
    return ok;
}


/* Adds @a column to @a querySchema. @a column can be in a form of
 table.field, tableAlias.field or field
*/
bool addColumn(KDbParseInfo *parseInfo, KDbExpression *columnExpr)
{
    if (!columnExpr->validate(parseInfo)) {
        setError(parseInfo->errorMessage(), parseInfo->errorDescription());
        return false;
    }

    KDbVariableExpression v_e(columnExpr->toVariable());
    if (columnExpr->expressionClass() == KDb::VariableExpression && !v_e.isNull()) {
        //it's a variable:
        if (v_e.name() == QLatin1String("*")) {//all tables asterisk
            if (parseInfo->querySchema()->tables()->isEmpty()) {
                setError(QObject::tr("\"*\" could not be used if no tables are specified."));
                return false;
            }
            parseInfo->querySchema()->addAsterisk(new KDbQueryAsterisk(parseInfo->querySchema()));
        } else if (v_e.tableForQueryAsterisk()) {//one-table asterisk
            parseInfo->querySchema()->addAsterisk(
                new KDbQueryAsterisk(parseInfo->querySchema(), v_e.tableForQueryAsterisk()));
        } else if (v_e.field()) {//"table.field" or "field" (bound to a table or not)
            parseInfo->querySchema()->addField(v_e.field(), v_e.tablePositionForField());
        } else {
            IMPL_ERROR("addColumn(): unknown case!");
            return false;
        }
        return true;
    }

    //it's complex expression
    parseInfo->querySchema()->addExpression(*columnExpr);
    return true;
}

KDbQuerySchema* buildSelectQuery(
    KDbQuerySchema* querySchema, KDbNArgExpression* _colViews,
    KDbNArgExpression* _tablesList, SelectOptionsInternal* options)
{
    KDbParseInfoInternal parseInfo(querySchema);

    // remove from heap (using heap was requered because parser uses union)
    KDbNArgExpression colViews;
    if (_colViews) {
        colViews = *_colViews;
        delete _colViews;
    }
    KDbNArgExpression tablesList;
    if (_tablesList) {
        tablesList = *_tablesList;
        delete _tablesList;
    }
    QScopedPointer<SelectOptionsInternal> optionsPtr(options);

    //-------tables list
    uint columnNum = 0;
    /*! @todo use this later if there are columns that use database fields,
              e.g. "SELECT 1 from table1 t, table2 t") is ok however. */
    //used to collect information about first repeated table name or alias:
    if (!tablesList.isEmpty()) {
        for (int i = 0; i < tablesList.argCount(); i++, columnNum++) {
            KDbExpression e(tablesList.arg(i));
            KDbVariableExpression t_e;
            QString aliasString;
            if (e.expressionClass() == KDb::SpecialBinaryExpression) {
                KDbBinaryExpression t_with_alias = e.toBinary();
                assert(e.isBinary());
                assert(t_with_alias.left().expressionClass() == KDb::VariableExpression);
                assert(t_with_alias.right().expressionClass() == KDb::VariableExpression
                       && (t_with_alias.token() == AS || t_with_alias.token() == AS_EMPTY));
                t_e = t_with_alias.left().toVariable();
                aliasString = t_with_alias.right().toVariable().name();
            } else {
                t_e = e.toVariable();
            }
            assert(t_e.isVariable());
            QString tname = t_e.name();
            KDbTableSchema *s = globalParser->connection()->tableSchema(tname);
            if (!s) {
                setError(
                    QObject::tr("Table \"%1\" does not exist.").arg(tname));
                return 0;
            }
            QString tableOrAliasName = KDb::iifNotEmpty(aliasString, tname);
            if (!aliasString.isEmpty()) {
//    KDbDbg << "- add alias for table: " << aliasString;
            }
            // 1. collect information about first repeated table name or alias
            //    (potential ambiguity)
            parseInfo.appendPositionForTableOrAliasName(tableOrAliasName, i);
//   KDbDbg << "addTable: " << tname;
            querySchema->addTable(s, aliasString);
        }
    }

    /* set parent table if there's only one */
    if (querySchema->tables()->count() == 1)
        querySchema->setMasterTable(querySchema->tables()->first());

    //-------add fields
    if (!colViews.isEmpty()) {
        columnNum = 0;
        bool containsAsteriskColumn = false; // used to check duplicated asterisks (disallowed)
        for (int i = 0; i < colViews.argCount(); i++, columnNum++) {
            KDbExpression e(colViews.arg(i));
            KDbExpression columnExpr(e);
            KDbVariableExpression aliasVariable;
            if (e.expressionClass() == KDb::SpecialBinaryExpression && e.isBinary()
                    && (e.token() == AS || e.token() == AS_EMPTY)) {
                //KDb::SpecialBinaryExpression: with alias
                columnExpr = e.toBinary().left();
                aliasVariable = e.toBinary().right().toVariable();
                if (aliasVariable.isNull()) {
                    setError(QObject::tr("Invalid alias definition for column \"%1\"")
                                  .arg(columnExpr.toString().toString())); //ok?
                    break;
                }
            }

            const int c = columnExpr.expressionClass();
            const bool isExpressionField =
                c == KDb::ConstExpression
                || c == KDb::UnaryExpression
                || c == KDb::ArithmeticExpression
                || c == KDb::LogicalExpression
                || c == KDb::RelationalExpression
                || c == KDb::FunctionExpression
                || c == KDb::AggregationExpression;

            if (c == KDb::VariableExpression) {
                if (columnExpr.toVariable().name() == QLatin1String("*")) {
                    if (containsAsteriskColumn) {
                        setError(QObject::tr("More than one asterisk (*) is not allowed"));
                        return 0;
                    }
                    else {
                        containsAsteriskColumn = true;
                    }
                }
                // addColumn() will handle this
            }
            else if (isExpressionField) {
                //expression object will be reused, take, will be owned, do not destroy
//  KDbDbg << colViews->list.count() << " " << it.current()->debugString();
#ifdef __GNUC__
#warning ok? //KDb: it.remove();
#else
#pragma WARNING(ok?)
#endif
            } else if (aliasVariable.isNull()) {
                setError(QObject::tr("Invalid \"%1\" column definition")
                         .arg(e.toString().toString())); //ok?
                break;
            }
            else {
                //take first (left) argument of the special binary expr, will be owned, do not destroy
                e.toBinary().setLeft(KDbExpression());
            }

            if (!addColumn(&parseInfo, &columnExpr)) {
                break;
            }

            if (!aliasVariable.isNull()) {
//    KDbDbg << "ALIAS \"" << aliasVariable->name << "\" set for column "
//     << columnNum;
                querySchema->setColumnAlias(columnNum, aliasVariable.name());
            }
        } // for
        if (!globalParser->error().message().isEmpty()) { // we could not return earlier (inside the loop)
                                                          // because we want run CLEANUP what could crash QMutableListIterator.
            return 0;
        }
    }
    //----- SELECT options
    if (options) {
        //----- WHERE expr.
        if (!options->whereExpr.isNull()) {
            if (!options->whereExpr.validate(&parseInfo)) {
                setError(parseInfo.errorMessage(), parseInfo.errorDescription());
                return 0;
            }
            querySchema->setWhereExpression(options->whereExpr);
        }
        //----- ORDER BY
        if (options->orderByColumns) {
            KDbOrderByColumnList *orderByColumnList = querySchema->orderByColumnList();
            uint count = options->orderByColumns->count();
            OrderByColumnInternal::ListConstIterator it(options->orderByColumns->constEnd());
            --it;
            for (;count > 0; --it, --count)
                /*opposite direction due to parser specifics*/
            {
                //first, try to find a column name or alias (outside of asterisks)
                KDbQueryColumnInfo *columnInfo = querySchema->columnInfo((*it).aliasOrName, false/*outside of asterisks*/);
                if (columnInfo) {
                    orderByColumnList->appendColumn(*columnInfo, (*it).ascending);
                } else {
                    //failed, try to find a field name within all the tables
                    if ((*it).columnNumber != -1) {
                        if (!orderByColumnList->appendColumn(*querySchema,
                                                            (*it).ascending, (*it).columnNumber - 1)) {
                            setError(QObject::tr("Could not define sorting - no column at position %1")
                                          .arg((*it).columnNumber));
                            return 0;
                        }
                    } else {
                        KDbField * f = querySchema->findTableField((*it).aliasOrName);
                        if (!f) {
                            setError(QObject::tr("Could not define sorting - "
                                          "column name or alias \"%1\" does not exist").arg((*it).aliasOrName));
                            return 0;
                        }
                        orderByColumnList->appendField(*f, (*it).ascending);
                    }
                }
            }
        }
    }
// KDbDbg << "Select ColViews=" << (colViews ? colViews->debugString() : QString())
//  << " Tables=" << (tablesList ? tablesList->debugString() : QString()s);
    return querySchema;
}

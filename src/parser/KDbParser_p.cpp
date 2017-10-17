/* This file is part of the KDE project
   Copyright (C) 2004-2017 Jarosław Staniek <staniek@kde.org>

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
#include "KDb.h"
#include "KDbConnection.h"
#include "KDbTableSchema.h"
#include "KDbQueryAsterisk.h"
#include "KDbQuerySchema.h"
#include "KDbQuerySchema_p.h"
#include "KDbOrderByColumn.h"
#include "kdb_debug.h"
#include "generated/sqlparser.h"

#include <QMutableListIterator>

KDbParser *globalParser = nullptr;
KDbField *globalField = nullptr;
QList<KDbField*> fieldList;
int globalCurrentPos = 0;
QByteArray globalToken;

extern int yylex_destroy(void);

//-------------------------------------

KDbParserPrivate::KDbParserPrivate()
    : table(nullptr), query(nullptr), connection(nullptr), initialized(false)
{
    reset();
}

KDbParserPrivate::~KDbParserPrivate()
{
    reset();
}

void KDbParserPrivate::reset()
{
    statementType = KDbParser::NoType;
    sql.clear();
    error = KDbParserError();
    delete table;
    table = nullptr;
    delete query;
    query = nullptr;
}

void KDbParserPrivate::setStatementType(KDbParser::StatementType type)
{
    statementType = type;
}

void KDbParserPrivate::setError(const KDbParserError &err)
{
    error = err;
}

void KDbParserPrivate::setTableSchema(KDbTableSchema *table)
{
    delete this->table;
    this->table = table;
}

void KDbParserPrivate::setQuerySchema(KDbQuerySchema *query)
{
    if (this->query != query) {
        delete this->query;
    }
    this->query = query;
}

KDbQuerySchema* KDbParserPrivate::createQuery()
{
    return query ? query : new KDbQuerySchema;
}

//-------------------------------------

KDbParseInfo::KDbParseInfo(KDbQuerySchema *query)
 : d(new Private)
{
    d->querySchema = query;
}

KDbParseInfo::~KDbParseInfo()
{
    delete d;
}

QList<int> KDbParseInfo::tablesAndAliasesForName(const QString &tableOrAliasName) const
{
    QList<int> result;
    const QList<int> *list = d->repeatedTablesAndAliases.value(tableOrAliasName);
    if (list) {
        result = *list;
    }
    if (result.isEmpty()) {
        int position = d->querySchema->tablePositionForAlias(tableOrAliasName);
        if (position == -1) {
            position = d->querySchema->tablePosition(tableOrAliasName);
            if (position != -1) {
                result.append(position);
            }
        } else {
            result.append(position);
        }
    }
    return result;
}

KDbQuerySchema* KDbParseInfo::querySchema() const
{
    return d->querySchema;
}

QString KDbParseInfo::errorMessage() const
{
    return d->errorMessage;
}

QString KDbParseInfo::errorDescription() const
{
    return d->errorDescription;
}

void KDbParseInfo::setErrorMessage(const QString &message)
{
    d->errorMessage = message;
}

void KDbParseInfo::setErrorDescription(const QString &description)
{
    d->errorDescription = description;
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

//-------------------------------------

extern int yyparse();
extern void tokenize(const char *data);

void yyerror(const char *str)
{
    kdbDebug() << "error: " << str;
    kdbDebug() << "at character " << globalCurrentPos << " near tooken " << globalToken;
    KDbParserPrivate::get(globalParser)->setStatementType(KDbParser::NoType);

    const bool otherError = (qstrnicmp(str, "other error", 11) == 0);
    const bool syntaxError = qstrnicmp(str, "syntax error", 12) == 0;
    if ((globalParser->error().type().isEmpty()
         && (str == nullptr || strlen(str) == 0 || syntaxError))
        || otherError)
    {
        kdbDebug() << globalParser->statement();
        QString ptrline(globalCurrentPos, QLatin1Char(' '));

        ptrline += QLatin1String("^");

        kdbDebug() << ptrline;

#if 0
        //lexer may add error messages
        QString lexerErr = globalParser->error().message();

        QString errtypestr = QLatin1String(str);
        if (lexerErr.isEmpty()) {
            if (errtypestr.startsWith(QString::fromLatin1("parse error, expecting `IDENTIFIER'"))) {
                lexerErr = KDbParser::tr("identifier was expected");
            }
        }
#endif

        //! @todo exact invalid expression can be selected in the editor, based on KDbParseInfo data
        if (!otherError) {
            const bool isKDbSqlKeyword = KDb::isKDbSqlKeyword(globalToken);
            if (isKDbSqlKeyword || syntaxError) {
                if (isKDbSqlKeyword) {
                    KDbParserPrivate::get(globalParser)->setError(KDbParserError(KDbParser::tr("Syntax Error"),
                                                          KDbParser::tr("\"%1\" is a reserved keyword.").arg(QLatin1String(globalToken)),
                                                          globalToken, globalCurrentPos));
                } else {
                    KDbParserPrivate::get(globalParser)->setError(KDbParserError(KDbParser::tr("Syntax Error"),
                                                          KDbParser::tr("Syntax error."),
                                                          globalToken, globalCurrentPos));
                }
            } else {
                KDbParserPrivate::get(globalParser)->setError(KDbParserError(KDbParser::tr("Error"),
                                                      KDbParser::tr("Error near \"%1\".").arg(QLatin1String(globalToken)),
                                                      globalToken, globalCurrentPos));
            }
        }
    }
}

void setError(const QString& errName, const QString& errDesc)
{
    KDbParserPrivate::get(globalParser)->setError(KDbParserError(errName, errDesc, globalToken, globalCurrentPos));
    yyerror(qPrintable(errName));
}

void setError(const QString& errDesc)
{
    setError(KDbParser::tr("Other error"), errDesc);
}

/* this is better than assert() */
#define IMPL_ERROR(errmsg) setError(KDbParser::tr("Implementation error"), QLatin1String(errmsg))

//! @internal Parses @a data for parser @a p
//! @todo Make it REENTRANT
bool parseData()
{
    fieldList.clear();

    const KDbEscapedString sql(globalParser->statement());
    if (sql.isEmpty()) {
        KDbParserError err(KDbParser::tr("Error"),
                           KDbParser::tr("No query statement specified."),
                           globalToken, globalCurrentPos);
        KDbParserPrivate::get(globalParser)->setError(err);
        yyerror("");
        return false;
    }

    const char *data = sql.constData();
    tokenize(data);
    if (!globalParser->error().type().isEmpty()) {
        return false;
    }

    bool ok = yyparse() == 0;
    if (ok && globalCurrentPos < sql.length()) {
        kdbDebug() << "Parse error: tokens left"
                   << "globalCurrentPos:" << globalCurrentPos << "sql.length():" << sql.length()
                   << "globalToken:" << QString::fromUtf8(globalToken);
        KDbParserError err(KDbParser::tr("Error"),
                           KDbParser::tr("Unexpected character."),
                           globalToken, globalCurrentPos);
        KDbParserPrivate::get(globalParser)->setError(err);
        yyerror("");
        ok = false;
    }
    if (ok && globalParser->statementType() == KDbParser::Select) {
        kdbDebug() << "parseData(): ok";
//   kdbDebug() << "parseData(): " << tableDict.count() << " loaded tables";
        /*   KDbTableSchema *ts;
              for(QDictIterator<KDbTableSchema> it(tableDict); KDbTableSchema *s = tableList.first(); s; s = tableList.next())
              {
                kdbDebug() << " " << s->name();
              }*/
    } else {
        ok = false;
    }
    yylex_destroy();
    return ok;
}


/*! Adds @a columnExpr to @a parseInfo
 The column can be in a form table.field, tableAlias.field or field.
 @return true on success. On error message in globalParser object is updated.
*/
bool addColumn(KDbParseInfo *parseInfo, const KDbExpression &columnExpr)
{
    if (!KDbExpression(columnExpr).validate(parseInfo)) { // (KDbExpression(columnExpr) used to avoid constness problem)
        setError(parseInfo->errorMessage(), parseInfo->errorDescription());
        return false;
    }

    const KDbVariableExpression v_e(columnExpr.toVariable());
    if (columnExpr.expressionClass() == KDb::VariableExpression && !v_e.isNull()) {
        //it's a variable:
        if (v_e.name() == QLatin1String("*")) {//all tables asterisk
            if (parseInfo->querySchema()->tables()->isEmpty()) {
                setError(KDbParser::tr("\"*\" could not be used if no tables are specified."));
                return false;
            }
            KDbQueryAsterisk *a = new KDbQueryAsterisk(parseInfo->querySchema());
            if (!parseInfo->querySchema()->addAsterisk(a)) {
                delete a;
                setError(KDbParser::tr("\"*\" could not be added."));
                return false;
            }
        } else if (v_e.tableForQueryAsterisk()) {//one-table asterisk
            KDbQueryAsterisk *a = new KDbQueryAsterisk(parseInfo->querySchema(), *v_e.tableForQueryAsterisk());
            if (!parseInfo->querySchema()->addAsterisk(a)) {
                delete a;
                setError(KDbParser::tr("\"<table>.*\" could not be added."));
                return false;
            }
        } else if (v_e.field()) {//"table.field" or "field" (bound to a table or not)
            if (!parseInfo->querySchema()->addField(v_e.field(), v_e.tablePositionForField())) {
                setError(KDbParser::tr("Could not add binding to a field."));
                return false;
            }
        } else {
            IMPL_ERROR("addColumn(): unknown case!");
            return false;
        }
        return true;
    }

    //it's complex expression
    return parseInfo->querySchema()->addExpression(columnExpr);
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
    QScopedPointer<KDbQuerySchema> querySchemaPtr(querySchema); // destroy query on any error

    //-------tables list
    int columnNum = 0;
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
                Q_ASSERT(e.isBinary());
                Q_ASSERT(t_with_alias.left().expressionClass() == KDb::VariableExpression);
                Q_ASSERT(t_with_alias.right().expressionClass() == KDb::VariableExpression
                       && (t_with_alias.token() == KDbToken::AS || t_with_alias.token() == KDbToken::AS_EMPTY));
                t_e = t_with_alias.left().toVariable();
                aliasString = t_with_alias.right().toVariable().name();
            } else {
                t_e = e.toVariable();
            }
            Q_ASSERT(t_e.isVariable());
            QString tname = t_e.name();
            KDbTableSchema *s = globalParser->connection()->tableSchema(tname);
            if (!s) {
                setError(KDbParser::tr("Table \"%1\" does not exist.").arg(tname));
                return nullptr;
            }
            QString tableOrAliasName = KDb::iifNotEmpty(aliasString, tname);
            if (!aliasString.isEmpty()) {
//    kdbDebug() << "- add alias for table: " << aliasString;
                const int tablePosition = querySchema->tablePositionForAlias(aliasString);
                if (tablePosition != -1 && tablePosition != i) {
                    KDbTableSchema* tableForAlias = querySchema->tables()->at(tablePosition);
                    setError(KDbParser::tr("Could not set alias \"%1\" for table \"%2\". "
                                           "This alias is already set for table \"%3\".")
                             .arg(aliasString).arg(tname).arg(tableForAlias->name()));
                    break;
                }
            }
            // 1. collect information about first repeated table name or alias
            //    (potential ambiguity)
            parseInfo.appendPositionForTableOrAliasName(tableOrAliasName, i);
//   kdbDebug() << "addTable: " << tname;
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
            const KDbExpression e(colViews.arg(i));
            KDbExpression columnExpr(e);
            KDbVariableExpression aliasVariable;
            if (e.expressionClass() == KDb::SpecialBinaryExpression && e.isBinary()
                    && (e.token() == KDbToken::AS || e.token() == KDbToken::AS_EMPTY)) {
                //KDb::SpecialBinaryExpression: with alias
                columnExpr = e.toBinary().left();
                aliasVariable = e.toBinary().right().toVariable();
                if (aliasVariable.isNull()) {
                    setError(KDbParser::tr("Invalid alias definition for column \"%1\".")
                                           .arg(columnExpr.toString(nullptr).toString())); //ok?
                    break;
                }
            }

            const KDb::ExpressionClass c = columnExpr.expressionClass();
            const bool isExpressionField =
                c == KDb::ConstExpression
                || c == KDb::UnaryExpression
                || c == KDb::ArithmeticExpression
                || c == KDb::LogicalExpression
                || c == KDb::RelationalExpression
                || c == KDb::FunctionExpression
                || c == KDb::AggregationExpression
                || c == KDb::QueryParameterExpression;

            if (c == KDb::VariableExpression) {
                if (columnExpr.toVariable().name() == QLatin1String("*")) {
                    if (containsAsteriskColumn) {
                        setError(KDbParser::tr("More than one asterisk \"*\" is not allowed."));
                        return nullptr;
                    }
                    else {
                        containsAsteriskColumn = true;
                    }
                }
                // addColumn() will handle this
            }
            else if (isExpressionField) {
                //expression object will be reused, take, will be owned, do not destroy
//  kdbDebug() << colViews->list.count() << " " << it.current()->debugString();
//! @todo IMPORTANT: it.remove();
            } else if (aliasVariable.isNull()) {
                setError(KDbParser::tr("Invalid \"%1\" column definition.")
                                       .arg(e.toString(nullptr).toString())); //ok?
                break;
            }
            else {
                //take first (left) argument of the special binary expr, will be owned, do not destroy
                e.toBinary().setLeft(KDbExpression());
            }

            if (!addColumn(&parseInfo, columnExpr)) {
                break;
            }

            if (!aliasVariable.isNull()) {
//    kdbDebug() << "ALIAS \"" << aliasVariable->name << "\" set for column "
//     << columnNum;
                const int currentColumn = querySchema->columnPositionForAlias(aliasVariable.name());
                if (currentColumn != -1) {
                    setError(KDbParser::tr("Could not set alias \"%1\" for column #%2. This alias is already set for column #%3.")
                             .arg(aliasVariable.name()).arg(columnNum + 1).arg(currentColumn + 1));
                    break;
                }
                if (!querySchema->setColumnAlias(columnNum, aliasVariable.name())) {
                    setError(KDbParser::tr("Could not set alias \"%1\" for column #%2.")
                             .arg(aliasVariable.name()).arg(columnNum + 1));
                    break;
                }
            }
        } // for
        if (!globalParser->error().message().isEmpty()) { // we could not return earlier (inside the loop)
                                                          // because we want run CLEANUP what could crash QMutableListIterator.
            return nullptr;
        }
    }
    //----- SELECT options
    if (options) {
        //----- WHERE expr.
        if (!options->whereExpr.isNull()) {
            if (!options->whereExpr.validate(&parseInfo)) {
                setError(parseInfo.errorMessage(), parseInfo.errorDescription());
                return nullptr;
            }
            KDbQuerySchemaPrivate::setWhereExpressionInternal(querySchema, options->whereExpr);
        }
        //----- ORDER BY
        if (options->orderByColumns) {
            KDbOrderByColumnList *orderByColumnList = querySchema->orderByColumnList();
            int count = options->orderByColumns->count();
            QList<OrderByColumnInternal>::ConstIterator it(options->orderByColumns->constEnd());
            --it;
            for (;count > 0; --it, --count)
                /*opposite direction due to parser specifics*/
            {
                // first, try to find a column name or alias (outside of asterisks)
                KDbQueryColumnInfo *columnInfo = querySchema->columnInfo(
                    globalParser->connection(), (*it).aliasOrName,
                    KDbQuerySchema::ExpandMode::Unexpanded /*outside of asterisks*/);
                if (columnInfo) {
                    orderByColumnList->appendColumn(columnInfo, (*it).order);
                } else {
                    //failed, try to find a field name within all the tables
                    if ((*it).columnNumber != -1) {
                        if (!orderByColumnList->appendColumn(globalParser->connection(),
                                                             querySchema, (*it).order,
                                                             (*it).columnNumber - 1))
                        {
                            setError(KDbParser::tr("Could not define sorting. Column at "
                                                   "position %1 does not exist.")
                                                   .arg((*it).columnNumber));
                            return nullptr;
                        }
                    } else {
                        KDbField * f = querySchema->findTableField((*it).aliasOrName);
                        if (!f) {
                            setError(KDbParser::tr("Could not define sorting. "
                                                   "Column name or alias \"%1\" does not exist.")
                                                   .arg((*it).aliasOrName));
                            return nullptr;
                        }
                        orderByColumnList->appendField(f, (*it).order);
                    }
                }
            }
        }
    }
// kdbDebug() << "Select ColViews=" << (colViews ? colViews->debugString() : QString())
//  << " Tables=" << (tablesList ? tablesList->debugString() : QString()s);
    return querySchemaPtr.take();
}

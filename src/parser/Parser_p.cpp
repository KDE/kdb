/* This file is part of the KDE project
   Copyright (C) 2004-2007 Jarosław Staniek <staniek@kde.org>

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

#include "Parser_p.h"
#include "SqlParser.h"

#include <QRegExp>
#include <QMutableListIterator>

#include <assert.h>

using namespace Predicate;

Parser *parser = 0;
Field *field = 0;
//bool requiresTable;
QList<Field*> fieldList;
int current = 0;
QByteArray ctoken;

extern int yylex_destroy(void);

//-------------------------------------

Parser::Private::Private()
        : initialized(false)
{
    clear();
    table = 0;
    select = 0;
    db = 0;
}

Parser::Private::~Private()
{
    delete select;
    delete table;
}

void Parser::Private::clear()
{
    operation = Parser::OP_None;
    error = ParserError();
}

//-------------------------------------

ParseInfo::ParseInfo(Predicate::QuerySchema *query)
        : querySchema(query)
{
//Qt 4 repeatedTablesAndAliases.setAutoDelete(true);
}

ParseInfo::~ParseInfo()
{
}

//-------------------------------------

extern int yyparse();
extern void tokenize(const char *data);

void yyerror(const char *str)
{
    PreDbg << "error: " << str;
    PreDbg << "at character " << current << " near tooken " << ctoken;
    parser->setOperation(Parser::OP_Error);

    const bool otherError = (qstrnicmp(str, "other error", 11) == 0);

    if ((   parser->error().type().isEmpty()
         && (str == 0 || strlen(str) == 0 || qstrnicmp(str, "syntax error", 12) == 0 || qstrnicmp(str, "parse error", 11) == 0)
        )
        || otherError
       )
    {
        PreDbg << parser->statement();
        QString ptrline;
        for (int i = 0; i < current; i++) {
            ptrline += QLatin1String(" ");
        }

        ptrline += QLatin1String("^");

        PreDbg << ptrline;

        //lexer may add error messages
        QString lexerErr = parser->error().error();

        QString errtypestr = QLatin1String(str);
        if (lexerErr.isEmpty()) {
#if 0
            if (errtypestr.startsWith("parse error, unexpected ")) {
                //something like "parse error, unexpected IDENTIFIER, expecting ',' or ')'"
                QString e = errtypestr.mid(24);
                PreDbg << e;
                QString token = "IDENTIFIER";
                if (e.startsWith(token)) {
                    QRegExp re("'.'");
                    int pos = 0;
                    pos = re.search(e, pos);
                    QStringList captured = re.capturedTexts();
                    if (captured.count() >= 2) {
//      PreDbg << "**" << captured.at(1);
//      PreDbg << "**" << captured.at(2);
                    }
                }



//    IDENTIFIER, expecting '")) {
                e = errtypestr.mid(47);
                PreDbg << e;
//    ,' or ')'
//  lexerErr QObject::tr("identifier was expected");

            } else
#endif
            if (errtypestr.startsWith(QString::fromLatin1("parse error, expecting `IDENTIFIER'"))) {
                lexerErr = QObject::tr("identifier was expected");
            }
        }

        if (!otherError) {
            if (!lexerErr.isEmpty()) {
                lexerErr.prepend(QLatin1String(": "));
            }

            if (Predicate::isPredicateSQLKeyword(ctoken))
                parser->setError(ParserError(QObject::tr("Syntax Error"),
                                             QObject::tr("\"%1\" is a reserved keyword")
                                                .arg(QLatin1String(ctoken)) + lexerErr,
                                             ctoken, current));
            else
                parser->setError(ParserError(QObject::tr("Syntax Error"),
                                             QObject::tr("Syntax Error near \"%1\"")
                                                .arg(QLatin1String(ctoken)) + lexerErr,
                                             ctoken, current));
        }
    }
}

void setError(const QString& errName, const QString& errDesc)
{
    parser->setError(ParserError(errName, errDesc, ctoken, current));
    yyerror(errName.toLatin1());
}

void setError(const QString& errDesc)
{
    setError(QObject::tr("Other error"), errDesc);
}

/* this is better than assert() */
#define IMPL_ERROR(errmsg) setError(QObject::tr("Implementation error"), QLatin1String(errmsg))

bool parseData(Parser *p, const char *data)
{
    /* todo: make this REENTRANT */
    parser = p;
    parser->clear();
    field = 0;
    fieldList.clear();
// requiresTable = false;

    if (!data) {
        ParserError err(QObject::tr("Error"), QObject::tr("No query specified"), ctoken, current);
        parser->setError(err);
        yyerror("");
        parser = 0;
        return false;
    }

    tokenize(data);
    if (!parser->error().type().isEmpty()) {
        parser = 0;
        return false;
    }
    yyparse();

    bool ok = true;
    if (parser->operation() == Parser::OP_Select) {
        PreDbg << "parseData(): ok";
//   PreDbg << "parseData(): " << tableDict.count() << " loaded tables";
        /*   TableSchema *ts;
              for(QDictIterator<TableSchema> it(tableDict); TableSchema *s = tableList.first(); s; s = tableList.next())
              {
                PreDbg << " " << s->name();
              }*/
        /*removed
              Field::ListIterator it = parser->select()->fieldsIterator();
              for(Field *item; (item = it.current()); ++it)
              {
                if(tableList.findRef(item->table()) == -1)
                {
                  ParserError err(QObject::tr("Field List Error"), QObject::tr("Unknown table '%1' in field list",item->table()->name()), ctoken, current);
                  parser->setError(err);

                  yyerror("fieldlisterror");
                  ok = false;
                }
              }*/
        //take the dummy table out of the query
//   parser->select()->removeTable(dummy);
    } else {
        ok = false;
    }

//  tableDict.clear();
    yylex_destroy();
    parser = 0;
    return ok;
}


/* Adds @a column to @a querySchema. @a column can be in a form of
 table.field, tableAlias.field or field
*/
bool addColumn(ParseInfo& parseInfo, Expression& columnExpr)
{
    if (!columnExpr.validate(parseInfo)) {
        setError(parseInfo.errMsg, parseInfo.errDescr);
        return false;
    }

    VariableExpression v_e(columnExpr.toVariable());
    if (columnExpr.expressionClass() == VariableExpressionClass && !v_e.isNull()) {
        //it's a variable:
        if (v_e.name() == QLatin1String("*")) {//all tables asterisk
            if (parseInfo.querySchema->tables()->isEmpty()) {
                setError(QObject::tr("\"*\" could not be used if no tables are specified"));
                return false;
            }
            parseInfo.querySchema->addAsterisk(new QueryAsterisk(parseInfo.querySchema));
        } else if (v_e.tableForQueryAsterisk()) {//one-table asterisk
            parseInfo.querySchema->addAsterisk(
                new QueryAsterisk(parseInfo.querySchema, v_e.tableForQueryAsterisk()));
        } else if (v_e.field()) {//"table.field" or "field" (bound to a table or not)
            parseInfo.querySchema->addField(v_e.field(), v_e.tablePositionForField());
        } else {
            IMPL_ERROR("addColumn(): unknown case!");
            return false;
        }
        return true;
    }

    //it's complex expression
    parseInfo.querySchema->addExpression(columnExpr);

#if 0
    PreDbg << "found variable name: " << varName;
    int dotPos = varName.find('.');
    QString tableName, fieldName;
//! @todo shall we also support db name?
    if (dotPos > 0) {
        tableName = varName.left(dotPos);
        fieldName = varName.mid(dotPos + 1);
    }
    if (tableName.isEmpty()) {//fieldname only
        fieldName = varName;
        if (fieldName == "*") {
            parseInfo.querySchema->addAsterisk(new QueryAsterisk(parseInfo.querySchema));
        } else {
            //find first table that has this field
            Field *firstField = 0;
            foreach(TableSchema *ts, *parseInfo.querySchema->tables()) {
                Field *f = ts->field(fieldName);
                if (f) {
                    if (!firstField) {
                        firstField = f;
                    } else if (f->table() != firstField->table()) {
                        //ambiguous field name
                        setError(QObject::tr("Ambiguous field name"),
                                 QObject::tr("Both table \"%1\" and \"%2\" have defined \"%3\" field. "
                                      "Use \"<tableName>.%4\" notation to specify table name.")
                                      .arg(firstField->table()->name(), f->table()->name()
                                      , fieldName, fieldName));
                        return false;
                    }
                }
            }
            if (!firstField) {
                setError(QObject::tr("Field not found"),
                         QObject::tr("Table containing \"%1\" field not found").arg(fieldName));
                return false;
            }
            //ok
            parseInfo.querySchema->addField(firstField);
        }
    } else {//table.fieldname or tableAlias.fieldname
        tableName = tableName.toLower();
        TableSchema *ts = parseInfo.querySchema->table(tableName);
        if (ts) {//table.fieldname
            //check if "table" is covered by an alias
            const QList<int> tPositions(parseInfo.querySchema->tablePositions(tableName));
            QList<int>::ConstIterator it = tPositions.constBegin();
            QByteArray tableAlias;
            bool covered = true;
            foreach(int position, tPositions) {
                tableAlias = parseInfo.querySchema->tableAlias(position).toLower();
                if (tableAlias.isEmpty() || tableAlias == tableName.toLatin1()) {
                    covered = false; //uncovered
                    break;
                }
                PreDbg << " --" << "covered by " << tableAlias << " alias";
            }
            if (covered) {
                setError(QObject::tr("Could not access the table directly using its name"),
                         QObject::tr("Table \"%1\" is covered by aliases. Instead of \"%2\", "
                              "you can write \"%3\"")
                              .arg( tableName
                                , (tableName + "." + fieldName)
                                , tableAlias + "." + fieldName.toLatin1()));
                return false;
            }
        }

        int tablePosition = -1;
        if (!ts) {//try to find tableAlias
            tablePosition = parseInfo.querySchema->tablePositionForAlias(tableName.toLatin1());
            if (tablePosition >= 0) {
                ts = parseInfo.querySchema->tables()->at(tablePosition);
                if (ts) {
//     PreDbg << " --it's a tableAlias.name";
                }
            }
        }


        if (ts) {
            if (!repeatedTablesAndAliases.contains(tableName)) {
                IMPL_ERROR(tableName + "." + fieldName + ", !positionsList ");
                return false;
            }
            const QList<int> positionsList(repeatedTablesAndAliases.value(tableName));

            if (fieldName == "*") {
                if (positionsList.count() > 1) {
                    setError(QObject::tr("Ambiguous \"%1.*\" expression", tableName),
                             QObject::tr("More than one \"%1\" table or alias defined").arg(tableName));
                    return false;
                }
                parseInfo.querySchema->addAsterisk(new QueryAsterisk(parseInfo.querySchema, ts));
            } else {
//    PreDbg << " --it's a table.name";
                Field *realField = ts->field(fieldName);
                if (realField) {
                    // check if table or alias is used twice and both have the same column
                    // (so the column is ambiguous)
                    int numberOfTheSameFields = 0;
                    foreach(int position, positionsList) {
                        TableSchema *otherTS = parseInfo.querySchema->tables()->at(position);
                        if (otherTS->field(fieldName))
                            numberOfTheSameFields++;
                        if (numberOfTheSameFields > 1) {
                            setError(QObject::tr("Ambiguous \"%1.%2\" expression").arg(tableName, fieldName),
                                     QObject::tr("More than one \"%1\" table or alias defined containing \"%2\" field")
                                          .arg(tableName, fieldName));
                            return false;
                        }
                    }

                    parseInfo.querySchema->addField(realField, tablePosition);
                } else {
                    setError(QObject::tr("Field not found"), QObject::tr("Table \"%1\" has no \"%2\" field")
                                                           .arg(tableName, fieldName));
                    return false;
                }
            }
        } else {
            tableNotFoundError(tableName);
            return false;
        }
    }
#endif
    return true;
}

//! clean up no longer needed temporary objects
#define CLEANUP \
    /*delete colViews;*/ \
    /*delete tablesList;*/ \
    delete options

QuerySchema* buildSelectQuery(
    QuerySchema* querySchema, NArgExpression* _colViews,
    NArgExpression* _tablesList, SelectOptionsInternal* options)
{
    ParseInfo parseInfo(querySchema);

    // remove from heap (using heap was requered because parser uses union)
    Predicate::NArgExpression colViews(*_colViews);
    delete _colViews;

    // remove from heap (using heap was requered because parser uses union)
    Predicate::NArgExpression tablesList(*_tablesList);
    delete _tablesList;

    //-------tables list
// assert( tablesList ); //&& tablesList->exprClass() == TableListExpressionClass );

    uint columnNum = 0;
    /*TODO: use this later if there are columns that use database fields,
            e.g. "SELECT 1 from table1 t, table2 t") is ok however. */
    //used to collect information about first repeated table name or alias:
// QDict<char> tableNamesAndTableAliases(997, false);
// QString repeatedTableNameOrTableAlias;
    if (!tablesList.isEmpty()) {
        for (int i = 0; i < tablesList.argCount(); i++, columnNum++) {
            Expression e(tablesList.arg(i));
            VariableExpression t_e;
            QString aliasString;
            if (e.expressionClass() == SpecialBinaryExpressionClass) {
                BinaryExpression t_with_alias = e.toBinary();
                assert(e.isBinary());
                assert(t_with_alias.left().expressionClass() == VariableExpressionClass);
                assert(t_with_alias.right().expressionClass() == VariableExpressionClass
                       && (t_with_alias.token() == AS || t_with_alias.token() == 0));
                t_e = t_with_alias.left().toVariable();
                aliasString = t_with_alias.right().toVariable().name();
            } else {
                t_e = e.toVariable();
            }
            assert(t_e.isVariable());
            QString tname = t_e.name();
            TableSchema *s = parser->db()->tableSchema(tname);
            if (!s) {
                setError(//QObject::tr("Field List Error"),
                    QObject::tr("Table \"%1\" does not exist").arg(tname));
                //   yyerror("fieldlisterror");
                CLEANUP;
                return 0;
            }
            QString tableOrAliasName;
            if (!aliasString.isEmpty()) {
                tableOrAliasName = aliasString;
//    PreDbg << "- add alias for table: " << aliasString;
            } else {
                tableOrAliasName = tname;
            }
            // 1. collect information about first repeated table name or alias
            //    (potential ambiguity)
            QList<int> list(parseInfo.repeatedTablesAndAliases.value(tableOrAliasName));
            list.append(i);
            parseInfo.repeatedTablesAndAliases.insert(tableOrAliasName, list);
            /*  if (repeatedTableNameOrTableAlias.isEmpty()) {
                  if (tableNamesAndTableAliases[tname])
                    repeatedTableNameOrTableAlias=tname;
                  else
                    tableNamesAndTableAliases.insert(tname, (const char*)1);
                }
                if (!aliasString.isEmpty()) {
                  PreDbg << "- add alias for table: " << aliasString;
            //   querySchema->setTableAlias(columnNum, aliasString);
                  //2. collect information about first repeated table name or alias
                  //   (potential ambiguity)
                  if (repeatedTableNameOrTableAlias.isEmpty()) {
                    if (tableNamesAndTableAliases[aliasString])
                      repeatedTableNameOrTableAlias=aliasString;
                    else
                      tableNamesAndTableAliases.insert(aliasString, (const char*)1);
                  }
                }*/
//   PreDbg << "addTable: " << tname;
            querySchema->addTable(s, aliasString);
        }
    }

    /* set parent table if there's only one */
// if (parser->select()->tables()->count()==1)
    if (querySchema->tables()->count() == 1)
        querySchema->setMasterTable(querySchema->tables()->first());

    //-------add fields
    if (!colViews.isEmpty()) {
        columnNum = 0;
        for (int i = 0; i < colViews.argCount(); i++, columnNum++) {
            Expression e(colViews.arg(i));
//Qt4   bool moveNext = true; //used to avoid ++it when an item is taken from the list
            Expression columnExpr(e);
            VariableExpression aliasVariable;
            if (e.expressionClass() == SpecialBinaryExpressionClass && e.isBinary()
                    && (e.token() == AS || e.token() == 0)) {
                //SpecialBinaryExpressionClass: with alias
                columnExpr = e.toBinary().left();
                //   isFieldWithAlias = true;
                aliasVariable = e.toBinary().right().toVariable();
                if (aliasVariable.isNull()) {
                    setError(QObject::tr("Invalid alias definition for column \"%1\"")
                                  .arg(columnExpr.toString().toString())); //ok?
                    break;
                }
            }

            const int c = columnExpr.expressionClass();
            const bool isExpressionField =
                c == ConstExpressionClass
                || c == UnaryExpressionClass
                || c == ArithmeticExpressionClass
                || c == LogicalExpressionClass
                || c == RelationalExpressionClass
                || c == ConstExpressionClass
                || c == FunctionExpressionClass
                || c == AggregationExpressionClass;

            if (c == VariableExpressionClass) {
                //just a variable, do nothing, addColumn() will handle this
            } else if (isExpressionField) {
                //expression object will be reused, take, will be owned, do not destroy
//  PreDbg << colViews->list.count() << " " << it.current()->debugString();
#warning ok?  //predicate: it.remove();
//Qt4    moveNext = false;
            } else if (aliasVariable.isNull()) {
                setError(QObject::tr("Invalid \"%1\" column definition")
                         .arg(e.toString().toString())); //ok?
                break;
            }
            else {
                //take first (left) argument of the special binary expr, will be owned, do not destroy
                e.toBinary().setLeft(Expression());
            }

            if (!addColumn(parseInfo, columnExpr)) {
                break;
            }

            if (!aliasVariable.isNull()) {
//    PreDbg << "ALIAS \"" << aliasVariable->name << "\" set for column "
//     << columnNum;
                querySchema->setColumnAlias(columnNum, aliasVariable.name());
            }
            /*  if (e->exprClass() == SpecialBinaryExpressionClass && dynamic_cast<BinaryExpression*>(e)
                  && (e->type()==AS || e->type()==0))
                {
                  //also add alias
                  VariableExpr* aliasVariable =
                    dynamic_cast<VariableExpr*>(dynamic_cast<BinaryExpression*>(e)->right());
                  if (!aliasVariable) {
                    setError(QObject::tr("Invalid column alias definition")); //ok?
                    return 0;
                  }
                  PreDbg << "ALIAS \"" << aliasVariable->name << "\" set for column "
                    << columnNum;
                  querySchema->setColumnAlias(columnNum, aliasVariable->name.toLatin1());
                }*/

//Qt4   if (moveNext) {
//Qt4    colViews->list.next();
//Qt4   }
        } // for
        if (!parser->error().error().isEmpty()) { // we could not return earlier (inside the loop)
            // because we want run CLEANUP what could crash QMutableListIterator.
            CLEANUP;
            return 0;
        }
    }
    //----- SELECT options
    if (options) {
        //----- WHERE expr.
        if (!options->whereExpr.isNull()) {
            if (!options->whereExpr.validate(parseInfo)) {
                setError(parseInfo.errMsg, parseInfo.errDescr);
                CLEANUP;
                return false;
            }
            querySchema->setWhereExpression(options->whereExpr);
        }
        //----- ORDER BY
        if (options->orderByColumns) {
            OrderByColumnList &orderByColumnList = querySchema->orderByColumnList();
            uint count = options->orderByColumns->count();
            OrderByColumnInternal::ListConstIterator it(options->orderByColumns->constEnd());
            --it;
            for (;count > 0; --it, --count)
                /*opposite direction due to parser specifics*/
            {
                //first, try to find a column name or alias (outside of asterisks)
                QueryColumnInfo *columnInfo = querySchema->columnInfo((*it).aliasOrName, false/*outside of asterisks*/);
                if (columnInfo) {
                    orderByColumnList.appendColumn(*columnInfo, (*it).ascending);
                } else {
                    //failed, try to find a field name within all the tables
                    if ((*it).columnNumber != -1) {
                        if (!orderByColumnList.appendColumn(*querySchema,
                                                            (*it).ascending, (*it).columnNumber - 1)) {
                            setError(QObject::tr("Could not define sorting - no column at position %1")
                                          .arg((*it).columnNumber));
                            CLEANUP;
                            return 0;
                        }
                    } else {
                        Field * f = querySchema->findTableField((*it).aliasOrName);
                        if (!f) {
                            setError(QObject::tr("Could not define sorting - "
                                          "column name or alias \"%1\" does not exist").arg((*it).aliasOrName));
                            CLEANUP;
                            return 0;
                        }
                        orderByColumnList.appendField(*f, (*it).ascending);
                    }
                }
            }
        }
    }

// PreDbg << "Select ColViews=" << (colViews ? colViews->debugString() : QString())
//  << " Tables=" << (tablesList ? tablesList->debugString() : QString()s);

    CLEANUP;
    return querySchema;
}

#undef CLEANUP


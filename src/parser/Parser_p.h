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

#ifndef PREDICATE_PARSER_P_H
#define PREDICATE_PARSER_P_H

#include <QList>
#include <QHash>
#include <QByteArray>
#include <QCache>
#include <QString>

#include <Predicate/QuerySchema.h>
#include <Predicate/TableSchema.h>
#include <Predicate/Connection.h>
#include <Predicate/Expression.h>
#include "SqlTypes.h"
#include "Parser.h"

namespace Predicate
{

//! @internal
class Parser::Private
{
public:
    Private();
    ~Private();

    void clear();

    int operation;
    TableSchema *table;
    QuerySchema *select;
    Connection *db;
    EscapedString statement;
    ParserError error;
    bool initialized;
};


/*! Data used on parsing. @internal */
class ParseInfo
{
public:
    ParseInfo(QuerySchema *query);
    ~ParseInfo();

    //! collects positions of tables/aliases with the same names
    QHash< QString, QList<int> > repeatedTablesAndAliases;

    QString errMsg, errDescr; //helpers
    QuerySchema *querySchema;
};

}

void yyerror(const char *str);
void setError(const QString& errName, const QString& errDesc);
void setError(const QString& errDesc);
//bool parseData(Predicate::Parser *p, const char *data);
bool addColumn(Predicate::ParseInfo& parseInfo, Predicate::BaseExpr* columnExpr);
Predicate::QuerySchema* buildSelectQuery(
    Predicate::QuerySchema* querySchema, Predicate::NArgExpr* colViews,
    Predicate::NArgExpr* tablesList = 0, SelectOptionsInternal * options = 0); //Predicate::BaseExpr* whereExpr = 0 );

extern Predicate::Parser *parser;
extern Predicate::Field *field;


#endif

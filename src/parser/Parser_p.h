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

#ifndef PREDICATE_PARSER_P_H
#define PREDICATE_PARSER_P_H

#include <QList>
#include <QHash>
#include <QByteArray>
#include <QCache>
#include <QString>

#include <Predicate/QuerySchema>
#include <Predicate/TableSchema>
#include <Predicate/Connection>
#include <Predicate/Expression>
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

class ParseInfo::Private
{
public:
    Private() {}
    ~Private() {
        qDeleteAll(repeatedTablesAndAliases);
    }

    //! collects positions of tables/aliases with the same names
    QHash< QString, QList<int>* > repeatedTablesAndAliases;

    QString errorMessage, errorDescription; // helpers

    QuerySchema *querySchema;
};

/*! Internal info used on parsing (writable). */
class PREDICATE_TEST_EXPORT ParseInfoInternal : public ParseInfo
{
public:
    //! Constructs parse info structure for query @a query.
    explicit ParseInfoInternal(QuerySchema *query);

    ~ParseInfoInternal();

    //! Appends position @a pos for table or alias @a tableOrAliasName.
    void appendPositionForTableOrAliasName(const QString &tableOrAliasName, int pos);

    //! Sets error message to @a message.
    void setErrorMessage(const QString &message);

    //! Sets error description to @a description.
    void setErrorDescription(const QString &description);

private:
    Q_DISABLE_COPY(ParseInfoInternal)
};

}

void yyerror(const char *str);
void setError(const QString& errName, const QString& errDesc);
void setError(const QString& errDesc);
bool addColumn(Predicate::ParseInfo* parseInfo, Predicate::Expression* columnExpr);
Predicate::QuerySchema* buildSelectQuery(
    Predicate::QuerySchema* querySchema, Predicate::NArgExpression* colViews,
    Predicate::NArgExpression* tablesList = 0, SelectOptionsInternal * options = 0);

extern Predicate::Parser *parser;
extern Predicate::Field *field;


#endif

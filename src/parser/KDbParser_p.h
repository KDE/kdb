/* This file is part of the KDE project
   Copyright (C) 2004-2015 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_PARSER_P_H
#define KDB_PARSER_P_H

#include "config-kdb.h"
#include "KDbParser.h"
#include "KDbQuerySchema.h"
#include "KDbTableSchema.h"
#include "KDbConnection.h"
#include "KDbExpression.h"
#include "KDbSqlTypes.h"

#include <QList>
#include <QHash>
#include <QCache>
#include <QString>

//! @internal
class KDbParser::Private
{
public:
    Private();
    ~Private();

    void clear();

    int operation;
    KDbTableSchema *table;
    KDbQuerySchema *select;
    KDbConnection *db;
    KDbEscapedString sql;
    KDbParserError error;
    bool initialized;
};

class KDbParseInfo::Private
{
public:
    Private() {}
    ~Private() {
        qDeleteAll(repeatedTablesAndAliases);
    }

    //! collects positions of tables/aliases with the same names
    QHash< QString, QList<int>* > repeatedTablesAndAliases;

    QString errorMessage, errorDescription; // helpers

    KDbQuerySchema *querySchema;
};

/*! Internal info used on parsing (writable). */
class KDB_TESTING_EXPORT KDbParseInfoInternal : public KDbParseInfo
{
public:
    //! Constructs parse info structure for query @a query.
    explicit KDbParseInfoInternal(KDbQuerySchema *query);

    ~KDbParseInfoInternal();

    //! Appends position @a pos for table or alias @a tableOrAliasName.
    void appendPositionForTableOrAliasName(const QString &tableOrAliasName, int pos);

private:
    Q_DISABLE_COPY(KDbParseInfoInternal)
};

void yyerror(const char *str);
void setError(const QString& errName, const QString& errDesc);
void setError(const QString& errDesc);
bool addColumn(KDbParseInfo* parseInfo, KDbExpression* columnExpr);
KDbQuerySchema* buildSelectQuery(
    KDbQuerySchema* querySchema, KDbNArgExpression* colViews,
    KDbNArgExpression* tablesList = 0, SelectOptionsInternal * options = 0);

extern KDbParser *globalParser;
extern KDbField *globalField;

#endif

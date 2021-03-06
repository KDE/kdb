/****************************************************************************
 * Created by generate_parser_code.sh
 * WARNING! All changes made in this file will be lost!
 ****************************************************************************/
#ifndef KDBSQLPARSER_H
#define KDBSQLPARSER_H

#include "KDbDateTime.h"
#include "KDbExpression.h"
#include "KDbField.h"
#include "KDbOrderByColumn.h"

struct OrderByColumnInternal;
struct SelectOptionsInternal;

/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_KDBSQLPARSER_TAB_H_INCLUDED
# define YY_YY_KDBSQLPARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    SQL_TYPE = 258,
    AS = 259,
    AS_EMPTY = 260,
    ASC = 261,
    AUTO_INCREMENT = 262,
    BIT = 263,
    BITWISE_SHIFT_LEFT = 264,
    BITWISE_SHIFT_RIGHT = 265,
    BY = 266,
    CHARACTER_STRING_LITERAL = 267,
    CONCATENATION = 268,
    CREATE = 269,
    DESC = 270,
    DISTINCT = 271,
    DOUBLE_QUOTED_STRING = 272,
    FROM = 273,
    JOIN = 274,
    KEY = 275,
    LEFT = 276,
    LESS_OR_EQUAL = 277,
    GREATER_OR_EQUAL = 278,
    SQL_NULL = 279,
    SQL_IS = 280,
    SQL_IS_NULL = 281,
    SQL_IS_NOT_NULL = 282,
    ORDER = 283,
    PRIMARY = 284,
    SELECT = 285,
    INTEGER_CONST = 286,
    REAL_CONST = 287,
    RIGHT = 288,
    SQL_ON = 289,
    DATE_CONST = 290,
    DATETIME_CONST = 291,
    TIME_CONST = 292,
    TABLE = 293,
    IDENTIFIER = 294,
    IDENTIFIER_DOT_ASTERISK = 295,
    QUERY_PARAMETER = 296,
    VARCHAR = 297,
    WHERE = 298,
    SQL = 299,
    SQL_TRUE = 300,
    SQL_FALSE = 301,
    UNION = 302,
    SCAN_ERROR = 303,
    AND = 304,
    BETWEEN = 305,
    NOT_BETWEEN = 306,
    EXCEPT = 307,
    SQL_IN = 308,
    INTERSECT = 309,
    LIKE = 310,
    ILIKE = 311,
    NOT_LIKE = 312,
    NOT = 313,
    NOT_EQUAL = 314,
    NOT_EQUAL2 = 315,
    OR = 316,
    SIMILAR_TO = 317,
    NOT_SIMILAR_TO = 318,
    XOR = 319,
    UMINUS = 320,
    TABS_OR_SPACES = 321,
    DATE_TIME_INTEGER = 322,
    TIME_AM = 323,
    TIME_PM = 324
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 505 "KDbSqlParser.y" /* yacc.c:1909  */

    QString* stringValue;
    QByteArray* binaryValue;
    qint64 integerValue;
    bool booleanValue;
    KDbDate* dateValue;
    KDbYear* yearValue;
    KDbTime* timeValue;
    KDbTime::Period timePeriodValue;
    KDbDateTime* dateTimeValue;
    KDbOrderByColumn::SortOrder sortOrderValue;
    KDbField::Type colType;
    KDbField *field;
    KDbExpression *expr;
    KDbNArgExpression *exprList;
    KDbConstExpression *constExpression;
    KDbQuerySchema *querySchema;
    SelectOptionsInternal *selectOptions;
    QList<OrderByColumnInternal> *orderByColumns;
    QVariant *variantValue;

#line 146 "KDbSqlParser.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_KDBSQLPARSER_TAB_H_INCLUDED  */
#endif

#ifndef _SQLPARSER_H_
#define _SQLPARSER_H_
#include <Predicate/Field.h>
#include "Parser.h"
#include "SqlTypes.h"

bool parseData(Predicate::Parser *p, const char *data);
/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
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


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
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
     SQL_NULL = 278,
     SQL_IS = 279,
     SQL_IS_NULL = 280,
     SQL_IS_NOT_NULL = 281,
     ORDER = 282,
     PRIMARY = 283,
     SELECT = 284,
     INTEGER_CONST = 285,
     REAL_CONST = 286,
     RIGHT = 287,
     SQL_ON = 288,
     DATE_CONST = 289,
     DATETIME_CONST = 290,
     TIME_CONST = 291,
     TABLE = 292,
     IDENTIFIER = 293,
     IDENTIFIER_DOT_ASTERISK = 294,
     QUERY_PARAMETER = 295,
     VARCHAR = 296,
     WHERE = 297,
     SQL = 298,
     SQL_TRUE = 299,
     SQL_FALSE = 300,
     SCAN_ERROR = 301,
     __LAST_TOKEN = 302,
     EXCEPT = 303,
     UNION = 304,
     INTERSECT = 305,
     OR = 306,
     XOR = 307,
     AND = 308,
     NOT = 309,
     GREATER_OR_EQUAL = 310,
     NOT_EQUAL2 = 311,
     NOT_EQUAL = 312,
     NOT_SIMILAR_TO = 313,
     SIMILAR_TO = 314,
     ILIKE = 315,
     LIKE = 316,
     SQL_IN = 317,
     BETWEEN = 318,
     UMINUS = 319
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 2068 of yacc.c  */
#line 514 "SqlParser.y"

    QString* stringValue;
    qint64 integerValue;
    bool booleanValue;
    struct realType realValue;
    Predicate::Field::Type colType;
    Predicate::Field *field;
    Predicate::Expression *expr;
    Predicate::NArgExpression *exprList;
    Predicate::ConstExpression *constExpression;
    Predicate::QuerySchema *querySchema;
    SelectOptionsInternal *selectOptions;
    OrderByColumnInternal::List *orderByColumns;
    QVariant *variantValue;



/* Line 2068 of yacc.c  */
#line 132 "SqlParser.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


#endif

/* This file is part of the KDE project
   Copyright (C) 2004 Lucijan Busch <lucijan@kde.org>
   Copyright (C) 2004, 2006 Jarosław Staniek <staniek@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

// To keep binary compatibility, do not reorder tokens! Add new only at the end.
%token SQL_TYPE
%token AS
%token AS_EMPTY /* used for aliases with skipped AS keyword */
%token ASC
%token AUTO_INCREMENT
%token BIT
%token BITWISE_SHIFT_LEFT
%token BITWISE_SHIFT_RIGHT
%token BY
%token CHARACTER_STRING_LITERAL
%token CONCATENATION /* || */
%token CREATE
%token DESC
%token DISTINCT
%token DOUBLE_QUOTED_STRING
%token FROM
%token JOIN
%token KEY
%token LEFT
%token LESS_OR_EQUAL
%token SQL_NULL
%token SQL_IS
%token SQL_IS_NULL /*helper */
%token SQL_IS_NOT_NULL /*helper */
%token ORDER
%token PRIMARY
%token SELECT
%token INTEGER_CONST
%token REAL_CONST
%token RIGHT
%token SQL_ON
%token DATE_CONST
%token DATETIME_CONST
%token TIME_CONST
%token TABLE
%token IDENTIFIER
%token IDENTIFIER_DOT_ASTERISK
%token QUERY_PARAMETER
%token VARCHAR
%token WHERE
%token SQL
%token SQL_TRUE
%token SQL_FALSE

%token SCAN_ERROR

//%token UMINUS
//%token SQL_ABS
//%token ACOS
//%token AMPERSAND
//%token SQL_ABSOLUTE
//%token ADA
//%token ADD
//%token ADD_DAYS
//%token ADD_HOURS
//%token ADD_MINUTES
//%token ADD_MONTHS
//%token ADD_SECONDS
//%token ADD_YEARS
//%token ALL
//%token ALLOCATE
//%token ALTER
//%token AND
//%token ANY
//%token ARE
//%token ASIN
//%token ASCII
//%token ASSERTION
//%token ATAN
//%token ATAN2
//%token AUTHORIZATION
//%token AVG
//%token BEFORE
//%token SQL_BEGIN
//%token BIGINT
//%token BINARY
//%token BIT_LENGTH
//%token BREAK
//%token CASCADE
//%token CASCADED
//%token CASE
//%token CAST
//%token CATALOG
//%token CEILING
//%token CENTER
//%token SQL_CHAR
//%token CHAR_LENGTH
//%token CHECK
//%token CLOSE
//%token COALESCE
//%token COBOL
//%token COLLATE
//%token COLLATION
//%token COLUMN
//%token COMMIT
//%token COMPUTE
//%token CONCAT
//%token CONNECT
//%token CONNECTION
//%token CONSTRAINT
//%token CONSTRAINTS
//%token CONTINUE
//%token CONVERT
//%token CORRESPONDING
//%token COS
//%token COT
//%token COUNT
//%token CURDATE
//%token CURRENT
//%token CURRENT_DATE
//%token CURRENT_TIME
//%token CURRENT_TIMESTAMP
//%token CURTIME
//%token CURSOR
//%token DATABASE
//%token SQL_DATE
//%token DATE_FORMAT
//%token DATE_REMAINDER
//%token DATE_VALUE
//%token DAY
//%token DAYOFMONTH
//%token DAYOFWEEK
//%token DAYOFYEAR
//%token DAYS_BETWEEN
//%token DEALLOCATE
//%token DEC
//%token DECLARE
//%token DEFAULT
//%token DEFERRABLE
//%token DEFERRED
//%token SQL_DELETE
//%token DESCRIBE
//%token DESCRIPTOR
//%token DIAGNOSTICS
//%token DICTIONARY
//%token DIRECTORY
//%token DISCONNECT
//%token DISPLACEMENT
//%token DOMAIN_TOKEN
//%token SQL_DOUBLE
//%token DROP
//%token ELSE
//%token END
//%token END_EXEC
//%token ESCAPE
//%token EXCEPT
//%token SQL_EXCEPTION
//%token EXEC
//%token EXECUTE
//%token EXISTS
//%token EXP
//%token EXPONENT
//%token EXTERNAL
//%token EXTRACT
//%token FETCH
//%token FIRST
//%token SQL_FLOAT
//%token FLOOR
//%token FN
//%token FOR
//%token FOREIGN
//%token FORTRAN
//%token FOUND
//%token FOUR_DIGITS
//%token FULL
//%token GET
//%token GLOBAL
//%token GO
//%token GOTO
//%token GRANT
//conflict %token GROUP
//%token HAVING
//%token HOUR
//%token HOURS_BETWEEN
//%token IDENTITY
//%token IFNULL
//%token SQL_IGNORE
//%token IMMEDIATE
//%token INCLUDE
//%token INDEX
//%token INDICATOR
//%token INITIALLY
//%token INNER
//%token SQL_INPUT
//%token INSENSITIVE
//%token INSERT
//%token INTEGER
//%token INTERSECT
//%token INTERVAL
//%token INTO
//%token IS
//%token ISOLATION
//%token JUSTIFY
//%token LANGUAGE
//%token LAST
//%token LCASE
//%token LENGTH
//%token LEVEL
//%token LINE_WIDTH
//%token LOCAL
//%token LOCATE
//%token LOG
//%token SQL_LONG
//%token LOWER
//%token LTRIM
//%token LTRIP
//%token MATCH
//%token SQL_MAX
//%token MICROSOFT
//%token SQL_MIN
//%token MINUTE
//%token MINUTES_BETWEEN
//%token MOD
//%token MODIFY
//%token MODULE
//%token MONTH
//%token MONTHS_BETWEEN
//%token MUMPS
//%token NAMES
//%token NATIONAL
//%token NCHAR
//%token NEXT
//%token NODUP
//%token NONE
//%token NOT
//%token NOW
//%token NULLIF
//%token NUMERIC
//%token OCTET_LENGTH
//%token ODBC
//%token OF
//%token SQL_OFF
//%token ONLY
//%token OPEN
//%token OPTION
//%token OR
//%token OUTER
//%token OUTPUT
//%token OVERLAPS
//%token PAGE
//%token PARTIAL
//%token SQL_PASCAL
//%token PERSISTENT
//%token CQL_PI
//%token PLI
//%token POSITION
//%token PRECISION
//%token PREPARE
//%token PRESERVE
//%token PRIOR
//%token PRIVILEGES
//%token PROCEDURE
//%token PRODUCT
//%token PUBLIC
//%token QUARTER
//%token QUIT
//%token RAND
//%token READ_ONLY
//%token REAL
//%token REFERENCES
//%token REPEAT
//%token REPLACE
//%token RESTRICT
//%token REVOKE
//%token ROLLBACK
//%token ROWS
//%token RPAD
//%token RTRIM
//%token SCHEMA
//%token SCREEN_WIDTH
//%token SCROLL
//%token SECOND
//%token SECONDS_BETWEEN
//%token SEQUENCE
//%token SETOPT
//%token SET
//%token SHOWOPT
//%token SIGN
//%token SIMILAR
//%token SIN
//%token SQL_SIZE
//%token SMALLINT
//%token SOME
//%token SPACE
//%token SQLCA
//%token SQLCODE
//%token SQLERROR
//%token SQLSTATE
//%token SQLWARNING
//%token SQRT
//%token STDEV
//%token SUBSTRING
//%token SUM
//%token SYSDATE
//%token SYSDATE_FORMAT
//%token SYSTEM
//%token TAN
//%token TEMPORARY
//%token THEN
//%token THREE_DIGITS
//%token TIME
//%token TIMESTAMP
//%token TIMEZONE_HOUR
//%token TIMEZONE_MINUTE
//%token TINYINT
//%token TO
//%token TO_CHAR
//%token TO_DATE
//%token TRANSACTION
//%token TRANSLATE
//%token TRANSLATION
//%token TRUNCATE
//%token GENERAL_TITLE
//%token TWO_DIGITS
//%token UCASE
//%token UNION
//%token UNIQUE
//%token SQL_UNKNOWN
//%token UNSIGNED_INTEGER
//%token UPDATE
//%token UPPER
//%token USAGE
//%token USER
//%token ERROR_DIGIT_BEFORE_IDENTIFIER
//%token USING
//%token VALUE
//%token VALUES
//%token VARBINARY
//%token VARYING
//%token VENDOR
//%token VIEW
//%token WEEK
//%token WHEN
//%token WHENEVER
//%token WHERE_CURRENT_OF
//%token WITH
//%token WORD_WRAPPED
//%token WORK
//%token WRAPPED
//%token XOR
//%token YEAR
//%token YEARS_BETWEEN

%token '-'
%token '+'
%token '*'
%token '%'
%token '@'
%token ';'
%token ','
%token '.'
%token '$'
//%token '<'
//%token '>'
%token '('
%token ')'
%token '?'
%token '\''
%token '/'

%type <stringValue> IDENTIFIER
%type <stringValue> IDENTIFIER_DOT_ASTERISK
%type <stringValue> QUERY_PARAMETER
%type <stringValue> CHARACTER_STRING_LITERAL
%type <stringValue> DOUBLE_QUOTED_STRING

/*
%type <field> ColExpression
%type <field> ColView
*/
%type <expr> ColExpression
%type <expr> ColWildCard
//%type <expr> ColView
%type <expr> ColItem
%type <exprList> ColViews
%type <expr> aExpr
%type <expr> aExpr2
%type <expr> aExpr3
%type <expr> aExpr4
%type <expr> aExpr5
%type <expr> aExpr6
%type <expr> aExpr7
%type <expr> aExpr8
%type <expr> aExpr9
%type <expr> aExpr10
%type <exprList> aExprList
%type <exprList> aExprList2
%type <expr> WhereClause
%type <orderByColumns> OrderByClause
%type <booleanValue> OrderByOption
%type <variantValue> OrderByColumnId
%type <selectOptions> SelectOptions
%type <expr> FlatTable
%type <exprList> Tables
%type <exprList> FlatTableList
%type <querySchema> SelectStatement
%type <querySchema> Select
/*todo : list*/
%type <querySchema> StatementList
/*todo: not onlu select*/
%type <querySchema> Statement

%type <colType> SQL_TYPE
%type <integerValue> INTEGER_CONST
%type <realValue> REAL_CONST
/*%type <integerValue> SIGNED_INTEGER */

%{
#include <stdio.h>
#include <string.h>
#include <string>
#include <iostream>
#include <assert.h>
#include <limits.h>
//TODO OK?
#ifdef Q_WS_WIN
//workaround for bug on msvc
# undef LLONG_MIN
#endif
#ifndef LLONG_MAX
# define LLONG_MAX     0x7fffffffffffffffLL
#endif
#ifndef LLONG_MIN
# define LLONG_MIN     0x8000000000000000LL
#endif
#ifndef LLONG_MAX
# define ULLONG_MAX    0xffffffffffffffffLL
#endif

#ifdef _WIN32
# include <malloc.h>
#endif

#include <QObject>
#include <QList>
#include <QVariant>
#include <QPoint>

#include "KDbConnection.h"
#include "KDbQuerySchema.h"
#include "KDbField.h"
#include "KDbTableSchema.h"

#include "KDbParser.h"
#include "KDbParser_p.h"
#include "KDbSqlTypes.h"
#ifdef Q_OS_SOLARIS
#include <alloca.h>
#endif

inline static KDbExpression fromPtr(KDbExpression* e)
{
    KDbExpression exp(*e);
    delete e;
    return exp;
}

QDebug operator<<(QDebug dbg, const KDbExpressionPtr& expr)
{
    dbg.nospace() << expr.e;
    return dbg.space();
}

int yylex();

#define YY_NO_UNPUT
#define YYSTACK_USE_ALLOCA 1
#define YYMAXDEPTH 255

    extern "C"
    {
        int yywrap()
        {
            return 1;
        }
    }

%}

%union {
    QString* stringValue;
    qint64 integerValue;
    bool booleanValue;
    struct realType realValue;
    KDbField::Type colType;
    KDbField *field;
    KDbExpression *expr;
    KDbNArgExpression *exprList;
    KDbConstExpression *constExpression;
    KDbQuerySchema *querySchema;
    SelectOptionsInternal *selectOptions;
    OrderByColumnInternal::List *orderByColumns;
    QVariant *variantValue;
}

//%left '=' NOT_EQUAL '>' GREATER_OR_EQUAL '<' LESS_OR_EQUAL LIKE '%' NOT
//%left '+' '-'
//%left ASTERISK SLASH

/* precedence: lowest to highest */
%left     UNION
%left     EXCEPT
%left     INTERSECT
%left     OR
%left     AND
%left     XOR
%right    NOT
%nonassoc '='
%nonassoc '<'
%nonassoc '>'
%nonassoc LESS_OR_EQUAL
%nonassoc GREATER_OR_EQUAL
%nonassoc NOT_EQUAL
%nonassoc NOT_EQUAL2
%nonassoc SQL_IN
%nonassoc LIKE
%nonassoc ILIKE
%nonassoc SIMILAR_TO
%nonassoc NOT_SIMILAR_TO
//%nonassoc    LIKE
//%nonassoc    ILIKE
//%nonassoc    SIMILAR
//%nonassoc    ESCAPE
//%nonassoc    OVERLAPS
%nonassoc    BETWEEN
//%nonassoc    IN_P
//%left        POSTFIXOP        // dummy for postfix Op rules
//%left        Op OPERATOR        // multi-character ops and user-defined operators
//%nonassoc    NOTNULL
//%nonassoc    ISNULL
//%nonassoc    IS               // sets precedence for IS NULL, etc
//%nonassoc    NULL_P
//%nonassoc    TRUE_P
//%nonassoc    FALSE_P
//%nonassoc    UNKNOWN
%left        '+'
%left        '-'
%left        '*'
%left        '/'
%left        '%'
%left        '^'
%left        UMINUS
// Unary Operators 
//%left        AT ZONE            // sets precedence for AT TIME ZONE
//%right        UMINUS
%left        '['
%left        ']'
%left        '('
%left        ')'
//%left        TYPECAST
%left        '.'

%nonassoc NOT_LIKE

// <-- To keep binary compatibility insert new tokens here.

/*
 * These might seem to be low-precedence, but actually they are not part
 * of the arithmetic hierarchy at all in their use as JOIN operators.
 * We make them high-precedence to support their use as function names.
 * They wouldn't be given a precedence at all, were it not that we need
 * left-associativity among the JOIN rules themselves.
 */
/*
%left JOIN
%left UNIONJOIN
%left CROSS
%left LEFT
%left FULL
%left RIGHT
%left INNER_P
%left NATURAL
*/

%%

TopLevelStatement :
StatementList
{
//todo: multiple statements
//todo: not only "select" statements
    globalParser->setOperation(KDbParser::OP_Select);
    globalParser->setQuerySchema($1);
}
;

StatementList:
Statement ';' StatementList
{
//todo: multiple statements
}
| Statement
| Statement ';'
{
    $$ = $1;
}
;

/*        Statement CreateTableStatement         { YYACCEPT; }
    | Statement SelectStatement         {  }
*/
Statement :
CreateTableStatement
{
YYACCEPT;
}
| SelectStatement
{
    $$ = $1;
}
;

CreateTableStatement :
CREATE TABLE IDENTIFIER
{
    globalParser->setOperation(KDbParser::OP_CreateTable);
    globalParser->createTable($3->toLatin1());
    delete $3;
}
'(' ColDefs ')'
;

ColDefs:
ColDefs ',' ColDef|ColDef
{
}
;

ColDef:
IDENTIFIER ColType
{
    KDbDbg << "adding field " << *$1;
    globalField->setName(*$1);
    globalParser->table()->addField(globalField);
    globalField = 0;
    delete $1;
}
| IDENTIFIER ColType ColKeys
{
    KDbDbg << "adding field " << *$1;
    globalField->setName(*$1);
    delete $1;
    globalParser->table()->addField(globalField);

//    if(globalField->isPrimaryKey())
//        globalParser->table()->addPrimaryKey(globalField->name());

//    delete globalField;
//    globalField = 0;
}
;

ColKeys:
ColKeys ColKey|ColKey
{
}
;

ColKey:
PRIMARY KEY
{
    globalField->setPrimaryKey(true);
    KDbDbg << "primary";
}
| NOT SQL_NULL
{
    globalField->setNotNull(true);
    KDbDbg << "not_null";
}
| AUTO_INCREMENT
{
    globalField->setAutoIncrement(true);
    KDbDbg << "ainc";
}
;

ColType:
SQL_TYPE
{
    globalField = new KDbField();
    globalField->setType($1);
}
| SQL_TYPE '(' INTEGER_CONST ')'
{
    KDbDbg << "sql + length";
    globalField = new KDbField();
    globalField->setPrecision($3);
    globalField->setType($1);
}
| VARCHAR '(' INTEGER_CONST ')'
{
    globalField = new KDbField();
    globalField->setPrecision($3);
    globalField->setType(KDbField::Text);
}
|
{
    // SQLITE compatibillity
    globalField = new KDbField();
    globalField->setType(KDbField::InvalidType);
}
;

SelectStatement:
Select
{
    KDbDbg << "Select";
    if (!($$ = buildSelectQuery( $1, 0 )))
        return 0;
}
| Select ColViews
{
    KDbDbg << "Select ColViews=" << *$2;

    if (!($$ = buildSelectQuery( $1, $2 )))
        return 0;
}
| Select ColViews Tables
{
    if (!($$ = buildSelectQuery( $1, $2, $3 )))
        return 0;
}
| Select Tables
{
    KDbDbg << "Select ColViews Tables";
    if (!($$ = buildSelectQuery( $1, 0, $2 )))
        return 0;
}
| Select ColViews SelectOptions
{
    KDbDbg << "Select ColViews Conditions";
    if (!($$ = buildSelectQuery( $1, $2, 0, $3 )))
        return 0;
}
| Select ColViews Tables SelectOptions
{
    KDbDbg << "Select ColViews Tables SelectOptions";
    if (!($$ = buildSelectQuery( $1, $2, $3, $4 )))
        return 0;
}
;

Select:
SELECT
{
    KDbDbg << "SELECT";
//    globalParser->createSelect();
//    globalParser->setOperation(KDbParser::OP_Select);
    $$ = new KDbQuerySchema();
}
;

SelectOptions: /* todo: more options (having, group by, limit...) */
WhereClause
{
    KDbDbg << "WhereClause";
    $$ = new SelectOptionsInternal;
    $$->whereExpr = *$1;
    delete $1;
}
| ORDER BY OrderByClause
{
    KDbDbg << "OrderByClause";
    $$ = new SelectOptionsInternal;
    $$->orderByColumns = $3;
}
| WhereClause ORDER BY OrderByClause
{
    KDbDbg << "WhereClause ORDER BY OrderByClause";
    $$ = new SelectOptionsInternal;
    $$->whereExpr = *$1;
    delete $1;
    $$->orderByColumns = $4;
} 
| ORDER BY OrderByClause WhereClause
{
    KDbDbg << "OrderByClause WhereClause";
    $$ = new SelectOptionsInternal;
    $$->whereExpr = *$4;
    delete $4;
    $$->orderByColumns = $3;
}
;

WhereClause:
WHERE aExpr
{
    $$ = $2;
}
;

/* todo: support "ORDER BY NULL" as described here http://dev.mysql.com/doc/refman/5.1/en/select.html */
/* todo: accept expr and position as well */
OrderByClause:
OrderByColumnId
{
    KDbDbg << "ORDER BY IDENTIFIER";
    $$ = new OrderByColumnInternal::List;
    OrderByColumnInternal orderByColumn;
    orderByColumn.setColumnByNameOrNumber( *$1 );
    $$->append( orderByColumn );
    delete $1;
}
| OrderByColumnId OrderByOption
{
    KDbDbg << "ORDER BY IDENTIFIER OrderByOption";
    $$ = new OrderByColumnInternal::List;
    OrderByColumnInternal orderByColumn;
    orderByColumn.setColumnByNameOrNumber( *$1 );
    orderByColumn.ascending = $2;
    $$->append( orderByColumn );
    delete $1;
}
| OrderByColumnId ',' OrderByClause
{
    $$ = $3;
    OrderByColumnInternal orderByColumn;
    orderByColumn.setColumnByNameOrNumber( *$1 );
    $$->append( orderByColumn );
    delete $1;
}
| OrderByColumnId OrderByOption ',' OrderByClause
{
    $$ = $4;
    OrderByColumnInternal orderByColumn;
    orderByColumn.setColumnByNameOrNumber( *$1 );
    orderByColumn.ascending = $2;
    $$->append( orderByColumn );
    delete $1;
}
;

OrderByColumnId:
IDENTIFIER
{
    $$ = new QVariant( *$1 );
    KDbDbg << "OrderByColumnId: " << *$$;
    delete $1;
}
| IDENTIFIER '.' IDENTIFIER
{
    $$ = new QVariant( *$1 + QLatin1Char('.') + *$3 );
    KDbDbg << "OrderByColumnId: " << *$$;
    delete $1;
    delete $3;
}
| INTEGER_CONST
{
    $$ = new QVariant($1);
    KDbDbg << "OrderByColumnId: " << *$$;
}

OrderByOption:
ASC
{
    $$ = true;
}
| DESC
{
    $$ = false;
}
;

aExpr:
aExpr2
;

/* --- binary logical --- */
aExpr2:
aExpr3 AND aExpr2
{
//    KDbDbg << "AND " << $3.debugString();
    $$ = new KDbBinaryExpression(*$1, AND, *$3);
    delete $1;
    delete $3;
}
| aExpr3 OR aExpr2
{
    $$ = new KDbBinaryExpression(*$1, OR, *$3);
    delete $1;
    delete $3;
}
| aExpr3 XOR aExpr2
{
    $$ = new KDbBinaryExpression(*$1, XOR, *$3);
    delete $1;
    delete $3;
}
|
aExpr3
;

/* relational op precedence */
aExpr3:
aExpr4 '>' %prec GREATER_OR_EQUAL aExpr3
{
    $$ = new KDbBinaryExpression(*$1, '>', *$3);
    delete $1;
    delete $3;
}
| aExpr4 GREATER_OR_EQUAL aExpr3
{
    $$ = new KDbBinaryExpression(*$1, GREATER_OR_EQUAL, *$3);
    delete $1;
    delete $3;
}
| aExpr4 '<' %prec LESS_OR_EQUAL aExpr3
{
    $$ = new KDbBinaryExpression(*$1, '<', *$3);
    delete $1;
    delete $3;
}
| aExpr4 LESS_OR_EQUAL aExpr3
{
    $$ = new KDbBinaryExpression(*$1, LESS_OR_EQUAL, *$3);
    delete $1;
    delete $3;
}
| aExpr4 '=' aExpr3
{
    $$ = new KDbBinaryExpression(*$1, '=', *$3);
    delete $1;
    delete $3;
}
|
aExpr4
;

/* relational (equality) op precedence */
aExpr4:
aExpr5 NOT_EQUAL aExpr4
{
    $$ = new KDbBinaryExpression(*$1, NOT_EQUAL, *$3);
    delete $1;
    delete $3;
}
| aExpr5 NOT_EQUAL2 aExpr4
{
    $$ = new KDbBinaryExpression(*$1, NOT_EQUAL2, *$3);
    delete $1;
    delete $3;
}
| aExpr5 LIKE aExpr4
{
    $$ = new KDbBinaryExpression(*$1, LIKE, *$3);
    delete $1;
    delete $3;
}
| aExpr5 NOT_LIKE aExpr4
{
    $$ = new KDbBinaryExpression(*$1, NOT_LIKE, *$3);
    delete $1;
    delete $3;
}
| aExpr5 SQL_IN aExpr4
{
    $$ = new KDbBinaryExpression(*$1, SQL_IN, *$3);
    delete $1;
    delete $3;
}
| aExpr5 SIMILAR_TO aExpr4
{
    $$ = new KDbBinaryExpression(*$1, SIMILAR_TO, *$3);
    delete $1;
    delete $3;
}
| aExpr5 NOT_SIMILAR_TO aExpr4
{
    $$ = new KDbBinaryExpression(*$1, NOT_SIMILAR_TO, *$3);
    delete $1;
    delete $3;
}
|
aExpr5
;

/* --- unary logical right --- */
aExpr5:
aExpr5 SQL_IS_NULL
{
    $$ = new KDbUnaryExpression( SQL_IS_NULL, *$1 );
    delete $1;
}
| aExpr5 SQL_IS_NOT_NULL
{
    $$ = new KDbUnaryExpression( SQL_IS_NOT_NULL, *$1 );
    delete $1;
}
|
aExpr6
;

/* arithm. lowest precedence */
aExpr6:
aExpr7 BITWISE_SHIFT_LEFT aExpr6
{
    $$ = new KDbBinaryExpression(*$1, BITWISE_SHIFT_LEFT, *$3);
    delete $1;
    delete $3;
}
| aExpr7 BITWISE_SHIFT_RIGHT aExpr6
{
    $$ = new KDbBinaryExpression(*$1, BITWISE_SHIFT_RIGHT, *$3);
    delete $1;
    delete $3;
}
|
aExpr7
;

/* arithm. lower precedence */
aExpr7:
aExpr8 '+' aExpr7
{
    $$ = new KDbBinaryExpression(*$1, '+', *$3);
    delete $1;
    delete $3;
}
| aExpr8 CONCATENATION aExpr7
{
    $$ = new KDbBinaryExpression(*$1, CONCATENATION, *$3);
}
| aExpr8 '-' %prec UMINUS aExpr7
{
    $$ = new KDbBinaryExpression(*$1, '-', *$3);
    delete $1;
    delete $3;
}
| aExpr8 '&' aExpr7
{
    $$ = new KDbBinaryExpression(*$1, '&', *$3);
    delete $1;
    delete $3;
}
| aExpr8 '|' aExpr7
{
    $$ = new KDbBinaryExpression(*$1, '|', *$3);
    delete $1;
    delete $3;
}
|
aExpr8
;

/* arithm. higher precedence */
aExpr8:
aExpr9 '/' aExpr8
{
    $$ = new KDbBinaryExpression(*$1, '/', *$3);
    delete $1;
    delete $3;
}
| aExpr9 '*' aExpr8
{
    $$ = new KDbBinaryExpression(*$1, '*', *$3);
    delete $1;
    delete $3;
}
| aExpr9 '%' aExpr8
{
    $$ = new KDbBinaryExpression(*$1, '%', *$3);
    delete $1;
    delete $3;
}
|
aExpr9
;

/* parenthesis, unary operators, and terminals precedence */
aExpr9:
/* --- unary logical left --- */
'-' aExpr9
{
    $$ = new KDbUnaryExpression( '-', *$2 );
    delete $2;
}
| '+' aExpr9
{
    $$ = new KDbUnaryExpression( '+', *$2 );
    delete $2;
}
| '~' aExpr9
{
    $$ = new KDbUnaryExpression( '~', *$2 );
    delete $2;
}
| NOT aExpr9
{
    $$ = new KDbUnaryExpression( NOT, *$2 );
    delete $2;
}
| IDENTIFIER
{
    $$ = new KDbVariableExpression( *$1 );

//TODO: simplify this later if that's 'only one field name' expression
    KDbDbg << "  + identifier: " << *$1;
    delete $1;
}
| QUERY_PARAMETER
{
    $$ = new KDbQueryParameterExpression( *$1 );
    KDbDbg << "  + query parameter:" << *$$;
    delete $1;
}
| IDENTIFIER aExprList
{
    KDbDbg << "  + function:" << *$1 << "(" << *$2 << ")";
    $$ = new KDbFunctionExpression(*$1, *$2);
    delete $1;
    delete $2;
}
/*TODO: shall we also support db name? */
| IDENTIFIER '.' IDENTIFIER
{
    $$ = new KDbVariableExpression( *$1 + QLatin1Char('.') + *$3 );
    KDbDbg << "  + identifier.identifier:" << *$1 << "." << *$3;
    delete $1;
    delete $3;
}
| SQL_NULL
{
    $$ = new KDbConstExpression( SQL_NULL, QVariant() );
    KDbDbg << "  + NULL";
//    $$ = new KDbField();
    //$$->setName(QString::null);
}
| SQL_TRUE
{
    $$ = new KDbConstExpression( SQL_TRUE, true );
}
| SQL_FALSE
{
    $$ = new KDbConstExpression( SQL_FALSE, false );
}
| CHARACTER_STRING_LITERAL
{
    $$ = new KDbConstExpression( CHARACTER_STRING_LITERAL, *$1 );
    KDbDbg << "  + constant " << $1;
    delete $1;
}
| INTEGER_CONST
{
    QVariant val;
    if ($1 <= INT_MAX && $1 >= INT_MIN)
        val = (int)$1;
    else if ($1 <= UINT_MAX && $1 >= 0)
        val = (uint)$1;
    else if ($1 <= LLONG_MAX && $1 >= LLONG_MIN)
        val = (qint64)$1;

//    if ($1 < ULLONG_MAX)
//        val = (quint64)$1;
//TODO ok?

    $$ = new KDbConstExpression( INTEGER_CONST, val );
    KDbDbg << "  + int constant: " << val.toString();
}
| REAL_CONST
{
    $$ = new KDbConstExpression( REAL_CONST, QPoint( $1.integer, $1.fractional ) );
    KDbDbg << "  + real constant: " << $1.integer << "." << $1.fractional;
}
|
aExpr10
;


aExpr10:
'(' aExpr ')'
{
    KDbDbg << "(expr)";
    $$ = new KDbUnaryExpression('(', *$2);
    delete $2;
}
;

aExprList:
'(' aExprList2 ')'
{
//    $$ = new KDbNArgExpression(KDb::UnknownExpression, 0);
//    $$->add( $1 );
//    $$->add( $3 );
    $$ = $2;
}
;

aExprList2:
aExpr ',' aExprList2
{
    $$ = $3;
    $$->prepend( *$1 );
    delete $1;
}
| aExpr ',' aExpr
{
    $$ = new KDbNArgExpression(KDb::UnknownExpression, 0);
    $$->append( *$1 );
    $$->append( *$3 );
    delete $1;
    delete $3;
}
;

Tables:
FROM FlatTableList
{
    $$ = $2;
}
/*
| Tables LEFT JOIN IDENTIFIER SQL_ON ColExpression
{
    KDbDbg << "LEFT JOIN: '" << *$4 << "' ON " << $6;
    addTable($4->toQString());
    delete $4;
}
| Tables LEFT OUTER JOIN IDENTIFIER SQL_ON ColExpression
{
    KDbDbg << "LEFT OUTER JOIN: '" << $5 << "' ON " << $7;
    addTable($5);
}
| Tables INNER JOIN IDENTIFIER SQL_ON ColExpression
{
    KDbDbg << "INNER JOIN: '" << *$4 << "' ON " << $6;
    addTable($4->toQString());
    delete $4;
}
| Tables RIGHT JOIN IDENTIFIER SQL_ON ColExpression
{
    KDbDbg << "RIGHT JOIN: '" << *$4 << "' ON " << $6;
    addTable(*$4);
    delete $4;
}
| Tables RIGHT OUTER JOIN IDENTIFIER SQL_ON ColExpression
{
    KDbDbg << "RIGHT OUTER JOIN: '" << *$5 << "' ON " << $7;
    addTable($5->toQString());
    delete $5;
}*/
;

/*
FlatTableList:
aFlatTableList
{
    $$
}
;*/

FlatTableList:
FlatTableList ',' FlatTable
{
    $$ = $1;
    $$->append(*$3);
}
|FlatTable
{
    $$ = new KDbNArgExpression(KDb::TableListExpression, IDENTIFIER); //ok?
    $$->append(*$1);
    delete $1;
}
;

FlatTable:
IDENTIFIER
{
    KDbDbg << "FROM: '" << *$1 << "'";
    $$ = new KDbVariableExpression(*$1);

    /*
//TODO: this isn't ok for more tables:
    KDbField::ListIterator it = globalParser->select()->fieldsIterator();
    for(KDbField *item; (item = it.current()); ++it)
    {
        if(item->table() == dummy)
        {
            item->setTable(schema);
        }

        if(item->table() && !item->isQueryAsterisk())
        {
            KDbField *f = item->table()->field(item->name());
            if(!f)
            {
                KDbParserError err(QObject::tr("Field List Error"), QObject::tr("Unknown column '%1' in table '%2'",item->name(),schema->name()), ctoken, current);
                globalParser->setError(err);
                yyerror("fieldlisterror");
            }
        }
    }*/
    delete $1;
}
| IDENTIFIER IDENTIFIER
{
    //table + alias
    $$ = new KDbBinaryExpression(
        KDbVariableExpression(*$1), AS_EMPTY,
        KDbVariableExpression(*$2)
    );
    delete $1;
    delete $2;
}
| IDENTIFIER AS IDENTIFIER
{
    //table + alias
    $$ = new KDbBinaryExpression(
        KDbVariableExpression(*$1), AS,
        KDbVariableExpression(*$3)
    );
    delete $1;
    delete $3;
}
;



ColViews:
ColViews ',' ColItem
{
    $$ = $1;
    $$->append(*$3);
    delete $3;
    KDbDbg << "ColViews: ColViews , ColItem";
}
|ColItem
{
    $$ = new KDbNArgExpression(KDb::FieldListExpression, 0);
    $$->append(*$1);
    delete $1;
    KDbDbg << "ColViews: ColItem";
}
;

ColItem:
ColExpression
{
//    $$ = new KDbField();
//    dummy->addField($$);
//    $$->setExpression( $1 );
//    globalParser->select()->addField($$);
    $$ = $1;
    KDbDbg << " added column expr:" << *$1;
}
| ColWildCard
{
    $$ = $1;
    KDbDbg << " added column wildcard:" << *$1;
}
| ColExpression AS IDENTIFIER
{
    $$ = new KDbBinaryExpression(
        *$1, AS,
        KDbVariableExpression(*$3)
    );
    KDbDbg << " added column expr:" << *$$;
    delete $1;
    delete $3;
}
| ColExpression IDENTIFIER
{
    $$ = new KDbBinaryExpression(
        *$1, AS_EMPTY,
        KDbVariableExpression(*$2)
    );
    KDbDbg << " added column expr:" << *$$;
    delete $1;
    delete $2;
}
;

ColExpression:
aExpr
{
    $$ = $1;
}
/* HANDLED BY 'IDENTIFIER aExprList'
| IDENTIFIER '(' ColViews ')'
{
    $$ = new KDbFunctionExpression( $1, $3 );
}*/
/*
| SUM '(' ColExpression ')'
{
    KDbFunctionExpression(
//    $$ = new AggregationExpression( SUM,  );
//TODO
//    $$->setName("SUM(" + $3->name() + ")");
//wait    $$->containsGroupingAggregate(true);
//wait    globalParser->select()->grouped(true);
}
| SQL_MIN '(' ColExpression ')'
{
    $$ = $3;
//TODO
//    $$->setName("MIN(" + $3->name() + ")");
//wait    $$->containsGroupingAggregate(true);
//wait    globalParser->select()->grouped(true);
}
| SQL_MAX '(' ColExpression ')'
{
    $$ = $3;
//TODO
//    $$->setName("MAX(" + $3->name() + ")");
//wait    $$->containsGroupingAggregate(true);
//wait    globalParser->select()->grouped(true);
}
| AVG '(' ColExpression ')'
{
    $$ = $3;
//TODO
//    $$->setName("AVG(" + $3->name() + ")");
//wait    $$->containsGroupingAggregate(true);
//wait    globalParser->select()->grouped(true);
}*/
//?
| DISTINCT '(' ColExpression ')' 
{
    $$ = $3;
//! @todo DISTINCT '(' ColExpression ')'
//    $$->setName("DISTINCT(" + $3->name() + ")");
}
;

ColWildCard:
'*'
{
    $$ = new KDbVariableExpression(QLatin1String("*"));
    KDbDbg << "all columns";

//    KDbQueryAsterisk *ast = new KDbQueryAsterisk(globalParser->select(), dummy);
//    globalParser->select()->addAsterisk(ast);
//    requiresTable = true;
}
| IDENTIFIER '.' '*'
{
    QString s( *$1 );
    s += QLatin1String(".*");
    $$ = new KDbVariableExpression(s);
    KDbDbg << "  + all columns from " << s;
    delete $1;
}
/*| ERROR_DIGIT_BEFORE_IDENTIFIER
{
    $$ = new KDbVariableExpression($1);
    KDbDbg << "  Invalid identifier! " << $1;
    setError(QObject::tr("Invalid identifier \"%1\"",$1));
}*/
;

%%


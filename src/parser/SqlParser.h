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
     UMINUS = 258,
     SQL_TYPE = 259,
     SQL_ABS = 260,
     ACOS = 261,
     AMPERSAND = 262,
     SQL_ABSOLUTE = 263,
     ADA = 264,
     ADD = 265,
     ADD_DAYS = 266,
     ADD_HOURS = 267,
     ADD_MINUTES = 268,
     ADD_MONTHS = 269,
     ADD_SECONDS = 270,
     ADD_YEARS = 271,
     ALL = 272,
     ALLOCATE = 273,
     ALTER = 274,
     AND = 275,
     ANY = 276,
     ARE = 277,
     AS = 278,
     AS_EMPTY = 279,
     ASIN = 280,
     ASC = 281,
     ASCII = 282,
     ASSERTION = 283,
     ATAN = 284,
     ATAN2 = 285,
     AUTHORIZATION = 286,
     AUTO_INCREMENT = 287,
     AVG = 288,
     BEFORE = 289,
     SQL_BEGIN = 290,
     BETWEEN = 291,
     BIGINT = 292,
     BINARY = 293,
     BIT = 294,
     BIT_LENGTH = 295,
     BITWISE_SHIFT_LEFT = 296,
     BITWISE_SHIFT_RIGHT = 297,
     BREAK = 298,
     BY = 299,
     CASCADE = 300,
     CASCADED = 301,
     CASE = 302,
     CAST = 303,
     CATALOG = 304,
     CEILING = 305,
     CENTER = 306,
     SQL_CHAR = 307,
     CHAR_LENGTH = 308,
     CHARACTER_STRING_LITERAL = 309,
     CHECK = 310,
     CLOSE = 311,
     COALESCE = 312,
     COBOL = 313,
     COLLATE = 314,
     COLLATION = 315,
     COLUMN = 316,
     COMMIT = 317,
     COMPUTE = 318,
     CONCAT = 319,
     CONCATENATION = 320,
     CONNECT = 321,
     CONNECTION = 322,
     CONSTRAINT = 323,
     CONSTRAINTS = 324,
     CONTINUE = 325,
     CONVERT = 326,
     CORRESPONDING = 327,
     COS = 328,
     COT = 329,
     COUNT = 330,
     CREATE = 331,
     CURDATE = 332,
     CURRENT = 333,
     CURRENT_DATE = 334,
     CURRENT_TIME = 335,
     CURRENT_TIMESTAMP = 336,
     CURTIME = 337,
     CURSOR = 338,
     DATABASE = 339,
     SQL_DATE = 340,
     DATE_FORMAT = 341,
     DATE_REMAINDER = 342,
     DATE_VALUE = 343,
     DAY = 344,
     DAYOFMONTH = 345,
     DAYOFWEEK = 346,
     DAYOFYEAR = 347,
     DAYS_BETWEEN = 348,
     DEALLOCATE = 349,
     DEC = 350,
     DECLARE = 351,
     DEFAULT = 352,
     DEFERRABLE = 353,
     DEFERRED = 354,
     SQL_DELETE = 355,
     DESC = 356,
     DESCRIBE = 357,
     DESCRIPTOR = 358,
     DIAGNOSTICS = 359,
     DICTIONARY = 360,
     DIRECTORY = 361,
     DISCONNECT = 362,
     DISPLACEMENT = 363,
     DISTINCT = 364,
     DOMAIN_TOKEN = 365,
     SQL_DOUBLE = 366,
     DOUBLE_QUOTED_STRING = 367,
     DROP = 368,
     ELSE = 369,
     END = 370,
     END_EXEC = 371,
     EQUAL = 372,
     ESCAPE = 373,
     EXCEPT = 374,
     SQL_EXCEPTION = 375,
     EXEC = 376,
     EXECUTE = 377,
     EXISTS = 378,
     EXP = 379,
     EXPONENT = 380,
     EXTERNAL = 381,
     EXTRACT = 382,
     SQL_FALSE = 383,
     FETCH = 384,
     FIRST = 385,
     SQL_FLOAT = 386,
     FLOOR = 387,
     FN = 388,
     FOR = 389,
     FOREIGN = 390,
     FORTRAN = 391,
     FOUND = 392,
     FOUR_DIGITS = 393,
     FROM = 394,
     FULL = 395,
     GET = 396,
     GLOBAL = 397,
     GO = 398,
     GOTO = 399,
     GRANT = 400,
     GREATER_OR_EQUAL = 401,
     HAVING = 402,
     HOUR = 403,
     HOURS_BETWEEN = 404,
     IDENTITY = 405,
     IFNULL = 406,
     SQL_IGNORE = 407,
     IMMEDIATE = 408,
     SQL_IN = 409,
     INCLUDE = 410,
     INDEX = 411,
     INDICATOR = 412,
     INITIALLY = 413,
     INNER = 414,
     SQL_INPUT = 415,
     INSENSITIVE = 416,
     INSERT = 417,
     INTEGER = 418,
     INTERSECT = 419,
     INTERVAL = 420,
     INTO = 421,
     IS = 422,
     ISOLATION = 423,
     JOIN = 424,
     JUSTIFY = 425,
     KEY = 426,
     LANGUAGE = 427,
     LAST = 428,
     LCASE = 429,
     LEFT = 430,
     LENGTH = 431,
     LESS_OR_EQUAL = 432,
     LEVEL = 433,
     LIKE = 434,
     LINE_WIDTH = 435,
     LOCAL = 436,
     LOCATE = 437,
     LOG = 438,
     SQL_LONG = 439,
     LOWER = 440,
     LTRIM = 441,
     LTRIP = 442,
     MATCH = 443,
     SQL_MAX = 444,
     MICROSOFT = 445,
     SQL_MIN = 446,
     MINUS = 447,
     MINUTE = 448,
     MINUTES_BETWEEN = 449,
     MOD = 450,
     MODIFY = 451,
     MODULE = 452,
     MONTH = 453,
     MONTHS_BETWEEN = 454,
     MUMPS = 455,
     NAMES = 456,
     NATIONAL = 457,
     NCHAR = 458,
     NEXT = 459,
     NODUP = 460,
     NONE = 461,
     NOT = 462,
     NOT_EQUAL = 463,
     NOT_EQUAL2 = 464,
     NOW = 465,
     SQL_NULL = 466,
     SQL_IS = 467,
     SQL_IS_NULL = 468,
     SQL_IS_NOT_NULL = 469,
     NULLIF = 470,
     NUMERIC = 471,
     OCTET_LENGTH = 472,
     ODBC = 473,
     OF = 474,
     SQL_OFF = 475,
     SQL_ON = 476,
     ONLY = 477,
     OPEN = 478,
     OPTION = 479,
     OR = 480,
     ORDER = 481,
     OUTER = 482,
     OUTPUT = 483,
     OVERLAPS = 484,
     PAGE = 485,
     PARTIAL = 486,
     SQL_PASCAL = 487,
     PERSISTENT = 488,
     CQL_PI = 489,
     PLI = 490,
     POSITION = 491,
     PRECISION = 492,
     PREPARE = 493,
     PRESERVE = 494,
     PRIMARY = 495,
     PRIOR = 496,
     PRIVILEGES = 497,
     PROCEDURE = 498,
     PRODUCT = 499,
     PUBLIC = 500,
     QUARTER = 501,
     QUIT = 502,
     RAND = 503,
     READ_ONLY = 504,
     REAL = 505,
     REFERENCES = 506,
     REPEAT = 507,
     REPLACE = 508,
     RESTRICT = 509,
     REVOKE = 510,
     RIGHT = 511,
     ROLLBACK = 512,
     ROWS = 513,
     RPAD = 514,
     RTRIM = 515,
     SCHEMA = 516,
     SCREEN_WIDTH = 517,
     SCROLL = 518,
     SECOND = 519,
     SECONDS_BETWEEN = 520,
     SELECT = 521,
     SEQUENCE = 522,
     SETOPT = 523,
     SET = 524,
     SHOWOPT = 525,
     SIGN = 526,
     SIMILAR_TO = 527,
     NOT_SIMILAR_TO = 528,
     INTEGER_CONST = 529,
     REAL_CONST = 530,
     DATE_CONST = 531,
     DATETIME_CONST = 532,
     TIME_CONST = 533,
     SIN = 534,
     SQL_SIZE = 535,
     SMALLINT = 536,
     SOME = 537,
     SPACE = 538,
     SQL = 539,
     SQL_TRUE = 540,
     SQLCA = 541,
     SQLCODE = 542,
     SQLERROR = 543,
     SQLSTATE = 544,
     SQLWARNING = 545,
     SQRT = 546,
     STDEV = 547,
     SUBSTRING = 548,
     SUM = 549,
     SYSDATE = 550,
     SYSDATE_FORMAT = 551,
     SYSTEM = 552,
     TABLE = 553,
     TAN = 554,
     TEMPORARY = 555,
     THEN = 556,
     THREE_DIGITS = 557,
     TIME = 558,
     TIMESTAMP = 559,
     TIMEZONE_HOUR = 560,
     TIMEZONE_MINUTE = 561,
     TINYINT = 562,
     TO = 563,
     TO_CHAR = 564,
     TO_DATE = 565,
     TRANSACTION = 566,
     TRANSLATE = 567,
     TRANSLATION = 568,
     TRUNCATE = 569,
     GENERAL_TITLE = 570,
     TWO_DIGITS = 571,
     UCASE = 572,
     UNION = 573,
     UNIQUE = 574,
     SQL_UNKNOWN = 575,
     UPDATE = 576,
     UPPER = 577,
     USAGE = 578,
     USER = 579,
     IDENTIFIER = 580,
     IDENTIFIER_DOT_ASTERISK = 581,
     QUERY_PARAMETER = 582,
     USING = 583,
     VALUE = 584,
     VALUES = 585,
     VARBINARY = 586,
     VARCHAR = 587,
     VARYING = 588,
     VENDOR = 589,
     VIEW = 590,
     WEEK = 591,
     WHEN = 592,
     WHENEVER = 593,
     WHERE = 594,
     WHERE_CURRENT_OF = 595,
     WITH = 596,
     WORD_WRAPPED = 597,
     WORK = 598,
     WRAPPED = 599,
     XOR = 600,
     YEAR = 601,
     YEARS_BETWEEN = 602,
     SCAN_ERROR = 603,
     __LAST_TOKEN = 604,
     ILIKE = 605
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 2068 of yacc.c  */
#line 516 "SqlParser.y"

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
#line 418 "SqlParser.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


#endif

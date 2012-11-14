/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 439 "SqlParser.y"

#ifndef YYDEBUG /* compat. */
# define YYDEBUG 0
#endif
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

#include <Predicate/Connection>
#include <Predicate/QuerySchema>
#include <Predicate/Field>
#include <Predicate/TableSchema>

#include "Parser.h"
#include "Parser_p.h"
#include "SqlTypes.h"
#ifdef Q_OS_SOLARIS
#include <alloca.h>
#endif

inline static Predicate::Expression fromPtr(Predicate::Expression* e)
{
    Predicate::Expression exp(*e);
    delete e;
    return exp;
}

QDebug operator<<(QDebug dbg, const ExpressionPtr& expr)
{
    dbg.nospace() << expr.e;
    return dbg.space();
}

int yylex();

//    using namespace std;
using namespace Predicate;

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



/* Line 268 of yacc.c  */
#line 149 "SqlParser.cpp"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


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

/* Line 293 of yacc.c  */
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



/* Line 293 of yacc.c  */
#line 553 "SqlParser.cpp"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 565 "SqlParser.cpp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  10
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   340

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  374
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  37
/* YYNRULES -- Number of rules.  */
#define YYNRULES  112
/* YYNRULES -- Number of states.  */
#define YYNSTATES  180

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   605

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,   358,   353,   371,   362,
     359,   360,   352,   351,   356,   350,   357,   363,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   355,
     365,   364,   366,   361,   354,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   369,     2,   370,   368,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,   372,     2,   373,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   367
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     9,    11,    14,    16,    18,    19,
      27,    31,    33,    36,    40,    43,    45,    48,    51,    53,
      55,    60,    65,    66,    68,    71,    75,    78,    82,    87,
      89,    91,    95,   100,   105,   108,   110,   113,   117,   122,
     124,   128,   130,   132,   134,   136,   140,   144,   148,   150,
     154,   158,   162,   166,   170,   172,   176,   180,   184,   188,
     192,   196,   198,   201,   204,   206,   210,   214,   216,   220,
     224,   228,   232,   236,   238,   242,   246,   250,   252,   255,
     258,   261,   264,   266,   268,   271,   275,   277,   279,   281,
     283,   285,   287,   289,   293,   297,   301,   305,   308,   312,
     314,   316,   319,   323,   327,   329,   331,   333,   337,   340,
     342,   347,   349
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     375,     0,    -1,   376,    -1,   377,   355,   376,    -1,   377,
      -1,   377,   355,    -1,   378,    -1,   385,    -1,    -1,    76,
     298,   325,   379,   359,   380,   360,    -1,   380,   356,   381,
      -1,   381,    -1,   325,   384,    -1,   325,   384,   382,    -1,
     382,   383,    -1,   383,    -1,   240,   171,    -1,   207,   211,
      -1,    32,    -1,     4,    -1,     4,   359,   274,   360,    -1,
     332,   359,   274,   360,    -1,    -1,   386,    -1,   386,   407,
      -1,   386,   407,   404,    -1,   386,   404,    -1,   386,   407,
     387,    -1,   386,   407,   404,   387,    -1,   266,    -1,   388,
      -1,   226,    44,   389,    -1,   388,   226,    44,   389,    -1,
     226,    44,   389,   388,    -1,   339,   392,    -1,   390,    -1,
     390,   391,    -1,   390,   356,   389,    -1,   390,   391,   356,
     389,    -1,   325,    -1,   325,   357,   325,    -1,   274,    -1,
      26,    -1,   101,    -1,   393,    -1,   394,    20,   393,    -1,
     394,   225,   393,    -1,   394,   345,   393,    -1,   394,    -1,
     395,   366,   394,    -1,   395,   146,   394,    -1,   395,   365,
     394,    -1,   395,   177,   394,    -1,   395,   364,   394,    -1,
     395,    -1,   396,   208,   395,    -1,   396,   209,   395,    -1,
     396,   179,   395,    -1,   396,   154,   395,    -1,   396,   272,
     395,    -1,   396,   273,   395,    -1,   396,    -1,   396,   213,
      -1,   396,   214,    -1,   397,    -1,   398,    41,   397,    -1,
     398,    42,   397,    -1,   398,    -1,   399,   351,   398,    -1,
     399,    65,   398,    -1,   399,   350,   398,    -1,   399,   371,
     398,    -1,   399,   372,   398,    -1,   399,    -1,   400,   363,
     399,    -1,   400,   352,   399,    -1,   400,   353,   399,    -1,
     400,    -1,   350,   400,    -1,   351,   400,    -1,   373,   400,
      -1,   207,   400,    -1,   325,    -1,   327,    -1,   325,   402,
      -1,   325,   357,   325,    -1,   211,    -1,   285,    -1,   128,
      -1,    54,    -1,   274,    -1,   275,    -1,   401,    -1,   359,
     392,   360,    -1,   359,   403,   360,    -1,   392,   356,   403,
      -1,   392,   356,   392,    -1,   139,   405,    -1,   405,   356,
     406,    -1,   406,    -1,   325,    -1,   325,   325,    -1,   325,
      23,   325,    -1,   407,   356,   408,    -1,   408,    -1,   409,
      -1,   410,    -1,   409,    23,   325,    -1,   409,   325,    -1,
     392,    -1,   109,   359,   409,   360,    -1,   352,    -1,   325,
     357,   352,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   585,   585,   595,   599,   600,   610,   614,   622,   621,
     631,   631,   637,   645,   661,   661,   667,   672,   677,   685,
     690,   697,   704,   712,   718,   725,   730,   736,   742,   751,
     761,   768,   774,   782,   793,   802,   811,   821,   829,   841,
     847,   854,   861,   865,   872,   877,   884,   890,   897,   902,
     908,   914,   920,   926,   933,   938,   945,   951,   957,   963,
     969,   976,   981,   986,   992,   997,  1003,  1010,  1015,  1021,
    1025,  1031,  1037,  1044,  1049,  1055,  1061,  1068,  1074,  1079,
    1084,  1089,  1094,  1102,  1108,  1116,  1123,  1130,  1134,  1138,
    1144,  1161,  1167,  1172,  1181,  1191,  1197,  1208,  1253,  1258,
    1267,  1295,  1305,  1320,  1327,  1337,  1346,  1351,  1361,  1374,
    1418,  1427,  1436
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UMINUS", "SQL_TYPE", "SQL_ABS", "ACOS",
  "AMPERSAND", "SQL_ABSOLUTE", "ADA", "ADD", "ADD_DAYS", "ADD_HOURS",
  "ADD_MINUTES", "ADD_MONTHS", "ADD_SECONDS", "ADD_YEARS", "ALL",
  "ALLOCATE", "ALTER", "AND", "ANY", "ARE", "AS", "AS_EMPTY", "ASIN",
  "ASC", "ASCII", "ASSERTION", "ATAN", "ATAN2", "AUTHORIZATION",
  "AUTO_INCREMENT", "AVG", "BEFORE", "SQL_BEGIN", "BETWEEN", "BIGINT",
  "BINARY", "BIT", "BIT_LENGTH", "BITWISE_SHIFT_LEFT",
  "BITWISE_SHIFT_RIGHT", "BREAK", "BY", "CASCADE", "CASCADED", "CASE",
  "CAST", "CATALOG", "CEILING", "CENTER", "SQL_CHAR", "CHAR_LENGTH",
  "CHARACTER_STRING_LITERAL", "CHECK", "CLOSE", "COALESCE", "COBOL",
  "COLLATE", "COLLATION", "COLUMN", "COMMIT", "COMPUTE", "CONCAT",
  "CONCATENATION", "CONNECT", "CONNECTION", "CONSTRAINT", "CONSTRAINTS",
  "CONTINUE", "CONVERT", "CORRESPONDING", "COS", "COT", "COUNT", "CREATE",
  "CURDATE", "CURRENT", "CURRENT_DATE", "CURRENT_TIME",
  "CURRENT_TIMESTAMP", "CURTIME", "CURSOR", "DATABASE", "SQL_DATE",
  "DATE_FORMAT", "DATE_REMAINDER", "DATE_VALUE", "DAY", "DAYOFMONTH",
  "DAYOFWEEK", "DAYOFYEAR", "DAYS_BETWEEN", "DEALLOCATE", "DEC", "DECLARE",
  "DEFAULT", "DEFERRABLE", "DEFERRED", "SQL_DELETE", "DESC", "DESCRIBE",
  "DESCRIPTOR", "DIAGNOSTICS", "DICTIONARY", "DIRECTORY", "DISCONNECT",
  "DISPLACEMENT", "DISTINCT", "DOMAIN_TOKEN", "SQL_DOUBLE",
  "DOUBLE_QUOTED_STRING", "DROP", "ELSE", "END", "END_EXEC", "EQUAL",
  "ESCAPE", "EXCEPT", "SQL_EXCEPTION", "EXEC", "EXECUTE", "EXISTS", "EXP",
  "EXPONENT", "EXTERNAL", "EXTRACT", "SQL_FALSE", "FETCH", "FIRST",
  "SQL_FLOAT", "FLOOR", "FN", "FOR", "FOREIGN", "FORTRAN", "FOUND",
  "FOUR_DIGITS", "FROM", "FULL", "GET", "GLOBAL", "GO", "GOTO", "GRANT",
  "GREATER_OR_EQUAL", "HAVING", "HOUR", "HOURS_BETWEEN", "IDENTITY",
  "IFNULL", "SQL_IGNORE", "IMMEDIATE", "SQL_IN", "INCLUDE", "INDEX",
  "INDICATOR", "INITIALLY", "INNER", "SQL_INPUT", "INSENSITIVE", "INSERT",
  "INTEGER", "INTERSECT", "INTERVAL", "INTO", "IS", "ISOLATION", "JOIN",
  "JUSTIFY", "KEY", "LANGUAGE", "LAST", "LCASE", "LEFT", "LENGTH",
  "LESS_OR_EQUAL", "LEVEL", "LIKE", "LINE_WIDTH", "LOCAL", "LOCATE", "LOG",
  "SQL_LONG", "LOWER", "LTRIM", "LTRIP", "MATCH", "SQL_MAX", "MICROSOFT",
  "SQL_MIN", "MINUS", "MINUTE", "MINUTES_BETWEEN", "MOD", "MODIFY",
  "MODULE", "MONTH", "MONTHS_BETWEEN", "MUMPS", "NAMES", "NATIONAL",
  "NCHAR", "NEXT", "NODUP", "NONE", "NOT", "NOT_EQUAL", "NOT_EQUAL2",
  "NOW", "SQL_NULL", "SQL_IS", "SQL_IS_NULL", "SQL_IS_NOT_NULL", "NULLIF",
  "NUMERIC", "OCTET_LENGTH", "ODBC", "OF", "SQL_OFF", "SQL_ON", "ONLY",
  "OPEN", "OPTION", "OR", "ORDER", "OUTER", "OUTPUT", "OVERLAPS", "PAGE",
  "PARTIAL", "SQL_PASCAL", "PERSISTENT", "CQL_PI", "PLI", "POSITION",
  "PRECISION", "PREPARE", "PRESERVE", "PRIMARY", "PRIOR", "PRIVILEGES",
  "PROCEDURE", "PRODUCT", "PUBLIC", "QUARTER", "QUIT", "RAND", "READ_ONLY",
  "REAL", "REFERENCES", "REPEAT", "REPLACE", "RESTRICT", "REVOKE", "RIGHT",
  "ROLLBACK", "ROWS", "RPAD", "RTRIM", "SCHEMA", "SCREEN_WIDTH", "SCROLL",
  "SECOND", "SECONDS_BETWEEN", "SELECT", "SEQUENCE", "SETOPT", "SET",
  "SHOWOPT", "SIGN", "SIMILAR_TO", "NOT_SIMILAR_TO", "INTEGER_CONST",
  "REAL_CONST", "DATE_CONST", "DATETIME_CONST", "TIME_CONST", "SIN",
  "SQL_SIZE", "SMALLINT", "SOME", "SPACE", "SQL", "SQL_TRUE", "SQLCA",
  "SQLCODE", "SQLERROR", "SQLSTATE", "SQLWARNING", "SQRT", "STDEV",
  "SUBSTRING", "SUM", "SYSDATE", "SYSDATE_FORMAT", "SYSTEM", "TABLE",
  "TAN", "TEMPORARY", "THEN", "THREE_DIGITS", "TIME", "TIMESTAMP",
  "TIMEZONE_HOUR", "TIMEZONE_MINUTE", "TINYINT", "TO", "TO_CHAR",
  "TO_DATE", "TRANSACTION", "TRANSLATE", "TRANSLATION", "TRUNCATE",
  "GENERAL_TITLE", "TWO_DIGITS", "UCASE", "UNION", "UNIQUE", "SQL_UNKNOWN",
  "UPDATE", "UPPER", "USAGE", "USER", "IDENTIFIER",
  "IDENTIFIER_DOT_ASTERISK", "QUERY_PARAMETER", "USING", "VALUE", "VALUES",
  "VARBINARY", "VARCHAR", "VARYING", "VENDOR", "VIEW", "WEEK", "WHEN",
  "WHENEVER", "WHERE", "WHERE_CURRENT_OF", "WITH", "WORD_WRAPPED", "WORK",
  "WRAPPED", "XOR", "YEAR", "YEARS_BETWEEN", "SCAN_ERROR", "__LAST_TOKEN",
  "'-'", "'+'", "'*'", "'%'", "'@'", "';'", "','", "'.'", "'$'", "'('",
  "')'", "'?'", "'\\''", "'/'", "'='", "'<'", "'>'", "ILIKE", "'^'", "'['",
  "']'", "'&'", "'|'", "'~'", "$accept", "TopLevelStatement",
  "StatementList", "Statement", "CreateTableStatement", "$@1", "ColDefs",
  "ColDef", "ColKeys", "ColKey", "ColType", "SelectStatement", "Select",
  "SelectOptions", "WhereClause", "OrderByClause", "OrderByColumnId",
  "OrderByOption", "aExpr", "aExpr2", "aExpr3", "aExpr4", "aExpr5",
  "aExpr6", "aExpr7", "aExpr8", "aExpr9", "aExpr10", "aExprList",
  "aExprList2", "Tables", "FlatTableList", "FlatTable", "ColViews",
  "ColItem", "ColExpression", "ColWildCard", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,   509,   510,   511,   512,   513,   514,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   530,   531,   532,   533,   534,
     535,   536,   537,   538,   539,   540,   541,   542,   543,   544,
     545,   546,   547,   548,   549,   550,   551,   552,   553,   554,
     555,   556,   557,   558,   559,   560,   561,   562,   563,   564,
     565,   566,   567,   568,   569,   570,   571,   572,   573,   574,
     575,   576,   577,   578,   579,   580,   581,   582,   583,   584,
     585,   586,   587,   588,   589,   590,   591,   592,   593,   594,
     595,   596,   597,   598,   599,   600,   601,   602,   603,   604,
      45,    43,    42,    37,    64,    59,    44,    46,    36,    40,
      41,    63,    39,    47,    61,    60,    62,   605,    94,    91,
      93,    38,   124,   126
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   374,   375,   376,   376,   376,   377,   377,   379,   378,
     380,   380,   381,   381,   382,   382,   383,   383,   383,   384,
     384,   384,   384,   385,   385,   385,   385,   385,   385,   386,
     387,   387,   387,   387,   388,   389,   389,   389,   389,   390,
     390,   390,   391,   391,   392,   393,   393,   393,   393,   394,
     394,   394,   394,   394,   394,   395,   395,   395,   395,   395,
     395,   395,   396,   396,   396,   397,   397,   397,   398,   398,
     398,   398,   398,   398,   399,   399,   399,   399,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   401,   402,   403,   403,   404,   405,   405,
     406,   406,   406,   407,   407,   408,   408,   408,   408,   409,
     409,   410,   410
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     3,     1,     2,     1,     1,     0,     7,
       3,     1,     2,     3,     2,     1,     2,     2,     1,     1,
       4,     4,     0,     1,     2,     3,     2,     3,     4,     1,
       1,     3,     4,     4,     2,     1,     2,     3,     4,     1,
       3,     1,     1,     1,     1,     3,     3,     3,     1,     3,
       3,     3,     3,     3,     1,     3,     3,     3,     3,     3,
       3,     1,     2,     2,     1,     3,     3,     1,     3,     3,
       3,     3,     3,     1,     3,     3,     3,     1,     2,     2,
       2,     2,     1,     1,     2,     3,     1,     1,     1,     1,
       1,     1,     1,     3,     3,     3,     3,     2,     3,     1,
       1,     2,     3,     3,     1,     1,     1,     3,     2,     1,
       4,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    29,     0,     2,     4,     6,     7,    23,     0,
       1,     5,    89,     0,    88,     0,     0,    86,    90,    91,
      87,    82,    83,     0,     0,   111,     0,     0,   109,    44,
      48,    54,    61,    64,    67,    73,    77,    92,    26,    24,
     104,   105,   106,     8,     3,     0,   100,    97,    99,    82,
      81,     0,     0,    84,    78,    79,     0,    80,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      62,    63,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    27,    30,    25,
       0,   108,     0,     0,     0,   101,     0,     0,    85,   112,
       0,     0,    93,    45,    46,    47,    50,    52,    53,    51,
      49,    58,    57,    55,    56,    59,    60,    65,    66,    69,
      70,    68,    71,    72,    75,    76,    74,     0,    34,   103,
       0,    28,   107,     0,   110,   102,    98,     0,    94,    41,
      39,    31,    35,     0,    22,     0,    11,    96,    95,     0,
      33,    42,    43,     0,    36,    32,    19,     0,    12,     0,
       9,    40,    37,     0,     0,     0,    18,     0,     0,    13,
      15,    10,    38,     0,     0,    17,    16,    14,    20,    21
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,     4,     5,     6,    92,   145,   146,   169,   170,
     158,     7,     8,    87,    88,   141,   142,   154,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    53,   101,
      38,    47,    48,    39,    40,    41,    42
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -338
static const yytype_int16 yypact[] =
{
     -66,  -281,  -338,    20,  -338,  -330,  -338,  -338,   -51,  -290,
    -338,   -66,  -338,  -319,  -338,  -282,   -33,  -338,  -338,  -338,
    -338,  -331,  -338,   -33,   -33,  -338,   -33,   -33,  -338,  -338,
     -18,  -133,  -143,  -338,     9,   -38,  -321,  -338,  -338,  -135,
    -338,   -16,  -338,  -338,  -338,   -45,   -15,  -301,  -338,  -303,
    -338,  -307,   -33,  -338,  -338,  -338,  -300,  -338,   -33,   -33,
     -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33,
    -338,  -338,   -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33,
     -33,   -33,   -33,   -33,    13,   -33,   -48,  -338,  -167,  -214,
    -257,  -338,  -287,  -291,  -252,  -338,  -282,  -249,  -338,  -338,
    -278,  -279,  -338,  -338,  -338,  -338,  -338,  -338,  -338,  -338,
    -338,  -338,  -338,  -338,  -338,  -338,  -338,  -338,  -338,  -338,
    -338,  -338,  -338,  -338,  -338,  -338,  -338,  -258,  -338,  -338,
      38,  -338,  -338,  -246,  -338,  -338,  -338,   -33,  -338,  -338,
    -270,  -250,   -26,  -258,    -3,  -337,  -338,  -278,  -338,  -235,
    -338,  -338,  -338,  -258,  -264,  -338,  -266,  -265,   -27,  -246,
    -338,  -338,  -338,  -258,  -168,  -166,  -338,  -104,   -62,   -27,
    -338,  -338,  -338,  -248,  -247,  -338,  -338,  -338,  -338,  -338
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -338,  -338,    99,  -338,  -338,  -338,  -338,   -44,  -338,   -58,
    -338,  -338,  -338,    27,   -24,  -129,  -338,  -338,   -11,   -21,
      35,   -20,  -338,   -12,    25,     3,     6,  -338,  -338,   -23,
      79,  -338,    23,  -338,    34,    76,  -338
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
     151,   156,    58,    12,    15,   166,    12,    90,    94,    12,
       1,    66,    84,    61,   155,    56,   139,     9,    98,   159,
      10,    12,    50,   160,   162,    11,    51,    76,    52,    54,
      55,    81,    82,    57,   172,    43,    67,   103,   104,   105,
      45,   100,    83,    46,    62,    99,   111,   112,   113,   114,
      74,    75,   115,   116,    97,    96,    52,   127,    13,   130,
     102,    13,   117,   118,    13,    68,    69,   140,   132,   134,
      70,    71,   133,   135,   128,   152,    98,    14,   137,   144,
      14,   138,   143,    14,   124,   125,   126,   149,    15,    85,
     161,    84,   163,   164,   165,    14,   106,   107,   108,   109,
     110,   119,   120,   121,   122,   123,   173,   175,   174,   176,
      44,   177,   178,   179,   148,   171,   131,   150,    89,   136,
     129,    93,     0,     0,     0,    85,   147,     0,     0,    72,
      73,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    16,     0,     0,    16,
      17,     0,    16,    17,     0,     0,    17,     0,     0,     0,
       0,     0,     0,     0,    16,     0,     0,     0,    17,     0,
     167,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       2,     0,     0,     0,    85,     0,     0,    59,     0,     0,
       0,     0,     0,   168,     0,     0,     0,     0,     0,     0,
       0,    86,     0,    18,    19,     0,    18,    19,     0,    18,
      19,    63,    64,    65,    20,     0,     0,    20,     0,     0,
      20,    18,    19,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    20,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    21,     0,    22,    21,     0,    22,
      49,     0,    22,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    49,     0,    22,     0,     0,     0,     0,    23,
      24,    25,    23,    24,    25,    23,    24,     0,    26,    91,
      95,    26,    77,    78,    26,     0,     0,    23,    24,     0,
       0,     0,    27,     0,     0,    27,    26,    60,    27,   157,
     153,     0,     0,    79,    80,     0,     0,     0,     0,     0,
      27
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-338))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
      26,     4,    20,    54,   139,    32,    54,    23,    23,    54,
      76,   154,   226,   146,   143,    26,   274,   298,   325,   356,
       0,    54,    16,   360,   153,   355,   357,    65,   359,    23,
      24,   352,   353,    27,   163,   325,   179,    58,    59,    60,
     359,    52,   363,   325,   177,   352,    66,    67,    68,    69,
      41,    42,    72,    73,   357,   356,   359,    44,   109,   226,
     360,   109,    74,    75,   109,   208,   209,   325,   325,   360,
     213,   214,   359,   325,    85,   101,   325,   128,   356,   325,
     128,   360,    44,   128,    81,    82,    83,   357,   139,   339,
     325,   226,   356,   359,   359,   128,    61,    62,    63,    64,
      65,    76,    77,    78,    79,    80,   274,   211,   274,   171,
      11,   169,   360,   360,   137,   159,    89,   141,    39,    96,
      86,    45,    -1,    -1,    -1,   339,   137,    -1,    -1,   272,
     273,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   207,    -1,    -1,   207,
     211,    -1,   207,   211,    -1,    -1,   211,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   207,    -1,    -1,    -1,   211,    -1,
     207,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     266,    -1,    -1,    -1,   339,    -1,    -1,   225,    -1,    -1,
      -1,    -1,    -1,   240,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   356,    -1,   274,   275,    -1,   274,   275,    -1,   274,
     275,   364,   365,   366,   285,    -1,    -1,   285,    -1,    -1,
     285,   274,   275,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   285,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   325,    -1,   327,   325,    -1,   327,
     325,    -1,   327,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   325,    -1,   327,    -1,    -1,    -1,    -1,   350,
     351,   352,   350,   351,   352,   350,   351,    -1,   359,   325,
     325,   359,   350,   351,   359,    -1,    -1,   350,   351,    -1,
      -1,    -1,   373,    -1,    -1,   373,   359,   345,   373,   332,
     356,    -1,    -1,   371,   372,    -1,    -1,    -1,    -1,    -1,
     373
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,    76,   266,   375,   376,   377,   378,   385,   386,   298,
       0,   355,    54,   109,   128,   139,   207,   211,   274,   275,
     285,   325,   327,   350,   351,   352,   359,   373,   392,   393,
     394,   395,   396,   397,   398,   399,   400,   401,   404,   407,
     408,   409,   410,   325,   376,   359,   325,   405,   406,   325,
     400,   357,   359,   402,   400,   400,   392,   400,    20,   225,
     345,   146,   177,   364,   365,   366,   154,   179,   208,   209,
     213,   214,   272,   273,    41,    42,    65,   350,   351,   371,
     372,   352,   353,   363,   226,   339,   356,   387,   388,   404,
      23,   325,   379,   409,    23,   325,   356,   357,   325,   352,
     392,   403,   360,   393,   393,   393,   394,   394,   394,   394,
     394,   395,   395,   395,   395,   395,   395,   397,   397,   398,
     398,   398,   398,   398,   399,   399,   399,    44,   392,   408,
     226,   387,   325,   359,   360,   325,   406,   356,   360,   274,
     325,   389,   390,    44,   325,   380,   381,   392,   403,   357,
     388,    26,   101,   356,   391,   389,     4,   332,   384,   356,
     360,   325,   389,   356,   359,   359,    32,   207,   240,   382,
     383,   381,   389,   274,   274,   211,   171,   383,   360,   360
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1806 of yacc.c  */
#line 586 "SqlParser.y"
    {
//todo: multiple statements
//todo: not only "select" statements
    parser->setOperation(Parser::OP_Select);
    parser->setQuerySchema((yyvsp[(1) - (1)].querySchema));
}
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 596 "SqlParser.y"
    {
//todo: multiple statements
}
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 601 "SqlParser.y"
    {
    (yyval.querySchema) = (yyvsp[(1) - (2)].querySchema);
}
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 611 "SqlParser.y"
    {
YYACCEPT;
}
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 615 "SqlParser.y"
    {
    (yyval.querySchema) = (yyvsp[(1) - (1)].querySchema);
}
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 622 "SqlParser.y"
    {
    parser->setOperation(Parser::OP_CreateTable);
    parser->createTable((yyvsp[(3) - (3)].stringValue)->toLatin1());
    delete (yyvsp[(3) - (3)].stringValue);
}
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 632 "SqlParser.y"
    {
}
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 638 "SqlParser.y"
    {
    PreDbg << "adding field " << *(yyvsp[(1) - (2)].stringValue);
    field->setName(*(yyvsp[(1) - (2)].stringValue));
    parser->table()->addField(field);
    field = 0;
    delete (yyvsp[(1) - (2)].stringValue);
}
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 646 "SqlParser.y"
    {
    PreDbg << "adding field " << *(yyvsp[(1) - (3)].stringValue);
    field->setName(*(yyvsp[(1) - (3)].stringValue));
    delete (yyvsp[(1) - (3)].stringValue);
    parser->table()->addField(field);

//    if(field->isPrimaryKey())
//        parser->table()->addPrimaryKey(field->name());

//    delete field;
//    field = 0;
}
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 662 "SqlParser.y"
    {
}
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 668 "SqlParser.y"
    {
    field->setPrimaryKey(true);
    PreDbg << "primary";
}
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 673 "SqlParser.y"
    {
    field->setNotNull(true);
    PreDbg << "not_null";
}
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 678 "SqlParser.y"
    {
    field->setAutoIncrement(true);
    PreDbg << "ainc";
}
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 686 "SqlParser.y"
    {
    field = new Field();
    field->setType((yyvsp[(1) - (1)].colType));
}
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 691 "SqlParser.y"
    {
    PreDbg << "sql + length";
    field = new Field();
    field->setPrecision((yyvsp[(3) - (4)].integerValue));
    field->setType((yyvsp[(1) - (4)].colType));
}
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 698 "SqlParser.y"
    {
    field = new Field();
    field->setPrecision((yyvsp[(3) - (4)].integerValue));
    field->setType(Field::Text);
}
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 704 "SqlParser.y"
    {
    // SQLITE compatibillity
    field = new Field();
    field->setType(Field::InvalidType);
}
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 713 "SqlParser.y"
    {
    PreDbg << "Select";
    if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[(1) - (1)].querySchema), 0 )))
        return 0;
}
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 719 "SqlParser.y"
    {
    PreDbg << "Select ColViews=" << *(yyvsp[(2) - (2)].exprList);

    if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[(1) - (2)].querySchema), (yyvsp[(2) - (2)].exprList) )))
        return 0;
}
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 726 "SqlParser.y"
    {
    if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[(1) - (3)].querySchema), (yyvsp[(2) - (3)].exprList), (yyvsp[(3) - (3)].exprList) )))
        return 0;
}
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 731 "SqlParser.y"
    {
    PreDbg << "Select ColViews Tables";
    if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[(1) - (2)].querySchema), 0, (yyvsp[(2) - (2)].exprList) )))
        return 0;
}
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 737 "SqlParser.y"
    {
    PreDbg << "Select ColViews Conditions";
    if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[(1) - (3)].querySchema), (yyvsp[(2) - (3)].exprList), 0, (yyvsp[(3) - (3)].selectOptions) )))
        return 0;
}
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 743 "SqlParser.y"
    {
    PreDbg << "Select ColViews Tables SelectOptions";
    if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[(1) - (4)].querySchema), (yyvsp[(2) - (4)].exprList), (yyvsp[(3) - (4)].exprList), (yyvsp[(4) - (4)].selectOptions) )))
        return 0;
}
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 752 "SqlParser.y"
    {
    PreDbg << "SELECT";
//    parser->createSelect();
//    parser->setOperation(Parser::OP_Select);
    (yyval.querySchema) = new QuerySchema();
}
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 762 "SqlParser.y"
    {
    PreDbg << "WhereClause";
    (yyval.selectOptions) = new SelectOptionsInternal;
    (yyval.selectOptions)->whereExpr = *(yyvsp[(1) - (1)].expr);
    delete (yyvsp[(1) - (1)].expr);
}
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 769 "SqlParser.y"
    {
    PreDbg << "OrderByClause";
    (yyval.selectOptions) = new SelectOptionsInternal;
    (yyval.selectOptions)->orderByColumns = (yyvsp[(3) - (3)].orderByColumns);
}
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 775 "SqlParser.y"
    {
    PreDbg << "WhereClause ORDER BY OrderByClause";
    (yyval.selectOptions) = new SelectOptionsInternal;
    (yyval.selectOptions)->whereExpr = *(yyvsp[(1) - (4)].expr);
    delete (yyvsp[(1) - (4)].expr);
    (yyval.selectOptions)->orderByColumns = (yyvsp[(4) - (4)].orderByColumns);
}
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 783 "SqlParser.y"
    {
    PreDbg << "OrderByClause WhereClause";
    (yyval.selectOptions) = new SelectOptionsInternal;
    (yyval.selectOptions)->whereExpr = *(yyvsp[(4) - (4)].expr);
    delete (yyvsp[(4) - (4)].expr);
    (yyval.selectOptions)->orderByColumns = (yyvsp[(3) - (4)].orderByColumns);
}
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 794 "SqlParser.y"
    {
    (yyval.expr) = (yyvsp[(2) - (2)].expr);
}
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 803 "SqlParser.y"
    {
    PreDbg << "ORDER BY IDENTIFIER";
    (yyval.orderByColumns) = new OrderByColumnInternal::List;
    OrderByColumnInternal orderByColumn;
    orderByColumn.setColumnByNameOrNumber( *(yyvsp[(1) - (1)].variantValue) );
    (yyval.orderByColumns)->append( orderByColumn );
    delete (yyvsp[(1) - (1)].variantValue);
}
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 812 "SqlParser.y"
    {
    PreDbg << "ORDER BY IDENTIFIER OrderByOption";
    (yyval.orderByColumns) = new OrderByColumnInternal::List;
    OrderByColumnInternal orderByColumn;
    orderByColumn.setColumnByNameOrNumber( *(yyvsp[(1) - (2)].variantValue) );
    orderByColumn.ascending = (yyvsp[(2) - (2)].booleanValue);
    (yyval.orderByColumns)->append( orderByColumn );
    delete (yyvsp[(1) - (2)].variantValue);
}
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 822 "SqlParser.y"
    {
    (yyval.orderByColumns) = (yyvsp[(3) - (3)].orderByColumns);
    OrderByColumnInternal orderByColumn;
    orderByColumn.setColumnByNameOrNumber( *(yyvsp[(1) - (3)].variantValue) );
    (yyval.orderByColumns)->append( orderByColumn );
    delete (yyvsp[(1) - (3)].variantValue);
}
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 830 "SqlParser.y"
    {
    (yyval.orderByColumns) = (yyvsp[(4) - (4)].orderByColumns);
    OrderByColumnInternal orderByColumn;
    orderByColumn.setColumnByNameOrNumber( *(yyvsp[(1) - (4)].variantValue) );
    orderByColumn.ascending = (yyvsp[(2) - (4)].booleanValue);
    (yyval.orderByColumns)->append( orderByColumn );
    delete (yyvsp[(1) - (4)].variantValue);
}
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 842 "SqlParser.y"
    {
    (yyval.variantValue) = new QVariant( *(yyvsp[(1) - (1)].stringValue) );
    PreDbg << "OrderByColumnId: " << *(yyval.variantValue);
    delete (yyvsp[(1) - (1)].stringValue);
}
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 848 "SqlParser.y"
    {
    (yyval.variantValue) = new QVariant( *(yyvsp[(1) - (3)].stringValue) + QLatin1Char('.') + *(yyvsp[(3) - (3)].stringValue) );
    PreDbg << "OrderByColumnId: " << *(yyval.variantValue);
    delete (yyvsp[(1) - (3)].stringValue);
    delete (yyvsp[(3) - (3)].stringValue);
}
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 855 "SqlParser.y"
    {
    (yyval.variantValue) = new QVariant((yyvsp[(1) - (1)].integerValue));
    PreDbg << "OrderByColumnId: " << *(yyval.variantValue);
}
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 862 "SqlParser.y"
    {
    (yyval.booleanValue) = true;
}
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 866 "SqlParser.y"
    {
    (yyval.booleanValue) = false;
}
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 878 "SqlParser.y"
    {
//    PreDbg << "AND " << $3.debugString();
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), AND, *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 885 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), OR, *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 891 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), XOR, *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 903 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), '>', *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 909 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), GREATER_OR_EQUAL, *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 915 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), '<', *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 921 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), LESS_OR_EQUAL, *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 927 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), '=', *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 939 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), NOT_EQUAL, *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 946 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), NOT_EQUAL2, *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 952 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), LIKE, *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 958 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), SQL_IN, *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 964 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), SIMILAR_TO, *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 970 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), NOT_SIMILAR_TO, *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 982 "SqlParser.y"
    {
    (yyval.expr) = new UnaryExpression( SQL_IS_NULL, *(yyvsp[(1) - (2)].expr) );
    delete (yyvsp[(1) - (2)].expr);
}
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 987 "SqlParser.y"
    {
    (yyval.expr) = new UnaryExpression( SQL_IS_NOT_NULL, *(yyvsp[(1) - (2)].expr) );
    delete (yyvsp[(1) - (2)].expr);
}
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 998 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), BITWISE_SHIFT_LEFT, *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 1004 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), BITWISE_SHIFT_RIGHT, *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 1016 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), '+', *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 1022 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), CONCATENATION, *(yyvsp[(3) - (3)].expr));
}
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 1026 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), '-', *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 1032 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), '&', *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 1038 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), '|', *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 1050 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), '/', *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 1056 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), '*', *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 1062 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[(1) - (3)].expr), '%', *(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 1075 "SqlParser.y"
    {
    (yyval.expr) = new UnaryExpression( '-', *(yyvsp[(2) - (2)].expr) );
    delete (yyvsp[(2) - (2)].expr);
}
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 1080 "SqlParser.y"
    {
    (yyval.expr) = new UnaryExpression( '+', *(yyvsp[(2) - (2)].expr) );
    delete (yyvsp[(2) - (2)].expr);
}
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 1085 "SqlParser.y"
    {
    (yyval.expr) = new UnaryExpression( '~', *(yyvsp[(2) - (2)].expr) );
    delete (yyvsp[(2) - (2)].expr);
}
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 1090 "SqlParser.y"
    {
    (yyval.expr) = new UnaryExpression( NOT, *(yyvsp[(2) - (2)].expr) );
    delete (yyvsp[(2) - (2)].expr);
}
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 1095 "SqlParser.y"
    {
    (yyval.expr) = new VariableExpression( *(yyvsp[(1) - (1)].stringValue) );

//TODO: simplify this later if that's 'only one field name' expression
    PreDbg << "  + identifier: " << *(yyvsp[(1) - (1)].stringValue);
    delete (yyvsp[(1) - (1)].stringValue);
}
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 1103 "SqlParser.y"
    {
    (yyval.expr) = new QueryParameterExpression( *(yyvsp[(1) - (1)].stringValue) );
    PreDbg << "  + query parameter:" << *(yyval.expr);
    delete (yyvsp[(1) - (1)].stringValue);
}
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 1109 "SqlParser.y"
    {
    PreDbg << "  + function:" << *(yyvsp[(1) - (2)].stringValue) << "(" << *(yyvsp[(2) - (2)].exprList) << ")";
    (yyval.expr) = new FunctionExpression(*(yyvsp[(1) - (2)].stringValue), *(yyvsp[(2) - (2)].exprList));
    delete (yyvsp[(1) - (2)].stringValue);
    delete (yyvsp[(2) - (2)].exprList);
}
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 1117 "SqlParser.y"
    {
    (yyval.expr) = new VariableExpression( *(yyvsp[(1) - (3)].stringValue) + QLatin1Char('.') + *(yyvsp[(3) - (3)].stringValue) );
    PreDbg << "  + identifier.identifier:" << *(yyvsp[(1) - (3)].stringValue) << "." << *(yyvsp[(3) - (3)].stringValue);
    delete (yyvsp[(1) - (3)].stringValue);
    delete (yyvsp[(3) - (3)].stringValue);
}
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 1124 "SqlParser.y"
    {
    (yyval.expr) = new ConstExpression( SQL_NULL, QVariant() );
    PreDbg << "  + NULL";
//    $$ = new Field();
    //$$->setName(QString::null);
}
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 1131 "SqlParser.y"
    {
    (yyval.expr) = new ConstExpression( SQL_TRUE, true );
}
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 1135 "SqlParser.y"
    {
    (yyval.expr) = new ConstExpression( SQL_FALSE, false );
}
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 1139 "SqlParser.y"
    {
    (yyval.expr) = new ConstExpression( CHARACTER_STRING_LITERAL, *(yyvsp[(1) - (1)].stringValue) );
    PreDbg << "  + constant " << (yyvsp[(1) - (1)].stringValue);
    delete (yyvsp[(1) - (1)].stringValue);
}
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 1145 "SqlParser.y"
    {
    QVariant val;
    if ((yyvsp[(1) - (1)].integerValue) <= INT_MAX && (yyvsp[(1) - (1)].integerValue) >= INT_MIN)
        val = (int)(yyvsp[(1) - (1)].integerValue);
    else if ((yyvsp[(1) - (1)].integerValue) <= UINT_MAX && (yyvsp[(1) - (1)].integerValue) >= 0)
        val = (uint)(yyvsp[(1) - (1)].integerValue);
    else if ((yyvsp[(1) - (1)].integerValue) <= LLONG_MAX && (yyvsp[(1) - (1)].integerValue) >= LLONG_MIN)
        val = (qint64)(yyvsp[(1) - (1)].integerValue);

//    if ($1 < ULLONG_MAX)
//        val = (quint64)$1;
//TODO ok?

    (yyval.expr) = new ConstExpression( INTEGER_CONST, val );
    PreDbg << "  + int constant: " << val.toString();
}
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 1162 "SqlParser.y"
    {
    (yyval.expr) = new ConstExpression( REAL_CONST, QPoint( (yyvsp[(1) - (1)].realValue).integer, (yyvsp[(1) - (1)].realValue).fractional ) );
    PreDbg << "  + real constant: " << (yyvsp[(1) - (1)].realValue).integer << "." << (yyvsp[(1) - (1)].realValue).fractional;
}
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 1173 "SqlParser.y"
    {
    PreDbg << "(expr)";
    (yyval.expr) = new UnaryExpression('(', *(yyvsp[(2) - (3)].expr));
    delete (yyvsp[(2) - (3)].expr);
}
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 1182 "SqlParser.y"
    {
//    $$ = new NArgExpression(UnknownExpressionClass, 0);
//    $$->add( $1 );
//    $$->add( $3 );
    (yyval.exprList) = (yyvsp[(2) - (3)].exprList);
}
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 1192 "SqlParser.y"
    {
    (yyval.exprList) = (yyvsp[(3) - (3)].exprList);
    (yyval.exprList)->prepend( *(yyvsp[(1) - (3)].expr) );
    delete (yyvsp[(1) - (3)].expr);
}
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 1198 "SqlParser.y"
    {
    (yyval.exprList) = new NArgExpression(UnknownExpressionClass, 0);
    (yyval.exprList)->append( *(yyvsp[(1) - (3)].expr) );
    (yyval.exprList)->append( *(yyvsp[(3) - (3)].expr) );
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].expr);
}
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 1209 "SqlParser.y"
    {
    (yyval.exprList) = (yyvsp[(2) - (2)].exprList);
}
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 1254 "SqlParser.y"
    {
    (yyval.exprList) = (yyvsp[(1) - (3)].exprList);
    (yyval.exprList)->append(*(yyvsp[(3) - (3)].expr));
}
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 1259 "SqlParser.y"
    {
    (yyval.exprList) = new NArgExpression(TableListExpressionClass, IDENTIFIER); //ok?
    (yyval.exprList)->append(*(yyvsp[(1) - (1)].expr));
    delete (yyvsp[(1) - (1)].expr);
}
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 1268 "SqlParser.y"
    {
    PreDbg << "FROM: '" << *(yyvsp[(1) - (1)].stringValue) << "'";
    (yyval.expr) = new VariableExpression(*(yyvsp[(1) - (1)].stringValue));

    /*
//TODO: this isn't ok for more tables:
    Field::ListIterator it = parser->select()->fieldsIterator();
    for(Field *item; (item = it.current()); ++it)
    {
        if(item->table() == dummy)
        {
            item->setTable(schema);
        }

        if(item->table() && !item->isQueryAsterisk())
        {
            Field *f = item->table()->field(item->name());
            if(!f)
            {
                ParserError err(QObject::tr("Field List Error"), QObject::tr("Unknown column '%1' in table '%2'",item->name(),schema->name()), ctoken, current);
                parser->setError(err);
                yyerror("fieldlisterror");
            }
        }
    }*/
    delete (yyvsp[(1) - (1)].stringValue);
}
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 1296 "SqlParser.y"
    {
    //table + alias
    (yyval.expr) = new BinaryExpression(
        VariableExpression(*(yyvsp[(1) - (2)].stringValue)), AS_EMPTY,
        VariableExpression(*(yyvsp[(2) - (2)].stringValue))
    );
    delete (yyvsp[(1) - (2)].stringValue);
    delete (yyvsp[(2) - (2)].stringValue);
}
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 1306 "SqlParser.y"
    {
    //table + alias
    (yyval.expr) = new BinaryExpression(
        VariableExpression(*(yyvsp[(1) - (3)].stringValue)), AS,
        VariableExpression(*(yyvsp[(3) - (3)].stringValue))
    );
    delete (yyvsp[(1) - (3)].stringValue);
    delete (yyvsp[(3) - (3)].stringValue);
}
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 1321 "SqlParser.y"
    {
    (yyval.exprList) = (yyvsp[(1) - (3)].exprList);
    (yyval.exprList)->append(*(yyvsp[(3) - (3)].expr));
    delete (yyvsp[(3) - (3)].expr);
    PreDbg << "ColViews: ColViews , ColItem";
}
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 1328 "SqlParser.y"
    {
    (yyval.exprList) = new NArgExpression(FieldListExpressionClass, 0);
    (yyval.exprList)->append(*(yyvsp[(1) - (1)].expr));
    delete (yyvsp[(1) - (1)].expr);
    PreDbg << "ColViews: ColItem";
}
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 1338 "SqlParser.y"
    {
//    $$ = new Field();
//    dummy->addField($$);
//    $$->setExpression( $1 );
//    parser->select()->addField($$);
    (yyval.expr) = (yyvsp[(1) - (1)].expr);
    PreDbg << " added column expr:" << *(yyvsp[(1) - (1)].expr);
}
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 1347 "SqlParser.y"
    {
    (yyval.expr) = (yyvsp[(1) - (1)].expr);
    PreDbg << " added column wildcard:" << *(yyvsp[(1) - (1)].expr);
}
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 1352 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(
        *(yyvsp[(1) - (3)].expr), AS,
        VariableExpression(*(yyvsp[(3) - (3)].stringValue))
    );
    PreDbg << " added column expr:" << *(yyval.expr);
    delete (yyvsp[(1) - (3)].expr);
    delete (yyvsp[(3) - (3)].stringValue);
}
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 1362 "SqlParser.y"
    {
    (yyval.expr) = new BinaryExpression(
        *(yyvsp[(1) - (2)].expr), AS_EMPTY,
        VariableExpression(*(yyvsp[(2) - (2)].stringValue))
    );
    PreDbg << " added column expr:" << *(yyval.expr);
    delete (yyvsp[(1) - (2)].expr);
    delete (yyvsp[(2) - (2)].stringValue);
}
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 1375 "SqlParser.y"
    {
    (yyval.expr) = (yyvsp[(1) - (1)].expr);
}
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 1419 "SqlParser.y"
    {
    (yyval.expr) = (yyvsp[(3) - (4)].expr);
//! @todo DISTINCT '(' ColExpression ')'
//    $$->setName("DISTINCT(" + $3->name() + ")");
}
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 1428 "SqlParser.y"
    {
    (yyval.expr) = new VariableExpression(QLatin1String("*"));
    PreDbg << "all columns";

//    QueryAsterisk *ast = new QueryAsterisk(parser->select(), dummy);
//    parser->select()->addAsterisk(ast);
//    requiresTable = true;
}
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 1437 "SqlParser.y"
    {
    QString s( *(yyvsp[(1) - (3)].stringValue) );
    s += QLatin1String(".*");
    (yyval.expr) = new VariableExpression(s);
    PreDbg << "  + all columns from " << s;
    delete (yyvsp[(1) - (3)].stringValue);
}
    break;



/* Line 1806 of yacc.c  */
#line 3248 "SqlParser.cpp"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 1452 "SqlParser.y"



const char* tname(int offset) { return yytname[offset]; }

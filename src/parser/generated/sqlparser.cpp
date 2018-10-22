/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

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
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 435 "KDbSqlParser.y" /* yacc.c:339  */

#include <stdio.h>
#include <string.h>
#include <string>
#include <iostream>
#include <limits.h>
//! @todo OK?
#ifdef Q_OS_WIN
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
#include <QVariant>
#include <QPoint>

#include "KDbConnection.h"
#include "KDbDateTime.h"
#include "KDbExpression.h"
#include "KDbField.h"
#include "KDbOrderByColumn.h"
#include "KDbParser.h"
#include "KDbParser_p.h"
#include "KDbQuerySchema.h"
#include "KDbQuerySchema_p.h"
#include "KDbSqlTypes.h"
#include "KDbTableSchema.h"
#include "kdb_debug.h"

struct OrderByColumnInternal;

#ifdef Q_OS_SOLARIS
#include <alloca.h>
#endif

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


#line 136 "sqlparser.cpp" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "KDbSqlParser.tab.h".  */
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
#line 505 "KDbSqlParser.y" /* yacc.c:355  */

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

#line 268 "sqlparser.cpp" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_KDBSQLPARSER_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 285 "sqlparser.cpp" /* yacc.c:358  */

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
#else
typedef signed char yytype_int8;
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
# elif ! defined YYSIZE_T
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
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  7
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   231

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  88
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  38
/* YYNRULES -- Number of rules.  */
#define YYNRULES  119
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  201

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   324

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,    84,     2,    82,    78,     2,
      86,    87,    81,    76,    71,    77,    72,    80,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    85,    70,
      74,    75,    73,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    79,     2,    83,     2,     2,     2,
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
      65,    66,    67,    68,    69
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   564,   564,   574,   578,   579,   594,   693,   699,   706,
     711,   717,   723,   729,   738,   746,   753,   759,   767,   778,
     787,   796,   806,   814,   826,   832,   839,   846,   850,   857,
     862,   869,   875,   882,   887,   893,   899,   905,   911,   918,
     923,   929,   935,   941,   947,   953,   959,   965,   975,   986,
     991,   996,  1002,  1007,  1013,  1020,  1025,  1031,  1037,  1043,
    1049,  1056,  1061,  1067,  1073,  1080,  1086,  1091,  1096,  1101,
    1106,  1114,  1120,  1128,  1135,  1142,  1146,  1150,  1156,  1173,
    1179,  1185,  1191,  1198,  1202,  1210,  1218,  1229,  1235,  1241,
    1250,  1258,  1266,  1278,  1282,  1289,  1293,  1297,  1304,  1314,
    1323,  1327,  1334,  1340,  1349,  1394,  1400,  1409,  1437,  1447,
    1462,  1469,  1479,  1488,  1493,  1503,  1516,  1562,  1571,  1580
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SQL_TYPE", "AS", "AS_EMPTY", "ASC",
  "AUTO_INCREMENT", "BIT", "BITWISE_SHIFT_LEFT", "BITWISE_SHIFT_RIGHT",
  "BY", "CHARACTER_STRING_LITERAL", "CONCATENATION", "CREATE", "DESC",
  "DISTINCT", "DOUBLE_QUOTED_STRING", "FROM", "JOIN", "KEY", "LEFT",
  "LESS_OR_EQUAL", "GREATER_OR_EQUAL", "SQL_NULL", "SQL_IS", "SQL_IS_NULL",
  "SQL_IS_NOT_NULL", "ORDER", "PRIMARY", "SELECT", "INTEGER_CONST",
  "REAL_CONST", "RIGHT", "SQL_ON", "DATE_CONST", "DATETIME_CONST",
  "TIME_CONST", "TABLE", "IDENTIFIER", "IDENTIFIER_DOT_ASTERISK",
  "QUERY_PARAMETER", "VARCHAR", "WHERE", "SQL", "SQL_TRUE", "SQL_FALSE",
  "UNION", "SCAN_ERROR", "AND", "BETWEEN", "NOT_BETWEEN", "EXCEPT",
  "SQL_IN", "INTERSECT", "LIKE", "ILIKE", "NOT_LIKE", "NOT", "NOT_EQUAL",
  "NOT_EQUAL2", "OR", "SIMILAR_TO", "NOT_SIMILAR_TO", "XOR", "UMINUS",
  "TABS_OR_SPACES", "DATE_TIME_INTEGER", "TIME_AM", "TIME_PM", "';'",
  "','", "'.'", "'>'", "'<'", "'='", "'+'", "'-'", "'&'", "'|'", "'/'",
  "'*'", "'%'", "'~'", "'#'", "':'", "'('", "')'", "$accept",
  "TopLevelStatement", "StatementList", "Statement", "SelectStatement",
  "Select", "SelectOptions", "WhereClause", "OrderByClause",
  "OrderByColumnId", "OrderByOption", "aExpr", "aExpr2", "aExpr3",
  "aExpr4", "aExpr5", "aExpr6", "aExpr7", "aExpr8", "aExpr9", "DateConst",
  "DateValue", "YearConst", "TimeConst", "TimeValue", "TimeMs",
  "TimePeriod", "DateTimeConst", "aExpr10", "aExprList", "aExprList2",
  "Tables", "FlatTableList", "FlatTable", "ColViews", "ColItem",
  "ColExpression", "ColWildCard", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
      59,    44,    46,    62,    60,    61,    43,    45,    38,   124,
      47,    42,    37,   126,    35,    58,    40,    41
};
# endif

#define YYPACT_NINF -143

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-143)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
       1,  -143,    52,  -143,    -8,  -143,    22,  -143,     1,  -143,
     -27,    25,  -143,  -143,  -143,   -60,  -143,  -143,  -143,   126,
     126,   126,  -143,   126,    44,   126,  -143,  -143,   -16,    13,
     163,  -143,    86,     0,    64,  -143,  -143,  -143,  -143,   -11,
      29,  -143,    10,  -143,  -143,   102,    11,    -5,  -143,   -23,
      -2,  -143,   -21,  -143,  -143,  -143,  -143,   -61,     4,    26,
     -43,    24,    20,    35,   126,   126,   126,   126,   126,   126,
     126,   126,  -143,  -143,   126,   126,   126,   126,   126,   126,
     126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
     126,   126,   126,    96,   126,  -143,    84,    78,  -143,   -11,
      74,  -143,    38,    98,  -143,    25,  -143,  -143,  -143,    71,
      48,   110,    89,    99,  -143,  -143,   101,  -143,   103,  -143,
    -143,  -143,  -143,  -143,  -143,  -143,  -143,  -143,  -143,   114,
     120,  -143,  -143,  -143,  -143,  -143,  -143,  -143,  -143,  -143,
    -143,  -143,  -143,  -143,  -143,  -143,  -143,  -143,    58,  -143,
     172,  -143,  -143,  -143,  -143,  -143,  -143,   126,  -143,   111,
     -12,   112,   115,   121,   126,   126,  -143,   128,   144,    12,
      58,  -143,    63,   134,   137,   -48,  -143,   138,  -143,  -143,
     167,  -143,  -143,  -143,    58,   136,  -143,  -143,  -143,  -143,
     139,  -143,  -143,  -143,  -143,  -143,  -143,    58,   -48,  -143,
    -143
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    14,     0,     2,     4,     6,     7,     1,     5,    77,
       0,     0,    74,    78,    79,    70,    71,    75,    76,     0,
       0,     0,   118,     0,     0,     0,   116,    29,    33,    39,
      49,    52,    55,    61,    65,    80,    81,    82,    83,    10,
       8,   111,   112,   113,     3,     0,   107,   104,   106,     0,
       0,    72,    70,    69,    67,    66,    68,    87,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,    51,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    15,     0,    11,     9,
       0,   115,     0,     0,   108,     0,    73,   119,   101,   103,
       0,     0,     0,     0,    88,    89,     0,    84,     0,    90,
      99,    30,    31,    32,    37,    35,    34,    36,    38,     0,
       0,    44,    42,    43,    40,    41,    45,    46,    53,    54,
      57,    56,    58,    59,    60,    62,    63,    64,     0,    19,
       0,   110,    13,   114,   117,   109,   105,     0,   100,     0,
      94,     0,     0,     0,     0,     0,    26,    24,    16,    20,
       0,   102,     0,     0,     0,    97,    98,     0,    47,    48,
       0,    18,    27,    28,     0,    21,    17,    87,    86,    93,
      94,    95,    96,    91,    85,    25,    22,     0,    97,    23,
      92
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -143,  -143,   200,  -143,  -143,  -143,   -29,    47,  -142,  -143,
    -143,   -25,    87,   106,   -73,  -143,    32,   107,    90,   108,
    -143,  -143,    45,  -143,   105,    34,    21,  -143,  -143,  -143,
      70,   188,  -143,   124,  -143,   133,   186,  -143
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     4,     5,     6,    95,    96,   168,   169,
     185,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    60,    61,    36,    62,   175,   193,    37,    38,    51,
     110,    39,    47,    48,    40,    41,    42,    43
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      63,   129,   130,   131,   132,   133,   134,   135,   136,   137,
       9,    98,    49,    85,   100,   103,   106,    93,   182,   112,
     191,   192,    12,   116,   113,   109,    50,   183,   186,    13,
      14,     1,    94,    64,     9,    67,    68,    52,    10,    16,
      11,   117,   196,    17,    18,    65,    12,    11,    66,   101,
     104,   111,     7,    13,    14,   199,    19,    93,   107,    45,
     173,    15,     8,    16,    46,    50,   105,    17,    18,   149,
     152,   114,    94,   174,    20,    21,    86,    87,    88,    89,
      19,    23,    24,   184,    25,   108,    69,    70,    71,   166,
       9,   178,   179,   115,    10,    83,    84,   167,    20,    21,
      97,   118,    12,    22,   119,    23,    24,   148,    25,    13,
      14,    57,   150,   153,     9,   138,   139,    15,    10,    16,
      58,    59,   120,    17,    18,   154,    12,    53,    54,    55,
     187,    56,   109,    13,    14,   158,    19,   155,     9,    58,
      59,    52,   157,    16,    90,    91,    92,    17,    18,   106,
      12,   121,   122,   123,    20,    21,   159,    13,    14,    22,
      19,    23,    24,   164,    25,    52,   160,    16,   161,   165,
     163,    17,    18,   124,   125,   126,   127,   128,    20,    21,
     145,   146,   147,   170,    19,    23,    24,    94,    25,    72,
      73,   172,   140,   141,   142,   143,   144,   113,   177,   176,
     180,   189,    20,    21,   190,   194,   195,   197,    44,    23,
      24,   173,    25,    74,    75,   181,    76,   188,    77,   200,
      78,   162,    79,    80,   198,    81,    82,   171,    99,   156,
     151,   102
};

static const yytype_uint8 yycheck[] =
{
      25,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      12,    40,    72,    13,     4,     4,    39,    28,     6,    80,
      68,    69,    24,    66,    85,    50,    86,    15,   170,    31,
      32,    30,    43,    49,    12,    22,    23,    39,    16,    41,
      18,    84,   184,    45,    46,    61,    24,    18,    64,    39,
      39,    72,     0,    31,    32,   197,    58,    28,    81,    86,
      72,    39,    70,    41,    39,    86,    71,    45,    46,    94,
      99,    67,    43,    85,    76,    77,    76,    77,    78,    79,
      58,    83,    84,    71,    86,    87,    73,    74,    75,    31,
      12,   164,   165,    67,    16,     9,    10,    39,    76,    77,
      71,    77,    24,    81,    84,    83,    84,    11,    86,    31,
      32,    67,    28,    39,    12,    83,    84,    39,    16,    41,
      76,    77,    87,    45,    46,    87,    24,    19,    20,    21,
      67,    23,   157,    31,    32,    87,    58,    39,    12,    76,
      77,    39,    71,    41,    80,    81,    82,    45,    46,    39,
      24,    64,    65,    66,    76,    77,    67,    31,    32,    81,
      58,    83,    84,    49,    86,    39,    67,    41,    67,    49,
      67,    45,    46,    67,    68,    69,    70,    71,    76,    77,
      90,    91,    92,    11,    58,    83,    84,    43,    86,    26,
      27,    80,    85,    86,    87,    88,    89,    85,    77,    84,
      72,    67,    76,    77,    67,    67,    39,    71,     8,    83,
      84,    72,    86,    50,    51,   168,    53,   172,    55,   198,
      57,   116,    59,    60,   190,    62,    63,   157,    40,   105,
      97,    45
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    30,    89,    90,    91,    92,    93,     0,    70,    12,
      16,    18,    24,    31,    32,    39,    41,    45,    46,    58,
      76,    77,    81,    83,    84,    86,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   111,   115,   116,   119,
     122,   123,   124,   125,    90,    86,    39,   120,   121,    72,
      86,   117,    39,   107,   107,   107,   107,    67,    76,    77,
     109,   110,   112,    99,    49,    61,    64,    22,    23,    73,
      74,    75,    26,    27,    50,    51,    53,    55,    57,    59,
      60,    62,    63,     9,    10,    13,    76,    77,    78,    79,
      80,    81,    82,    28,    43,    94,    95,    71,    94,   119,
       4,    39,   124,     4,    39,    71,    39,    81,    87,    99,
     118,    72,    80,    85,    67,    67,    66,    84,    77,    84,
      87,   100,   100,   100,   101,   101,   101,   101,   101,   102,
     102,   102,   102,   102,   102,   102,   102,   102,   104,   104,
     105,   105,   105,   105,   105,   106,   106,   106,    11,    99,
      28,   123,    94,    39,    87,    39,   121,    71,    87,    67,
      67,    67,   112,    67,    49,    49,    31,    39,    96,    97,
      11,   118,    80,    72,    85,   113,    84,    77,   102,   102,
      72,    95,     6,    15,    71,    98,    96,    67,   110,    67,
      67,    68,    69,   114,    67,    39,    96,    71,   113,    96,
     114
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    88,    89,    90,    90,    90,    91,    92,    92,    92,
      92,    92,    92,    92,    93,    94,    94,    94,    94,    95,
      96,    96,    96,    96,    97,    97,    97,    98,    98,    99,
     100,   100,   100,   100,   101,   101,   101,   101,   101,   101,
     102,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     103,   103,   103,   104,   104,   104,   105,   105,   105,   105,
     105,   105,   106,   106,   106,   106,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   108,   109,   109,   110,   110,   110,
     111,   112,   112,   113,   113,   114,   114,   114,   115,   116,
     117,   117,   118,   118,   119,   120,   120,   121,   121,   121,
     122,   122,   123,   123,   123,   123,   124,   124,   125,   125
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     3,     1,     2,     1,     1,     2,     3,
       2,     3,     3,     4,     1,     1,     3,     4,     4,     2,
       1,     2,     3,     4,     1,     3,     1,     1,     1,     1,
       3,     3,     3,     1,     3,     3,     3,     3,     3,     1,
       3,     3,     3,     3,     3,     3,     3,     5,     5,     1,
       2,     2,     1,     3,     3,     1,     3,     3,     3,     3,
       3,     1,     3,     3,     3,     1,     2,     2,     2,     2,
       1,     1,     2,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     5,     5,     1,     2,     2,
       3,     5,     7,     2,     0,     1,     1,     0,     5,     3,
       3,     2,     3,     1,     2,     3,     1,     1,     2,     3,
       3,     1,     1,     1,     3,     2,     1,     4,     1,     3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

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
#ifndef YYINITDEPTH
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
static YYSIZE_T
yystrlen (const char *yystr)
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
static char *
yystpcpy (char *yydest, const char *yysrc)
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
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
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
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
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

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

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

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
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
  int yytoken = 0;
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

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
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
      yychar = yylex ();
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
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
     '$$ = $1'.

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
#line 565 "KDbSqlParser.y" /* yacc.c:1646  */
    {
//todo: multiple statements
//todo: not only "select" statements
    KDbParserPrivate::get(globalParser)->setStatementType(KDbParser::Select);
    KDbParserPrivate::get(globalParser)->setQuerySchema((yyvsp[0].querySchema));
}
#line 1536 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 575 "KDbSqlParser.y" /* yacc.c:1646  */
    {
//todo: multiple statements
}
#line 1544 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 580 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.querySchema) = (yyvsp[-1].querySchema);
}
#line 1552 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 595 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.querySchema) = (yyvsp[0].querySchema);
}
#line 1560 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 694 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    sqlParserDebug() << "Select";
    if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[0].querySchema), nullptr )))
        YYABORT;
}
#line 1570 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 700 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    sqlParserDebug() << "Select ColViews=" << *(yyvsp[0].exprList);

    if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[-1].querySchema), (yyvsp[0].exprList) )))
        YYABORT;
}
#line 1581 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 707 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[-2].querySchema), (yyvsp[-1].exprList), (yyvsp[0].exprList) )))
        YYABORT;
}
#line 1590 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 712 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    sqlParserDebug() << "Select ColViews Tables";
    if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[-1].querySchema), nullptr, (yyvsp[0].exprList) )))
        YYABORT;
}
#line 1600 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 718 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    sqlParserDebug() << "Select ColViews Conditions";
    if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[-2].querySchema), (yyvsp[-1].exprList), nullptr, (yyvsp[0].selectOptions) )))
        YYABORT;
}
#line 1610 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 724 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    sqlParserDebug() << "Select Tables SelectOptions";
    if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[-2].querySchema), nullptr, (yyvsp[-1].exprList), (yyvsp[0].selectOptions) )))
        YYABORT;
}
#line 1620 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 730 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    sqlParserDebug() << "Select ColViews Tables SelectOptions";
    if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[-3].querySchema), (yyvsp[-2].exprList), (yyvsp[-1].exprList), (yyvsp[0].selectOptions) )))
        YYABORT;
}
#line 1630 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 739 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    sqlParserDebug() << "SELECT";
    (yyval.querySchema) = KDbParserPrivate::get(globalParser)->createQuery();
}
#line 1639 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 747 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    sqlParserDebug() << "WhereClause";
    (yyval.selectOptions) = new SelectOptionsInternal;
    (yyval.selectOptions)->whereExpr = *(yyvsp[0].expr);
    delete (yyvsp[0].expr);
}
#line 1650 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 754 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    sqlParserDebug() << "OrderByClause";
    (yyval.selectOptions) = new SelectOptionsInternal;
    (yyval.selectOptions)->orderByColumns = (yyvsp[0].orderByColumns);
}
#line 1660 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 760 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    sqlParserDebug() << "WhereClause ORDER BY OrderByClause";
    (yyval.selectOptions) = new SelectOptionsInternal;
    (yyval.selectOptions)->whereExpr = *(yyvsp[-3].expr);
    delete (yyvsp[-3].expr);
    (yyval.selectOptions)->orderByColumns = (yyvsp[0].orderByColumns);
}
#line 1672 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 768 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    sqlParserDebug() << "OrderByClause WhereClause";
    (yyval.selectOptions) = new SelectOptionsInternal;
    (yyval.selectOptions)->whereExpr = *(yyvsp[0].expr);
    delete (yyvsp[0].expr);
    (yyval.selectOptions)->orderByColumns = (yyvsp[-1].orderByColumns);
}
#line 1684 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 779 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = (yyvsp[0].expr);
}
#line 1692 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 788 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    sqlParserDebug() << "ORDER BY IDENTIFIER";
    (yyval.orderByColumns) = new QList<OrderByColumnInternal>;
    OrderByColumnInternal orderByColumn;
    orderByColumn.setColumnByNameOrNumber( *(yyvsp[0].variantValue) );
    (yyval.orderByColumns)->append( orderByColumn );
    delete (yyvsp[0].variantValue);
}
#line 1705 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 797 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    sqlParserDebug() << "ORDER BY IDENTIFIER OrderByOption";
    (yyval.orderByColumns) = new QList<OrderByColumnInternal>;
    OrderByColumnInternal orderByColumn;
    orderByColumn.setColumnByNameOrNumber( *(yyvsp[-1].variantValue) );
    orderByColumn.order = (yyvsp[0].sortOrderValue);
    (yyval.orderByColumns)->append( orderByColumn );
    delete (yyvsp[-1].variantValue);
}
#line 1719 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 807 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.orderByColumns) = (yyvsp[0].orderByColumns);
    OrderByColumnInternal orderByColumn;
    orderByColumn.setColumnByNameOrNumber( *(yyvsp[-2].variantValue) );
    (yyval.orderByColumns)->append( orderByColumn );
    delete (yyvsp[-2].variantValue);
}
#line 1731 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 815 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.orderByColumns) = (yyvsp[0].orderByColumns);
    OrderByColumnInternal orderByColumn;
    orderByColumn.setColumnByNameOrNumber( *(yyvsp[-3].variantValue) );
    orderByColumn.order = (yyvsp[-2].sortOrderValue);
    (yyval.orderByColumns)->append( orderByColumn );
    delete (yyvsp[-3].variantValue);
}
#line 1744 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 827 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.variantValue) = new QVariant( *(yyvsp[0].stringValue) );
    sqlParserDebug() << "OrderByColumnId: " << *(yyval.variantValue);
    delete (yyvsp[0].stringValue);
}
#line 1754 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 833 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.variantValue) = new QVariant( *(yyvsp[-2].stringValue) + QLatin1Char('.') + *(yyvsp[0].stringValue) );
    sqlParserDebug() << "OrderByColumnId: " << *(yyval.variantValue);
    delete (yyvsp[-2].stringValue);
    delete (yyvsp[0].stringValue);
}
#line 1765 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 840 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.variantValue) = new QVariant((yyvsp[0].integerValue));
    sqlParserDebug() << "OrderByColumnId: " << *(yyval.variantValue);
}
#line 1774 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 847 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.sortOrderValue) = KDbOrderByColumn::SortOrder::Ascending;
}
#line 1782 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 851 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.sortOrderValue) = KDbOrderByColumn::SortOrder::Descending;
}
#line 1790 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 863 "KDbSqlParser.y" /* yacc.c:1646  */
    {
//    sqlParserDebug() << "AND " << $3.debugString();
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), KDbToken::AND, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1801 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 870 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), KDbToken::OR, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1811 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 876 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), KDbToken::XOR, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1821 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 888 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), '>', *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1831 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 894 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), KDbToken::GREATER_OR_EQUAL, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1841 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 900 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), '<', *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1851 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 906 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), KDbToken::LESS_OR_EQUAL, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1861 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 912 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), '=', *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1871 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 924 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), KDbToken::NOT_EQUAL, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1881 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 930 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), KDbToken::NOT_EQUAL2, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1891 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 936 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), KDbToken::LIKE, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1901 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 43:
#line 942 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), KDbToken::NOT_LIKE, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1911 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 44:
#line 948 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), KDbToken::SQL_IN, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1921 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 954 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), KDbToken::SIMILAR_TO, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1931 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 46:
#line 960 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), KDbToken::NOT_SIMILAR_TO, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1941 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 966 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbNArgExpression(KDb::RelationalExpression, KDbToken::BETWEEN_AND);
    (yyval.expr)->toNArg().append( *(yyvsp[-4].expr) );
    (yyval.expr)->toNArg().append( *(yyvsp[-2].expr) );
    (yyval.expr)->toNArg().append( *(yyvsp[0].expr) );
    delete (yyvsp[-4].expr);
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1955 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 976 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbNArgExpression(KDb::RelationalExpression, KDbToken::NOT_BETWEEN_AND);
    (yyval.expr)->toNArg().append( *(yyvsp[-4].expr) );
    (yyval.expr)->toNArg().append( *(yyvsp[-2].expr) );
    (yyval.expr)->toNArg().append( *(yyvsp[0].expr) );
    delete (yyvsp[-4].expr);
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1969 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 50:
#line 992 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbUnaryExpression( KDbToken::SQL_IS_NULL, *(yyvsp[-1].expr) );
    delete (yyvsp[-1].expr);
}
#line 1978 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 51:
#line 997 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbUnaryExpression( KDbToken::SQL_IS_NOT_NULL, *(yyvsp[-1].expr) );
    delete (yyvsp[-1].expr);
}
#line 1987 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 53:
#line 1008 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), KDbToken::BITWISE_SHIFT_LEFT, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1997 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 54:
#line 1014 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), KDbToken::BITWISE_SHIFT_RIGHT, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2007 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 1026 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), '+', *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2017 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 1032 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), KDbToken::CONCATENATION, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2027 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 1038 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), '-', *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2037 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 1044 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), '&', *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2047 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 1050 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), '|', *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2057 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 1062 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), '/', *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2067 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 1068 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), '*', *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2077 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 1074 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(*(yyvsp[-2].expr), '%', *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2087 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 1087 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbUnaryExpression( '-', *(yyvsp[0].expr) );
    delete (yyvsp[0].expr);
}
#line 2096 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 1092 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbUnaryExpression( '+', *(yyvsp[0].expr) );
    delete (yyvsp[0].expr);
}
#line 2105 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 68:
#line 1097 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbUnaryExpression( '~', *(yyvsp[0].expr) );
    delete (yyvsp[0].expr);
}
#line 2114 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 69:
#line 1102 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbUnaryExpression( KDbToken::NOT, *(yyvsp[0].expr) );
    delete (yyvsp[0].expr);
}
#line 2123 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 70:
#line 1107 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbVariableExpression( *(yyvsp[0].stringValue) );

    //! @todo simplify this later if that's 'only one field name' expression
    sqlParserDebug() << "  + identifier: " << *(yyvsp[0].stringValue);
    delete (yyvsp[0].stringValue);
}
#line 2135 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 71:
#line 1115 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbQueryParameterExpression( *(yyvsp[0].stringValue) );
    sqlParserDebug() << "  + query parameter:" << *(yyval.expr);
    delete (yyvsp[0].stringValue);
}
#line 2145 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 72:
#line 1121 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    sqlParserDebug() << "  + function:" << *(yyvsp[-1].stringValue) << "(" << *(yyvsp[0].exprList) << ")";
    (yyval.expr) = new KDbFunctionExpression(*(yyvsp[-1].stringValue), *(yyvsp[0].exprList));
    delete (yyvsp[-1].stringValue);
    delete (yyvsp[0].exprList);
}
#line 2156 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 73:
#line 1129 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbVariableExpression( *(yyvsp[-2].stringValue) + QLatin1Char('.') + *(yyvsp[0].stringValue) );
    sqlParserDebug() << "  + identifier.identifier:" << *(yyvsp[-2].stringValue) << "." << *(yyvsp[0].stringValue);
    delete (yyvsp[-2].stringValue);
    delete (yyvsp[0].stringValue);
}
#line 2167 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 74:
#line 1136 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbConstExpression( KDbToken::SQL_NULL, QVariant() );
    sqlParserDebug() << "  + NULL";
//    $$ = new KDbField();
    //$$->setName(QString::null);
}
#line 2178 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 1143 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbConstExpression( KDbToken::SQL_TRUE, true );
}
#line 2186 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 1147 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbConstExpression( KDbToken::SQL_FALSE, false );
}
#line 2194 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 77:
#line 1151 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbConstExpression( KDbToken::CHARACTER_STRING_LITERAL, *(yyvsp[0].stringValue) );
    sqlParserDebug() << "  + constant " << (yyvsp[0].stringValue);
    delete (yyvsp[0].stringValue);
}
#line 2204 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 78:
#line 1157 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    QVariant val;
    if ((yyvsp[0].integerValue) <= INT_MAX && (yyvsp[0].integerValue) >= INT_MIN)
        val = (int)(yyvsp[0].integerValue);
    else if ((yyvsp[0].integerValue) <= UINT_MAX && (yyvsp[0].integerValue) >= 0)
        val = (uint)(yyvsp[0].integerValue);
    else if ((yyvsp[0].integerValue) <= LLONG_MAX && (yyvsp[0].integerValue) >= LLONG_MIN)
        val = (qint64)(yyvsp[0].integerValue);

//    if ($1 < ULLONG_MAX)
//        val = (quint64)$1;
//! @todo ok?

    (yyval.expr) = new KDbConstExpression( KDbToken::INTEGER_CONST, val );
    sqlParserDebug() << "  + int constant: " << val.toString();
}
#line 2225 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 79:
#line 1174 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbConstExpression( KDbToken::REAL_CONST, *(yyvsp[0].binaryValue) );
    sqlParserDebug() << "  + real constant: " << *(yyvsp[0].binaryValue);
    delete (yyvsp[0].binaryValue);
}
#line 2235 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 80:
#line 1180 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbConstExpression(KDbToken::DATE_CONST, QVariant::fromValue(*(yyvsp[0].dateValue)));
    sqlParserDebug() << "  + date constant:" << *(yyvsp[0].dateValue);
    delete (yyvsp[0].dateValue);
}
#line 2245 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 81:
#line 1186 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbConstExpression(KDbToken::TIME_CONST, QVariant::fromValue(*(yyvsp[0].timeValue)));
    sqlParserDebug() << "  + time constant:" << *(yyvsp[0].timeValue);
    delete (yyvsp[0].timeValue);
}
#line 2255 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 82:
#line 1192 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbConstExpression(KDbToken::DATETIME_CONST, QVariant::fromValue(*(yyvsp[0].dateTimeValue)));
    sqlParserDebug() << "  + datetime constant:" << *(yyvsp[0].dateTimeValue);
    delete (yyvsp[0].dateTimeValue);
}
#line 2265 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 1203 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.dateValue) = (yyvsp[-1].dateValue);
    sqlParserDebug() << "DateConst:" << *(yyval.dateValue);
}
#line 2274 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 85:
#line 1211 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.dateValue) = new KDbDate(*(yyvsp[-4].yearValue), *(yyvsp[-2].binaryValue), *(yyvsp[0].binaryValue));
    sqlParserDebug() << "DateValue:" << *(yyval.dateValue);
    delete (yyvsp[-4].yearValue);
    delete (yyvsp[-2].binaryValue);
    delete (yyvsp[0].binaryValue);
}
#line 2286 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 86:
#line 1219 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.dateValue) = new KDbDate(*(yyvsp[0].yearValue), *(yyvsp[-4].binaryValue), *(yyvsp[-2].binaryValue));
    sqlParserDebug() << "DateValue:" << *(yyval.dateValue);
    delete (yyvsp[-4].binaryValue);
    delete (yyvsp[-2].binaryValue);
    delete (yyvsp[0].yearValue);
}
#line 2298 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 87:
#line 1230 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.yearValue) = new KDbYear(KDbYear::Sign::None, *(yyvsp[0].binaryValue));
    sqlParserDebug() << "YearConst:" << *(yyval.yearValue);
    delete (yyvsp[0].binaryValue);
}
#line 2308 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 1236 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.yearValue) = new KDbYear(KDbYear::Sign::Plus, *(yyvsp[0].binaryValue));
    sqlParserDebug() << "YearConst:" << *(yyval.yearValue);
    delete (yyvsp[0].binaryValue);
}
#line 2318 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 89:
#line 1242 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.yearValue) = new KDbYear(KDbYear::Sign::Minus, *(yyvsp[0].binaryValue));
    sqlParserDebug() << "YearConst:" << *(yyval.yearValue);
    delete (yyvsp[0].binaryValue);
}
#line 2328 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 1251 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.timeValue) = (yyvsp[-1].timeValue);
    sqlParserDebug() << "TimeConst:" << *(yyval.timeValue);
}
#line 2337 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 91:
#line 1259 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.timeValue) = new KDbTime(*(yyvsp[-4].binaryValue), *(yyvsp[-2].binaryValue), {}, *(yyvsp[-1].binaryValue), (yyvsp[0].timePeriodValue));
    sqlParserDebug() << "TimeValue:" << *(yyval.timeValue);
    delete (yyvsp[-4].binaryValue);
    delete (yyvsp[-2].binaryValue);
    delete (yyvsp[-1].binaryValue);
}
#line 2349 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 1267 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.timeValue) = new KDbTime(*(yyvsp[-6].binaryValue), *(yyvsp[-4].binaryValue), *(yyvsp[-2].binaryValue), *(yyvsp[-1].binaryValue), (yyvsp[0].timePeriodValue));
    sqlParserDebug() << "TimeValue:" << *(yyval.timeValue);
    delete (yyvsp[-6].binaryValue);
    delete (yyvsp[-4].binaryValue);
    delete (yyvsp[-2].binaryValue);
    delete (yyvsp[-1].binaryValue);
}
#line 2362 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 93:
#line 1279 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.binaryValue) = (yyvsp[0].binaryValue);
}
#line 2370 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 94:
#line 1283 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.binaryValue) = new QByteArray;
}
#line 2378 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 95:
#line 1290 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.timePeriodValue) = KDbTime::Period::Am;
}
#line 2386 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 96:
#line 1294 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.timePeriodValue) = KDbTime::Period::Pm;
}
#line 2394 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 97:
#line 1298 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.timePeriodValue) = KDbTime::Period::None;
}
#line 2402 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 98:
#line 1305 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.dateTimeValue) = new KDbDateTime(*(yyvsp[-3].dateValue), *(yyvsp[-1].timeValue));
    sqlParserDebug() << "DateTimeConst:" << *(yyval.dateTimeValue);
    delete (yyvsp[-3].dateValue);
    delete (yyvsp[-1].timeValue);
}
#line 2413 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 99:
#line 1315 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    sqlParserDebug() << "(expr)";
    (yyval.expr) = new KDbUnaryExpression('(', *(yyvsp[-1].expr));
    delete (yyvsp[-1].expr);
}
#line 2423 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 1324 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.exprList) = (yyvsp[-1].exprList);
}
#line 2431 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 101:
#line 1328 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.exprList) = new KDbNArgExpression(KDb::ArgumentListExpression, ',');
}
#line 2439 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 1335 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.exprList) = (yyvsp[0].exprList);
    (yyval.exprList)->prepend( *(yyvsp[-2].expr) );
    delete (yyvsp[-2].expr);
}
#line 2449 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 1341 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.exprList) = new KDbNArgExpression(KDb::ArgumentListExpression, ',');
    (yyval.exprList)->append( *(yyvsp[0].expr) );
    delete (yyvsp[0].expr);
}
#line 2459 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 1350 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.exprList) = (yyvsp[0].exprList);
}
#line 2467 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 1395 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.exprList) = (yyvsp[-2].exprList);
    (yyval.exprList)->append(*(yyvsp[0].expr));
    delete (yyvsp[0].expr);
}
#line 2477 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 1401 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.exprList) = new KDbNArgExpression(KDb::TableListExpression, KDbToken::IDENTIFIER); //ok?
    (yyval.exprList)->append(*(yyvsp[0].expr));
    delete (yyvsp[0].expr);
}
#line 2487 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 1410 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    sqlParserDebug() << "FROM: '" << *(yyvsp[0].stringValue) << "'";
    (yyval.expr) = new KDbVariableExpression(*(yyvsp[0].stringValue));

    //! @todo this isn't ok for more tables:
    /*
    KDbField::ListIterator it = globalParser->query()->fieldsIterator();
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
                KDbParserError err(KDbParser::tr("Field List Error"), KDbParser::tr("Unknown column '%1' in table '%2'",item->name(),schema->name()), ctoken, current);
                globalParser->setError(err);
                yyerror("fieldlisterror");
            }
        }
    }*/
    delete (yyvsp[0].stringValue);
}
#line 2519 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 1438 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    //table + alias
    (yyval.expr) = new KDbBinaryExpression(
        KDbVariableExpression(*(yyvsp[-1].stringValue)), KDbToken::AS_EMPTY,
        KDbVariableExpression(*(yyvsp[0].stringValue))
    );
    delete (yyvsp[-1].stringValue);
    delete (yyvsp[0].stringValue);
}
#line 2533 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 1448 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    //table + alias
    (yyval.expr) = new KDbBinaryExpression(
        KDbVariableExpression(*(yyvsp[-2].stringValue)), KDbToken::AS,
        KDbVariableExpression(*(yyvsp[0].stringValue))
    );
    delete (yyvsp[-2].stringValue);
    delete (yyvsp[0].stringValue);
}
#line 2547 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 1463 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.exprList) = (yyvsp[-2].exprList);
    (yyval.exprList)->append(*(yyvsp[0].expr));
    delete (yyvsp[0].expr);
    sqlParserDebug() << "ColViews: ColViews , ColItem";
}
#line 2558 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 1470 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.exprList) = new KDbNArgExpression(KDb::FieldListExpression, KDbToken());
    (yyval.exprList)->append(*(yyvsp[0].expr));
    delete (yyvsp[0].expr);
    sqlParserDebug() << "ColViews: ColItem";
}
#line 2569 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 1480 "KDbSqlParser.y" /* yacc.c:1646  */
    {
//    $$ = new KDbField();
//    dummy->addField($$);
//    $$->setExpression( $1 );
//    globalParser->query()->addField($$);
    (yyval.expr) = (yyvsp[0].expr);
    sqlParserDebug() << " added column expr:" << *(yyvsp[0].expr);
}
#line 2582 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 1489 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = (yyvsp[0].expr);
    sqlParserDebug() << " added column wildcard:" << *(yyvsp[0].expr);
}
#line 2591 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 1494 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(
        *(yyvsp[-2].expr), KDbToken::AS,
        KDbVariableExpression(*(yyvsp[0].stringValue))
    );
    sqlParserDebug() << " added column expr:" << *(yyval.expr);
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].stringValue);
}
#line 2605 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 1504 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbBinaryExpression(
        *(yyvsp[-1].expr), KDbToken::AS_EMPTY,
        KDbVariableExpression(*(yyvsp[0].stringValue))
    );
    sqlParserDebug() << " added column expr:" << *(yyval.expr);
    delete (yyvsp[-1].expr);
    delete (yyvsp[0].stringValue);
}
#line 2619 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 1517 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = (yyvsp[0].expr);
}
#line 2627 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 1563 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = (yyvsp[-1].expr);
//! @todo DISTINCT '(' ColExpression ')'
//    $$->setName("DISTINCT(" + $3->name() + ")");
}
#line 2637 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 1572 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new KDbVariableExpression(QLatin1String("*"));
    sqlParserDebug() << "all columns";

//    KDbQueryAsterisk *ast = new KDbQueryAsterisk(globalParser->query(), dummy);
//    globalParser->query()->addAsterisk(ast);
//    requiresTable = true;
}
#line 2650 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 1581 "KDbSqlParser.y" /* yacc.c:1646  */
    {
    QString s( *(yyvsp[-2].stringValue) );
    s += QLatin1String(".*");
    (yyval.expr) = new KDbVariableExpression(s);
    sqlParserDebug() << "  + all columns from " << s;
    delete (yyvsp[-2].stringValue);
}
#line 2662 "sqlparser.cpp" /* yacc.c:1646  */
    break;


#line 2666 "sqlparser.cpp" /* yacc.c:1646  */
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

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
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

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

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

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


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

#if !defined yyoverflow || YYERROR_VERBOSE
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
  /* Do not reclaim the symbols of the rule whose action triggered
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
  return yyresult;
}
#line 1596 "KDbSqlParser.y" /* yacc.c:1906  */


KDB_TESTING_EXPORT const char* g_tokenName(unsigned int offset) {
    const int t = YYTRANSLATE(offset);
    if (t >= YYTRANSLATE(::SQL_TYPE)) {
        return yytname[t];
    }
    return nullptr;
}

//static
const int KDbToken::maxCharTokenValue = 253;

//static
const int KDbToken::maxTokenValue = YYMAXUTOK;

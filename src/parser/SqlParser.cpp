/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 435 "SqlParser.y" /* yacc.c:339  */

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


#line 143 "SqlParser.cpp" /* yacc.c:339  */

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
   by #include "SqlParser.tab.h".  */
#ifndef YY_YY_SQLPARSER_TAB_H_INCLUDED
# define YY_YY_SQLPARSER_TAB_H_INCLUDED
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
    UNION = 302,
    EXCEPT = 303,
    INTERSECT = 304,
    OR = 305,
    AND = 306,
    XOR = 307,
    NOT = 308,
    GREATER_OR_EQUAL = 309,
    NOT_EQUAL = 310,
    NOT_EQUAL2 = 311,
    SQL_IN = 312,
    LIKE = 313,
    ILIKE = 314,
    SIMILAR_TO = 315,
    NOT_SIMILAR_TO = 316,
    BETWEEN = 317,
    UMINUS = 318,
    __LAST_TOKEN = 319
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 512 "SqlParser.y" /* yacc.c:355  */

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

#line 264 "SqlParser.cpp" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SQLPARSER_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 279 "SqlParser.cpp" /* yacc.c:358  */

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
#define YYFINAL  10
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   191

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  88
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  37
/* YYNRULES -- Number of rules.  */
#define YYNRULES  112
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  180

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   319

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    55,    50,    85,    59,
      56,    57,    49,    48,    53,    47,    54,    60,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    52,
      69,    68,    70,    58,    51,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    82,     2,    83,    80,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    86,     2,    87,     2,     2,     2,
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
      45,    46,    61,    62,    63,    64,    65,    66,    67,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    81,    84
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   583,   583,   593,   597,   598,   608,   612,   620,   619,
     629,   629,   635,   643,   659,   659,   665,   670,   675,   683,
     688,   695,   702,   710,   716,   723,   728,   734,   740,   749,
     759,   766,   772,   780,   791,   800,   809,   819,   827,   839,
     845,   852,   859,   863,   870,   875,   882,   888,   895,   900,
     906,   912,   918,   924,   931,   936,   943,   949,   955,   961,
     967,   974,   979,   984,   990,   995,  1001,  1008,  1013,  1019,
    1023,  1029,  1035,  1042,  1047,  1053,  1059,  1066,  1072,  1077,
    1082,  1087,  1092,  1100,  1106,  1114,  1121,  1128,  1132,  1136,
    1142,  1159,  1165,  1170,  1179,  1189,  1195,  1206,  1251,  1256,
    1265,  1293,  1303,  1318,  1325,  1335,  1344,  1349,  1359,  1372,
    1416,  1425,  1434
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
  "LESS_OR_EQUAL", "SQL_NULL", "SQL_IS", "SQL_IS_NULL", "SQL_IS_NOT_NULL",
  "ORDER", "PRIMARY", "SELECT", "INTEGER_CONST", "REAL_CONST", "RIGHT",
  "SQL_ON", "DATE_CONST", "DATETIME_CONST", "TIME_CONST", "TABLE",
  "IDENTIFIER", "IDENTIFIER_DOT_ASTERISK", "QUERY_PARAMETER", "VARCHAR",
  "WHERE", "SQL", "SQL_TRUE", "SQL_FALSE", "SCAN_ERROR", "'-'", "'+'",
  "'*'", "'%'", "'@'", "';'", "','", "'.'", "'$'", "'('", "')'", "'?'",
  "'\\''", "'/'", "UNION", "EXCEPT", "INTERSECT", "OR", "AND", "XOR",
  "NOT", "'='", "'<'", "'>'", "GREATER_OR_EQUAL", "NOT_EQUAL",
  "NOT_EQUAL2", "SQL_IN", "LIKE", "ILIKE", "SIMILAR_TO", "NOT_SIMILAR_TO",
  "BETWEEN", "'^'", "UMINUS", "'['", "']'", "__LAST_TOKEN", "'&'", "'|'",
  "'~'", "$accept", "TopLevelStatement", "StatementList", "Statement",
  "CreateTableStatement", "$@1", "ColDefs", "ColDef", "ColKeys", "ColKey",
  "ColType", "SelectStatement", "Select", "SelectOptions", "WhereClause",
  "OrderByClause", "OrderByColumnId", "OrderByOption", "aExpr", "aExpr2",
  "aExpr3", "aExpr4", "aExpr5", "aExpr6", "aExpr7", "aExpr8", "aExpr9",
  "aExpr10", "aExprList", "aExprList2", "Tables", "FlatTableList",
  "FlatTable", "ColViews", "ColItem", "ColExpression", "ColWildCard", YY_NULLPTR
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
     295,   296,   297,   298,   299,   300,   301,    45,    43,    42,
      37,    64,    59,    44,    46,    36,    40,    41,    63,    39,
      47,   302,   303,   304,   305,   306,   307,   308,    61,    60,
      62,   309,   310,   311,   312,   313,   314,   315,   316,   317,
      94,   318,    91,    93,   319,    38,   124,   126
};
# endif

#define YYPACT_NINF -36

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-36)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      -3,   -27,   -36,    22,   -36,   -12,   -36,   -36,   -11,     9,
     -36,    -3,   -36,    29,    54,   -36,   -36,   -36,    34,   -36,
     -36,   -36,    79,    79,   -36,    79,    79,    79,   -36,   -36,
      56,    61,    26,   -36,     7,    -4,    24,   -36,   -36,   -14,
     -36,    10,   -36,   -36,   -36,    49,    17,    42,   -36,    -8,
      79,   -36,   105,   -36,   -36,    50,   -36,   -36,    79,    79,
      79,    79,    79,    79,    79,    79,   -36,   -36,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    79,    79,   100,    79,    19,   -36,    98,   -24,
      95,   -36,    89,    77,   122,   -36,    54,   -36,   -36,   109,
     106,   126,   -36,   -36,   -36,   -36,   -36,   -36,   -36,   -36,
     -36,   -36,   -36,   -36,   -36,   -36,   -36,   -36,   -36,   -36,
     -36,   -36,   -36,   -36,   -36,   -36,   -36,    -6,   -36,   -36,
     154,   -36,   -36,   129,   -36,   -36,   -36,    79,   -36,   -36,
     114,   127,     0,    -6,     5,     1,   -36,   109,   -36,   132,
     -36,   -36,   -36,    -6,   118,   -36,   116,   117,    -5,   129,
     -36,   -36,   -36,    -6,   144,   145,   -36,   156,   155,    -5,
     -36,   -36,   -36,   120,   123,   -36,   -36,   -36,   -36,   -36
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    29,     0,     2,     4,     6,     7,    23,     0,
       1,     5,    89,     0,     0,    86,    90,    91,    82,    83,
      87,    88,     0,     0,   111,     0,     0,     0,   109,    44,
      48,    54,    61,    64,    67,    73,    77,    92,    26,    24,
     104,   105,   106,     8,     3,     0,   100,    97,    99,     0,
       0,    84,    82,    78,    79,     0,    81,    80,     0,     0,
       0,     0,     0,     0,     0,     0,    62,    63,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    27,    30,    25,
       0,   108,     0,     0,     0,   101,     0,    85,   112,     0,
       0,     0,    93,    46,    45,    47,    52,    53,    51,    49,
      50,    55,    56,    58,    57,    59,    60,    65,    66,    69,
      70,    68,    71,    72,    75,    76,    74,     0,    34,   103,
       0,    28,   107,     0,   110,   102,    98,     0,    94,    41,
      39,    31,    35,     0,    22,     0,    11,    96,    95,     0,
      33,    42,    43,     0,    36,    32,    19,     0,    12,     0,
       9,    40,    37,     0,     0,     0,    18,     0,     0,    13,
      15,    10,    38,     0,     0,    16,    17,    14,    20,    21
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -36,   -36,   168,   -36,   -36,   -36,   -36,    23,   -36,    12,
     -36,   -36,   -36,    94,    43,   -35,   -36,   -36,   -25,    11,
      88,    69,   -36,     3,    78,    32,   121,   -36,   -36,    48,
     147,   -36,    91,   -36,   102,   146,   -36
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,     4,     5,     6,    92,   145,   146,   169,   170,
     158,     7,     8,    87,    88,   141,   142,   154,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    51,   100,
      38,    47,    48,    39,    40,    41,    42
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      55,    12,   166,    84,    14,    13,   151,    14,   156,    76,
       9,     1,    15,    84,    90,   152,    74,    75,    85,    16,
      17,    94,    10,   167,   139,    99,     2,    18,    85,    19,
      97,    12,   140,    20,    21,    13,    22,    23,    24,    86,
      11,    98,    15,    77,    78,    25,   157,    43,    91,    16,
      17,    66,    67,   153,   159,    95,    26,    18,   160,    19,
     128,    12,   168,    20,    21,    13,    22,    23,    24,   103,
     104,   105,    15,    81,    82,    25,    27,   117,   118,    16,
      17,    79,    80,    61,    83,    45,    26,    52,    49,    19,
      50,    12,    46,    20,    21,    96,    22,    23,    68,    69,
      70,    71,    15,    72,    73,    25,    27,   102,   155,    16,
      17,   127,   147,   124,   125,   126,    26,    52,   162,    19,
      58,    59,    60,    20,    21,   130,    22,    23,   172,    62,
      63,    64,    65,   132,   134,    25,    27,   111,   112,   113,
     114,   115,   116,    53,    54,   133,    26,    56,    57,   106,
     107,   108,   109,   110,   119,   120,   121,   122,   123,   101,
     135,    50,   137,   138,    97,   143,    27,   144,   149,    85,
     161,   163,   164,   165,   173,   174,   175,   178,   176,    44,
     179,   177,   171,   131,   150,   148,    89,   136,   129,     0,
       0,    93
};

static const yytype_int16 yycheck[] =
{
      25,    12,     7,    27,    18,    16,     6,    18,     3,    13,
      37,    14,    23,    27,     4,    15,     9,    10,    42,    30,
      31,     4,     0,    28,    30,    50,    29,    38,    42,    40,
      38,    12,    38,    44,    45,    16,    47,    48,    49,    53,
      52,    49,    23,    47,    48,    56,    41,    38,    38,    30,
      31,    25,    26,    53,    53,    38,    67,    38,    57,    40,
      85,    12,    67,    44,    45,    16,    47,    48,    49,    58,
      59,    60,    23,    49,    50,    56,    87,    74,    75,    30,
      31,    85,    86,    22,    60,    56,    67,    38,    54,    40,
      56,    12,    38,    44,    45,    53,    47,    48,    72,    73,
      74,    75,    23,    77,    78,    56,    87,    57,   143,    30,
      31,    11,   137,    81,    82,    83,    67,    38,   153,    40,
      64,    65,    66,    44,    45,    27,    47,    48,   163,    68,
      69,    70,    71,    38,    57,    56,    87,    68,    69,    70,
      71,    72,    73,    22,    23,    56,    67,    26,    27,    61,
      62,    63,    64,    65,    76,    77,    78,    79,    80,    54,
      38,    56,    53,    57,    38,    11,    87,    38,    54,    42,
      38,    53,    56,    56,    30,    30,    20,    57,    23,    11,
      57,   169,   159,    89,   141,   137,    39,    96,    86,    -1,
      -1,    45
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    14,    29,    89,    90,    91,    92,    99,   100,    37,
       0,    52,    12,    16,    18,    23,    30,    31,    38,    40,
      44,    45,    47,    48,    49,    56,    67,    87,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   118,   121,
     122,   123,   124,    38,    90,    56,    38,   119,   120,    54,
      56,   116,    38,   114,   114,   106,   114,   114,    64,    65,
      66,    22,    68,    69,    70,    71,    25,    26,    72,    73,
      74,    75,    77,    78,     9,    10,    13,    47,    48,    85,
      86,    49,    50,    60,    27,    42,    53,   101,   102,   118,
       4,    38,    93,   123,     4,    38,    53,    38,    49,   106,
     117,    54,    57,   107,   107,   107,   108,   108,   108,   108,
     108,   109,   109,   109,   109,   109,   109,   111,   111,   112,
     112,   112,   112,   112,   113,   113,   113,    11,   106,   122,
      27,   101,    38,    56,    57,    38,   120,    53,    57,    30,
      38,   103,   104,    11,    38,    94,    95,   106,   117,    54,
     102,     6,    15,    53,   105,   103,     3,    41,    98,    53,
      57,    38,   103,    53,    56,    56,     7,    28,    67,    96,
      97,    95,   103,    30,    30,    20,    23,    97,    57,    57
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    88,    89,    90,    90,    90,    91,    91,    93,    92,
      94,    94,    95,    95,    96,    96,    97,    97,    97,    98,
      98,    98,    98,    99,    99,    99,    99,    99,    99,   100,
     101,   101,   101,   101,   102,   103,   103,   103,   103,   104,
     104,   104,   105,   105,   106,   107,   107,   107,   107,   108,
     108,   108,   108,   108,   108,   109,   109,   109,   109,   109,
     109,   109,   110,   110,   110,   111,   111,   111,   112,   112,
     112,   112,   112,   112,   113,   113,   113,   113,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   115,   116,   117,   117,   118,   119,   119,
     120,   120,   120,   121,   121,   122,   122,   122,   122,   123,
     123,   124,   124
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
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
#line 584 "SqlParser.y" /* yacc.c:1646  */
    {
//todo: multiple statements
//todo: not only "select" statements
    parser->setOperation(Parser::OP_Select);
    parser->setQuerySchema((yyvsp[0].querySchema));
}
#line 1510 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 594 "SqlParser.y" /* yacc.c:1646  */
    {
//todo: multiple statements
}
#line 1518 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 599 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.querySchema) = (yyvsp[-1].querySchema);
}
#line 1526 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 609 "SqlParser.y" /* yacc.c:1646  */
    {
YYACCEPT;
}
#line 1534 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 613 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.querySchema) = (yyvsp[0].querySchema);
}
#line 1542 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 620 "SqlParser.y" /* yacc.c:1646  */
    {
    parser->setOperation(Parser::OP_CreateTable);
    parser->createTable((yyvsp[0].stringValue)->toLatin1());
    delete (yyvsp[0].stringValue);
}
#line 1552 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 630 "SqlParser.y" /* yacc.c:1646  */
    {
}
#line 1559 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 636 "SqlParser.y" /* yacc.c:1646  */
    {
    PreDbg << "adding field " << *(yyvsp[-1].stringValue);
    field->setName(*(yyvsp[-1].stringValue));
    parser->table()->addField(field);
    field = 0;
    delete (yyvsp[-1].stringValue);
}
#line 1571 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 644 "SqlParser.y" /* yacc.c:1646  */
    {
    PreDbg << "adding field " << *(yyvsp[-2].stringValue);
    field->setName(*(yyvsp[-2].stringValue));
    delete (yyvsp[-2].stringValue);
    parser->table()->addField(field);

//    if(field->isPrimaryKey())
//        parser->table()->addPrimaryKey(field->name());

//    delete field;
//    field = 0;
}
#line 1588 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 660 "SqlParser.y" /* yacc.c:1646  */
    {
}
#line 1595 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 666 "SqlParser.y" /* yacc.c:1646  */
    {
    field->setPrimaryKey(true);
    PreDbg << "primary";
}
#line 1604 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 671 "SqlParser.y" /* yacc.c:1646  */
    {
    field->setNotNull(true);
    PreDbg << "not_null";
}
#line 1613 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 676 "SqlParser.y" /* yacc.c:1646  */
    {
    field->setAutoIncrement(true);
    PreDbg << "ainc";
}
#line 1622 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 684 "SqlParser.y" /* yacc.c:1646  */
    {
    field = new Field();
    field->setType((yyvsp[0].colType));
}
#line 1631 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 689 "SqlParser.y" /* yacc.c:1646  */
    {
    PreDbg << "sql + length";
    field = new Field();
    field->setPrecision((yyvsp[-1].integerValue));
    field->setType((yyvsp[-3].colType));
}
#line 1642 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 696 "SqlParser.y" /* yacc.c:1646  */
    {
    field = new Field();
    field->setPrecision((yyvsp[-1].integerValue));
    field->setType(Field::Text);
}
#line 1652 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 702 "SqlParser.y" /* yacc.c:1646  */
    {
    // SQLITE compatibillity
    field = new Field();
    field->setType(Field::InvalidType);
}
#line 1662 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 711 "SqlParser.y" /* yacc.c:1646  */
    {
    PreDbg << "Select";
    if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[0].querySchema), 0 )))
        return 0;
}
#line 1672 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 717 "SqlParser.y" /* yacc.c:1646  */
    {
    PreDbg << "Select ColViews=" << *(yyvsp[0].exprList);

    if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[-1].querySchema), (yyvsp[0].exprList) )))
        return 0;
}
#line 1683 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 724 "SqlParser.y" /* yacc.c:1646  */
    {
    if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[-2].querySchema), (yyvsp[-1].exprList), (yyvsp[0].exprList) )))
        return 0;
}
#line 1692 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 729 "SqlParser.y" /* yacc.c:1646  */
    {
    PreDbg << "Select ColViews Tables";
    if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[-1].querySchema), 0, (yyvsp[0].exprList) )))
        return 0;
}
#line 1702 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 735 "SqlParser.y" /* yacc.c:1646  */
    {
    PreDbg << "Select ColViews Conditions";
    if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[-2].querySchema), (yyvsp[-1].exprList), 0, (yyvsp[0].selectOptions) )))
        return 0;
}
#line 1712 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 741 "SqlParser.y" /* yacc.c:1646  */
    {
    PreDbg << "Select ColViews Tables SelectOptions";
    if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[-3].querySchema), (yyvsp[-2].exprList), (yyvsp[-1].exprList), (yyvsp[0].selectOptions) )))
        return 0;
}
#line 1722 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 750 "SqlParser.y" /* yacc.c:1646  */
    {
    PreDbg << "SELECT";
//    parser->createSelect();
//    parser->setOperation(Parser::OP_Select);
    (yyval.querySchema) = new QuerySchema();
}
#line 1733 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 760 "SqlParser.y" /* yacc.c:1646  */
    {
    PreDbg << "WhereClause";
    (yyval.selectOptions) = new SelectOptionsInternal;
    (yyval.selectOptions)->whereExpr = *(yyvsp[0].expr);
    delete (yyvsp[0].expr);
}
#line 1744 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 767 "SqlParser.y" /* yacc.c:1646  */
    {
    PreDbg << "OrderByClause";
    (yyval.selectOptions) = new SelectOptionsInternal;
    (yyval.selectOptions)->orderByColumns = (yyvsp[0].orderByColumns);
}
#line 1754 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 773 "SqlParser.y" /* yacc.c:1646  */
    {
    PreDbg << "WhereClause ORDER BY OrderByClause";
    (yyval.selectOptions) = new SelectOptionsInternal;
    (yyval.selectOptions)->whereExpr = *(yyvsp[-3].expr);
    delete (yyvsp[-3].expr);
    (yyval.selectOptions)->orderByColumns = (yyvsp[0].orderByColumns);
}
#line 1766 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 781 "SqlParser.y" /* yacc.c:1646  */
    {
    PreDbg << "OrderByClause WhereClause";
    (yyval.selectOptions) = new SelectOptionsInternal;
    (yyval.selectOptions)->whereExpr = *(yyvsp[0].expr);
    delete (yyvsp[0].expr);
    (yyval.selectOptions)->orderByColumns = (yyvsp[-1].orderByColumns);
}
#line 1778 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 792 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = (yyvsp[0].expr);
}
#line 1786 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 801 "SqlParser.y" /* yacc.c:1646  */
    {
    PreDbg << "ORDER BY IDENTIFIER";
    (yyval.orderByColumns) = new OrderByColumnInternal::List;
    OrderByColumnInternal orderByColumn;
    orderByColumn.setColumnByNameOrNumber( *(yyvsp[0].variantValue) );
    (yyval.orderByColumns)->append( orderByColumn );
    delete (yyvsp[0].variantValue);
}
#line 1799 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 810 "SqlParser.y" /* yacc.c:1646  */
    {
    PreDbg << "ORDER BY IDENTIFIER OrderByOption";
    (yyval.orderByColumns) = new OrderByColumnInternal::List;
    OrderByColumnInternal orderByColumn;
    orderByColumn.setColumnByNameOrNumber( *(yyvsp[-1].variantValue) );
    orderByColumn.ascending = (yyvsp[0].booleanValue);
    (yyval.orderByColumns)->append( orderByColumn );
    delete (yyvsp[-1].variantValue);
}
#line 1813 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 820 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.orderByColumns) = (yyvsp[0].orderByColumns);
    OrderByColumnInternal orderByColumn;
    orderByColumn.setColumnByNameOrNumber( *(yyvsp[-2].variantValue) );
    (yyval.orderByColumns)->append( orderByColumn );
    delete (yyvsp[-2].variantValue);
}
#line 1825 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 828 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.orderByColumns) = (yyvsp[0].orderByColumns);
    OrderByColumnInternal orderByColumn;
    orderByColumn.setColumnByNameOrNumber( *(yyvsp[-3].variantValue) );
    orderByColumn.ascending = (yyvsp[-2].booleanValue);
    (yyval.orderByColumns)->append( orderByColumn );
    delete (yyvsp[-3].variantValue);
}
#line 1838 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 840 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.variantValue) = new QVariant( *(yyvsp[0].stringValue) );
    PreDbg << "OrderByColumnId: " << *(yyval.variantValue);
    delete (yyvsp[0].stringValue);
}
#line 1848 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 846 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.variantValue) = new QVariant( *(yyvsp[-2].stringValue) + QLatin1Char('.') + *(yyvsp[0].stringValue) );
    PreDbg << "OrderByColumnId: " << *(yyval.variantValue);
    delete (yyvsp[-2].stringValue);
    delete (yyvsp[0].stringValue);
}
#line 1859 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 853 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.variantValue) = new QVariant((yyvsp[0].integerValue));
    PreDbg << "OrderByColumnId: " << *(yyval.variantValue);
}
#line 1868 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 860 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.booleanValue) = true;
}
#line 1876 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 43:
#line 864 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.booleanValue) = false;
}
#line 1884 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 876 "SqlParser.y" /* yacc.c:1646  */
    {
//    PreDbg << "AND " << $3.debugString();
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), AND, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1895 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 46:
#line 883 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), OR, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1905 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 889 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), XOR, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1915 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 49:
#line 901 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), '>', *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1925 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 50:
#line 907 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), GREATER_OR_EQUAL, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1935 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 51:
#line 913 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), '<', *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1945 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 52:
#line 919 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), LESS_OR_EQUAL, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1955 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 53:
#line 925 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), '=', *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1965 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 55:
#line 937 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), NOT_EQUAL, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1975 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 944 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), NOT_EQUAL2, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1985 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 950 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), LIKE, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 1995 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 956 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), SQL_IN, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2005 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 962 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), SIMILAR_TO, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2015 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 968 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), NOT_SIMILAR_TO, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2025 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 980 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new UnaryExpression( SQL_IS_NULL, *(yyvsp[-1].expr) );
    delete (yyvsp[-1].expr);
}
#line 2034 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 985 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new UnaryExpression( SQL_IS_NOT_NULL, *(yyvsp[-1].expr) );
    delete (yyvsp[-1].expr);
}
#line 2043 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 65:
#line 996 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), BITWISE_SHIFT_LEFT, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2053 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 1002 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), BITWISE_SHIFT_RIGHT, *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2063 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 68:
#line 1014 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), '+', *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2073 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 69:
#line 1020 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), CONCATENATION, *(yyvsp[0].expr));
}
#line 2081 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 70:
#line 1024 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), '-', *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2091 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 71:
#line 1030 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), '&', *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2101 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 72:
#line 1036 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), '|', *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2111 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 74:
#line 1048 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), '/', *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2121 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 1054 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), '*', *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2131 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 1060 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(*(yyvsp[-2].expr), '%', *(yyvsp[0].expr));
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2141 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 78:
#line 1073 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new UnaryExpression( '-', *(yyvsp[0].expr) );
    delete (yyvsp[0].expr);
}
#line 2150 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 79:
#line 1078 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new UnaryExpression( '+', *(yyvsp[0].expr) );
    delete (yyvsp[0].expr);
}
#line 2159 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 80:
#line 1083 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new UnaryExpression( '~', *(yyvsp[0].expr) );
    delete (yyvsp[0].expr);
}
#line 2168 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 81:
#line 1088 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new UnaryExpression( NOT, *(yyvsp[0].expr) );
    delete (yyvsp[0].expr);
}
#line 2177 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 82:
#line 1093 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new VariableExpression( *(yyvsp[0].stringValue) );

//TODO: simplify this later if that's 'only one field name' expression
    PreDbg << "  + identifier: " << *(yyvsp[0].stringValue);
    delete (yyvsp[0].stringValue);
}
#line 2189 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 83:
#line 1101 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new QueryParameterExpression( *(yyvsp[0].stringValue) );
    PreDbg << "  + query parameter:" << *(yyval.expr);
    delete (yyvsp[0].stringValue);
}
#line 2199 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 1107 "SqlParser.y" /* yacc.c:1646  */
    {
    PreDbg << "  + function:" << *(yyvsp[-1].stringValue) << "(" << *(yyvsp[0].exprList) << ")";
    (yyval.expr) = new FunctionExpression(*(yyvsp[-1].stringValue), *(yyvsp[0].exprList));
    delete (yyvsp[-1].stringValue);
    delete (yyvsp[0].exprList);
}
#line 2210 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 85:
#line 1115 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new VariableExpression( *(yyvsp[-2].stringValue) + QLatin1Char('.') + *(yyvsp[0].stringValue) );
    PreDbg << "  + identifier.identifier:" << *(yyvsp[-2].stringValue) << "." << *(yyvsp[0].stringValue);
    delete (yyvsp[-2].stringValue);
    delete (yyvsp[0].stringValue);
}
#line 2221 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 86:
#line 1122 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new ConstExpression( SQL_NULL, QVariant() );
    PreDbg << "  + NULL";
//    $$ = new Field();
    //$$->setName(QString::null);
}
#line 2232 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 87:
#line 1129 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new ConstExpression( SQL_TRUE, true );
}
#line 2240 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 1133 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new ConstExpression( SQL_FALSE, false );
}
#line 2248 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 89:
#line 1137 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new ConstExpression( CHARACTER_STRING_LITERAL, *(yyvsp[0].stringValue) );
    PreDbg << "  + constant " << (yyvsp[0].stringValue);
    delete (yyvsp[0].stringValue);
}
#line 2258 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 1143 "SqlParser.y" /* yacc.c:1646  */
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
//TODO ok?

    (yyval.expr) = new ConstExpression( INTEGER_CONST, val );
    PreDbg << "  + int constant: " << val.toString();
}
#line 2279 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 91:
#line 1160 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new ConstExpression( REAL_CONST, QPoint( (yyvsp[0].realValue).integer, (yyvsp[0].realValue).fractional ) );
    PreDbg << "  + real constant: " << (yyvsp[0].realValue).integer << "." << (yyvsp[0].realValue).fractional;
}
#line 2288 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 93:
#line 1171 "SqlParser.y" /* yacc.c:1646  */
    {
    PreDbg << "(expr)";
    (yyval.expr) = new UnaryExpression('(', *(yyvsp[-1].expr));
    delete (yyvsp[-1].expr);
}
#line 2298 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 94:
#line 1180 "SqlParser.y" /* yacc.c:1646  */
    {
//    $$ = new NArgExpression(UnknownExpressionClass, 0);
//    $$->add( $1 );
//    $$->add( $3 );
    (yyval.exprList) = (yyvsp[-1].exprList);
}
#line 2309 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 95:
#line 1190 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.exprList) = (yyvsp[0].exprList);
    (yyval.exprList)->prepend( *(yyvsp[-2].expr) );
    delete (yyvsp[-2].expr);
}
#line 2319 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 96:
#line 1196 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.exprList) = new NArgExpression(UnknownExpressionClass, 0);
    (yyval.exprList)->append( *(yyvsp[-2].expr) );
    (yyval.exprList)->append( *(yyvsp[0].expr) );
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].expr);
}
#line 2331 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 97:
#line 1207 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.exprList) = (yyvsp[0].exprList);
}
#line 2339 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 98:
#line 1252 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.exprList) = (yyvsp[-2].exprList);
    (yyval.exprList)->append(*(yyvsp[0].expr));
}
#line 2348 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 99:
#line 1257 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.exprList) = new NArgExpression(TableListExpressionClass, IDENTIFIER); //ok?
    (yyval.exprList)->append(*(yyvsp[0].expr));
    delete (yyvsp[0].expr);
}
#line 2358 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 1266 "SqlParser.y" /* yacc.c:1646  */
    {
    PreDbg << "FROM: '" << *(yyvsp[0].stringValue) << "'";
    (yyval.expr) = new VariableExpression(*(yyvsp[0].stringValue));

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
    delete (yyvsp[0].stringValue);
}
#line 2390 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 101:
#line 1294 "SqlParser.y" /* yacc.c:1646  */
    {
    //table + alias
    (yyval.expr) = new BinaryExpression(
        VariableExpression(*(yyvsp[-1].stringValue)), AS_EMPTY,
        VariableExpression(*(yyvsp[0].stringValue))
    );
    delete (yyvsp[-1].stringValue);
    delete (yyvsp[0].stringValue);
}
#line 2404 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 1304 "SqlParser.y" /* yacc.c:1646  */
    {
    //table + alias
    (yyval.expr) = new BinaryExpression(
        VariableExpression(*(yyvsp[-2].stringValue)), AS,
        VariableExpression(*(yyvsp[0].stringValue))
    );
    delete (yyvsp[-2].stringValue);
    delete (yyvsp[0].stringValue);
}
#line 2418 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 1319 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.exprList) = (yyvsp[-2].exprList);
    (yyval.exprList)->append(*(yyvsp[0].expr));
    delete (yyvsp[0].expr);
    PreDbg << "ColViews: ColViews , ColItem";
}
#line 2429 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 1326 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.exprList) = new NArgExpression(FieldListExpressionClass, 0);
    (yyval.exprList)->append(*(yyvsp[0].expr));
    delete (yyvsp[0].expr);
    PreDbg << "ColViews: ColItem";
}
#line 2440 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 1336 "SqlParser.y" /* yacc.c:1646  */
    {
//    $$ = new Field();
//    dummy->addField($$);
//    $$->setExpression( $1 );
//    parser->select()->addField($$);
    (yyval.expr) = (yyvsp[0].expr);
    PreDbg << " added column expr:" << *(yyvsp[0].expr);
}
#line 2453 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 1345 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = (yyvsp[0].expr);
    PreDbg << " added column wildcard:" << *(yyvsp[0].expr);
}
#line 2462 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 1350 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(
        *(yyvsp[-2].expr), AS,
        VariableExpression(*(yyvsp[0].stringValue))
    );
    PreDbg << " added column expr:" << *(yyval.expr);
    delete (yyvsp[-2].expr);
    delete (yyvsp[0].stringValue);
}
#line 2476 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 1360 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpression(
        *(yyvsp[-1].expr), AS_EMPTY,
        VariableExpression(*(yyvsp[0].stringValue))
    );
    PreDbg << " added column expr:" << *(yyval.expr);
    delete (yyvsp[-1].expr);
    delete (yyvsp[0].stringValue);
}
#line 2490 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 1373 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = (yyvsp[0].expr);
}
#line 2498 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 1417 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = (yyvsp[-1].expr);
//! @todo DISTINCT '(' ColExpression ')'
//    $$->setName("DISTINCT(" + $3->name() + ")");
}
#line 2508 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 1426 "SqlParser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new VariableExpression(QLatin1String("*"));
    PreDbg << "all columns";

//    QueryAsterisk *ast = new QueryAsterisk(parser->select(), dummy);
//    parser->select()->addAsterisk(ast);
//    requiresTable = true;
}
#line 2521 "SqlParser.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 1435 "SqlParser.y" /* yacc.c:1646  */
    {
    QString s( *(yyvsp[-2].stringValue) );
    s += QLatin1String(".*");
    (yyval.expr) = new VariableExpression(s);
    PreDbg << "  + all columns from " << s;
    delete (yyvsp[-2].stringValue);
}
#line 2533 "SqlParser.cpp" /* yacc.c:1646  */
    break;


#line 2537 "SqlParser.cpp" /* yacc.c:1646  */
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
#line 1450 "SqlParser.y" /* yacc.c:1906  */


const char* tname(int offset) { return yytname[offset]; }

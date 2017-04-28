/* This file is part of the KDE project
   Copyright (C) 2003-2016 Jarosław Staniek <staniek@kde.org>

   Based on nexp.cpp : Parser module of Python-like language
   (C) 2001 Jarosław Staniek, MIMUW (www.mimuw.edu.pl)

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

#include "KDbExpression.h"
#include "KDb.h"
#include "KDbQuerySchema.h"
#include "KDbDriver.h"
#include "KDbParser.h"
#include "KDbParser_p.h"
#include "kdb_debug.h"

#include <QSet>

#include <vector>
#include <algorithm>

// Enable to add SQLite-specific functions
//#define KDB_ENABLE_SQLITE_SPECIFIC_FUNCTIONS

//! A set of names of aggregation SQL functions
class BuiltInAggregates : public QSet<QString>
{
public:
    BuiltInAggregates()
    {
        insert(QLatin1String("SUM"));
        insert(QLatin1String("MIN"));
        insert(QLatin1String("MAX"));
        insert(QLatin1String("AVG"));
        insert(QLatin1String("COUNT"));
        insert(QLatin1String("STD"));
        insert(QLatin1String("STDDEV"));
        insert(QLatin1String("VARIANCE"));
    }
};

Q_GLOBAL_STATIC(BuiltInAggregates, _builtInAggregates)

//! Type of a single function argument, used with KDbField::Type values.
//! Used to indicate that multiple types are allowed.
enum BuiltInFunctionArgumentType
{
    AnyText = KDbField::LastType + 1,
    AnyInt,
    AnyFloat,
    AnyNumber,
    Any
};

//! @return any concrete type matching rule @a argType
static KDbField::Type anyMatchingType(int argType)
{
    if (argType == AnyText || argType == Any) {
        return KDbField::Text;
    }
    else if (argType == AnyInt || argType == AnyNumber) {
        return KDbField::Integer;
    }
    else if (argType == AnyFloat) {
        return KDbField::Double;
    }
    return KDbField::InvalidType;
}

//! Declaration of a single built-in function. It can offer multiple signatures.
class BuiltInFunctionDeclaration
{
public:
    inline BuiltInFunctionDeclaration()
        : defaultReturnType(KDbField::InvalidType), copyReturnTypeFromArg(-1)
    {
    }
    virtual ~BuiltInFunctionDeclaration() {}
    virtual KDbField::Type returnType(const KDbFunctionExpressionData* f, KDbParseInfo* parseInfo) const {
        Q_UNUSED(parseInfo);
        const KDbNArgExpressionData *argsData = f->args.constData()->convertConst<KDbNArgExpressionData>();
        if (argsData->containsNullArgument()) {
            return KDbField::Null;
        }
        if (copyReturnTypeFromArg >= 0 && copyReturnTypeFromArg < argsData->children.count()) {
            KDbQueryParameterExpressionData *queryParameterExpressionData = argsData->children.at(copyReturnTypeFromArg)->convert<KDbQueryParameterExpressionData>();
            if (queryParameterExpressionData) {
                // Set query parameter type (if there are any) to deduced result type
                //! @todo Most likely but can be also other type
                for (size_t i = 0; i < signatures.size(); ++i) {
                    int** signature = signatures[i];
                    const KDbField::Type t = anyMatchingType(signature[copyReturnTypeFromArg][0]);
                    if (t != KDbField::InvalidType) {
                        queryParameterExpressionData->m_type = t;
                        return t;
                    }
                }
            }
            return argsData->children.at(copyReturnTypeFromArg)->type();
        }
        return defaultReturnType;
    }
    std::vector<int**> signatures;
protected:
    KDbField::Type defaultReturnType;
    int copyReturnTypeFromArg;
    friend class BuiltInFunctions;
private:
    Q_DISABLE_COPY(BuiltInFunctionDeclaration)
};

//! Declaration of a single built-in function COALESCE() and similar ones.
class CoalesceFunctionDeclaration : public BuiltInFunctionDeclaration
{
public:
    CoalesceFunctionDeclaration() {}
    KDbField::Type returnType(const KDbFunctionExpressionData* f, KDbParseInfo* parseInfo) const override {
        Q_UNUSED(parseInfo);
        // Find type
        //! @todo Most likely but can be also other type
        KDbField::Type t = KDbField::Integer;
        const KDbNArgExpressionData *argsData = f->args.constData()->convertConst<KDbNArgExpressionData>();
        foreach(const ExplicitlySharedExpressionDataPointer &expr, argsData->children) {
            KDbQueryParameterExpressionData *queryParameterExpressionData = expr->convert<KDbQueryParameterExpressionData>();
            const KDbField::Type currentType = expr->type();
            if (!queryParameterExpressionData && currentType != KDbField::Null) {
                t = currentType;
                break;
            }
        }
        foreach(const ExplicitlySharedExpressionDataPointer &expr, argsData->children) {
            KDbQueryParameterExpressionData *queryParameterExpressionData = expr->convert<KDbQueryParameterExpressionData>();
            if (queryParameterExpressionData) {
                // Set query parameter type (if there are any) to deduced result type
                queryParameterExpressionData->m_type = t;
            }
        }
        return t;
    }
private:
    Q_DISABLE_COPY(CoalesceFunctionDeclaration)
};

//! Declaration of a single built-in function MIN(), MAX() and similar ones.
//! Its return type is:
//! - NULL if any argument is NULL
//! - valid type if types of all arguments are compatible (e.g. text, numeric, date...)
//! - InvalidType if types of any two are incompatible
class MinMaxFunctionDeclaration : public BuiltInFunctionDeclaration
{
    Q_DECLARE_TR_FUNCTIONS(MinMaxFunctionDeclaration)
public:
    MinMaxFunctionDeclaration() {}
    KDbField::Type returnType(const KDbFunctionExpressionData* f, KDbParseInfo* parseInfo) const override {
        const KDbNArgExpressionData *argsData = f->args.constData()->convertConst<KDbNArgExpressionData>();
        if (argsData->children.isEmpty()) {
            return KDbField::Null;
        }
        const KDbField::Type type0 = argsData->children.at(0)->type(); // cache: evaluating type of expressions can be expensive
        if (nullOrInvalid(type0)) {
            return type0;
        }
        KDbField::TypeGroup prevTg = KDbField::typeGroup(type0); // use typegroup for simplicity
        bool prevTgIsAny = argsData->children.at(0)->convertConst<KDbQueryParameterExpressionData>();
        for(int i = 1; i < argsData->children.count(); ++i) {
            const ExplicitlySharedExpressionDataPointer expr = argsData->children.at(i);
            const KDbField::Type t = expr->type();
            if (nullOrInvalid(t)) {
                return t;
            }
            const KDbField::TypeGroup tg = KDbField::typeGroup(t);
            const bool tgIsAny = argsData->children.at(i)->convertConst<KDbQueryParameterExpressionData>();
            if (prevTgIsAny) {
                if (!tgIsAny) { // no longer "Any" (query parameter)
                    prevTgIsAny = false;
                    prevTg = tg;
                }
                continue;
            } else if (tgIsAny) {
                continue; // use previously found concrete type
            }
            if ((prevTg == KDbField::IntegerGroup || prevTg == KDbField::FloatGroup)
                && (tg == KDbField::IntegerGroup || tg == KDbField::FloatGroup))
            {
                if (prevTg == KDbField::IntegerGroup && tg == KDbField::FloatGroup) {
                    prevTg = KDbField::FloatGroup; // int -> float
                }
                continue;
            }
            if (prevTg == tg) {
                continue;
            }
            if (parseInfo) {
                parseInfo->setErrorMessage(
                    tr("Incompatible types in %1() function").arg(f->name));
                parseInfo->setErrorDescription(
                    tr("Argument #%1 of type \"%2\" in function %3() is not "
                       "compatible with previous arguments of type \"%4\".")
                            .arg(i+1)
                            .arg(KDbField::typeName(simpleTypeForGroup(tg)),
                                 f->name,
                                 KDbField::typeName(simpleTypeForGroup(prevTg))));
            }
            return KDbField::InvalidType;
        }
        if (prevTgIsAny) {
            //! @todo Most likely Integer but can be also Float/Double/Text/Date...
            return KDbField::Integer;
        }
        const KDbField::Type resultType = safeTypeForGroup(prevTg);
        // Set query parameter types (if there are any) to deduced result type
        for(ExplicitlySharedExpressionDataPointer expr : argsData->children) {
            KDbQueryParameterExpressionData *queryParameterExpressionData = expr->convert<KDbQueryParameterExpressionData>();
            if (queryParameterExpressionData) {
                queryParameterExpressionData->m_type = resultType;
            }
        }
        return resultType;
    }
private:
    static bool nullOrInvalid(KDbField::Type type) {
        return type == KDbField::Null || type == KDbField::InvalidType;
    }
    //! @return safe default type for type group @a tg (too big sizes better than too small)
    static KDbField::Type safeTypeForGroup(KDbField::TypeGroup tg) {
        switch (tg) {
        case KDbField::TextGroup: return KDbField::LongText;
        case KDbField::IntegerGroup: return KDbField::BigInteger;
        case KDbField::FloatGroup: return KDbField::Double;
        case KDbField::BooleanGroup: return KDbField::Boolean;
        case KDbField::DateTimeGroup: return KDbField::DateTime;
        case KDbField::BLOBGroup: return KDbField::BLOB;
        default: break;
        }
        return KDbField::InvalidType;
    }
    //! @return resonable default type for type group @a tg (used for displaying in error message)
    static KDbField::Type simpleTypeForGroup(KDbField::TypeGroup tg) {
        switch (tg) {
        case KDbField::TextGroup: return KDbField::Text;
        case KDbField::IntegerGroup: return KDbField::Integer;
        case KDbField::FloatGroup: return KDbField::Double;
        case KDbField::BooleanGroup: return KDbField::Boolean;
        case KDbField::DateTimeGroup: return KDbField::DateTime;
        case KDbField::BLOBGroup: return KDbField::BLOB;
        default: break;
        }
        return KDbField::InvalidType;
    }
    Q_DISABLE_COPY(MinMaxFunctionDeclaration)
};

//! Declaration of a single built-in function RANDOM() and RANDOM(X,Y).
//! Its return type is:
//! - Double when number of arguments is zero
//! - integer if there are two integer arguments (see KDb::maximumForIntegerFieldTypes())
//! - InvalidType for other number of arguments
class RandomFunctionDeclaration : public BuiltInFunctionDeclaration
{
    Q_DECLARE_TR_FUNCTIONS(RandomFunctionDeclaration)
public:
    RandomFunctionDeclaration() {}
    KDbField::Type returnType(const KDbFunctionExpressionData* f, KDbParseInfo* parseInfo) const override {
        Q_UNUSED(parseInfo);
        const KDbNArgExpressionData *argsData = f->args.constData()->convertConst<KDbNArgExpressionData>();
        if (argsData->children.isEmpty()) {
            return KDbField::Double;
        }
        if (argsData->children.count() == 2) {
            const KDbConstExpressionData *const0 = argsData->children.at(0)->convertConst<KDbConstExpressionData>();
            const KDbConstExpressionData *const1 = argsData->children.at(1)->convertConst<KDbConstExpressionData>();
            if (const0 && const1) {
                bool ok0;
                const qlonglong val0 = const0->value.toLongLong(&ok0);
                bool ok1;
                const qlonglong val1 = const1->value.toLongLong(&ok1);
                if (ok0 && ok1) {
                    if (val0 >= val1) {
                        if (parseInfo) {
                            parseInfo->setErrorMessage(
                                tr("Invalid arguments of %1() function").arg(f->name));
                            parseInfo->setErrorDescription(
                                tr("Value of the first argument should be less than "
                                   "value of the second argument."));
                        }
                        return KDbField::InvalidType;
                    }
                }
            }
            KDbField::Type t0;
            KDbField::Type t1;
            // deduce query parameter types
            KDbQueryParameterExpressionData *queryParameterExpressionData0 = argsData->children.at(0)->convert<KDbQueryParameterExpressionData>();
            KDbQueryParameterExpressionData *queryParameterExpressionData1 = argsData->children.at(1)->convert<KDbQueryParameterExpressionData>();
            if (queryParameterExpressionData0 && queryParameterExpressionData1) {
                queryParameterExpressionData0->m_type = KDbField::Integer;
                queryParameterExpressionData1->m_type = KDbField::Integer;
                t0 = KDbField::Integer;
                t1 = KDbField::Integer;
            } else if (queryParameterExpressionData0 && !queryParameterExpressionData1) {
                queryParameterExpressionData0->m_type = KDbField::Integer;
                t0 = queryParameterExpressionData0->m_type;
                t1 = argsData->children.at(1)->type();
            } else if (!queryParameterExpressionData0 && queryParameterExpressionData1) {
                queryParameterExpressionData1->m_type = KDbField::Integer;
                t0 = argsData->children.at(0)->type();
                t1 = queryParameterExpressionData1->m_type;
            } else {
                t0 = argsData->children.at(0)->type();
                t1 = argsData->children.at(1)->type();
            }
            return KDb::maximumForIntegerFieldTypes(t0, t1);
        }
        return KDbField::InvalidType;
    }
private:
    Q_DISABLE_COPY(RandomFunctionDeclaration)
};

//! Declaration of a single built-in function CEILING(X) and FLOOR(X).
//! Its return type is:
//! - integer if there are two integer arguments (see KDb::maximumForIntegerFieldTypes())
//! - InvalidType for other number of arguments
class CeilingFloorFunctionDeclaration : public BuiltInFunctionDeclaration
{
public:
    CeilingFloorFunctionDeclaration() {}
    KDbField::Type returnType(const KDbFunctionExpressionData* f, KDbParseInfo* parseInfo) const override {
        Q_UNUSED(parseInfo);
        const KDbNArgExpressionData *argsData = f->args.constData()->convertConst<KDbNArgExpressionData>();
        if (argsData->children.count() == 1) {
            KDbQueryParameterExpressionData *queryParameterExpressionData = argsData->children.at(0)->convert<KDbQueryParameterExpressionData>();
            if (queryParameterExpressionData) {
                // Set query parameter type (if there are any) to deduced result type
                //! @todo Most likely but can be also other type
                queryParameterExpressionData->m_type = KDbField::Double;
                return KDbField::BigInteger;
            }
            const KDbField::Type type = argsData->children.at(0)->type(); // cache: evaluating type of expressions can be expensive
            if (KDbField::isFPNumericType(type)) {
                return KDbField::BigInteger;
            }
            switch (type) {
            case KDbField::Byte: return KDbField::ShortInteger;
            case KDbField::ShortInteger: return KDbField::Integer;
            case KDbField::Integer: return KDbField::BigInteger;
            case KDbField::Null: return KDbField::Null;
            case KDbField::InvalidType: return KDbField::InvalidType;
            default:;
            }
        }
        return KDbField::InvalidType;
    }
private:
    Q_DISABLE_COPY(CeilingFloorFunctionDeclaration)
};

//! A map of built-in SQL functions
//! See https://community.kde.org/Kexi/Plugins/Queries/SQL_Functions for the status.
class BuiltInFunctions : public QHash<QString, BuiltInFunctionDeclaration*>
{
public:
    BuiltInFunctions();
    ~BuiltInFunctions() {
        qDeleteAll(*this);
    }

    //! @return function declaration's structure for name @a name
    //! If @a name is alias of the function, e.g. "MIN" for "LEAST", the original
    //! function's declaration is returned.
    BuiltInFunctionDeclaration* value(const QString &name) const;

    //! @return a list of function aliases.
    QStringList aliases() const;

    static int multipleArgs[];
private:
    QHash<QString, BuiltInFunctionDeclaration*> m_aliases;
    Q_DISABLE_COPY(BuiltInFunctions)
};

int BuiltInFunctions::multipleArgs[] = { 0 };

BuiltInFunctions::BuiltInFunctions()
    : QHash<QString, BuiltInFunctionDeclaration*>()
{
    BuiltInFunctionDeclaration *decl;
#define _TYPES(name, ...) static int name[] = { __VA_ARGS__, KDbField::InvalidType }
    _TYPES(argAnyTextOrNull, AnyText, KDbField::Null);
    _TYPES(argAnyIntOrNull, AnyInt, KDbField::Null);
    _TYPES(argAnyNumberOrNull, AnyNumber, KDbField::Null);
    _TYPES(argAnyFloatOrNull, AnyFloat, KDbField::Null);
    Q_UNUSED(argAnyFloatOrNull);
    _TYPES(argAnyOrNull, Any, KDbField::Null);
    _TYPES(argBLOBOrNull, KDbField::BLOB, KDbField::Null);
    Q_UNUSED(argBLOBOrNull);
    _TYPES(argAnyTextBLOBOrNull, AnyText, KDbField::BLOB, KDbField::Null);
#undef _TYPES

//! Adds a signature named @a name with specified arguments to declaration decl
#define _SIG(name, ...) \
    static int* name[] = { __VA_ARGS__, 0 }; \
    decl->signatures.push_back(name)

//! Adds a signature with no arguments to declaration decl
#define _SIG0 \
    decl->signatures.push_back(sig0)

    static int* sig0[] = { nullptr };

    insert(QLatin1String("ABS"), decl = new BuiltInFunctionDeclaration);
    // From https://www.sqlite.org/lang_corefunc.html
    /* The abs(X) function returns the absolute value of the numeric argument X.
     Abs(X) returns NULL if X is NULL. Abs(X) returns 0.0 if X is a string or blob that
     cannot be converted to a numeric value. If X is the integer -9223372036854775808
     then abs(X) throws an integer overflow error since there is no equivalent positive
     64-bit two complement value. */
    // example: SELECT ABS(-27), ABS(-3.1415), ABS(NULL + 1)
    // result: 27, 3.1415, NULL
    decl->copyReturnTypeFromArg = 0;
    _SIG(abs_1, argAnyNumberOrNull);

    insert(QLatin1String("CEILING"), decl = new CeilingFloorFunctionDeclaration);
    /* ceiling(X) returns the largest integer value not less than X. */
    // See also https://dev.mysql.com/doc/refman/5.1/en/mathematical-functions.html#function_ceiling
    // See also http://www.postgresql.org/docs/9.5/static/functions-math.html#FUNCTIONS-MATH-FUNC-TABLE
    // SQLite has no equivalent of ceiling() so this is used:
    // (CASE WHEN X = CAST(X AS INT) THEN CAST(X AS INT) WHEN X >= 0 THEN CAST(X AS INT) + 1 ELSE CAST(X AS INT) END)
    //! @todo add a custom function to SQLite to optimize/simplify things
    // example: SELECT CEILING(3.14), CEILING(-99.001)
    // result: 4, -99
    _SIG(ceiling, argAnyNumberOrNull);

    insert(QLatin1String("CHAR"), decl = new BuiltInFunctionDeclaration);
    // From https://www.sqlite.org/lang_corefunc.html
    /* The char(X1,X2,...,XN) function returns a string composed of characters having
     the unicode code point values of integers X1 through XN, respectively. */
    // example: SELECT CHAR(75,69,88,73), CHAR()
    // result: "KEXI" ""
    decl->defaultReturnType = KDbField::LongText;
    static int char_min_args[] = { 0 };
    _SIG(char_N, argAnyIntOrNull, multipleArgs, char_min_args);

    insert(QLatin1String("COALESCE"), decl = new CoalesceFunctionDeclaration);
    // From https://www.sqlite.org/lang_corefunc.html
    /* The coalesce() function returns a copy of its first non-NULL argument, or NULL if
     all arguments are NULL. Coalesce() must have at least 2 arguments. */
    // example: SELECT COALESCE(NULL, 17, NULL, "A")
    // result: 17
    static int coalesce_min_args[] = { 2 };
    _SIG(coalesce_N, argAnyOrNull, multipleArgs, coalesce_min_args);

    insert(QLatin1String("FLOOR"), decl = new CeilingFloorFunctionDeclaration);
    /* floor(X) returns the largest integer value not greater than X. */
    // See also https://dev.mysql.com/doc/refman/5.1/en/mathematical-functions.html#function_floor
    // See also http://www.postgresql.org/docs/9.5/static/functions-math.html#FUNCTIONS-MATH-FUNC-TABLE
    // SQLite has no equivalent of floor() so this is used:
    // (CASE WHEN X >= 0 OR X = CAST(X AS INT) THEN CAST(X AS INT) ELSE CAST(X AS INT) - 1 END)
    //! @todo add a custom function to SQLite to optimize/simplify things
    // example: SELECT FLOOR(3.14), FLOOR(-99.001)
    // result: 3, -100
    _SIG(floor, argAnyNumberOrNull);

    insert(QLatin1String("GREATEST"), decl = new MinMaxFunctionDeclaration);
    m_aliases.insert(QLatin1String("MAX"), decl);
    // From https://www.sqlite.org/lang_corefunc.html
    // For SQLite MAX() is used.
    // If arguments are of text type, to each argument default (unicode) collation
    // is assigned that is configured for SQLite by KDb.
    // Example: SELECT MAX('ą' COLLATE '', 'z' COLLATE '').
    // Example: SELECT MAX('ą' COLLATE '', 'z' COLLATE '').
    /* The multi-argument max() function returns the argument with the maximum value, or
    return NULL if any argument is NULL. The multi-argument max() function searches its
    arguments from left to right for an argument that defines a collating function and
    uses that collating function for all string comparisons. If none of the arguments to
    max() define a collating function, then the BINARY collating function is used. Note
    that max() is a simple function when it has 2 or more arguments but operates as an
    aggregate function if given only a single argument. */
    // For pgsql GREATEST() function ignores NULL values, it only returns NULL
    // if all the expressions evaluate to NULL. So this is used for MAX(v0,..,vN):
    // (CASE WHEN (v0) IS NULL OR .. OR (vN) IS NULL THEN NULL ELSE GREATEST(v0,..,vN) END)
    // See also http://www.postgresql.org/docs/9.5/static/functions-conditional.html#FUNCTIONS-GREATEST-LEAST
    //! @todo for pgsql CREATE FUNCTION can be used to speed up and simplify things
    // For mysql GREATEST() is used.
    // See https://dev.mysql.com/doc/refman/5.1/en/comparison-operators.html#function_greatest
    // Note: Before MySQL 5.0.13, GREATEST() returns NULL only if all arguments are NULL
    // (like pgsql). As of 5.0.13, it returns NULL if any argument is NULL (like sqlite's MAX()).
    // See also https://bugs.mysql.com/bug.php?id=15610
    //! @todo MySQL: check for server version and don't use the pgsql's approach for ver >= 5.0.13
    //!       We cannot do that now because we only have access to driver, not the connection.
    // example: SELECT GREATEST("Z", "ą", "AA"), MAX(0.1, 7.1, 7), GREATEST(9, NULL, -1)
    // result: "Z", 7.1, NULL
    static int greatest_min_args[] = { 2 };
    _SIG(greatest_N, argAnyOrNull, multipleArgs, greatest_min_args);

    insert(QLatin1String("HEX"), decl = new BuiltInFunctionDeclaration);
    // From https://www.sqlite.org/lang_corefunc.html
    // See also https://dev.mysql.com/doc/refman/5.1/en/string-functions.html#function_hex
    /* The hex() function interprets its argument as a BLOB and returns a string which is
    the upper-case hexadecimal rendering of the content of that blob. */
    /* For pgsql UPPER(ENCODE(val, 'hex')) is used,
       See http://www.postgresql.org/docs/9.5/static/functions-string.html#FUNCTIONS-STRING-OTHER */
    // example: SELECT HEX(X'BEEF'), HEX('DEAD')
    // result: "BEEF", "44454144"
    //! @todo HEX(int) for SQLite is not the same as HEX(int) for MySQL so we disable it
    //!       -- maybe can be wrapped?
    decl->defaultReturnType = KDbField::LongText;
    _SIG(hex_1, argAnyTextBLOBOrNull);

    insert(QLatin1String("IFNULL"), decl = new CoalesceFunctionDeclaration);
    // From https://www.sqlite.org/lang_corefunc.html
    /* The ifnull() function returns a copy of its first non-NULL argument, or NULL if
     both arguments are NULL. Ifnull() must have exactly 2 arguments. The ifnull() function
     is equivalent to coalesce() with two arguments. */
    // For postgresql coalesce() is used.
    // example: SELECT IFNULL(NULL, 17), IFNULL(NULL, NULL)
    // result: 17, NULL
    _SIG(ifnull_2, argAnyOrNull, argAnyOrNull);

    insert(QLatin1String("INSTR"), decl = new BuiltInFunctionDeclaration);
    // From https://www.sqlite.org/lang_corefunc.html
    /* The instr(X,Y) function finds the first occurrence of string Y within string X and
     returns the number of prior characters plus 1, or 0 if Y is nowhere found within X.
     If both arguments X and Y to instr(X,Y) are non-NULL and are not BLOBs then both are
     interpreted as strings. If either X or Y are NULL in instr(X,Y) then the result
     is NULL. */
    //! @todo PostgreSQL does not have instr() but CREATE FUNCTION can be used,
    //!       see http://www.postgresql.org/docs/9.5/static/plpgsql-porting.html
    //! @todo support (BLOB, BLOB)?
    /* From the same docs:
     Or, if X and Y are both BLOBs, then instr(X,Y) returns one more than the number bytes
     prior to the first occurrence of Y, or 0 if Y does not occur anywhere within X. */
    // example: SELECT INSTR("KEXI", "X"), INSTR("KEXI", "ZZ")
    // result: 3, 0
    decl->defaultReturnType = KDbField::Integer;
    _SIG(instr_2, argAnyTextOrNull, argAnyTextOrNull);

    insert(QLatin1String("LEAST"), decl = new MinMaxFunctionDeclaration);
    m_aliases.insert(QLatin1String("MIN"), decl);
    // From https://www.sqlite.org/lang_corefunc.html
    // For SQLite uses MIN().
    /* The multi-argument min() function returns the argument with the minimum value, or
    return NULL if any argument is NULL. The multi-argument min() function searches its
    arguments from left to right for an argument that defines a collating function and
    uses that collating function for all string comparisons. If none of the arguments to
    max() define a collating function, then the BINARY collating function is used. Note
    that max() is a simple function when it has 2 or more arguments but operates as an
    aggregate function if given only a single argument. */
    // For pgsql LEAST() function ignores NULL values, it only returns NULL
    // if all the expressions evaluate to NULL. So this is used for MAX(v0,..,vN):
    // (CASE WHEN (v0) IS NULL OR .. OR (vN) IS NULL THEN NULL ELSE LEAST(v0,..,vN) END)
    // See also http://www.postgresql.org/docs/9.5/static/functions-conditional.html#FUNCTIONS-GREATEST-LEAST
    //! @todo for pgsql CREATE FUNCTION can be used to speed up and simplify things
    // For mysql LEAST() is used.
    // See https://dev.mysql.com/doc/refman/5.1/en/comparison-operators.html#function_least
    // Note: Before MySQL 5.0.13, LEAST() returns NULL only if all arguments are NULL
    // (like pgsql). As of 5.0.13, it returns NULL if any argument is NULL (like sqlite's MIN()).
    //! @todo MySQL: check for server version and don't use the pgsql's approach for ver >= 5.0.13
    //!       We cannot do that now because we only have access to driver, not the connection.
    // See also https://bugs.mysql.com/bug.php?id=15610
    // example: SELECT LEAST("Z", "ą", "AA"), MIN(0.1, 7.1, 7), LEAST(9, NULL, -1)
    // result: "ą", 0.1, NULL
    static int least_min_args[] = { 2 };
    _SIG(least_N, argAnyOrNull, multipleArgs, least_min_args);

    insert(QLatin1String("LENGTH"), decl = new BuiltInFunctionDeclaration);
    // From https://www.sqlite.org/lang_corefunc.html
    // See also https://dev.mysql.com/doc/refman/5.1/en/string-functions.html#function_length
    /* For a string value X, the length(X) function returns the number of characters (not
    bytes) in X prior to the first NUL character. Since SQLite strings do not normally
    contain NUL characters, the length(X) function will usually return the total number
    of characters in the string X. For a blob value X, length(X) returns the number of
    bytes in the blob. If X is NULL then length(X) is NULL. If X is numeric then
    length(X) returns the length of a string representation of X. */
    /* For postgres octet_length(val) is used if val is a of BLOB type.
       length(val) for BLOB cannot be used because it returns number of bits. */
    /* For mysql char_length(val) is used.
       This is because length(val) in mysql returns number of bytes, what is not right for
       multibyte (unicode) encodings. */
    // example: SELECT LENGTH('Straße'), LENGTH(X'12FE')
    // result: 6, 2
    decl->defaultReturnType = KDbField::Integer;
    _SIG(length_1, argAnyTextBLOBOrNull);

    insert(QLatin1String("LOWER"), decl = new BuiltInFunctionDeclaration);
    // From https://www.sqlite.org/lang_corefunc.html
    /* The lower(X) function returns a copy of string X with all characters converted
     to lower case. */
    // Note: SQLite such as 3.8 without ICU extension does not convert non-latin1 characters
    // too well; Kexi uses ICU extension by default so the results are very good.
    // See also https://dev.mysql.com/doc/refman/5.1/en/string-functions.html#function_lower
    // See also http://www.postgresql.org/docs/9.5/static/functions-string.html#FUNCTIONS-STRING-SQL
    // example: SELECT LOWER("MEGSZENTSÉGTELENÍTHETETLENSÉGESKEDÉSEITEKÉRT")
    // result: "megszentségteleníthetetlenségeskedéseitekért"
    decl->defaultReturnType = KDbField::LongText;
    _SIG(lower_1, argAnyTextOrNull);

    insert(QLatin1String("LTRIM"), decl = new BuiltInFunctionDeclaration);
    // From https://www.sqlite.org/lang_corefunc.html
    /* The ltrim(X,Y) function returns a string formed by removing any and all characters
     that appear in Y from the left side of X. If the Y argument is omitted, ltrim(X)
     removes spaces from the left side of X.*/
    // See also https://dev.mysql.com/doc/refman/5.1/en/string-functions.html#function_ltrim
    //! @todo MySQL's LTRIM only supports one arg. TRIM() does not work too
    //! https://dev.mysql.com/doc/refman/5.1/en/string-functions.html#function_trim
    // See also http://www.postgresql.org/docs/9.5/static/functions-string.html#FUNCTIONS-STRING-SQL
    // example: SELECT LTRIM("  John Smith")
    // result: "John Smith"
    // example: SELECT LTRIM("a b or c", "ab ")
    // result: "or c"
    decl->defaultReturnType = KDbField::LongText;
    _SIG(ltrim_1, argAnyTextOrNull);
    _SIG(ltrim_2, argAnyTextOrNull, argAnyTextOrNull);

    insert(QLatin1String("NULLIF"), decl = new BuiltInFunctionDeclaration);
    // From https://www.sqlite.org/lang_corefunc.html
    /* The nullif(X,Y) function returns its first argument if the arguments are different
     and NULL if the arguments are the same. The nullif(X,Y) function searches its
     arguments from left to right for an argument that defines a collating function and
     uses that collating function for all string comparisons. If neither argument to
     nullif() defines a collating function then the BINARY is used. */
    // See also https://dev.mysql.com/doc/refman/5.1/en/control-flow-functions.html#function_nullif
    // See also http://www.postgresql.org/docs/9.5/static/functions-conditional.html#FUNCTIONS-NULLIF
    // example: SELECT NULLIF("John", "Smith"), NULLIF(177, 177)
    // result: "John", NULL
    decl->copyReturnTypeFromArg = 0;
    _SIG(nullif_2, argAnyOrNull, argAnyOrNull);

    insert(QLatin1String("RANDOM"), decl = new RandomFunctionDeclaration);
    /* RANDOM() returns a random floating-point value v in the range 0 <= v < 1.0.
     RANDOM(X,Y) - returns returns a random integer that is equal or greater than X
     and less than Y. */
    // For MySQL RANDOM() is equal to RAND().
    // For MySQL RANDOM(X,Y) is equal to (X + FLOOR(RAND() * (Y - X))
    // For PostreSQL RANDOM() is equal to RANDOM().
    // For PostreSQL RANDOM(X,Y) is equal to (X + FLOOR(RANDOM() * (Y - X))
    // Because SQLite returns integer between -9223372036854775808 and +9223372036854775807,
    // so RANDOM() for SQLite is equal to (RANDOM()+9223372036854775807)/18446744073709551615.
    // Similarly, RANDOM(X,Y) for SQLite is equal
    // to (X + CAST((Y - X) * (RANDOM()+9223372036854775807)/18446744073709551615 AS INT)).
    // See also https://dev.mysql.com/doc/refman/5.1/en/mathematical-functions.html#function_rand
    // See also http://www.postgresql.org/docs/9.5/static/functions-math.html#FUNCTIONS-MATH-RANDOM-TABLE
    //! @note rand(X) (where X is a seed value to set) isn't portable between MySQL and PostgreSQL,
    //! and does not exist in SQLite, so we don't support it.
    // example: SELECT RANDOM(), RANDOM(2, 5)
    // result: (some random floating-point value v where 0 <= v < 1.0)
    // example: SELECT RANDOM(2, 5)
    // result: (some random integer value v where 2 <= v < 5)
    decl->defaultReturnType = KDbField::Double;
    _SIG0;
    _SIG(random_2, argAnyIntOrNull, argAnyIntOrNull);

    insert(QLatin1String("ROUND"), decl = new BuiltInFunctionDeclaration);
    // From https://www.sqlite.org/lang_corefunc.html
    /* The round(X,Y) function returns a floating-point value X rounded to Y digits to the
     right of the decimal point. If the Y argument is omitted, it is assumed to be 0. */
    // See also https://dev.mysql.com/doc/refman/5.1/en/mathematical-functions.html#function_round
    // See also http://www.postgresql.org/docs/9.5/static/functions-math.html#FUNCTIONS-MATH-FUNC-TABLE
    //! @note round(X,Y) where Y < 0 is supported only by MySQL so we ignore this case
    // example: SELECT ROUND(-1.13), ROUND(-5.51), ROUND(5.51), ROUND(1.298, 1), ROUND(1.298, 0), ROUND(7)
    // result: -1, -6, 6, 1.3, 1, 7
    decl->copyReturnTypeFromArg = 0;
    _SIG(round_1, argAnyNumberOrNull);
    _SIG(round_2, argAnyNumberOrNull, argAnyIntOrNull);

    insert(QLatin1String("RTRIM"), decl = new BuiltInFunctionDeclaration);
    // From https://www.sqlite.org/lang_corefunc.html
    /* The rtrim(X,Y) function returns a string formed by removing any and all characters
     that appear in Y from the right side of X. If the Y argument is omitted, rtrim(X)
     removes spaces from the right side of X. */
    // See also https://dev.mysql.com/doc/refman/5.1/en/string-functions.html#function_ltrim
    //! @todo MySQL's RTRIM only supports one arg. TRIM() does not work too
    //! https://dev.mysql.com/doc/refman/5.1/en/string-functions.html#function_trim
    // See also http://www.postgresql.org/docs/9.5/static/functions-string.html#FUNCTIONS-STRING-SQL
    // example: SELECT RTRIM("John Smith   ")
    // result: "John Smith"
    // example: SELECT RTRIM("a b or c", "orc ")
    // result: "a b"
    decl->defaultReturnType = KDbField::LongText;
    _SIG(rtrim_1, argAnyTextOrNull);
    _SIG(rtrim_2, argAnyTextOrNull, argAnyTextOrNull);

    insert(QLatin1String("SOUNDEX"), decl = new BuiltInFunctionDeclaration);
    // From https://www.sqlite.org/lang_corefunc.html
    /* The soundex(X) function returns a string that is the soundex encoding of the string
     X. The string "?000" is returned if the argument is NULL or contains non-ASCII
     alphabetic characters. */
    // See also https://dev.mysql.com/doc/refman/5.1/en/string-functions.html#function_soundex
    // See also http://www.postgresql.org/docs/9.5/static/fuzzystrmatch.html#AEN165853
    //! @todo we call drv_executeVoidSQL("CREATE EXTENSION IF NOT EXISTS fuzzystrmatch") on connection,
    //!       do that on first use of SOUNDEX()
    // example: SELECT SOUNDEX("John")
    // result: "J500"
    decl->defaultReturnType = KDbField::Text;
    _SIG(soundex, argAnyTextOrNull);

    insert(QLatin1String("SUBSTR"), decl = new BuiltInFunctionDeclaration);
    // From https://www.sqlite.org/lang_corefunc.html
    /* The substr(X,Y) returns all characters through the end of the string X beginning with
    the Y-th. The left-most character of X is number 1. If Y is negative then the
    first character of the substring is found by counting from the right rather than
    the left. If Z is negative then the abs(Z) characters preceding the Y-th
    character are returned. If X is a string then characters indices refer to actual
    UTF-8 characters. If X is a BLOB then the indices refer to bytes. */
    _SIG(substr_2, argAnyTextOrNull, argAnyIntOrNull);
    /* The substr(X,Y,Z) function returns a substring of input string X that begins
    with the Y-th character and which is Z characters long. */
    _SIG(substr_3, argAnyTextOrNull, argAnyIntOrNull, argAnyIntOrNull);
    decl->copyReturnTypeFromArg = 0;

     insert(QLatin1String("TRIM"), decl = new BuiltInFunctionDeclaration);
     // From https://www.sqlite.org/lang_corefunc.html
     /* The trim(X,Y) function returns a string formed by removing any and all characters
      that appear in Y from both ends of X. If the Y argument is omitted, trim(X) removes
      spaces from both ends of X. */
     // See also https://dev.mysql.com/doc/refman/5.1/en/string-functions.html#function_trim
     //! @todo MySQL's TRIM only supports one arg. TRIM() does not work too
     //! https://dev.mysql.com/doc/refman/5.1/en/string-functions.html#function_trim
     // See also http://www.postgresql.org/docs/9.5/static/functions-string.html#FUNCTIONS-STRING-SQL
     // example: SELECT TRIM("  John Smith   ")
     // result: "John Smith"
     // example: SELECT TRIM("a b or c", "orca ")
     // result: "b"
     decl->defaultReturnType = KDbField::LongText;
     _SIG(trim_1, argAnyTextOrNull);
     _SIG(trim_2, argAnyTextOrNull, argAnyTextOrNull);

     insert(QLatin1String("UNICODE"), decl = new BuiltInFunctionDeclaration);
     // From https://www.sqlite.org/lang_corefunc.html
     /* The unicode(X) function returns the numeric unicode code point corresponding to
      the first character of the string X. If the argument to unicode(X) is not a string
      then the result is undefined. */
     // For MySQL ORD(CONVERT(X USING UTF16)) is used (ORD(X) returns a UTF-16 number)
     // For PostreSQL ASCII(X) is used.
     // example: SELECT UNICODE('A'), UNICODE('ą'), UNICODE('Δ'), UNICODE('葉')
     // result: 65, 261, 916, 33865
     decl->defaultReturnType = KDbField::Integer;
     _SIG(unicode_1, argAnyTextOrNull);

     insert(QLatin1String("UPPER"), decl = new BuiltInFunctionDeclaration);
     // From https://www.sqlite.org/lang_corefunc.html
     /* The upper(X) function returns a copy of string X with all characters converted
      to upper case. */
     // Note: SQLite such as 3.8 without ICU extension does not convert non-latin1 characters
     // too well; Kexi uses ICU extension by default so the results are very good.
     // See also https://dev.mysql.com/doc/refman/5.1/en/string-functions.html#function_upper
     // See also http://www.postgresql.org/docs/9.5/static/functions-string.html#FUNCTIONS-STRING-SQL
     // example: SELECT UPPER("megszentségteleníthetetlenségeskedéseitekért")
     // result: "MEGSZENTSÉGTELENÍTHETETLENSÉGESKEDÉSEITEKÉRT"
     decl->defaultReturnType = KDbField::LongText;
     _SIG(upper_1, argAnyTextOrNull);

#ifdef KDB_ENABLE_SQLITE_SPECIFIC_FUNCTIONS
    insert(QLatin1String("GLOB"), decl = new BuiltInFunctionDeclaration);
    //! @todo GLOB(X,Y) is SQLite-specific and is not present in MySQL so we don't expose it; use GLOB operator instead.
    //! We may want to address it in raw SQL generation time.
    // From https://www.sqlite.org/lang_corefunc.html
    /* The glob(X,Y) function is equivalent to the expression "Y GLOB X". Note that the
     X and Y arguments are reversed in the glob() function relative to the infix GLOB
     operator. */
    // example: SELECT GLOB("Foo*", "FooBar"), GLOB("Foo*", "foobar")
    // result: TRUE, FALSE
    decl->defaultReturnType = KDbField::Boolean;
    _SIG(glob_2, argAnyTextOrNull, argAnyOrNull /* will be casted to text */);

    insert(QLatin1String("LIKE"), decl = new BuiltInFunctionDeclaration);
    //! @todo LIKE(X,Y,[Z]) not present in MySQL so we don't expose it; use LIKE operator instead.
    //! We may want to address it in raw SQL generation time.
    // From https://www.sqlite.org/lang_corefunc.html
    /* The like() function is used to implement the "Y LIKE X [ESCAPE Z]" expression. If the
    optional ESCAPE clause is present, then the like() function is invoked with three
    arguments. Otherwise, it is invoked with two arguments only. Note that the X and Y
    parameters are reversed in the like() function relative to the infix LIKE operator.*/
    decl->defaultReturnType = KDbField::Boolean;
    _SIG(like_2, argAnyTextOrNull, argAnyTextOrNull);
    _SIG(like_3, argAnyTextOrNull, argAnyTextOrNull, argAnyTextOrNull);
#endif
}

BuiltInFunctionDeclaration* BuiltInFunctions::value(const QString &name) const
{
    BuiltInFunctionDeclaration* f = QHash<QString, BuiltInFunctionDeclaration*>::value(name);
    if (!f) {
        f = m_aliases.value(name);
    }
    return f;
}

QStringList BuiltInFunctions::aliases() const
{
    return m_aliases.keys();
}

Q_GLOBAL_STATIC(BuiltInFunctions, _builtInFunctions)

//=========================================

KDbFunctionExpressionData::KDbFunctionExpressionData()
 : KDbExpressionData()
{
    ExpressionDebug << "FunctionExpressionData" << ref;
    setArguments(ExplicitlySharedExpressionDataPointer());
}

KDbFunctionExpressionData::KDbFunctionExpressionData(const QString& aName,
                                                     ExplicitlySharedExpressionDataPointer arguments)
        : KDbExpressionData()
        , name(aName)
{
    setArguments(arguments);
    ExpressionDebug << "FunctionExpressionData" << ref << *args;
}

KDbFunctionExpressionData::~KDbFunctionExpressionData()
{
    ExpressionDebug << "~FunctionExpressionData" << ref;
}

KDbFunctionExpressionData* KDbFunctionExpressionData::clone()
{
    ExpressionDebug << "FunctionExpressionData::clone" << *this;
    KDbFunctionExpressionData *cloned = new KDbFunctionExpressionData(*this);
    ExpressionDebug << "FunctionExpressionData::clone" << *cloned;
    cloned->args = args->clone();
    return cloned;
}

void KDbFunctionExpressionData::debugInternal(QDebug dbg, KDb::ExpressionCallStack* callStack) const
{
    dbg.nospace() << "FunctionExp(" << name;
    if (args.data()) {
        dbg.nospace() << ',';
        args.data()->debug(dbg, callStack);
    }
    dbg.nospace() << QString::fromLatin1(",type=%1)").arg(KDbDriver::defaultSQLTypeName(type()));
}

static QByteArray greatestOrLeastName(const QByteArray &name)
{
    if (name == "MAX") {
        return "GREATEST";
    }
    if (name == "MIN") {
        return "LEAST";
    }
    return name;
}

KDbEscapedString KDbFunctionExpressionData::toStringInternal(
                                        const KDbDriver *driver,
                                        KDbQuerySchemaParameterValueListIterator* params,
                                        KDb::ExpressionCallStack* callStack) const
{
    KDbNArgExpressionData *argsData = args->convert<KDbNArgExpressionData>();
    if (name == QLatin1String("HEX")) {
        if (driver) {
            return driver->hexFunctionToString(KDbNArgExpression(args), params, callStack);
        }
    }
    else if (name == QLatin1String("IFNULL")) {
        if (driver) {
            return driver->ifnullFunctionToString(KDbNArgExpression(args), params, callStack);
        }
    }
    else if (name == QLatin1String("LENGTH")) {
        if (driver) {
            return driver->lengthFunctionToString(KDbNArgExpression(args), params, callStack);
        }
    }
    else if (name == QLatin1String("GREATEST") || name == QLatin1String("MAX")
             || name == QLatin1String("LEAST") || name == QLatin1String("MIN"))
    {
        if (driver) {
            return driver->greatestOrLeastFunctionToString(
                QString::fromLatin1(greatestOrLeastName(name.toLatin1())), KDbNArgExpression(args), params, callStack);
        }
        // else: don't change MIN/MAX
    }
    else if (name == QLatin1String("RANDOM")) {
        if (driver) {
            return driver->randomFunctionToString(KDbNArgExpression(args), params, callStack);
        }
    }
    else if (name == QLatin1String("CEILING") || name == QLatin1String("FLOOR")) {
        if (driver) {
            return driver->ceilingOrFloorFunctionToString(name, KDbNArgExpression(args), params, callStack);
        }
    }
    else if (name == QLatin1String("UNICODE")) {
        if (driver) {
            return driver->unicodeFunctionToString(KDbNArgExpression(args), params, callStack);
        }
    }
    return KDbFunctionExpressionData::toString(name, driver, argsData, params, callStack);
}

void KDbFunctionExpressionData::getQueryParameters(QList<KDbQuerySchemaParameter>* params)
{
    Q_ASSERT(params);
    args->getQueryParameters(params);
}

KDbField::Type KDbFunctionExpressionData::typeInternal(KDb::ExpressionCallStack* callStack) const
{
    Q_UNUSED(callStack);
    const BuiltInFunctionDeclaration *decl = _builtInFunctions->value(name);
    if (decl) {
        return decl->returnType(this, nullptr);
    }
    //! @todo
    return KDbField::InvalidType;
}

static void setIncorrectNumberOfArgumentsErrorMessage(KDbParseInfo *parseInfo, int count,
                                                      const std::vector<int> &argCounts,
                                                      const QString &name)
{
    parseInfo->setErrorMessage(
                KDbFunctionExpressionData::tr("Incorrect number of arguments (%1)").arg(count));
    const int maxArgCount = argCounts[argCounts.size() - 1];
    const int minArgCount = argCounts[0];
    QString firstSentence;
    if (count > maxArgCount) {
        firstSentence = KDbFunctionExpressionData::tr("Too many arguments.%1", "don't use space before %1")
                                          .arg(QLatin1String(" "));
    }
    if (count < minArgCount) {
        firstSentence = KDbFunctionExpressionData::tr("Too few arguments.%1", "don't use space before %1")
                                          .arg(QLatin1String(" "));
    }
    if (argCounts.size() == 1) {
        const int c = argCounts[0];
        if (c == 0) {
            parseInfo->setErrorDescription(
                KDbFunctionExpressionData::tr("%1%2() function does not accept any arguments.")
                                              .arg(firstSentence, name));
        }
        else if (c == 1) {
            parseInfo->setErrorDescription(
                KDbFunctionExpressionData::tr("%1%2() function requires 1 argument.")
                                             .arg(firstSentence, name));
        }
        else {
            //~ singular %1%2() function requires %3 argument.
            //~ plural %1%2() function requires %3 arguments.
            parseInfo->setErrorDescription(
                KDbFunctionExpressionData::tr("%1%2() function requires %3 argument(s).", "", c)
                                             .arg(firstSentence, name).arg(c));
        }
    }
    else if (argCounts.size() == 2) {
        const int c1 = argCounts[0];
        const int c2 = argCounts[1];
        if (c2 == 1) {
            parseInfo->setErrorDescription(
                KDbFunctionExpressionData::tr("%1%2() function requires 0 or 1 argument.",
                                  "the function requires zero or one argument")
                                              .arg(firstSentence, name));
        }
        else {
            //~ singular %1%2() function requires %3 or %4 argument.
            //~ plural %1%2() function requires %3 or %4 arguments.
            parseInfo->setErrorDescription(
                KDbFunctionExpressionData::tr("%1%2() function requires %3 or %4 argument(s).", "", c2)
                                             .arg(firstSentence, name).arg(c1).arg(c2));
        }
    }
    else if (argCounts.size() == 3) {
        //~ singular %1%2() function requires %3 or %4 or %5 argument.
        //~ plural %1%2() function requires %3 or %4 or %5 arguments.
        parseInfo->setErrorDescription(
            KDbFunctionExpressionData::tr("%1%2() function requires %3 or %4 or %5 argument(s).", "", argCounts[2])
                                         .arg(firstSentence, name).arg(argCounts[0])
                                         .arg(argCounts[1]).arg(argCounts[2]));
    }
    else {
        QString listCounts;
        for(std::vector<int>::const_iterator it(argCounts.begin()); it != argCounts.end(); ++it) {
            if (listCounts.isEmpty()) {
                listCounts += QString::number(*it);
            } else {
                listCounts = KDbFunctionExpressionData::tr("%1 or %2").arg(listCounts).arg(*it);
            }
        }
        parseInfo->setErrorDescription(
            KDbFunctionExpressionData::tr("%1%2() function requires %3 argument(s).", "",
                              argCounts[argCounts.size() - 1])
                              .arg(firstSentence, name, listCounts));
    }
}

static void setIncorrectTypeOfArgumentsErrorMessage(KDbParseInfo *parseInfo, int argNum,
                                                    KDbField::Type type,
                                                    int *argTypes, const QString &name)
{
    QString listTypes;
    int *argType = argTypes;
    while(*argType != KDbField::InvalidType) {
        if (!listTypes.isEmpty()) {
            listTypes += KDbFunctionExpressionData::tr(" or ");
        }
        const KDbField::Type realFieldType = KDb::intToFieldType(*argType);
        if (realFieldType != KDbField::InvalidType) {
            listTypes += KDbFunctionExpressionData::tr("\"%1\"")
                            .arg(KDbField::typeName(realFieldType));
        }
        else if (*argType == KDbField::Null) {
            listTypes += KDbFunctionExpressionData::tr("\"%1\"")
                            .arg(KDbField::typeName(KDbField::Null));
        }
        else if (*argType == AnyText) {
            listTypes += KDbFunctionExpressionData::tr("\"%1\"")
                            .arg(KDbField::typeName(KDbField::Text));
        }
        else if (*argType == AnyInt) {
            listTypes += KDbFunctionExpressionData::tr("\"%1\"")
                            .arg(KDbField::typeName(KDbField::Integer));
        }
        else if (*argType == AnyFloat) {
            listTypes += KDbFunctionExpressionData::tr("\"%1\"")
                            .arg(KDbField::typeGroupName(KDbField::FloatGroup));
                         // better than typeName() in this case
        }
        else if (*argType == AnyNumber) {
            listTypes += KDbFunctionExpressionData::tr("\"Number\"");
        }
        else if (*argType == Any) {
            listTypes += KDbFunctionExpressionData::tr("\"Any\"", "Any data type");
        }
        ++argType;
    }
    parseInfo->setErrorMessage(KDbFunctionExpressionData::tr("Incorrect type of argument"));
    QString lastSentence
        = KDbFunctionExpressionData::tr("Specified argument is of type \"%1\".")
            .arg(KDbField::typeName(type));
    if (argNum == 0) {
        parseInfo->setErrorDescription(
            KDbFunctionExpressionData::tr("%1() function's first argument should be of type %2. %3")
                                          .arg(name, listTypes, lastSentence));
    }
    else if (argNum == 1) {
        parseInfo->setErrorDescription(
            KDbFunctionExpressionData::tr("%1() function's second argument should be of type %2. %3")
                                          .arg(name, listTypes, lastSentence));
    }
    else if (argNum == 2) {
        parseInfo->setErrorDescription(
            KDbFunctionExpressionData::tr("%1() function's third argument should be of type %2. %3")
                                          .arg(name, listTypes, lastSentence));
    }
    else if (argNum == 3) {
        parseInfo->setErrorDescription(
            KDbFunctionExpressionData::tr("%1() function's fourth argument should be of type %2. %3")
                                          .arg(name, listTypes, lastSentence));
    }
    else if (argNum == 4) {
        parseInfo->setErrorDescription(
            KDbFunctionExpressionData::tr("%1() function's fifth argument should be of type %2. %3")
                                          .arg(name, listTypes, lastSentence));
    }
    else {
        parseInfo->setErrorDescription(
            KDbFunctionExpressionData::tr("%1() function's %2 argument should be of type %3. %4")
                                          .arg(name).arg(argNum + 1).arg(listTypes, lastSentence));
    }
}

//! @return true if type rule @a argType matches concrete type @a actualType
static bool typeMatches(int argType, KDbField::Type actualType)
{
    if (argType == AnyText) {
        if (KDbField::isTextType(actualType)) {
            return true;
        }
    }
    else if (argType == AnyInt) {
        if (KDbField::isIntegerType(actualType)) {
            return true;
        }
    }
    else if (argType == AnyFloat) {
        if (KDbField::isFPNumericType(actualType)) {
            return true;
        }
    }
    else if (argType == AnyNumber) {
        if (KDbField::isNumericType(actualType)) {
            return true;
        }
    }
    else if (argType == Any) {
        return true;
    }
    else {
        if (argType == actualType) {
            return true;
        }
    }
    return false;
}

static int findMatchingType(int *argTypePtr, KDbField::Type actualType)
{
    for (; *argTypePtr != KDbField::InvalidType; ++argTypePtr) {
        if (typeMatches(*argTypePtr, actualType)) {
            break;
        }
    }
    return *argTypePtr;
}

bool KDbFunctionExpressionData::validateInternal(KDbParseInfo *parseInfo,
                                                 KDb::ExpressionCallStack* callStack)
{
    if (!args->validate(parseInfo, callStack)) {
        return false;
    }
    if (args->token != ',') { // arguments required: NArgExpr with token ','
        return false;
    }
    if (args->children.count() > KDB_MAX_FUNCTION_ARGS) {
        parseInfo->setErrorMessage(
            tr("Too many arguments for function."));
        parseInfo->setErrorDescription(
            tr("Maximum number of arguments for function %1() is %2.")
               .arg(args->children.count()).arg(KDB_MAX_FUNCTION_ARGS));
        return false;
    }
    if (!args->validate(parseInfo)) {
        return false;
    }
    if (name.isEmpty()) {
        return false;
    }
    const BuiltInFunctionDeclaration *decl = _builtInFunctions->value(name);
    if (!decl) {
        return false;
    }
    const KDbNArgExpressionData *argsData = args->convertConst<KDbNArgExpressionData>();
    if (argsData->containsInvalidArgument()) {
        return false;
    }

    // Find matching signature
    const int count = args->children.count();
    bool properArgCount = false;
    std::vector<int> argCounts;
    int i = 0;
    argCounts.resize(decl->signatures.size());
    int **signature = nullptr;
    bool multipleArgs = false; // special case, e.g. for CHARS(v1, ... vN)
    for(std::vector<int**>::const_iterator it(decl->signatures.begin());
        it != decl->signatures.end(); ++it, ++i)
    {
        signature = *it;
        int **arg = signature;
        int expectedCount = 0;
        while(*arg && *arg != BuiltInFunctions::multipleArgs) {
            ++arg;
            ++expectedCount;
        }
        multipleArgs = *arg == BuiltInFunctions::multipleArgs;
        if (multipleArgs) {
            ++arg;
            const int minArgs = arg[0][0];
            properArgCount = count >= minArgs;
            if (!properArgCount) {
                parseInfo->setErrorMessage(
                    tr("Incorrect number of arguments (%1)").arg(count));
                if (minArgs == 1) {
                    parseInfo->setErrorDescription(
                        tr("Too few arguments. %1() function requires "
                           "at least one argument.").arg(name));
                }
                else if (minArgs == 2) {
                    parseInfo->setErrorDescription(
                        tr("Too few arguments. %1() function requires "
                           "at least two arguments.").arg(name));
                }
                else if (minArgs == 3) {
                    parseInfo->setErrorDescription(
                        tr("Too few arguments. %1() function requires "
                           "at least three arguments.").arg(name));
                }
                else {
                    parseInfo->setErrorDescription(
                        tr("Too few arguments. %1() function requires "
                           "at least %2 arguments.").arg(name).arg(minArgs));
                }
                return false;
            }
            break;
        }
        else if (count == expectedCount) { // arg # matches
            properArgCount = true;
            break;
        }
        else {
            argCounts[i] = expectedCount;
        }
    }
    if (!properArgCount) {
        std::unique(argCounts.begin(), argCounts.end());
        std::sort(argCounts.begin(), argCounts.end()); // sort so we can easier check the case
        setIncorrectNumberOfArgumentsErrorMessage(parseInfo, count, argCounts, name);
        return false;
    }

    // Verify types
    if (multipleArgs) { // special signature: {typesForAllArgs, [multipleArgs-token], MIN, 0}
        int **arg = signature;
        int *typesForAllArgs = arg[0];
        int i = 0;
        foreach(const ExplicitlySharedExpressionDataPointer &expr, args->children) {
            const KDbField::Type exprType = expr->type(); // cache: evaluating type of expressions can be expensive
            const bool isQueryParameter = expr->convertConst<KDbQueryParameterExpressionData>();
            if (!isQueryParameter) { // (query parameter always matches)
                const int matchingType = findMatchingType(typesForAllArgs, exprType);
                if (matchingType == KDbField::InvalidType) {
                    setIncorrectTypeOfArgumentsErrorMessage(parseInfo, i, exprType, typesForAllArgs, name);
                    return false;
                }
            }
            ++i;
        }
    }
    else { // typical signature: array of type-lists
        int **arg = signature;
        int i=0;
        foreach(const ExplicitlySharedExpressionDataPointer &expr, args->children) {
            const KDbField::Type exprType = expr->type(); // cache: evaluating type of expressions can be expensive
            const bool isQueryParameter = expr->convertConst<KDbQueryParameterExpressionData>();
            if (!isQueryParameter) { // (query parameter always matches)
                const int matchingType = findMatchingType(arg[0], exprType);
                if (matchingType == KDbField::InvalidType) {
                    setIncorrectTypeOfArgumentsErrorMessage(parseInfo, i, exprType, arg[0], name);
                    return false;
                }
            }
            ++arg;
            ++i;
        }
    }

    // Check type just now. If we checked earlier, possible error message would be less informative.
    if (decl->returnType(this, parseInfo) == KDbField::InvalidType) {
        return false;
    }
    return true;
}

void KDbFunctionExpressionData::setArguments(ExplicitlySharedExpressionDataPointer arguments)
{
    args = (arguments && arguments->convert<KDbNArgExpressionData>())
            ? arguments : ExplicitlySharedExpressionDataPointer(new KDbNArgExpressionData);
    children.append(args);
    args->parent = this;
    args->token = ',';
    args->expressionClass = KDb::ArgumentListExpression;
}

//static
KDbEscapedString KDbFunctionExpressionData::toString(
                                        const QString &name,
                                        const KDbDriver *driver,
                                        const KDbNArgExpressionData *args,
                                        KDbQuerySchemaParameterValueListIterator* params,
                                        KDb::ExpressionCallStack* callStack)
{
    return KDbEscapedString(name + QLatin1Char('('))
           + args->toString(driver, params, callStack)
           + KDbEscapedString(')');
}

//=========================================

inline KDb::ExpressionClass classForFunctionName(const QString& name)
{
    if (KDbFunctionExpression::isBuiltInAggregate(name))
        return KDb::AggregationExpression;
    else
        return KDb::FunctionExpression;
}

KDbFunctionExpression::KDbFunctionExpression()
 : KDbExpression(new KDbFunctionExpressionData)
{
    ExpressionDebug << "KDbFunctionExpression() ctor" << *this;
}

KDbFunctionExpression::KDbFunctionExpression(const QString& name)
        : KDbExpression(new KDbFunctionExpressionData(name),
              classForFunctionName(name), KDbToken()/*undefined*/)
{
}

KDbFunctionExpression::KDbFunctionExpression(const QString& name,
                                             const KDbNArgExpression& arguments)
        : KDbExpression(new KDbFunctionExpressionData(name.toUpper(), arguments.d),
              classForFunctionName(name), KDbToken()/*undefined*/)
{
}

KDbFunctionExpression::KDbFunctionExpression(const KDbFunctionExpression& expr)
        : KDbExpression(expr)
{
}

KDbFunctionExpression::KDbFunctionExpression(KDbExpressionData* data)
    : KDbExpression(data)
{
    ExpressionDebug << "KDbFunctionExpression ctor (KDbExpressionData*)" << *this;
}

KDbFunctionExpression::KDbFunctionExpression(const ExplicitlySharedExpressionDataPointer &ptr)
    : KDbExpression(ptr)
{
}

KDbFunctionExpression::~KDbFunctionExpression()
{
}

// static
bool KDbFunctionExpression::isBuiltInAggregate(const QString& function)
{
    return _builtInAggregates->contains(function.toUpper());
}

// static
QStringList KDbFunctionExpression::builtInAggregates()
{
    return _builtInAggregates->toList();
}

//static
KDbEscapedString KDbFunctionExpression::toString(
                                        const QString &name,
                                        const KDbDriver *driver,
                                        const KDbNArgExpression& args,
                                        KDbQuerySchemaParameterValueListIterator* params,
                                        KDb::ExpressionCallStack* callStack)
{
    const KDbNArgExpressionData *argsData = args.d.constData()->convertConst<KDbNArgExpressionData>();
    return KDbFunctionExpressionData::toString(name, driver, argsData, params, callStack);
}

QString KDbFunctionExpression::name() const
{
    return d->convert<KDbFunctionExpressionData>()->name;
}

void KDbFunctionExpression::setName(const QString &name)
{
    d->convert<KDbFunctionExpressionData>()->name = name;
}

KDbNArgExpression KDbFunctionExpression::arguments()
{
    return KDbNArgExpression(d->convert<KDbFunctionExpressionData>()->args);
}

void KDbFunctionExpression::setArguments(const KDbNArgExpression &arguments)
{
    d->convert<KDbFunctionExpressionData>()->setArguments(arguments.d);
}

// static
KDbEscapedString KDbFunctionExpression::greatestOrLeastFunctionUsingCaseToString(
                                        const QString &name,
                                        const KDbDriver *driver,
                                        const KDbNArgExpression &args,
                                        KDbQuerySchemaParameterValueListIterator* params,
                                        KDb::ExpressionCallStack* callStack)
{
    // (CASE WHEN (v0) IS NULL OR .. OR (vN) IS NULL THEN NULL ELSE F(v0,..,vN) END)
    if (args.argCount() >= 2) {
        KDbEscapedString whenSQL;
        whenSQL.reserve(256);
        foreach(const ExplicitlySharedExpressionDataPointer &child, args.d.constData()->children) {
            if (!whenSQL.isEmpty()) {
                whenSQL += " OR ";
            }
            whenSQL += QLatin1Char('(') + child->toString(driver, params, callStack)
                    + QLatin1String(") IS NULL");
        }
        return KDbEscapedString("(CASE WHEN (") + whenSQL
               + QLatin1String(") THEN NULL ELSE (")
               + KDbFunctionExpression::toString(name, driver, args, params, callStack)
               + QLatin1String(") END)");
    }
    return KDbFunctionExpression::toString(name, driver, args, params, callStack);
}

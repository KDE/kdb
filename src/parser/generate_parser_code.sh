#!/bin/bash
#
#   Copyright (C) 2006-2015 Jarosław Staniek <staniek@kde.org>
#
#   This program is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public
#   License as published by the Free Software Foundation; either
#   version 2 of the License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; see the file COPYING.  If not, write to
#   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#   Boston, MA 02110-1301, USA.
#
# Generates parser and lexer code using bison and flex
#

me=generate_parser_code.sh

BISON_MIN=3.0.4      # keep updated for best results
BISON_MIN_NUM=30004  # keep updated for best results
FLEX_MIN=2.5.37      # keep updated for best results
FLEX_MIN_NUM=20537   # keep updated for best results

# Check minimum version of bison
bisonv=`bison --version | head -n 1| cut -f4 -d" "`
bisonv1=`echo $bisonv | cut -f1 -d.`
bisonv2=`echo $bisonv | cut -f2 -d.`
bisonv3=`echo $bisonv | cut -f3 -d.`
if [ -z $bisonv3 ] ; then bisonv3=0; fi
bisonvnum=`expr $bisonv1 \* 10000 + $bisonv2 \* 100 + $bisonv3`
if [ $bisonvnum -lt $BISON_MIN_NUM ] ; then
    echo "$bisonv is too old bison version, the minimum is $BISON_MIN."
    exit 1
fi

# Check minimum version of flex
flexv=`flex --version | head -n 1| cut -f2 -d" "`
flexv1=`echo $flexv | cut -f1 -d.`
flexv2=`echo $flexv | cut -f2 -d.`
flexv3=`echo $flexv | cut -f3 -d.`
flexvnum=`expr $flexv1 \* 10000 + $flexv2 \* 100 + $flexv3`
if [ $flexvnum -lt $FLEX_MIN_NUM ] ; then
    echo "$flexv is too old flex version, the minimum is $FLEX_MIN."
    exit 1
fi

# Generate lexer and parser
builddir=$PWD
srcdir=`dirname $0`
cd $srcdir

flex -ogenerated/sqlscanner.cpp KDbSqlScanner.l
# Correct a few yy_size_t vs size_t vs int differences that some bisons cause
sed --in-place 's/int yyleng/yy_size_t yyleng/g;s/int yyget_leng/yy_size_t yyget_leng/g;s/yyleng = (int)/yyleng = (size_t)/g;' generated/sqlscanner.cpp
bison -d KDbSqlParser.y -Wall -fall -rall --report-file=$builddir/KDbSqlParser.output

# postprocess
cat << EOF > generated/sqlparser.h
/****************************************************************************
 * Created by $me
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

EOF

# Fine-tune the code: extra functions and remove trailing white space
cat KDbSqlParser.tab.h >> generated/sqlparser.h
echo '#endif' >> generated/sqlparser.h
sed --in-place 's/[[:space:]]\+$//;s/\t/        /g' generated/sqlparser.h

cat KDbSqlParser.tab.c | sed -e "s/KDbSqlParser\.tab\.c/sqlparser.cpp/g" > generated/sqlparser.cpp
cat << EOF >> generated/sqlparser.cpp
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
EOF
sed --in-place 's/[[:space:]]\+$//;s/\t/        /g' generated/sqlparser.cpp

# Extract table of SQL tokens
# unused ./extract_tokens.sh > generated/tokens.cpp
rm -f KDbSqlParser.tab.h KDbSqlParser.tab.c

# Create KDbToken.h
cat << EOF > generated/KDbToken.h
/****************************************************************************
 * Created by $me
 * WARNING! All changes made in this file will be lost!
 ****************************************************************************/
/* This file is part of the KDE project
   Copyright (C) 2015-2018 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_TOKEN_H
#define KDB_TOKEN_H

#include "kdb_export.h"

#include <QDebug>

class KDbDriver;

/*! @brief A type-safe KDbSQL token
 It can be used in KDb expressions
 @see KDbExpression */
class KDB_EXPORT KDbToken
{
public:
    //! @todo add KDbToken(const QByteArray &name)

    //! Creates an invalid token
    inline KDbToken() : v(0) {}

    KDbToken(const KDbToken &other) : v(other.v) {}

    //! Creates a single-character token
    //! Only characters that belong to the grammar are accepted:
EOF
(echo -n "    //! "; grep "\"'.'\","  generated/sqlparser.cpp \
    | sed -e "s/\"\('.'\)\",/\1/g;s/\"[0-9A-Za-z_$]*\",[ ]*//g;" | tr --delete '\n' \
    | sed -e "s/ $//g;") >> generated/KDbToken.h

cat << EOF >> generated/KDbToken.h

    //! Invalid KDbToken is created for character that is not accepted.
    KDbToken(char charToken);

    //! @return true if this token is valid
    inline bool isValid() const { return v != 0; }

    //! @return name of this token
    //! Useful for debugging.
    //! For example "NOT_EQUAL" string is returned for the NOT_EQUAL token.
    //! A single character is returned for printable single-character tokens.
    //! A number is returned for non-printable single-character.
    //! "<INVALID_TOKEN>" is returned for an invalid string.
    QString name() const;

    //! @return string interpretation of this token (as visibe to the user)
    //! For example "<>" is returned for the NOT_EQUAL token.
    //! Empty string is returned for an invalid string
    //! The result may depend on the optional @a driver parameter.
    //! If @a driver is @c nullptr, representation for portable KDbSQL dialect is returned.
    QString toString(const KDbDriver *driver = nullptr) const;

    //! Like toString(const KDbDriver *driver)
    static QString toString(KDbToken token, const KDbDriver *driver = nullptr);

    //! Maximum character token value (253)
    static const int maxCharTokenValue;

    //! Maximum character token value
    static const int maxTokenValue;

    //! @return character equivalent of this token
    //! Only character-based tokens are supported this way (toInt() <= maxCharTokenValue).
    //! For unsupported tokens @c nullptr is returned.
    inline char toChar() const { return v <= maxCharTokenValue ? v : 0; }

    //! @return numeric value of this token
    inline int value() const { return v; }

    //! @return true if this token is equal to @a other token
    inline bool operator==(KDbToken other) const { return v == other.v; }

    //! @return true if this token is not equal to @a other token
    inline bool operator!=(KDbToken other) const { return v != other.v; }

    //! @return true if this token is equal to @a other token
    inline bool operator==(char charToken) const { return v == charToken; }

    //! @return true if this token is not equal to @a other token
    inline bool operator!=(char charToken) const { return v != charToken; }

    static QList<KDbToken> allTokens();

    // -- constants go here --
EOF

function extractTokens()
{
    grep -E  "    [A-Z0-9_]+ = [[:digit:]]+" generated/sqlparser.h \
        | sed -e "s/^    //g;s/ = / /g;s/,//g"
}

extractTokens | while read token value; do
    echo "    static const KDbToken $token;"
done >> generated/KDbToken.h

function customTokens()
{
cat << EOF
BETWEEN_AND 0x1001
NOT_BETWEEN_AND 0x1002
EOF
}

echo "    //! Custom tokens are not used in parser but used as an extension in expression classes." >> generated/KDbToken.h

customTokens | while read token value; do
    echo "    static const KDbToken $token;"
done >> generated/KDbToken.h

cat << EOF >> generated/KDbToken.h
    // -- end of constants --

    class List;
private:
    inline KDbToken(int value) : v(value) {}
    int v;
};

//! Sends information about token @a token to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, KDbToken token);

#endif
EOF

cat << EOF > generated/KDbToken.cpp
/****************************************************************************
 * Created by $me
 * WARNING! All changes made in this file will be lost!
 ****************************************************************************/
/* This file is part of the KDE project
   Copyright (C) 2015-2018 Jarosław Staniek <staniek@kde.org>

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

#include "KDbToken.h"
#include "KDbDriver.h"
#include "KDbDriver_p.h"
#include "KDbDriverBehavior.h"
#include "sqlparser.h"
#include "parser/KDbParser_p.h"

#include <QGlobalStatic>

KDbToken::KDbToken(char charToken)
    : v(g_tokenName(charToken) == nullptr ? 0 : charToken)
{
}

QString KDbToken::name() const
{
    if (!isValid()) {
        return QLatin1String("<INVALID_TOKEN>");
    }
    if (v > maxCharTokenValue) {
        return QLatin1String(g_tokenName(v));
    }
    if (isprint(v)) {
        return QString(QLatin1Char(char(v)));
    }
    else {
        return QLatin1String(QByteArray::number(v));
    }
}

QString KDbToken::toString(const KDbDriver *driver) const
{
    if (toChar() > 0) {
        return name();
    }
    // other arithmetic operations: << >>
    // NOTE: only include cases that have toString() != name() or are dependent on driver
    switch (v) {
    case ::BITWISE_SHIFT_RIGHT: return QLatin1String(">>");
    case ::BITWISE_SHIFT_LEFT: return QLatin1String("<<");
        // other relational operations: <= >= <> (or !=) LIKE IN
    case ::NOT_EQUAL: return QLatin1String("<>");
    case ::NOT_EQUAL2: return QLatin1String("!=");
    case ::LESS_OR_EQUAL: return QLatin1String("<=");
    case ::GREATER_OR_EQUAL: return QLatin1String(">=");
    case ::LIKE: return driver ? KDbDriverPrivate::behavior(driver)->LIKE_OPERATOR : QLatin1String("LIKE");
    case ::NOT_LIKE:
        return driver
            ? (QString::fromLatin1("NOT ") + KDbDriverPrivate::behavior(driver)->LIKE_OPERATOR)
            : QString::fromLatin1("NOT LIKE");
    case ::SQL_IN: return QLatin1String("IN");
        // other logical operations: OR (or ||) AND (or &&) XOR
    case ::SIMILAR_TO: return QLatin1String("SIMILAR TO");
    case ::NOT_SIMILAR_TO: return QLatin1String("NOT SIMILAR TO");
        // other string operations: || (as CONCATENATION)
    case ::CONCATENATION: return QLatin1String("||");
        // SpecialBinary "pseudo operators":
        /* not handled here */
    default:;
    }
    const QString s = name();
    if (!s.isEmpty()) {
        return s;
    }
    return QString::fromLatin1("<INVALID_TOKEN#%1> ").arg(v);
}

//static
QString KDbToken::toString(KDbToken token, const KDbDriver *driver)
{
    return token.toString(driver);
}

KDB_EXPORT QDebug operator<<(QDebug dbg, KDbToken token)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace().noquote() << token.name();
    return dbg.maybeSpace();
}

//! @internal
class KDbToken::List
{
public:
    List()
    {
        for (int i = 0; i < KDbToken::maxTokenValue; ++i) {
            if (g_tokenName(i)) {
                data.append(KDbToken(i));
            }
        }
    }
    QList<KDbToken> data;
};

Q_GLOBAL_STATIC(KDbToken::List, g_allTokens)

//static
QList<KDbToken> KDbToken::allTokens()
{
    return g_allTokens->data;
}

EOF

extractTokens | while read token value; do
    echo "const KDbToken KDbToken::$token(::$token);"
done >> generated/KDbToken.cpp

customTokens | while read token value; do
    echo "const KDbToken KDbToken::$token($value);"
done >> generated/KDbToken.cpp

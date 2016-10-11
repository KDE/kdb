/****************************************************************************
 * Created by generate_parser_code.sh
 * WARNING! All changes made in this file will be lost!
 ****************************************************************************/
/* This file is part of the KDE project
   Copyright (C) 2015 Jarosław Staniek <staniek@kde.org>

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

/*! @brief A type-safe KDBSQL token
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
    //! ';' ',' '.' '>' '<' '=' '+' '-' '&' '|'  '/' '*' '%' '~' '(' ')'
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
    //! If @a driver is 0, representation for portable KDbSQL dialect is returned.
    QString toString(const KDbDriver *driver = 0) const;

    //! Like toString(const KDbDriver *driver)
    static QString toString(KDbToken token, const KDbDriver *driver = 0);

    //! Maximum character token value (253)
    static const int maxCharTokenValue;

    //! Maximum character token value
    static const int maxTokenValue;

    //! @return character equivalent of this token
    //! Only character-based tokens are supported this way (toInt() <= maxCharTokenValue).
    //! For unsupported tokens 0 is returned.
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
    static const KDbToken SQL_TYPE;
    static const KDbToken AS;
    static const KDbToken AS_EMPTY;
    static const KDbToken ASC;
    static const KDbToken AUTO_INCREMENT;
    static const KDbToken BIT;
    static const KDbToken BITWISE_SHIFT_LEFT;
    static const KDbToken BITWISE_SHIFT_RIGHT;
    static const KDbToken BY;
    static const KDbToken CHARACTER_STRING_LITERAL;
    static const KDbToken CONCATENATION;
    static const KDbToken CREATE;
    static const KDbToken DESC;
    static const KDbToken DISTINCT;
    static const KDbToken DOUBLE_QUOTED_STRING;
    static const KDbToken FROM;
    static const KDbToken JOIN;
    static const KDbToken KEY;
    static const KDbToken LEFT;
    static const KDbToken LESS_OR_EQUAL;
    static const KDbToken GREATER_OR_EQUAL;
    static const KDbToken SQL_NULL;
    static const KDbToken SQL_IS;
    static const KDbToken SQL_IS_NULL;
    static const KDbToken SQL_IS_NOT_NULL;
    static const KDbToken ORDER;
    static const KDbToken PRIMARY;
    static const KDbToken SELECT;
    static const KDbToken INTEGER_CONST;
    static const KDbToken REAL_CONST;
    static const KDbToken RIGHT;
    static const KDbToken SQL_ON;
    static const KDbToken DATE_CONST;
    static const KDbToken DATETIME_CONST;
    static const KDbToken TIME_CONST;
    static const KDbToken TABLE;
    static const KDbToken IDENTIFIER;
    static const KDbToken IDENTIFIER_DOT_ASTERISK;
    static const KDbToken QUERY_PARAMETER;
    static const KDbToken VARCHAR;
    static const KDbToken WHERE;
    static const KDbToken SQL;
    static const KDbToken SQL_TRUE;
    static const KDbToken SQL_FALSE;
    static const KDbToken UNION;
    static const KDbToken SCAN_ERROR;
    static const KDbToken AND;
    static const KDbToken BETWEEN;
    static const KDbToken NOT_BETWEEN;
    static const KDbToken EXCEPT;
    static const KDbToken SQL_IN;
    static const KDbToken INTERSECT;
    static const KDbToken LIKE;
    static const KDbToken ILIKE;
    static const KDbToken NOT_LIKE;
    static const KDbToken NOT;
    static const KDbToken NOT_EQUAL;
    static const KDbToken NOT_EQUAL2;
    static const KDbToken OR;
    static const KDbToken SIMILAR_TO;
    static const KDbToken NOT_SIMILAR_TO;
    static const KDbToken XOR;
    static const KDbToken UMINUS;
    //! Custom tokens are not used in parser but used as an extension in expression classes.
    static const KDbToken BETWEEN_AND;
    static const KDbToken NOT_BETWEEN_AND;
    // -- end of constants --

    class List;
private:
    inline KDbToken(int value) : v(value) {}
    int v;
};

//! Sends information about token @a token to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, KDbToken token);

#endif

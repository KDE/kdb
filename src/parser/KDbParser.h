/* This file is part of the KDE project
   Copyright (C) 2003 Lucijan Busch <lucijan@kde.org>
   Copyright (C) 2004-2017 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_PARSER_H
#define KDB_PARSER_H

#include "kdb_export.h"

#include <QString>
#include <QCoreApplication>

class KDbConnection;
class KDbQuerySchema;
class KDbTableSchema;
class KDbEscapedString;

/**
 * Provides detailed error description about KDbParser.
 *
 * @todo Make it explicitly shared using SDC
 * @todo change type to enum
 */
class KDB_EXPORT KDbParserError
{
public:
    /**
     * Empty constructor.
     */
    KDbParserError();

    /**
     * Constructor.
     *
     * @param type The error type.
     * @param message A description of the error.
     * @param token Token where the Error happend.
     * @param position The position where the error happened.
     */
    KDbParserError(const QString &type, const QString &message, const QByteArray &token, int position);

    /**
     * Copy constructor.
     */
    KDbParserError(const KDbParserError &other);

    ~KDbParserError();

    KDbParserError& operator=(const KDbParserError &other);

    bool operator==(const KDbParserError &other) const;

    inline bool operator!=(const KDbParserError &other) const { return !operator==(other); }

    /**
     * @return the error type.
     */
    QString type() const;

    /**
     * @return translated error message.
     */
    QString message() const;

    /**
     * @return (character) position where the error happened.
     */
    int position() const;

private:
    class Private;
    Private * const d;
};

class KDbParserPrivate; //!< @internal

/**
 * A parser tool for SQL statements.
 *
 * The KDbParser class offers functionality of a SQL parser for database-backend-independent
 * KDbSQL dialect. Schema objects such as KDbQuerySchema that are created after successful parsing
 * can be then used for running the queries on actual data or used for further modification.
 *
 * @todo Add examples
 * @todo Support more types than the SELECT
 */
class KDB_EXPORT KDbParser
{
    Q_DECLARE_TR_FUNCTIONS(KDbParser)
public:

    /**
     * The type of the statement.
     */
    enum StatementType {
        NoType,      //!< No statement type specified or detected
        Select,      //!< Query-statement
        CreateTable, //!< Create a new table
        AlterTable,  //!< Alter schema of an existing table
        Insert,      //!< Insert new records
        Update,      //!< Update existing records
        Delete       //!< Delete existing records
    };

    /**
     * Constructs an new parser object.
     * @a connection is used to obtain context, for example wildcards "T.*" resolution
     * is possible only if information about table T is available.
     */
    explicit KDbParser(KDbConnection *connection);

    ~KDbParser();

    /**
     * @brief Clears the parser's status and runs the parsing for a raw SQL statement
     *
     * If parsing of @a sql results in a proper query and @a query is present, it will be set to
     * representation of the parsed query.
     * @since 3.1
     */
    bool parse(const KDbEscapedString &sql, KDbQuerySchema *query = nullptr);

    /**
     * Reset the parser's status (table, query, error, statement, statement type).
     */
    void reset();

    /**
     * @return the resulting statement type
     * NoType is returned if parsing failed or it has not been yet performed or reset() was called.
     */
    StatementType statementType() const;

    /**
     * @return the resulting statement type as string. It is not translated.
     */
    QString statementTypeString() const;

    /**
     * @return a pointer to a query schema if 'CREATE TABLE ...' statement was parsed
     * or @c nullptr for any other statements or on error.
     * @note A proper table schema is returned only once for each successful parse() call,
     * and the object is owned by the caller. In all other cases @c nullptr is returned.
     *
     * @todo Implement this
     */
    KDbTableSchema *table() Q_REQUIRED_RESULT;

    /**
     * @return a pointer to a new query schema created by parsing 'SELECT ...' statement
     * or @c nullptr for any other statements or on error.
     * If existing query was supplied to parse() @c nullptr is returned.
     * @note A proper query schema is returned only once for each successful parse() call,
     * and the object is owned by the caller. In all other cases nullptr is returned.
     */
    KDbQuerySchema *query() Q_REQUIRED_RESULT;

    /**
     * @return a pointer to the used database connection or @c nullptr if it was not set.
     */
    KDbConnection *connection();

    //! @overload
    //! @since 3.1
    const KDbConnection *connection() const;

    /**
     * @return detailed information about last error.
     * If no error occurred KDbParserError::type() is empty.
     */
    KDbParserError error() const;

    /**
     * @return the statement passed on the most recent call of parse().
     */
    KDbEscapedString statement() const;

private:
    void init();

    friend class KDbParserPrivate;
    KDbParserPrivate * const d; //!< @internal d-pointer class.
    Q_DISABLE_COPY(KDbParser)
};

//! Sends information about parser error @a error to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbParserError& error);

#endif

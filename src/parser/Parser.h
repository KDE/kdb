/* This file is part of the KDE project
   Copyright (C) 2003 Lucijan Busch <lucijan@kde.org>
   Copyright (C) 2004-2012 Jarosław Staniek <staniek@kde.org>

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

#ifndef PREDICATE_PARSER_H
#define PREDICATE_PARSER_H

#include <QObject>

namespace Predicate
{

class Connection;
class QuerySchema;
class TableSchema;

/**
 * Provides detailed i18n'ed error description about the @a Parser .
 */
class PREDICATE_EXPORT ParserError
{
public:

    /**
     * Empty constructor.
     */
    ParserError();

    /**
     * Constructor.
     *
     * @param type The error type.
     * @param message A description of the error.
     * @param token Token where the Error happend.
     * @param position The position where the error happened.
     */
    ParserError(const QString &type, const QString &message, const QByteArray &token, int position);

    /**
     * Destructor.
     */
    ~ParserError();

    /**
     * @return the errortype.
     */
    QString type() const {
        return m_type;
    }

    /**
     * @return error message.
     */
    QString message() const {
        return m_message;
    }

    /**
     * @return position where the error happened.
     */
    int position() const {
        return m_position;
    }

private:
    QString m_type;
    QString m_message;
    QByteArray m_token;
    int m_position;
};

class ParseInfoInternal;

/*! Info used on parsing. */
class PREDICATE_EXPORT ParseInfo
{
public:
    ~ParseInfo();

    //! @return positions of tables/aliases having the same name @a tableOrAliasName.
    QList<int> tablesAndAliasesForName(const QString &tableOrAliasName) const;

    //! @return query schema for this parsing
    QuerySchema* querySchema() const;

    //! @return error message for the parsing process
    QString errorMessage() const;

    //! @return detailed error description for the parsing process
    QString errorDescription() const;

protected:
    //! Constructs parse info structure for query @a query.
    explicit ParseInfo(QuerySchema *query);

    Q_DISABLE_COPY(ParseInfo)
    class Private;
    Private * const d;
};

/**
 * Parser for SQL statements.
 *
 * The best and prefeerred way to run queries is using the Predicate::Parser functionality
 * and use the resulting QuerySchema object since this offers a database-backend-independent
 * way to deal with SQL statements on the one hand and offers high level
 * functionality on the other. Also BLOBs like images are handled that way.
 *
 * For example if we like to use the SELECT statement
 * "SELECT dir.path, media.filename FROM dir, media WHERE dir.id=media.dirId AND media.id=%s"
 * we are able to use the @a Connection::prepareStatement method which takes the type of
 * the statement (in our case @a PreparedStatement::SelectStatement ), a list of fields (in
 * our case dir.path and media.filename) and returns a @a PreparedStatement instance.
 * By using the @a QuerySchema::addRelationship and @a QuerySchema::addToWhereExpression methods
 * the SQL statement could be extended with relationships and WHERE expressions.
 *
 * For more, see @a Predicate::PreparedStatement and @a Connection::selectStatement() . A more
 * complex example that looks at what the user has defined and carefully builds
 * @a Predicate::QuerySchema object, including the WHERE expression can be found in
 * the Query Designer's source code in the method @a KexiQueryDesignerGuiEditor::buildSchema().
 */
class PREDICATE_EXPORT Parser
{
public:

    /**
     * The operation-code of the statement.
     */
    enum OPCode {
        OP_None = 0, /// No statement parsed or reseted.
        OP_Error, /// Error while parsing.
        OP_CreateTable, /// Create a table.
        OP_AlterTable, /// Alter an existing table
        OP_Select, /// Query-statement.
        OP_Insert, /// Insert new content.
        OP_Update, /// Update existing content.
        OP_Delete  /// Delete existing content.
    };

    /**
     * constructs an empty object of the parser
     * @param connection is used for things like wildcard resolution. If 0 parser works in "pure mode"
     */
    Parser(Connection *connection);
    ~Parser();

    /**
     * clears previous results and runs the parser
     */
    bool parse(const EscapedString &statement);

    /**
     * rests results
     */
    void clear();

    /**
     * @return the resulting operation or OP_Error if failed
     */
    OPCode operation() const;

    /**
     * @return the resulting operation as string.
     */
    QString operationString() const;

    /**
     * @return a pointer to a table schema on CREATE TABLE
     * or 0 on any other operation or error. Returned object is owned by you.
     * You can call this method only once every time after doing parse().
     * Next time, the call will return 0.
     */
    TableSchema *table();

    /**
     * @return a pointer to a query schema if 'SELECT ...' was called
     * or 0 on any other operation or error. Returned object is owned by you.
     * You can call this method only once every time after doing parse().
     * Next time, the call will return 0.
     */
    QuerySchema *query();

    /**
     * @return a pointer to the used database connection or 0 if not set
     * You can call this method only once every time after doing parse().
     * Next time, the call will return 0.
     */
    Connection *db() const;

    /**
     * @return detailed information about last error.
     * If no error occurred ParserError isNull()
     */
    ParserError error() const;

    /**
     * @return the statement passed on the last @a parse method-call.
     */
    EscapedString statement() const;

    /**
     * @internal
     * sets the operation (only parser will need to call this)
     */
    void setOperation(OPCode op);

    /**
     * @internal
     * creates a new table (only parser will need to call this)
     */
    void createTable(const char *t);

    /**
     * @internal
     * sets @a query schema object (only parser will need to call this)
     */
//todo: other query types
    void setQuerySchema(QuerySchema *query);

    /**
     * @internal
     * @return query schema
     */
    QuerySchema *select() const;

    /**
     * @internal
     * INTERNAL use only: sets a error
     */
    void setError(const ParserError &err);

    /**
     * @return true if the @param str is an reserved
     * keyword (see tokens.cpp for a list of reserved
     * keywords).
     */
    bool isReservedKeyword(const QByteArray& str);

protected:
    void init();

    ParserError m_error; //!< detailed information about last error.
    class Private;
    Private * const d; //!< @internal d-pointer class.
};

}

//! Sends information about parser error @a error to debug output @a dbg.
PREDICATE_EXPORT QDebug operator<<(QDebug dbg, const Predicate::ParserError& error);

#endif

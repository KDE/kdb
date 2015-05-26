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

#ifndef KDB_PARSER_H
#define KDB_PARSER_H

#include <QObject>

#include "kdb_export.h"

class KDbConnection;
class KDbQuerySchema;
class KDbTableSchema;
class KDbEscapedString;

/**
 * Provides detailed i18n'ed error description about KDbParser.
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
     * Destructor.
     */
    ~KDbParserError();

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

class KDbParseInfoInternal;

/*! Info used on parsing. */
class KDB_EXPORT KDbParseInfo
{
public:
    ~KDbParseInfo();

    //! @return positions of tables/aliases having the same name @a tableOrAliasName.
    QList<int> tablesAndAliasesForName(const QString &tableOrAliasName) const;

    //! @return query schema for this parsing
    KDbQuerySchema* querySchema() const;

    //! @return error message for the parsing process
    QString errorMessage() const;

    //! Sets error message for the parsing process to @a message
    void setErrorMessage(const QString &message);

    //! @return detailed error description for the parsing process
    QString errorDescription() const;

    //! Sets detailed error description for the parsing process to @a description
    void setErrorDescription(const QString &description);

protected:
    //! Constructs parse info structure for query @a query.
    explicit KDbParseInfo(KDbQuerySchema *query);

    Q_DISABLE_COPY(KDbParseInfo)
    class Private;
    Private * const d;
};

/**
 * Parser for SQL statements.
 *
 * The best and prefeerred way to run queries is using the KDbParser functionality
 * and use the resulting KDbQuerySchema object since this offers a database-backend-independent
 * way to deal with SQL statements on the one hand and offers high level
 * functionality on the other. Also BLOBs like images are handled that way.
 *
 * For example if we like to use the SELECT statement
 * "SELECT dir.path, media.filename FROM dir, media WHERE dir.id=media.dirId AND media.id=%s"
 * we are able to use the @a KDbConnection::prepareStatement method which takes the type of
 * the statement (in our case @a KDbPreparedStatement::SelectStatement ), a list of fields (in
 * our case dir.path and media.filename) and returns a @a KDbPreparedStatement instance.
 * By using the @a KDbQuerySchema::addRelationship and @a KDbQuerySchema::addToWhereExpression methods
 * the SQL statement could be extended with relationships and WHERE expressions.
 *
 * For more, see @a KDbPreparedStatement and @a KDbConnection::selectStatement() . A more
 * complex example that looks at what the user has defined and carefully builds
 * @a KDbQuerySchema object, including the WHERE expression can be found in
 * the Query Designer's source code in the method @a KexiQueryDesignerGuiEditor::buildSchema().
 */
class KDB_EXPORT KDbParser
{
public:

    /**
     * The operation-code of the statement.
     */
    enum OPCode {
        OP_None, //!< No statement parsed or reseted.
        OP_Error, //!< Error while parsing.
        OP_CreateTable, //!< Create a table.
        OP_AlterTable, //!< Alter an existing table
        OP_Select, //!< Query-statement.
        OP_Insert, //!< Insert new content.
        OP_Update, //!< Update existing content.
        OP_Delete  //!< Delete existing content.
    };

    /**
     * constructs an empty object of the parser
     * @param connection is used for things like wildcard resolution. If 0 parser works in "pure mode"
     */
    explicit KDbParser(KDbConnection *connection);
    ~KDbParser();

    /**
     * Clears the parser's status and runs the parsing for a raw SQL statement @a sql .
     */
    bool parse(const KDbEscapedString &sql);

    /**
     * Clear the parser's status.
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
    KDbTableSchema *table();

    /**
     * @return a pointer to a query schema if 'SELECT ...' was called
     * or 0 on any other operation or error. Returned object is owned by you.
     * You can call this method only once every time after doing parse().
     * Next time, the call will return 0.
     */
    KDbQuerySchema *query();

    /**
     * @return a pointer to the used database connection or 0 if it is not set.
     * You can call this method only once every time after doing parse().
     * Next time, the call will return 0.
     */
    KDbConnection *connection() const;

    /**
     * @return detailed information about last error.
     * If no error occurred KDbParserError isNull()
     */
    KDbParserError error() const;

    /**
     * @return the statement passed on the last @a parse method-call.
     */
    KDbEscapedString statement() const;

    /**
     * @internal
     * sets the operation (only parser will need to call this)
     */
    void setOperation(OPCode op);

    /**
     * @internal
     * creates a new table with name @a name (only parser will need to call this)
     */
    void createTable(const QByteArray &name);

    /**
     * @internal
     * sets @a query schema object (only parser will need to call this)
     */
//! @todo other query types
    void setQuerySchema(KDbQuerySchema *query);

    /**
     * @internal
     * @return query schema
     */
    KDbQuerySchema *select() const;

    /**
     * @internal
     * INTERNAL use only: sets a error
     */
    void setError(const KDbParserError &err);

    /**
     * @return true if the @param str is an reserved
     * keyword (see tokens.cpp for a list of reserved
     * keywords).
     */
    bool isReservedKeyword(const QByteArray& str);

protected:
    void init();

    KDbParserError m_error; //!< detailed information about last error.
    class Private;
    Private * const d; //!< @internal d-pointer class.
};

//! Sends information about parser error @a error to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbParserError& error);

#endif

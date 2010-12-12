/* This file is part of the KDE project
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2003-2010 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef PREDICATE_RESULT_H
#define PREDICATE_RESULT_H

#include <Predicate/Error.h>
#include <Predicate/EscapedString.h>

namespace Predicate
{

class MessageHandler;

/*! Stores result of the operation.
*/
shared class export=PREDICATE_EXPORT Result
{
public:
    /*!
    @getter
    @return result code, defaults to 0.
    @setter
    Sets the result code if there was error.
    */
    data_member int code default=0 default_setter=ERR_OTHER;

    /*!
    @getter
    @return true if there is error.
    */
    data_member bool hasError default=false getter=isError no_setter;

    /*!
    @getter
    @return engine-specific last server-side operation result number.
    Use this to give users more precise information about the result.

    For example, use this for your driver - default implementation just returns 0.
    Note that this result value is not the same as the one returned
    by code() (Object::m_errno member)
    @sa serverMessage()
    */
    data_member int serverResultCode default=0;

    /*!
    @getter
    @return (localized) message if there was error.
    @setter
    Sets (localized) message to @a message.
    */
    data_member QString message;

    /*!
    @getter
    @return message title that sometimes is provided and prepended
    to the main warning/error message. Used by MessageHandler.
    */
    data_member QString messageTitle;

    data_member EscapedString errorSql;

    data_member EscapedString sql;

    /*!
    @getter
    @return message from server.
    Predicate library offers detailed result numbers using resultCode()
    and detailed result i18n-ed messages using message().
    This information is not engine-dependent (almost).
    Use this in your code to give users more information on the result of operatoim.
    This method returns (non-i18n-ed !) engine-specific message,
    if there was any error during last server-side operation,
    otherwise empty string.
    @setter
    Sets message from server.
    */
    data_member QString serverMessage;

    Result(int code, const QString& message);

    explicit Result(const QString& message);

    void prependMessage(int code, const QString& message);

    void prependMessage(const QString& message) { prependMessage(0, message); }

    //! Efficient clearing of the sql attribute, equivalent of setSql(QString()).
    void clearSql() {
        d->sql.clear();
    }
/*
    Used to store actually executed SQL statement.
    data_member QString sql no_getter no_setter;
*/
/*    data_member MessageHandler::MessageType errorType;
*/

#if 0
    /*! Clears error flag.
     Also calls drv_clearServerResultCode().
     You can reimplement this method in subclasses to clear even more members,
     but remember to also call Object::clearError(). */
    virtual void clearError();
#endif

    /*! @return sql string of actually executed SQL statement,
     usually using drv_executeSQL(). If there was error during executing SQL statement,
     before, that string is returned instead. */
    virtual EscapedString recentSQLString() const {
        return d->errorSql;
    }

protected:
    void init(int code, const QString& message);
#if 0
    /*! Interactively asks a question. Console or GUI can be used for this,
     depending on installed message handler. For GUI version, message boxes are used.
     See Predicate::MessageHandler::askQuestion() for details. */
    virtual MessageHandler::ButtonCode askQuestion(
            MessageHandler::QuestionType messageType,
            const QString& message,
            const QString &caption = QString(),
            MessageHandler::ButtonCode defaultResult = MessageHandler::Yes,
            const GuiItem &buttonYes = GuiItem(),
            const GuiItem &buttonNo = GuiItem(),
            const QString &dontShowAskAgainName = QString(),
            MessageHandler::Options options = 0,
            MessageHandler* msgHandler = 0);

    /*! Clears number of last server operation's result stored
     as a single integer. Formally, this integer should be set to value
     that means "NO ERRORS" or "OK". This method is called by clearError().
     For reimplementation. By default does nothing.
     \sa serverMessage()
    */
    virtual void drv_clearServerResultCode() {}
#endif
};

//! Interface for classes providing result.
class PREDICATE_EXPORT Resultable
{
public:
    Result result() const { return m_result; }

    void clearResult() { m_result = Result(); }

    /*! Stores previous error.
    */
//    void storePreviousError();

    /*!
    @return engine-specific last server-side operation result name, (name for serverResultCode()).
    Use this in your application to give users more information on what's up.

    Use this for your driver - default implementation just returns empty string.
    Note that this result name is not the same as the error message returned by Result::serverMessage() or Result::message().
    @sa Result::serverMessage()
    */
    virtual QString serverResultName() const;

protected:
#if 0
    /*!
    Previous server result code, for displaying.
    */
    int m_previousServerResultCode;

    int m_previousServerResultCode2;

    /*!
    Previous server result name, for displaying.
    */
    QString m_previousServerResultName;

    QString m_previousServerResultName2;
#endif
    Result m_result;
};

} // namespace Predicate

//! Sends result @a result to debug output @a dbg.
PREDICATE_EXPORT QDebug operator<<(QDebug dbg, const Predicate::Result& result);

#endif // PREDICATE_RESULT_H

/* This file is part of the KDE project
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2003-2015 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_RESULT_H
#define KDB_RESULT_H

#include "KDbError.h"
#include "KDbEscapedString.h"

#include <QCoreApplication>

class KDbMessageHandler;
class KDbMessageTitleSetter;

/*! Stores detailed information about result of recent operation.
*/
shared class export=KDB_EXPORT virtual_dtor operator== KDbResult
{
    Q_DECLARE_TR_FUNCTIONS(KDbResult)
public:
    /*!
    @getter
    @return result code, default is ERR_NONE (0).
    @setter
    Sets the result code if there was error.
    */
    data_member int code default=ERR_NONE default_setter=ERR_OTHER;

    /*!
    @getter
    @return an implementation-specific last server-side operation result number.
    Use this to give users more precise information about the result.

    For example, use this for your driver - default implementation just returns 0.
    Note that this value is not the same as the one returned by code().
    @sa serverMessage()
    */
    data_member int serverErrorCode default=0 no_setter;

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
    to the main warning/error message. Used by KDbMessageHandler.
    */
    data_member QString messageTitle;

    data_member KDbEscapedString errorSql;

    data_member KDbEscapedString sql;

    /*!
    @getter
    @return message from server.
    KDb framework offers detailed result numbers using resultCode() and detailed
    result i18n-ed messages using message(). These both are (almost) not engine-dependent.
    Use setServerMessage() to users more information on the result of operation that is
    non-i18n-ed and engine-specific, usually coming from the server server side.
    @setter
    Sets message from the server.
    */
    data_member QString serverMessage;

    data_member bool serverErrorCodeSet default=false no_getter no_setter;

    KDbResult(int code, const QString& message);

    explicit KDbResult(const QString& message);

    //! @return true if there is an error i.e. a nonempty message, error code other
    //!         than ERR_NONE or server result has been set.
    bool isError() const;

    //! Sets an implementation-specific error code of server-side operation.
    //! Use this to give users more precise information. Implies isError() == true.
    //! The only way to clear already set server result code is to create a new KDbResult object.
    void setServerErrorCode(int errorCode);

    //! Sets result code and prepends message to an existing message.
    void prependMessage(int code, const QString& message);

    //! Prepends message to an existing message.
    void prependMessage(const QString& message);

    //! Efficient clearing of the sql attribute, equivalent of setSql(QString()).
    void clearSql() {
        d->sql.clear();
    }

    /*! @return sql string of actually executed SQL statement,
     usually using drv_executeSQL(). If there was error during executing SQL statement,
     before, that string is returned instead. */
    virtual KDbEscapedString recentSQLString() const {
        return d->errorSql;
    }

protected:
    void init(int code, const QString& message);
#if 0
    /*! Interactively asks a question. Console or GUI can be used for this,
     depending on installed message handler. For GUI version, message boxes are used.
     See KDbMessageHandler::askQuestion() for details. */
    virtual KDbMessageHandler::ButtonCode askQuestion(
            KDbMessageHandler::QuestionType messageType,
            const QString& message,
            const QString &caption = QString(),
            KDbMessageHandler::ButtonCode defaultResult = KDbMessageHandler::Yes,
            const KDbGuiItem &buttonYes = KDbGuiItem(),
            const KDbGuiItem &buttonNo = KDbGuiItem(),
            const QString &dontShowAskAgainName = QString(),
            KDbMessageHandler::Options options = 0,
            KDbMessageHandler* msgHandler = 0);

    /*! Clears number of last server operation's result stored
     as a single integer. Formally, this integer should be set to value
     that means "NO ERRORS" or "OK". This method is called by clearError().
     For reimplementation. By default does nothing.
     @sa serverMessage()
    */
    virtual void drv_clearServerResultCode() {}
#endif
};

//! Interface for classes providing a result.
class KDB_EXPORT KDbResultable
{
public:
    virtual ~KDbResultable();

    KDbResult result() const;

    void clearResult();

    /*!
    @return engine-specific last server-side operation result name, a name for KDbResult::serverErrorCode().
    Use this in your application to give users more information on what's up.

    Use this for your driver - default implementation just returns empty string.
    Note that this result name is not the same as the error message returned by KDbResult::serverMessage() or KDbResult::message().
    @sa KDbResult::serverMessage()
    */
    virtual QString serverResultName() const;

protected:
    friend class KDbMessageTitleSetter;
    KDbResult m_result;
};

//! Sends result @a result to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbResult& result);

#endif // KDB_RESULT_H

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

#ifndef KDB_RESULT_H
#define KDB_RESULT_H

#include "KDbError.h"
#include "KDbEscapedString.h"

class KDbMessageHandler;

/*! Stores result of the operation.
*/
shared class export=KDB_EXPORT virtual_dtor KDbResult
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
    by code() (KDbObject::m_errno member)
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
    to the main warning/error message. Used by KDbMessageHandler.
    */
    data_member QString messageTitle;

    data_member KDbEscapedString errorSql;

    data_member KDbEscapedString sql;

    /*!
    @getter
    @return message from server.
    KDb library offers detailed result numbers using resultCode()
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

    KDbResult(int code, const QString& message);

    explicit KDbResult(const QString& message);

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
/*    data_member KDbMessageHandler::MessageType errorType;
*/

#if 0
    /*! Clears error flag.
     Also calls drv_clearServerResultCode().
     You can reimplement this method in subclasses to clear even more members,
     but remember to also call KDbObject::clearError(). */
    virtual void clearError();
#endif

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
    @return engine-specific last server-side operation result name, (name for serverResultCode()).
    Use this in your application to give users more information on what's up.

    Use this for your driver - default implementation just returns empty string.
    Note that this result name is not the same as the error message returned by KDbResult::serverMessage() or KDbResult::message().
    @sa KDbResult::serverMessage()
    */
    virtual QString serverResultName() const;

protected:
    KDbResult m_result;
};

//! Sends result @a result to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbResult& result);

#endif // KDB_RESULT_H

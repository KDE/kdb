/* This file is part of the KDE project
   Copyright (C) 2004-2010 Jarosław Staniek <staniek@kde.org>

   Contains parts of kmessagebox.h
   Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

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

#ifndef KDB_MSGHANDLER_H
#define KDB_MSGHANDLER_H

#include <QPointer>
#include <QHash>
#include <QVariant>
#include <QWidget>

#include "kdb_export.h"

class KDbResult;
class KDbResultable;

/*! Helper for setting temporary message title for an KDbResult object.
 Message title is a text prepended to error or warning messages.
 Use it as follows:
 @code
 KDbMessageTitleSetter ts(&m_result, tr("Terrible error occurred"));
 @endcode
 After leaving the current code block, myResultableObject's message title will be set back to the previous value.
*/
class KDB_EXPORT KDbMessageTitleSetter
{
public:
    explicit KDbMessageTitleSetter(KDbResult* result, const QString& message = QString());
    explicit KDbMessageTitleSetter(KDbResultable* resultable, const QString& message = QString());
    ~KDbMessageTitleSetter();

protected:
    KDbResult* m_result;
    QString m_prevMsgTitle;
};

//! An abstract class used to specify GUI information such as button texts tooltips and icons.
class KDbGuiItem : private QHash<QByteArray, QVariant>
{
public:
    KDbGuiItem() : QHash<QByteArray, QVariant>() {}
    ~KDbGuiItem() {}
    KDbGuiItem& setProperty(const QByteArray& name, const QVariant& value)
        { insert(name, value); return *this; }
    void removeProperty(const QByteArray& name) { remove(name); }
    bool isEmpty() const { return QHash<QByteArray, QVariant>::isEmpty(); }
    QVariant property(const QByteArray& name, const QVariant& defaultValue = QVariant()) const
        { return value(name, defaultValue); }
    bool hasProperty(const QByteArray& name) const { return contains(name); }
    QList<QByteArray> propertyNames() const { return keys(); }
    void clear() { QHash<QByteArray, QVariant>::clear(); }
};

/*! A prototype for Message Handler usable
 for reacting on messages sent by KDbObject object(s).
*/
class KDB_EXPORT KDbMessageHandler
{
public:
    //! Message types
    enum MessageType
    {
        Information = 1,
        Error = 2,
        Warning = 3,
        Sorry = 4,
        Fatal = 5
    };

    //! Question types
    enum QuestionType
    {
        QuestionYesNo = 1,
        QuestionYesNoCancel = 2,
        WarningYesNo = 3,
        WarningContinueCancel = 4,
        WarningYesNoCancel = 5
    };

    //! Button codes
    enum ButtonCode
    {
        Ok = 1,
        Cancel = 2,
        Yes = Ok,
        No = 3,
        Continue = 4
    };

    //! Message options
    enum Option
    {
        Notify = 1,        ///< Emit a KNotify event
        AllowLink = 2,     ///< The message may contain links.
        Dangerous = 4      ///< The action to be confirmed by the dialog is a potentially destructive one
    };
    Q_DECLARE_FLAGS(Options, Option)

    /*! Constructs message handler, @a parent is a widget that will be a parent
     for displaying gui elements (e.g. message boxes). Can be 0 for non-gui usage. */
    explicit KDbMessageHandler(QWidget *parent = 0);

    virtual ~KDbMessageHandler();

    /*! @return true if the handler is enables so messages are not blocked.
     @see setEnabled(bool) */
    bool messagesEnabled() const;

    /*! Enables or disabled the handler to block/unblock its messages.
     Sometimes both lower- and higher-level messages are received,
     what is not optimal as only one of them should be displayed (e.g. a higher level
     with details). This can be solved by calling setEnabled(false) shortly before
     an action that can send the unwanted message. Afterwards messages can be enabled again
     by calling setEnabled(true).
     By default messages are enabled. */
    void setMessagesEnabled(bool enable);

    /*! Shows error message with @a title (it is not caption) and details. */
    virtual void showErrorMessage(
        KDbMessageHandler::MessageType messageType,
        const QString &message,
        const QString &details = QString(),
        const QString &caption = QString()
    ) = 0;

    /*! Shows error message with @a message text. Existing error message from @a obj object
     is also copied, if present. */
    virtual void showErrorMessage(
        const KDbResult& result,
        KDbMessageHandler::MessageType messageType = Error,
        const QString& message = QString(),
        const QString& caption = QString()
    ) = 0;

    /*! Interactively asks a question. For GUI version, message boxes are used.
     @a defaultResult is returned in case when no message handler is installed.
     @a message should contain translated string.
     Value of ButtonCode is returned.
     Reimplement this. This implementation does nothing, just returns @a defaultResult. */
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

    //! @return message redirection for this handler or 0 if there is no redirection.
    KDbMessageHandler* redirection();

    //! @overload KDbMessageHandler* redirection()
    const KDbMessageHandler* redirection() const;

    /*! Sets redirection of all messages for this handler to @a otherHandler.
     Passing 0 removes redirection. Setting new redirection replaces previous. */
    void setRedirection(KDbMessageHandler *otherHandler);

protected:
    /*! @return a widget that will be parent for displaying gui elements (e.g. message boxes).
     Can be 0 for non-gui cases. */
    QWidget *parentWidget();

    Q_DISABLE_COPY(KDbMessageHandler)
    class Private;
    Private * const d;
};

#endif

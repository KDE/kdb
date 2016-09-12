/* This file is part of the KDE project
   Copyright (C) 2004-2015 Jarosław Staniek <staniek@kde.org>

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

#include "KDbMessageHandler.h"

#include "KDbResult.h"

class KDbMessageGuard::Private
{
public:
    Private() {}
    const KDbResult *result;
    KDbResultable *resultable;
    KDbMessageHandler *handler;
private:
    Q_DISABLE_COPY(Private)
};

KDbMessageGuard::KDbMessageGuard(KDbResultable *resultable)
    : d(new Private)
{
    Q_ASSERT(resultable);
    d->result = 0;
    d->resultable = resultable;
    d->handler = 0;
}

KDbMessageGuard::KDbMessageGuard(const KDbResult &result, KDbMessageHandler *handler)
 : d(new Private)
{
    Q_ASSERT(handler);
    d->result = &result;
    d->resultable = 0;
    d->handler = handler;
}

KDbMessageGuard::~KDbMessageGuard()
{
    if (d->handler && d->result && d->result->isError()) { // variant 1
        d->handler->showErrorMessage(*d->result);
    }
    else if (d->resultable && d->resultable->messageHandler() && d->resultable->result().isError()){ // variant 2
        d->resultable->messageHandler()->showErrorMessage(d->resultable->result());
    }
    delete d;
}

//------------------------------------------------

KDbMessageTitleSetter::KDbMessageTitleSetter(KDbResult* result, const QString& message)
        : m_result(result)
        , m_prevMsgTitle(result->messageTitle())
{
    m_result->setMessageTitle(message);
}

KDbMessageTitleSetter::KDbMessageTitleSetter(KDbResultable* resultable, const QString& message)
        : m_result(&resultable->m_result)
        , m_prevMsgTitle(resultable->result().messageTitle())
{
    m_result->setMessageTitle(message);
}

KDbMessageTitleSetter::~KDbMessageTitleSetter()
{
    m_result->setMessageTitle(m_prevMsgTitle);
}

//------------------------------------------------

class KDbMessageHandler::Private
{
public:
    Private()
     : messageRedirection(0)
     , enableMessages(true)
    {
    }

    QPointer<QWidget> messageHandlerParentWidget;
    KDbMessageHandler *messageRedirection;
    bool enableMessages;
private:
    Q_DISABLE_COPY(Private)
};

//------------------------------------------------

KDbMessageHandler::KDbMessageHandler(QWidget *parent)
 : d(new Private)
{
    d->messageHandlerParentWidget = parent;
}

KDbMessageHandler::~KDbMessageHandler()
{
    delete d;
}

bool KDbMessageHandler::messagesEnabled() const
{
    return d->enableMessages;
}

void KDbMessageHandler::setMessagesEnabled(bool enable)
{
    d->enableMessages = enable;
}

KDbMessageHandler::ButtonCode KDbMessageHandler::askQuestion(
    KDbMessageHandler::QuestionType messageType,
    const QString& message,
    const QString &caption,
    KDbMessageHandler::ButtonCode defaultResult,
    const KDbGuiItem &buttonYes,
    const KDbGuiItem &buttonNo,
    const QString &dontShowAskAgainName,
    KDbMessageHandler::Options options,
    KDbMessageHandler* msgHandler)
{
    if (d->enableMessages && d->messageRedirection) {
        return d->messageRedirection->askQuestion(messageType, message, caption, defaultResult,
                                                  buttonYes, buttonNo, dontShowAskAgainName,
                                                  options, msgHandler);
    }
    return defaultResult;
}

KDbMessageHandler* KDbMessageHandler::redirection()
{
    return d->messageRedirection;
}

const KDbMessageHandler* KDbMessageHandler::redirection() const
{
    return d->messageRedirection;
}

void KDbMessageHandler::setRedirection(KDbMessageHandler *otherHandler)
{
    d->messageRedirection = otherHandler;
}

QWidget* KDbMessageHandler::parentWidget()
{
    return d->messageHandlerParentWidget;
}

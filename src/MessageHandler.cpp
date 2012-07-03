/* This file is part of the KDE project
   Copyright (C) 2004-2010 Jarosław Staniek <staniek@kde.org>

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

#include "MessageHandler.h"

#include "Result.h"

using namespace Predicate;

MessageTitleSetter::MessageTitleSetter(Result* result, const QString& message)
        : m_result(result)
        , m_prevMsgTitle(result->messageTitle())
{
    m_result->setMessageTitle(message);
}

MessageTitleSetter::~MessageTitleSetter()
{
    m_result->setMessageTitle(m_prevMsgTitle);
}

//------------------------------------------------

MessageHandler::MessageHandler(QWidget *parent)
        : m_messageHandlerParentWidget(parent)
        , m_enableMessages(true)
{
}

MessageHandler::~MessageHandler()
{
}

MessageHandler::ButtonCode MessageHandler::askQuestion(
    MessageHandler::QuestionType messageType,
    const QString& message,
    const QString &caption,
    MessageHandler::ButtonCode defaultResult,
    const GuiItem &buttonYes,
    const GuiItem &buttonNo,
    const QString &dontShowAskAgainName,
    MessageHandler::Options options,
    MessageHandler* msgHandler)
{
    Q_UNUSED(messageType);
    Q_UNUSED(message);
    Q_UNUSED(caption);
    Q_UNUSED(buttonYes);
    Q_UNUSED(buttonNo);
    Q_UNUSED(dontShowAskAgainName);
    Q_UNUSED(options);
    Q_UNUSED(msgHandler);
    return defaultResult;
}

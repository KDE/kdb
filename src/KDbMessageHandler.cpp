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

#include "KDbMessageHandler.h"

#include "KDbResult.h"

KDbMessageTitleSetter::KDbMessageTitleSetter(KDbResult* result, const QString& message)
        : m_result(result)
        , m_prevMsgTitle(result->messageTitle())
{
    m_result->setMessageTitle(message);
}

KDbMessageTitleSetter::~KDbMessageTitleSetter()
{
    m_result->setMessageTitle(m_prevMsgTitle);
}

//------------------------------------------------

KDbMessageHandler::KDbMessageHandler(QWidget *parent)
        : m_messageHandlerParentWidget(parent)
        , m_enableMessages(true)
{
}

KDbMessageHandler::~KDbMessageHandler()
{
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

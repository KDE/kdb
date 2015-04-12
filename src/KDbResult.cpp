/* This file is part of the KDE project
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

#include "KDbResult.h"
//#include "KDbMessageHandler.h"

#include <QtDebug>

#define ERRMSG(a) \
    { if (m_msgHandler) m_msgHandler->showErrorMessage(a); }

#define STORE_PREV_ERR \

KDbResult::KDbResult(int code, const QString& message)
    : d(new Data)
{
    init(code, message);
}

KDbResult::KDbResult(const QString& message)
    : d(new Data)
{
    init(ERR_OTHER, message);
}

KDbResult::~KDbResult()
{
}

void KDbResult::init(int code, const QString& message)
{
    d->code = code;
    d->errorSql = d->sql;
    if (d->code == ERR_OTHER && message.isEmpty())
        d->message = QObject::tr("Unspecified error encountered");
    else
        d->message = message;
    d->hasError = d->code != ERR_NONE;
//! @todo
/*    if (m_hasError)
        ERRMSG(this);*/
}

void KDbResult::prependMessage(int code, const QString& message)
{
    if (d->code == 0) {
        if (code == 0)
            d->code = ERR_OTHER;
        else
            d->code = code;
        d->hasError = d->code != ERR_NONE;
    }
    if (!message.isEmpty()) {
        if (d->message.isEmpty())
            d->message = message;
        else
            d->message = message + QLatin1Char(' ') + d->message;
    }
//    if (m_hasError)
#ifdef __GNUC__
#warning TODO ERRMSG(this);
#else
#pragma WARNING(TODO ERRMSG(this);)
#endif
}

QDebug operator<<(QDebug dbg, const KDbResult& result)
{
    if (result.isError()) {
        dbg.nospace() << "KDbResult:";
        dbg.space() << "CODE=" << result.code();
        if (!result.message().isEmpty())
            dbg.space() << "MESSAGE=" << result.message();
        if (!result.messageTitle().isEmpty())
            dbg.space() << "TITLE=" << result.messageTitle();
        if (!result.sql().isEmpty())
            dbg.space() << "SQL=" << result.sql();
        if (!result.errorSql().isEmpty())
            dbg.space() << "ERR_SQL=" << result.errorSql();
        dbg.space() << "SERVER_RESULT=(CODE:" << result.serverResultCode() /*<< "NAME:" << result.serverResultName()*/ << ")";
        if (!result.serverMessage().isEmpty())
            dbg.nospace() << "MESSAGE:" << result.serverMessage();
        /*dbg.space() << "PREV_SERVER_RESULT=(CODE:" << result.previousServerResultCode() << "NAME:" <<
             result.previousServerResultName() << ")";*/
    } else {
        dbg.nospace() << "KDbResult: OK";
    }
    return dbg.space();
}
/*
KDbMessageHandler::ButtonCode KDbObject::askQuestion(
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
    if (msgHandler)
        return msgHandler->askQuestion(messageType, message, caption, defaultResult, buttonYes, buttonNo,
                                       dontShowAskAgainName, options);

    if (m_msgHandler)
        return m_msgHandler->askQuestionInternal(messageType, message, caption, defaultResult, buttonYes, buttonNo,
                                                 dontShowAskAgainName, options);

    return defaultResult;
}*/

/*
void KDbResultable::storePreviousError()
{
    m_previousServerResultCode = m_previousServerResultCode2;
    m_previousServerResultName = m_previousServerResultName2;
    m_previousServerResultCode2 = m_serverResultCode;
    m_previousServerResultName2 = m_serverResultName;
    qDebug() << "Object ERROR:" << m_previousServerResultCode2 << ":" << m_previousServerResultName2;
}*/

QString KDbResultable::serverResultName() const
{
    return QString();
}

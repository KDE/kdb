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

#include "Result.h"
//#include "MessageHandler.h"

#include <QtDebug>

using namespace Predicate;

#define ERRMSG(a) \
    { if (m_msgHandler) m_msgHandler->showErrorMessage(a); }

#define STORE_PREV_ERR \

Result::Result(int code, const QString& message)
    : d(new Data)
{
    init(code, message);
}

Result::Result(const QString& message)
    : d(new Data)
{
    init(ERR_OTHER, message);
}

Result::~Result()
{
}

void Result::init(int code, const QString& message)
{
    storePreviousError();
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

void Result::storePreviousError()
{
    d->previousServerResultCode = d->previousServerResultCode2;
    d->previousServerResultName = d->previousServerResultName2;
    d->previousServerResultCode2 = d->serverResultCode;
    d->previousServerResultName2 = d->serverResultName;
    qDebug() << "Object ERROR:" << d->previousServerResultCode2 << ":" << d->previousServerResultName2;
}

// void Object::setError(Predicate::Object *obj, int code, const QString& prependMessage)
// {
//     if (obj && (obj->errorNum() != 0 || !obj->serverErrorMsg().isEmpty())) {
//         STORE_PREV_ERR;
// 
//         m_errno = obj->errorNum();
//         m_hasError = obj->error();
//         if (m_errno == 0) {
//             m_errno = code;
//             m_hasError = true;
//         }
//         m_errMsg = (prependMessage.isEmpty() ? QString() : (prependMessage + " "))
//                    + obj->errorMsg();
//         m_sql = obj->m_sql;
//         m_errorSql = obj->m_errorSql;
//         m_serverResult = obj->serverResult();
//         if (m_serverResult == 0) //try copied
//             m_serverResult = obj->m_serverResult;
//         m_serverResultName = obj->serverResultName();
//         if (m_serverResultName.isEmpty()) //try copied
//             m_serverResultName = obj->m_serverResultName;
//         m_serverErrorMsg = obj->serverErrorMsg();
//         if (m_serverErrorMsg.isEmpty()) //try copied
//             m_serverErrorMsg = obj->m_serverErrorMsg;
//         //override
//         if (code != 0 && code != ERR_OTHER)
//             m_errno = code;
//         if (m_hasError)
//             ERRMSG(this);
//     } else {
//         setError(code != 0 ? code : ERR_OTHER, prependMessage);
//     }
// }
// 
// void Object::clearError()
// {
//     m_errno = 0;
//     m_hasError = false;
//     m_errMsg.clear();
//     m_sql.clear();
//     m_errorSql.clear();
//     m_serverResult = 0;
//     m_serverResultName.clear();
//     m_serverErrorMsg.clear();
//     drv_clearServerResult();
// }

void Result::prependMessage(int code, const QString& message)
{
    storePreviousError();
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
            d->message = message + " " + d->message;
    }
//    if (m_hasError)
#warning TODO ERRMSG(this);
}

QDebug operator<<(QDebug dbg, const Result& result)
{
    if (result.isError()) {
        dbg.nospace() << "Predicate::Result:";
        dbg.space() << "CODE=" << result.code();
        if (!result.message().isEmpty())
            dbg.space() << "MESSAGE=" << result.message();
        if (!result.messageTitle().isEmpty())
            dbg.space() << "TITLE=" << result.messageTitle();
        if (!result.sql().isEmpty())
            dbg.space() << "SQL=" << result.sql();
        if (!result.errorSql().isEmpty())
            dbg.space() << "ERR_SQL=" << result.errorSql();
        dbg.space() << "SERVER_RESULT=(CODE:" << result.serverResultCode() << "NAME:" << result.serverResultName() << ")";
        if (!result.serverMessage().isEmpty())
            dbg.nospace() << "MESSAGE:" << result.serverMessage();
        dbg.space() << "PREV_SERVER_RESULT=(CODE:" << result.previousServerResultCode() << "NAME:" <<
             result.previousServerResultName() << ")";
    } else {
        dbg.nospace() << "Predicate::Result: OK";
    }
    return dbg.space();
}
/*
MessageHandler::ButtonCode Object::askQuestion(
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
    if (msgHandler)
        return msgHandler->askQuestion(messageType, message, caption, defaultResult, buttonYes, buttonNo,
                                       dontShowAskAgainName, options);

    if (m_msgHandler)
        return m_msgHandler->askQuestion(messageType, message, caption, defaultResult, buttonYes, buttonNo,
                                         dontShowAskAgainName, options);

    return defaultResult;
}*/

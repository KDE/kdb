/* This file is part of the KDE project
   Copyright (C) 2003-2016 Jarosław Staniek <staniek@kde.org>

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
#include "KDbMessageHandler.h"
#include "kdb_debug.h"
#include "KDb_p.h"

#if 0 // needed by lupdate to avoid "Qualifying with unknown namespace/class" because header is generated
class KDbResult { Q_DECLARE_TR_FUNCTIONS(KDbResult) };
class KDbResultable {};
#endif

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
        d->message = tr("Unspecified error encountered");
    else
        d->message = message;
//! @todo
/*    if (m_hasError)
        ERRMSG(this);*/
}

bool KDbResult::isError() const
{
    return d->code != ERR_NONE
            || d->serverErrorCodeSet
            || !d->message.isEmpty()
            || !d->messageTitle.isEmpty()
            || !d->errorSql.isEmpty()
            || !d->serverMessage.isEmpty();
}

void KDbResult::setServerErrorCode(int errorCode)
{
    d->serverErrorCode = errorCode;
    d->serverErrorCodeSet = true;
}

void KDbResult::prependMessage(int code, const QString& message)
{
    if (d->code == ERR_NONE) {
        if (code == ERR_NONE)
            d->code = ERR_OTHER;
        else
            d->code = code;
    }
    if (!message.isEmpty()) {
        if (d->message.isEmpty())
            d->message = message;
        else
            d->message = message + QLatin1Char(' ') + d->message;
    }
//    if (m_hasError)
//! @todo IMPORTANT: ERRMSG(this);
}

void KDbResult::prependMessage(const QString& message)
{
    prependMessage(ERR_NONE, message);
}

QDebug operator<<(QDebug dbg, const KDbResult& result)
{
    if (result.isError()) {
        dbg.nospace() << "KDbResult:";
        dbg.nospace() << " CODE=" << result.code();
        if (!result.message().isEmpty())
            dbg.nospace() << " MESSAGE=" << result.message();
        if (!result.messageTitle().isEmpty())
            dbg.nospace() << " TITLE=" << result.messageTitle();
        if (!result.sql().isEmpty())
            dbg.nospace() << " SQL=" << result.sql();
        if (!result.errorSql().isEmpty())
            dbg.nospace() << " ERR_SQL=" << result.errorSql();
        dbg.nospace() << " SERVER_ERROR_CODE=" << result.serverErrorCode() /*<< "NAME:" << result.serverResultName()*/;
        if (!result.serverMessage().isEmpty())
            dbg.nospace() << " SERVER_MESSAGE=" << result.serverMessage();
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
    kdbDebug() << "Object ERROR:" << m_previousServerResultCode2 << ":" << m_previousServerResultName2;
}*/

class Q_DECL_HIDDEN KDbResultable::Private
{
public:
    Private() : messageHandler(nullptr)
    {
    }
    Private(const Private &other) {
        copy(other);
    }
#define KDbResultablePrivateArgs(o) std::tie(o.messageHandler)
    void copy(const Private &other) {
        KDbResultablePrivateArgs((*this)) = KDbResultablePrivateArgs(other);
    }
    bool operator==(const Private &other) const {
        return KDbResultablePrivateArgs((*this)) == KDbResultablePrivateArgs(other);
    }
    KDbMessageHandler* messageHandler;
};

KDbResultable::KDbResultable()
 : d(new Private)
{
}

KDbResultable::KDbResultable(const KDbResultable &other)
    : m_result(other.m_result)
    , d(new Private(*other.d))
{
}

KDbResultable::~KDbResultable()
{
    delete d;
}

KDbResultable& KDbResultable::operator=(const KDbResultable &other)
{
    d->messageHandler = other.messageHandler();
    m_result = other.m_result;
    return *this;
}

bool KDbResultable::operator==(const KDbResultable &other) const
{
    return m_result == other.m_result && *d == *other.d;
}

KDbResult KDbResultable::result() const
{
    return m_result;
}

void KDbResultable::clearResult()
{
    m_result = KDbResult();
}

QString KDbResultable::serverResultName() const
{
    return QString();
}

void KDbResultable::setMessageHandler(KDbMessageHandler *handler)
{
    d->messageHandler = handler;
}

KDbMessageHandler* KDbResultable::messageHandler() const
{
    return d->messageHandler;
}

void KDbResultable::showMessage()
{
    if (d->messageHandler && m_result.isError()) {
        d->messageHandler->showErrorMessage(m_result);
    }
}

QDebug operator<<(QDebug dbg, const KDbResultInfo &info)
{
    dbg.nospace() << "ResultInfo(";
    dbg.space() << "success:" << info.success
                << "allowToDiscardChanges:" << info.allowToDiscardChanges
                << "message:" << info.message
                << "description:" << info.description
                << "column:" << info.column;
    dbg.nospace() << ")";
    return dbg.space();
}

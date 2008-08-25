/* This file is part of the KDE project
   Copyright (C) 2006 Jarosław Staniek <staniek@kde.org>

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

#ifndef PREDICATE_UTILS_P_H
#define PREDICATE_UTILS_P_H

#include <QTimer>
#include <QWaitCondition>

#include <QProgressDialog>

#include "MessageHandler.h"
#include "ConnectionData.h"

class ConnectionTestThread;

class ConnectionTestDialog : protected QProgressDialog
{
    Q_OBJECT
public:
    ConnectionTestDialog(QWidget* parent,
                         const Predicate::ConnectionData& data, Predicate::MessageHandler& msgHandler);
    virtual ~ConnectionTestDialog();

    int exec();

    void error(Predicate::Object *obj);

protected slots:
    void slotTimeout();
    virtual void reject();

protected:
    ConnectionTestThread* m_thread;
    Predicate::ConnectionData m_connData;
    QTimer m_timer;
    Predicate::MessageHandler* m_msgHandler;
    uint m_elapsedTime;
    Predicate::Object *m_errorObj;
    QWaitCondition m_wait;
bool m_stopWaiting : 1;
};

#endif

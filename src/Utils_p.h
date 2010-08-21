/* This file is part of the KDE project
   Copyright (C) 2006-2010 Jarosław Staniek <staniek@kde.org>

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
#include <QThread>
#include <QProgressDialog>

#include <Predicate/MessageHandler.h>
#include <Predicate/ConnectionData.h>

namespace Predicate {
class Driver;

class ConnectionTestDialog;

class ConnectionTestThread : public QThread
{
    Q_OBJECT
public:
    ConnectionTestThread(ConnectionTestDialog *dlg, const ConnectionData& connData);
    virtual void run();
signals:
    void error(const QString& msg, const QString& details);
protected:
    void emitError(const Result& result);

    ConnectionTestDialog* m_dlg;
    ConnectionData m_connData;
    Driver *m_driver;
};

class ConnectionTestDialog : public QProgressDialog
{
    Q_OBJECT
public:
    ConnectionTestDialog(QWidget* parent, const ConnectionData& data, MessageHandler& msgHandler);
    virtual ~ConnectionTestDialog();

    int exec();

public slots:
    void error(const QString& msg, const QString& details);

protected slots:
    void slotTimeout();
    virtual void reject();

protected:
    ConnectionTestThread* m_thread;
    ConnectionData m_connData;
    QTimer m_timer;
    MessageHandler* m_msgHandler;
    uint m_elapsedTime;
    bool m_error;
    QString m_msg;
    QString m_details;
    bool m_stopWaiting;
};

} // namespace Predicate

#endif

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

#include "KDbTableSchemaChangeListener.h"
#include "KDbConnection.h"
#include "KDbConnection_p.h"

class Q_DECL_HIDDEN KDbTableSchemaChangeListener::Private
{
public:
    Private() {}
    QString name;
    Q_DISABLE_COPY(Private)
};

KDbTableSchemaChangeListener::KDbTableSchemaChangeListener()
 : d(new Private)
{
}

KDbTableSchemaChangeListener::~KDbTableSchemaChangeListener()
{
    delete d;
}

QString KDbTableSchemaChangeListener::name() const
{
    return d->name;
}

void KDbTableSchemaChangeListener::setName(const QString &name)
{
    d->name = name;
}

// static
void KDbTableSchemaChangeListener::registerForChanges(KDbConnection *conn,
                                                      KDbTableSchemaChangeListener* listener,
                                                      const KDbTableSchema* table)
{
    QSet<KDbTableSchemaChangeListener*>* listeners = conn->d->tableSchemaChangeListeners.value(table);
    if (!listeners) {
        listeners = new QSet<KDbTableSchemaChangeListener*>();
        conn->d->tableSchemaChangeListeners.insert(table, listeners);
    }
    listeners->insert(listener);
}

// static
void KDbTableSchemaChangeListener::unregisterForChanges(KDbConnection *conn,
                                         KDbTableSchemaChangeListener* listener,
                                         const KDbTableSchema* table)
{
    QSet<KDbTableSchemaChangeListener*>* listeners = conn->d->tableSchemaChangeListeners.value(table);
    if (!listeners)
        return;
    listeners->remove(listener);
}

// static
void KDbTableSchemaChangeListener::unregisterForChanges(KDbConnection *conn,
                                         KDbTableSchemaChangeListener* listener)
{
    foreach(QSet<KDbTableSchemaChangeListener*> *listeners, conn->d->tableSchemaChangeListeners) {
        listeners->remove(listener);
    }
}

// static
QList<KDbTableSchemaChangeListener*> KDbTableSchemaChangeListener::listeners(
        const KDbConnection *conn, const KDbTableSchema* table)
{
    //kdbDebug() << d->tableSchemaChangeListeners.count();
    QSet<KDbTableSchemaChangeListener*>* set = conn->d->tableSchemaChangeListeners.value(table);
    return set ? set->toList() : QList<KDbTableSchemaChangeListener*>();
}

// static
tristate KDbTableSchemaChangeListener::closeListeners(KDbConnection *conn, const KDbTableSchema* table)
{
    QSet<KDbTableSchemaChangeListener*> *listeners = conn->d->tableSchemaChangeListeners.value(table);
    if (!listeners) {
        return true;
    }

    //try to close every window
    tristate res = true;
    QList<KDbTableSchemaChangeListener*> list(listeners->toList());
    foreach (KDbTableSchemaChangeListener* listener, list) {
        const tristate localResult = listener->closeListener();
        if (localResult != true) {
            res = localResult;
        }
    }
    return res;
}

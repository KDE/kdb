/* This file is part of the KDE project
   Copyright (C) 2003-2010 Jarosław Staniek <staniek@kde.org>

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

#include <kdebug.h>
#include <kcomponentdata.h>

#include <KDbDriverManager>
#include <KDbDriver>
#include <KDbConnection>
#include <KDbCursor>

int main(int argc, char * argv[])
{
    KComponentData componentData("newapi");
    KDbDriverManager manager;
    QStringList names = manager.driverNames();
    qDebug() << "DRIVERS: ";
    for (QStringList::ConstIterator it = names.constBegin(); it != names.constEnd() ; ++it)
        qDebug() << *it;
    if (manager.error()) {
        qDebug() << manager.errorMsg();
        return 1;
    }

    //get driver
    KDbDriver *driver = manager.driver("mySQL");
    if (manager.error()) {
        qDebug() << manager.errorMsg();
        return 1;
    }

    //connection data that can be later reused
    KDbConnectionData conn_data;

    conn_data.userName = "root";
    if (argc > 1)
        conn_data.password = argv[1];
    else
        conn_data.password = "mysql";
    conn_data.hostName = "localhost";

    KDbConnection *conn = driver->createConnection(conn_data);
    if (driver->error()) {
        qDebug() << driver->errorMsg();
        return 1;
    }
    if (!conn->connect()) {
        qDebug() << conn->errorMsg();
        return 1;
    }

    if (!conn->useDatabase("test")) {
        qDebug() << "use db:" << conn->errorMsg();
        return 1;
    }

    qDebug() << "Creating first cursor";
    KDbCursor *c = conn->executeQuery("select * from Applications");
    if (!c) qDebug() << conn->errorMsg();
    qDebug() << "Creating second cursor";
    KDbCursor *c2 = conn->executeQuery("select * from Applications");
    if (!c2) qDebug() << conn->errorMsg();

    QStringList l = conn->databaseNames();
    if (l.isEmpty()) qDebug() << conn->errorMsg();
    qDebug() << "Databases:";
    for (QStringList::ConstIterator it = l.constBegin(); it != l.constEnd() ; ++it)
        qDebug() << *it;

    if (c) {
        while (c->moveNext()) {
            qDebug() << "KDbCursor: Value(0)" << c->value(0).toString();
            qDebug() << "KDbCursor: Value(1)" << c->value(1).toString();
        }
        qDebug() << "KDbCursor error:" << c->errorMsg();
    }
    if (c2) {
        while (c2->moveNext()) {
            qDebug() << "Cursor2: Value(0)" << c2->value(0).toString();
            qDebug() << "Cursor2: Value(1)" << c2->value(1).toString();
        }
    }
    if (c) {
        qDebug() << "KDbCursor::prev";
        while (c->movePrev()) {
            qDebug() << "KDbCursor: Value(0)" << c->value(0).toString();
            qDebug() << "KDbCursor: Value(1)" << c->value(1).toString();

        }
        qDebug() << "up/down";
        c->moveNext();
        qDebug() << "KDbCursor: Value(0)" << c->value(0).toString();
        qDebug() << "KDbCursor: Value(1)" << c->value(1).toString();
        c->moveNext();
        qDebug() << "KDbCursor: Value(0)" << c->value(0).toString();
        qDebug() << "KDbCursor: Value(1)" << c->value(1).toString();
        c->movePrev();
        qDebug() << "KDbCursor: Value(0)" << c->value(0).toString();
        qDebug() << "KDbCursor: Value(1)" << c->value(1).toString();
        c->movePrev();
        qDebug() << "KDbCursor: Value(0)" << c->value(0).toString();
        qDebug() << "KDbCursor: Value(1)" << c->value(1).toString();

    }
#if 0
    KDbTable *t = conn->tableSchema("persons");
    if (t)
        t->debug();
    t = conn->tableSchema("cars");
    if (t)
        t->debug();

// conn->tableNames();

    if (!conn->disconnect()) {
        qDebug() << conn->errorMsg();
        return 1;
    }
    debug("before del");
    delete conn;
    debug("after del");
#endif
    return 0;
}

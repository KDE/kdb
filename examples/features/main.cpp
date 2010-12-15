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

#include <QApplication>
#include <QFileInfo>
#include <QPointer>
#include <QtDebug>
#include <QTextStream>

#include <Predicate/DriverManager.h>
#include <Predicate/Driver.h>
#include <Predicate/Connection.h>
#include <Predicate/Cursor.h>
#include <Predicate/Field.h>
#include <Predicate/TableSchema.h>
#include <Predicate/QuerySchema.h>
#include <Predicate/IndexSchema.h>
#include <Predicate/parser/Parser.h>
#include <Predicate/Utils.h>

#include <iostream>

using namespace std;

QByteArray prgname;
QString db_name;
QString drv_name;
QString test_name;
int cursor_options = 0;
bool db_name_required = true;

Predicate::ConnectionData conn_data;
#warning replace QPointer<Predicate::Connection> conn;
Predicate::Connection* conn = 0;
#warning replace QPointer<Predicate::Driver> driver;
Predicate::Driver* driver;
QApplication *app = 0;
//qtonly KComponentData *instance = 0;

#include "dbcreation_test.h"
#include "cursors_test.h"
#include "schema_test.h"
#include "tables_test.h"
#ifndef NO_GUI
# include "tableview_test.h"
#endif
#include "parser_test.h"
#include "dr_prop_test.h"

static int finish(int code)
{
    qDebug() << "main:" << test_name << "test:" << (code==0?"PASSED":"ERROR");
    if (code != 0 && conn) {
        qDebug() << "main:" << conn->result();
        conn->disconnect();
        delete conn;
    }
    return code;
}

//! @return true if option @a option or @a shortOption is found
//! Removes the option.
static bool takeOption(QStringList &args, const QString &option,
                       const QString &shortOption = QString())
{
    return args.removeOne(QLatin1String("-") + (shortOption.isEmpty() ? option : shortOption))
        || args.removeOne(QLatin1String("--") + option);
}

//! @return next element after option @a option or @a shortOption, what should mean parameter
//! Removes option and its argument.
static QString takeOptionWithArg(QStringList &args, const QString &option,
                                 const QString &shortOption = QString())
{
    int index = args.indexOf(QLatin1String("-") + (shortOption.isEmpty() ? option : shortOption));
    if (index == -1)
        index = args.indexOf(QLatin1String("--") + option);
    if (index == -1)
        return QString();
    args.removeAt(index);
    return args.takeAt(index); // option's argument
}

static void showHelp()
{
    QTextStream s(stdout);
    s <<
"predicatefeaturestest, version " PREDICATE_VERSION_STRING
"\n"
"\nA set of tests for the Predicate library API."
"\nEvery test is mostly driver-independent."
"\n (c) 2003-2010, Kexi Team"
"\n (c) 2003-2006, OpenOffice Software LLC."
"\n"
"\nUsage: predicatefeaturestest --test <test_name> [options]"
"\n                             driver_name [db_name] [sql_statement]"
"\n"
"\nGeneric options:"
"\n  --help                    Show help about options"
"\n"
"\nOptions:"
"\n  -t, --test <test_name>    Specifies test to execute. Required."
"\n                            Available tests:"
"\n                            - cursors: test for cursors behaviour"
"\n                            - schema: test for db schema retrieving"
"\n                            - dbcreation: test for new db creation"
"\n                            - tables: test for tables creation and data"
"\n                                      inserting"
//"\n                            - tableview: test for KexiDataTableView data-aware
//"\n                               widget
"\n                            - parser: test for parsing sql statements,"
"\n                                      returns debug string for a given"
"\n                                      sql statement or error message"
"\n                            - dr_prop: shows properties of selected driver"
"\n  -h, --host <name>         Host name to use when connecting to server-based"
"\n                            backends."
"\n  --buffered-cursors        Optional switch: turns cursors used in any tests"
"\n                            to be buffered"
"\n  -p, --password <name>     Password to use when connecting to server-based"
"\n                            backends."
"\n  -P, --port <number>       Port number to use when connecting to server-based"
"\n                            backends."
"\n  --query-params <params>   Query parameters separated"
"\n                            by '|' character that will be passed to query"
"\n                            statement to replace [...] placeholders."
"\n  -u, --user <name>         User name to use when connecting to server-based"
"\n                            backends."
"\n"
"\nNotes:"
"\n1. 'dr_prop' requires <db_name> argument."
"\n2. 'parser' test requires <db_name>, <driver_name> and <sql_statement> arguments"
"\n3. All other tests require <db_name> and <driver_name> arguments."
"\n4. 'tables' test automatically runs 'dbcreation' test."
"\n   <new_db_name> is removed if already exists."
"\n5. <db_name> must be a valid kexi database e.g. created with 'tables' test."
"\n"
"\nArguments:"
"\n  driver_name               Driver name"
"\n  db_name                   Database name"
"\n  sql_statement             Optional SQL statement (for parser test)"
"\n";
}

int main(int argc, char** argv)
{
    int minargs = 2;
#ifndef NO_GUI
    bool gui = false;
#endif
    QFileInfo info = QFileInfo(argv[0]);
    prgname = info.baseName().toLatin1();
    QStringList args;
    for (int i=1; i<argc; i++)
        args.append(QFile::decodeName(argv[i]));
    if (takeOption(args, "help")) {
        showHelp();
        return 0;
    }
    QStringList tests;
    tests << "cursors" << "schema" << "dbcreation" << "tables"
#ifndef NO_GUI
        << "tableview"
#endif
        << "parser" << "dr_prop";
    test_name = takeOptionWithArg(args, "test", "t");
    if (test_name.isEmpty()) {
        qDebug() << "No test specified. Use --help.";
        return 1;
    }
    if (!tests.contains(test_name)) {
        qDebug() << QString("No such test \"%1\". Use --help.").arg(test_name);
        return finish(1);
    }

#ifndef NO_GUI
    if (test_name == "tableview") {
        gui = true;
    } else 
#endif
    if (test_name == "parser") {
        minargs = 3;
    } else if (test_name == "dr_prop") {
        minargs = 1;
        db_name_required = false;
    }

#ifndef NO_GUI
    if (gui) {
        app = new QApplication(argc, argv, true);
    } else
#endif
    {
        app = new QApplication(argc, argv, false);
    }

    QString hostName = takeOptionWithArg(args, "host" "h");
    if (!hostName.isEmpty()) {
        conn_data.setHostName(hostName);
    }
    QString userName = takeOptionWithArg(args, "user", "u");
    if (!userName.isEmpty()) {
        conn_data.setUserName(userName);
    }
    QString password = takeOptionWithArg(args, "password", "p");
    if (!password.isEmpty()) {
        conn_data.setPassword(password);
    }
    QString port = takeOptionWithArg(args, "port", "P");
    if (!port.isEmpty()) {
        bool ok;
        conn_data.setPort(port.toInt(&ok));
        if (!ok) {
            return finish(1);
        }
    }

    drv_name = args.first();

    Predicate::DriverManager manager;
    const QStringList driverNames = manager.driverNames();
    qDebug() << "DRIVERS: " << driverNames;
    if (driverNames.isEmpty()) {
        qWarning() << "No drivers found";
        return finish(1);
    }
    if (manager.result().isError()) {
        qDebug() << manager.result();
        return finish(1);
    }

    //get driver
    driver = manager.driver(drv_name);
    if (!driver || manager.result().isError()) {
        qDebug() << manager.result();
        return finish(1);
    }
    qDebug() << "main: MIME types for" << driver->name() << ":" << driver->info().mimeTypes();

    const bool bufCursors = takeOption(args, "buffered-cursors");
    QString queryParams = takeOptionWithArg(args, "query-params");

    //open connection
    if (args.count() >= 2)
        db_name = args[1];

    if (db_name_required && db_name.isEmpty()) {
        qDebug() << prgname << ": database name?";
        return finish(1);
    }
    conn_data.setDatabaseName(db_name);
    if (!db_name.isEmpty() ) {
        //additional switches:
        if (bufCursors) {
            cursor_options |= Predicate::Cursor::Buffered;
        }
        conn = driver->createConnection(conn_data);

        if (!conn || driver->result().isError()) {
            qDebug() << driver->result();
            return finish(1);
        }
        qDebug() << "main: Connection object created.";
        if (!conn->connect()) {
            qDebug() << conn->result();
            return finish(1);
        }
        qDebug() << "main: Connection::connect() OK.";
    }

    //start test:
    int r = 0;
    if (test_name == "cursors")
        r = cursorsTest();
    else if (test_name == "schema")
        r = schemaTest();
    else if (test_name == "dbcreation")
        r = dbCreationTest();
    else if (test_name == "tables")
        r = tablesTest();
#ifndef NO_GUI
    else if (test_name == "tableview")
        r = tableViewTest();
#endif
    else if (test_name == "parser") {
        QStringList params;
        if (!queryParams.isEmpty())
            params = queryParams.split("|");
        r = parserTest(Predicate::EscapedString(args[2]), params);
    } else if (test_name == "dr_prop")
        r = drPropTest();
    else {
        qWarning() << "No such test:" << test_name;
        qWarning() << "Available tests are:" << tests;
//  usage();
        return finish(1);
    }

#ifndef NO_GUI
    if (app && r == 0 && gui)
        app->exec();
#endif

    if (r)
        qDebug() << "RECENT SQL STATEMENT: " << conn->recentSQLString();

    if (conn && !conn->disconnect())
        r = 1;

// qDebug() << "!!! Predicate::Transaction::globalcount == " << Predicate::Transaction::globalCount();
// qDebug() << "!!! Predicate::TransactionData::globalcount == " << Predicate::TransactionData::globalCount();

    delete app;

    return finish(r);
}

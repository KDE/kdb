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
#include <QTextStream>

#include <KDbDriverManager>
#include <KDbDriver>
#include <KDbDriverMetaData>
#include <KDbConnection>
#include <KDbConnectionData>
#include <KDbCursor>
#include <KDbTableSchema>
#include <KDbQuerySchema>
#include <KDbIndexSchema>

#include <iostream>

using namespace std;

QByteArray prgname;
QString db_name;
QString drv_id;
QString test_name;
KDbCursor::Options cursor_options;
bool db_name_required = true;

KDbConnectionData conn_data;

//! @todo IMPORTANT: replace QPointer<KDbConnection> conn;
KDbConnection* conn = nullptr;

//! @todo IMPORTANT: replace QPointer<KDbDriver> driver;
KDbDriver* driver;
QApplication *app = nullptr;

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

#define APPNAME "kdbfeaturestest"

static void showHelp()
{
    QTextStream s(stdout);
    s <<
APPNAME ", version " KDB_VERSION_STRING
"\n"
"\nA set of tests for the KDb library API."
"\nEvery test is mostly driver-independent."
"\n (c) 2003-2016, Kexi Team"
"\n (c) 2003-2006, OpenOffice Software LLC."
"\n"
"\nUsage: " APPNAME " --test <test_name> [options]"
"\n         driver_id [db_name] [sql_statement]"
"\n"
"\nOptions:"
"\n  --help                    Displays this help and exits"
"\n  --buffered-cursors        Optional switch: turns cursors used in any"
"\n                            tests to be buffered"
"\n  -h, --host <name>         Host name to use when connecting"
"\n                            to server backends"
"\n  -p, --password <name>     Password to use when connecting to server"
"\n                            backends"
"\n  -P, --port <number>       Port number to use when connecting to server"
"\n                            backends"
"\n  --query-params <params>   Query parameters separated by '|' character"
"\n                            that will be passed to query statement"
"\n                            to replace [...] placeholders"
"\n  -t, --test <test_name>    Specifies test to execute; required"
"\n                            Available tests:"
"\n                            - cursors: test for cursors behavior"
"\n                            - schema: test for db schema retrieving"
"\n                            - dbcreation: test for new db creation"
"\n                            - tables: test for tables creation and data"
"\n                                      inserting"
//"\n                            - tableview: test for KexiDataTableView data-aware
//"\n                               widget
"\n                            - parser: test for parsing sql statements,"
"\n                                      returns debug string for a given"
"\n                                      sql statement or error message"
"\n                            - dr_prop: shows properties of selected"
"\n                                       driver"
"\n  -u, --user <name>         User name to use when connecting to servers"
"\n                            backends"
"\n"
"\nNotes:"
"\n1. 'dr_prop' requires <driver_id> argument"
"\n2. 'parser' test requires <db_name>, <driverid> and <sql_statement>"
"\n     arguments"
"\n3. All other tests require <db_name> and <driver_id> arguments"
"\n4. 'tables' test automatically runs 'dbcreation' test"
"\n     <new_db_name> is removed if already exists"
"\n5. <db_name> must be a valid database created using KDb,"
"\n     e.g. using the \"tables\" test"
"\n"
"\nArguments:"
"\n  driver_id                 Driver ID, e.g. org.kde.kdb.sqlite;"
"\n                            if a word without \".\" is used,"
"\n                            \"org.kde.kdb.\" will be prepended"
"\n  db_name                   Database name"
"\n  sql_statement             Optional SQL statement (for parser test)"
"\n"
"\nExamples:"
"\n  " APPNAME " -t dr_prop sqlite"
"\n                            Shows properties of the SQLite driver"
"\n  " APPNAME " -p PASSWORD -u USER -t tables mysql mysqltest"
"\n                            Creates database \"mysqltest\" with test"
"\n                            tables and datas"
"\n  " APPNAME " -p PASSWORD -u USER -t tables -h myhost.org \\"
"\n    postgresql pgsqltest"
"\n                            Creates database \"pgsqltest\" with test"
"\n                            tables and data on host \"myhost.org\""
"\n";
}

int main(int argc, char** argv)
{
//    int minargs = 2;
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
//        minargs = 3;
    } else if (test_name == "dr_prop") {
//        minargs = 1;
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

    const bool bufCursors = takeOption(args, "buffered-cursors");
    QString queryParams = takeOptionWithArg(args, "query-params");

    drv_id = args.first();
    if (!drv_id.contains('.')) {
        drv_id.prepend(QLatin1String("org.kde.kdb."));
    }

    KDbDriverManager manager;
    const QStringList driverIds = manager.driverIds();
    qDebug() << "DRIVERS: " << driverIds;
    if (driverIds.isEmpty()) {
        qWarning() << "No drivers found";
        return finish(1);
    }
    if (manager.result().isError()) {
        qDebug() << manager.result();
        return finish(1);
    }

    //get driver
    driver = manager.driver(drv_id);
    if (!driver || manager.result().isError()) {
        qDebug() << manager.result();
        return finish(1);
    }
    if (manager.driverMetaData(drv_id)->isFileBased()) {
        qDebug() << "main: MIME types for" << driver->metaData()->id() << ":"
                 << driver->metaData()->mimeTypes();
    }

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
            cursor_options |= KDbCursor::Option::Buffered;
        }
        conn = driver->createConnection(conn_data);

        if (!conn || driver->result().isError()) {
            qDebug() << driver->result();
            return finish(1);
        }
        qDebug() << "main: KDbConnection object created.";
        if (!conn->connect()) {
            qDebug() << conn->result();
            return finish(1);
        }
        qDebug() << "main: KDbConnection::connect() OK.";
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
        r = tablesTest(conn);
#ifndef NO_GUI
    else if (test_name == "tableview")
        r = tableViewTest();
#endif
    else if (test_name == "parser") {
        QStringList params;
        if (!queryParams.isEmpty())
            params = queryParams.split("|");
        r = parserTest(KDbEscapedString(args[2]), params);
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
        qDebug() << "RECENT SQL STATEMENT: " << conn->recentSqlString();

    if (conn && !conn->disconnect())
        r = 1;

// qDebug() << "!!! KDbTransaction::globalcount == " << KDbTransaction::globalCount();
// qDebug() << "!!! KDbTransactionData::globalcount == " << KDbTransactionData::globalCount();

    delete app;

    return finish(r);
}

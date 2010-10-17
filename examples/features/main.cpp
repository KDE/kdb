/* This file is part of the KDE project
   Copyright (C) 2003-2008 Jarosław Staniek <staniek@kde.org>

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
/*qtonly
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kcomponentdata.h>
#include <kiconloader.h>
#include <kaboutdata.h>*/

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
Predicate::Connection* conn;
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

#define RETURN(code) \
    qDebug()<< test_name << " TEST: " << (code==0?"PASSED":"ERROR"); \
    return code

//! @return true if option @a option is found
//! Removes the option.
bool takeOption(QStringList &args, const QString &option)
{
    return args.removeOne(QLatin1String("-") + option)
        || args.removeOne(QLatin1String("--") + option);
}

//! @return next element after option @a option, what should mean parameter
//! Removes option and its argument.
QString takeOptionWithArg(QStringList &args, const QString &option)
{
    int index = args.indexOf(QLatin1String("-") + option);
    if (index == -1)
        index = args.indexOf(QLatin1String("--") + option);
    if (index == -1)
        return QString();
    args.removeAt(index);
    return args.takeAt(index); // option's argument
}

int main(int argc, char** argv)
{
    int minargs = 2;
#ifndef NO_GUI
    bool gui = false;
#endif
    /* if (argc < minargs) {
        usage();
        RETURN(0);
      }*/
    QFileInfo info = QFileInfo(argv[0]);
    prgname = info.baseName().toLatin1();

#if 0 //qtonly: TODO?
    KCmdLineArgs::init(argc, argv,
                       new KAboutData(prgname, 0, ki18n("KexiDBTest"),
                                      "0.1.2", KLocalizedString(), KAboutData::License_GPL,
                                      ki18n("(c) 2003-2006, Kexi Team\n"
                                            "(c) 2003-2006, OpenOffice Polska Ltd.\n"),
                                      KLocalizedString(),
                                      "http://www.koffice.org/kexi",
                                      "submit@bugs.kde.org"
                                     )
                      );

    KCmdLineOptions options;
    options.add("test <test_name>", ki18n("Available tests:\n"
                                          "- cursors: test for cursors behaviour\n"
                                          "- schema: test for db schema retrieving\n"
                                          "- dbcreation: test for new db creation\n"
                                          "- tables: test for tables creation and data\n"
                                          "   inserting\n"
#ifndef NO_GUI
                                          "- tableview: test for KexiDataTableView data-aware\n"
                                          "   widget\n"
#endif
                                          "- parser: test for parsing sql statements,\n"
                                          "   returns debug string for a given\n"
                                          "   sql statement or error message\n"
                                          "- dr_prop: shows properties of selected driver"));
    options.add("buffered-cursors", ki18n("Optional switch :turns cursors used in any tests\n"
                                          " to be buffered"));
    options.add("query-params <params>", ki18n("Query parameters separated\n"
                "by '|' character that will be passed to query\n"
                "statement to replace [...] placeholders."));
    options.add("", ki18n(" Notes:\n"
                          "1. 'dr_prop' requires <db_name> argument.\n"
                          "2. 'parser' test requires <db_name>,\n"
                          " <driver_name> and <sql_statement> arguments\n"
                          "3. All other tests require <db_name>\n"
                          " and <driver_name> arguments.\n"
                          "4. 'tables' test automatically runs 'dbcreation'\n"
                          " test. (<new_db_name> is removed if already exists.\n"
                          "5. <db_name> must be a valid kexi database\n"
                          " e.g. created with 'tables' test."));
    options.add("+driver_name", ki18n("Driver name"));
    options.add("+[db_name]", ki18n("Database name"));
    options.add("+[sql_statement]", ki18n("Optional SQL statement (for parser test)"));
    KCmdLineArgs::addCmdLineOptions(options);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
#endif //0
    QStringList args;
    for (int i=1; i<argc; i++)
        args.append(QFile::decodeName(argv[i]));
    QStringList tests;
    tests << "cursors" << "schema" << "dbcreation" << "tables"
#ifndef NO_GUI
    << "tableview"
#endif
    << "parser" << "dr_prop";
    test_name = takeOptionWithArg(args, "test");
    if (test_name.isEmpty()) {
        qDebug() << "No test specified. Use --help.";
        RETURN(1);
    }
    if (!tests.contains(test_name)) {
        qDebug() << QString("No such test \"%1\". Use --help.").arg(test_name);
        RETURN(1);
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
/*    if ((int)args->count() < minargs) {
        qDebug() << QString("Not enough args (%1 required). Use --help.").arg(minargs);
        RETURN(1);
    }*/

#ifndef NO_GUI
    if (gui) {
        app = new QApplication(argc, argv, true);
//qtonly        instance = new KComponentData(KGlobal::mainComponent());
//qtonly        KIconLoader::global()->addAppDir("kexi");
    } else
#endif
    {
        app = new QApplication(argc, argv, false);
//qtonly        instance = new KComponentData(prgname);
    }

    drv_name = args.first();

    Predicate::DriverManager manager;
    const QStringList driverNames = manager.driverNames();
    qDebug() << "DRIVERS: " << driverNames;
    if (driverNames.isEmpty()) {
        qWarning() << "No drivers found";
        RETURN(1);
    }
    if (manager.result().isError()) {
        qDebug() << manager.result();
        RETURN(1);
    }

    //get driver
    driver = manager.driver(drv_name);
    if (!driver || manager.result().isError()) {
        qDebug() << manager.result();
        RETURN(1);
    }
    qDebug() << "MIME types for" << driver->name() << ":" << driver->info().mimeTypes();

    const bool bufCursors = takeOption(args, "buffered-cursors");
    QString queryParams = takeOptionWithArg(args, "query-params");

    //open connection
    if (args.count() >= 2)
        db_name = args[1];

    if (db_name_required && db_name.isEmpty()) {
        qDebug() << prgname << ": database name?";
        RETURN(1);
    }
    if (!db_name.isEmpty()) {
        //additional switches:
        if (bufCursors) {
            cursor_options |= Predicate::Cursor::Buffered;
        }
        conn_data.setFileName(db_name);
        conn = driver->createConnection(conn_data);

        if (!conn || driver->result().isError()) {
            qDebug() << driver->result();
            RETURN(1);
        }
        if (!conn->connect()) {
            qDebug() << conn->result();
            RETURN(1);
        }
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
        r = parserTest(args[2], params);
    } else if (test_name == "dr_prop")
        r = drPropTest();
    else {
        qWarning() << "No such test: " << test_name;
//  usage();
        RETURN(1);
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

    RETURN(r);
}

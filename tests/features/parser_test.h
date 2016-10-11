/* This file is part of the KDE project
   Copyright (C) 2003-2004 Jarosław Staniek <staniek@kde.org>

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

#ifndef PARSER_TEST_H
#define PARSER_TEST_H

#include <KDbParser>
#include <KDbConnection>
#include <KDbNativeStatementBuilder>

int parserTest(const KDbEscapedString &st, const QStringList &params)
{
    int r = 0;
    if (!conn->useDatabase()) {
        qDebug() << conn->result();
        return 1;
    }

    KDbParser parser(conn);

    const bool ok = parser.parse(st);
    KDbQuerySchema *q = parser.query();
    QList<QVariant> variantParams;
    for(const QString &param : params) {
        variantParams.append(param.toLocal8Bit());
    }
    if (ok && q) {
        cout << qPrintable(KDbUtils::debugString<KDbQuerySchema>(*q)) << '\n';
        KDbNativeStatementBuilder builder(conn);
        KDbEscapedString sql;
        if (builder.generateSelectStatement(&sql, q, variantParams)) {
            cout << "-STATEMENT:\n" << sql.toByteArray().constData() << '\n';
        }
        else {
            cout << "-CANNOT GENERATE STATEMENT\n";
        }
    } else {
        qDebug() << parser.error();
        r = 1;
    }
    delete q;
    q = 0;


    if (!conn->closeDatabase()) {
        qDebug() << conn->result();
        return 1;
    }

    return r;
}

#endif


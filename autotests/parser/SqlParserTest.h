/* This file is part of the KDE project
   Copyright (C) 2012 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_SQLPARSERTEST_H
#define KDB_SQLPARSERTEST_H

#include <QObject>
#include <QScopedPointer>

#include <KDbParser>

#include "KDbTestUtils.h"

class SqlParserTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testParse_data();
    void testParse();
    //! Tests a few tokens, they should have certain values, needed for maintaining BC
    void testTokens();
    void cleanupTestCase();

private:
    //! Opens database needed for tests.
    bool openDatabase(const QString &path);
    KDbEscapedString parse(const KDbEscapedString& sql, bool *ok);

    KDbTestUtils m_utils;
    QScopedPointer<KDbParser> m_parser;
};

#endif

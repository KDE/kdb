/* This file is part of the KDE project
   Copyright (C) 2015 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_TESTUTILS_H
#define KDB_TESTUTILS_H

#include "kdbtestutils_export.h"

#include <QPointer>
#include <KDbDriver>
#include <KDbDriverManager>

class KDBTESTUTILS_EXPORT KDbTestUtils : public QObject
{
    Q_OBJECT
public:
    KDbDriverManager manager;
    QPointer<KDbDriver> driver;

public Q_SLOTS:
    void testDriverManager();
    void testSqliteDriver();
};

#endif

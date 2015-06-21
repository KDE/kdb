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

//! @internal for KDB_VERIFY
template<typename T>
const T* KDB_POINTER_WRAPPER(const T &t) { return &t; }

//! @internal for KDB_VERIFY
template<typename T>
const T* KDB_POINTER_WRAPPER(const T *t) { return t; }

//! @internal for KDB_VERIFY
template<typename T>
T* KDB_POINTER_WRAPPER(T *t) { return t; }

//! @internal for KDB_VERIFY
template<typename T>
T* KDB_POINTER_WRAPPER(const QPointer<T> &t) { return t.data(); }

//! Calls @a call and verifies status of @a resultable
//! On error displays the status on debug and does the same as QVERIFY with @a errorMessage
#define KDB_VERIFY(resultable, call, errorMessage) \
    do { \
        bool KDB_VERIFY_ok = (call); \
        const KDbResultable *KDB_VERIFY_resultablePtr = KDB_POINTER_WRAPPER(resultable); \
        if (KDB_VERIFY_resultablePtr->result().isError()) { \
            qDebug() << KDB_VERIFY_resultablePtr->result(); \
        } \
        if (!QTest::qVerify(KDB_VERIFY_ok && !KDB_VERIFY_resultablePtr->result().isError(), # call, (errorMessage), __FILE__, __LINE__)) {\
            return; \
        } \
    } \
    while (false)

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

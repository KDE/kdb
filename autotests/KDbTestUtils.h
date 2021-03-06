/* This file is part of the KDE project
   Copyright (C) 2015-2019 Jarosław Staniek <staniek@kde.org>

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

#include <KDbConnection>
#include <KDbConnectionOptions>
#include <KDbDriver>
#include <KDbDriverManager>

#include <QPointer>
#include <QTest>

class KDbNativeStatementBuilder;

Q_DECLARE_METATYPE(KDbField::TypeGroup)
Q_DECLARE_METATYPE(KDbField::Type)
Q_DECLARE_METATYPE(KDb::Signedness)
Q_DECLARE_METATYPE(QList<KDbField::Type>)
Q_DECLARE_METATYPE(KDb::BLOBEscapingType)

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

//! @internal for KDB_VERIFY
template<typename T>
T* KDB_POINTER_WRAPPER(const QScopedPointer<T> &t) { return t.data(); }

namespace QTest
{
KDBTESTUTILS_EXPORT bool qCompare(const KDbEscapedString &val1, const KDbEscapedString &val2,
                                  const char *actual, const char *expected, const char *file,
                                  int line);
KDBTESTUTILS_EXPORT bool qCompare(const KDbEscapedString &val1, const char *val2,
                                  const char *actual, const char *expected, const char *file,
                                  int line);
KDBTESTUTILS_EXPORT bool qCompare(const char *val1, const KDbEscapedString &val2,
                                  const char *actual, const char *expected, const char *file,
                                  int line);
KDBTESTUTILS_EXPORT bool qCompare(const KDbEscapedString &val1, const QString &val2,
                                  const char *actual, const char *expected, const char *file,
                                  int line);
KDBTESTUTILS_EXPORT bool qCompare(const QString &val1, const KDbEscapedString &val2,
                                  const char *actual, const char *expected, const char *file,
                                  int line);

KDBTESTUTILS_EXPORT bool qCompare(const QStringList &val1, const QStringList &val2,
                                  const char *actual, const char *expected, const char *file,
                                  int line);

KDBTESTUTILS_EXPORT bool qCompare(const QByteArray &val1, const char *val2, const char *actual,
                                  const char *expected, const char *file, int line);
KDBTESTUTILS_EXPORT bool qCompare(const QString &val1, const char *val2, const char *actual,
                                  const char *expected, const char *file, int line);
}

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

//! Calls @a call and verifies status of @a resultable
//! On error displays the status on debug and does the same as QVERIFY with @a errorMessage
#define KDB_EXPECT_FAIL(resultable, call, expectedErrorCode, errorMessage) \
    do { \
        bool KDB_VERIFY_ok = (call); \
        const KDbResultable *KDB_VERIFY_resultablePtr = KDB_POINTER_WRAPPER(resultable); \
        if (KDB_VERIFY_resultablePtr->result().isError()) { \
            qDebug() << KDB_VERIFY_resultablePtr->result(); \
        } \
        QVERIFY(KDB_VERIFY_resultablePtr->result().isError()); \
        if (!QTest::qVerify(!KDB_VERIFY_ok, # call, (errorMessage), __FILE__, __LINE__)) {\
            return; \
        } \
        if (!QTest::qCompare(KDB_VERIFY_resultablePtr->result().code(), expectedErrorCode, # call, # expectedErrorCode, __FILE__, __LINE__)) {\
            return; \
        } \
    } \
    while (false)

//! Declares method @a name that returns false on test failure, it can be called as utility function.
//! Also declared internal method name ## Internal which performs the actual test.
//! This way users of this method can call QVERIFY(utils.<name>());
#define KDBTEST_METHOD_DECL(name, argsDecl, args) \
public: \
    Q_REQUIRED_RESULT bool name argsDecl { name ## Internal args ; return !QTest::currentTestFailed(); } \
private Q_SLOTS: \
    void name ## Internal argsDecl

//! Test utilities that provide basic database features
class KDBTESTUTILS_EXPORT KDbTestUtils : public QObject
{
    Q_OBJECT
public:
    KDbTestUtils();

    ~KDbTestUtils();

    KDbDriverManager manager;
    QPointer<KDbDriver> driver;

    /**
     * Returns associated connection
     */
    KDbConnection* connection();

    /**
     * Returns builder for generating KDb SQL statements
     */
    KDbNativeStatementBuilder* kdbBuilder();

    /**
     * Returns builder for generating driver-native SQL statements
     */
    KDbNativeStatementBuilder* driverBuilder();

    KDBTEST_METHOD_DECL(testDriverManager, (), ());
    KDBTEST_METHOD_DECL(testSqliteDriver, (), ());

    //! Connects to a database
    //! @since 3.2
    KDBTEST_METHOD_DECL(testConnect,
                        (const KDbConnectionData &cdata,
                         const KDbConnectionOptions &options = KDbConnectionOptions()),
                        (cdata, options));

    KDBTEST_METHOD_DECL(testUse, (), ());

    //! Convenience method that performs testConnect and testUse in one go
    //! @since 3.2
    KDBTEST_METHOD_DECL(testConnectAndUse,
                        (const KDbConnectionData &cdata,
                         const KDbConnectionOptions &options = KDbConnectionOptions()),
                        (cdata, options));

    //! Overload of testConnectAndUse for file-based databases
    //! @since 3.2
    KDBTEST_METHOD_DECL(testConnectAndUse,
                        (const QString &path,
                         const KDbConnectionOptions &options = KDbConnectionOptions()),
                        (path, options));

    //! Creates database with name @a dbName
    //! Does not use the database.
    //! @todo don't hardcode SQLite here
    //! @note dbName should not include ".kexi" extension or path
    KDBTEST_METHOD_DECL(testCreateDb, (const QString &dbName), (dbName));

    //! Creates database with name @a dbName, then uses it and creates test tables
    //! The database stays used.
    //! @note dbName should not include ".kexi" extension or path
    KDBTEST_METHOD_DECL(testCreateDbWithTables, (const QString &dbName), (dbName));
    KDBTEST_METHOD_DECL(testProperties, (), ());
    KDBTEST_METHOD_DECL(testCreateTables, (), ());
    KDBTEST_METHOD_DECL(testDisconnect, (), ());
    KDBTEST_METHOD_DECL(testDropDb, (), ());
    KDBTEST_METHOD_DECL(testDisconnectAndDropDb, (), ());

protected:
    void testDisconnectPrivate();
    void testDriver(const QString &driverId, bool fileBased, const QStringList &expectedMimeTypes,
                    const QStringList &possiblyInvalidMimeTypes);
    void testDriverManagerInternal(bool forceEmpty);

private:
    Q_DISABLE_COPY(KDbTestUtils)
    class Private;
    Private * const d;
};

#endif

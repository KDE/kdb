/* This file is part of the KDE project
   Copyright (C) 2019 Jarosław Staniek <staniek@kde.org>

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

#include <KDb>

#include <QObject>
#include <QTest>

//! Autotests for KDbUtils.cpp
class UtilsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

//TODO:
//    namespace KDbUtils
//    {
//    KDB_EXPORT bool hasParent(QObject* par, QObject* o);
//    template<class type>
//    inline type findParent(QObject* o, const char* className = nullptr)

    //! QString toISODateStringWithMs(const QTime& time);
    //! QString toISODateStringWithMs(const QDateTime& dateTime);
    void testTimeToISODateStringAndFromStringWithMs_data();
    void testTimeToISODateStringAndFromStringWithMs();

    //! QTime timeFromISODateStringWithMs(const QString& string);
    //! QDateTime dateTimeFromISODateStringWithMs(const QString& string);
    void testDateTimeToISODateStringAndFromStringWithMs_data();
    void testDateTimeToISODateStringAndFromStringWithMs();

//    KDB_EXPORT QDateTime stringToHackedQTime(const QString& s);
//    KDB_EXPORT void serializeMap(const QMap<QString, QString>& map, QByteArray *array);
//    KDB_EXPORT void serializeMap(const QMap<QString, QString>& map, QString *string);
//    KDB_EXPORT QMap<QString, QString> deserializeMap(const QByteArray& array);
//    KDB_EXPORT QMap<QString, QString> deserializeMap(const QString& string);
//    KDB_EXPORT QString stringToFileName(const QString& string);
//    KDB_EXPORT void simpleCrypt(QString *string);
//    KDB_EXPORT bool simpleDecrypt(QString *string);
//    KDB_EXPORT QString pointerToStringInternal(void* pointer, int size);
//    KDB_EXPORT void* stringToPointerInternal(const QString& string, int size);
//    template<class type>
//    QString pointerToString(type *pointer);
//    template<class type>
//    type* stringToPointer(const QString& string);
//    KDB_EXPORT QVariant squeezedValue(const QVariant &value);
//    template <class Key, class T>
//    class AutodeletedHash : public QHash<Key, T>;
//    template <typename T>
//    class AutodeletedList : public QList<T>;
//    template <typename Key, typename T>
//    class CaseInsensitiveHash : public QHash<Key, T>;
//    template <typename T>
//    QString debugString(const T& object)
//    QString findExe(const QString& appname,
//                    const QString& path = QString(),
//                    FindExeOptions options = FindExeOption::None);
//    class KDB_EXPORT Property {
//    public:
//        Property();
//        Property(const QVariant &aValue, const QString &aCaption);
//        Property(const Property &other);
//        bool operator==(const Property &other) const;
//        bool operator!=(const Property &other) const { return !operator==(other); }
//        bool isNull() const;
//        QVariant value() const;
//        void setValue(const QVariant &value);
//        QString caption() const;
//        void setCaption(const QString &caption);
//    };
//    class KDB_EXPORT PropertySet
//    {
//    public:
//        PropertySet();
//        PropertySet(const PropertySet &other);
//        PropertySet& operator=(const PropertySet &other);
//        bool operator==(const PropertySet &other) const;
//        bool operator!=(const PropertySet &other) const { return !operator==(other); }
//        void insert(const QByteArray &name, const QVariant &value, const QString &caption = QString());
//        void setCaption(const QByteArray &name, const QString &caption);
//        void setValue(const QByteArray &name, const QVariant &value);
//        void remove(const QByteArray &name);
//        Property property(const QByteArray &name) const;
//        QList<QByteArray> names() const;
//    };

    void cleanupTestCase();
};

QTEST_GUILESS_MAIN(UtilsTest)

void UtilsTest::initTestCase()
{
}

void UtilsTest::testTimeToISODateStringAndFromStringWithMs_data()
{
    QTest::addColumn<QTime>("time");
    QTest::addColumn<QString>("string");

    QTest::newRow("null") << QTime() << "";
    QTest::newRow("invalid") << QTime(27, 1, 1) << "";
    QTest::newRow("valid1") << QTime(21, 12, 13, 1) << "21:12:13.001";
    QTest::newRow("valid2") << QTime(0, 0, 1, 81) << "00:00:01.081";
    QTest::newRow("valid3") << QTime(21, 12, 13, 981) << "21:12:13.981";
    QTest::newRow("valid4") << QTime(21, 12, 13) << "21:12:13.000";
}

void UtilsTest::testTimeToISODateStringAndFromStringWithMs()
{
    QFETCH(QTime, time);
    QFETCH(QString, string);

    qDebug() << string << time;
    QCOMPARE(KDbUtils::toISODateStringWithMs(time), string);
    QCOMPARE(KDbUtils::timeFromISODateStringWithMs(string), time);
}

void UtilsTest::testDateTimeToISODateStringAndFromStringWithMs_data()
{
    QTest::addColumn<QDateTime>("dateTime");
    QTest::addColumn<QString>("string");

    QTest::newRow("null") << QDateTime() << "";
    QTest::newRow("invalid") << QDateTime(QDate(0, 1, 1), QTime(27, 1, 1)) << "";
    QTest::newRow("valid1") << QDateTime(QDate(1999, 12, 4), QTime(21, 12, 13, 1)) << "1999-12-04T21:12:13.001";
    QTest::newRow("valid2") << QDateTime(QDate(1999, 12, 4), QTime(0, 0, 1, 81)) << "1999-12-04T00:00:01.081";
    QTest::newRow("valid3") << QDateTime(QDate(1999, 12, 4), QTime(21, 12, 13, 981)) << "1999-12-04T21:12:13.981";
    QTest::newRow("valid4") << QDateTime(QDate(1999, 12, 4), QTime(21, 12, 13)) << "1999-12-04T21:12:13.000";
}

void UtilsTest::testDateTimeToISODateStringAndFromStringWithMs()
{
    QFETCH(QDateTime, dateTime);
    QFETCH(QString, string);

    qDebug() << string << dateTime << KDbUtils::dateTimeFromISODateStringWithMs(string);
    QCOMPARE(KDbUtils::toISODateStringWithMs(dateTime), string);
    QCOMPARE(KDbUtils::dateTimeFromISODateStringWithMs(string), dateTime);
}

void UtilsTest::cleanupTestCase()
{
}

#include "UtilsTest.moc"

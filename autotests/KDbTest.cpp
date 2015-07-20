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

#include "KDbTest.h"

#include <KDb>
#include <KDbVersionInfo>

#include <QTest>

QTEST_GUILESS_MAIN(KDbTest)

Q_DECLARE_METATYPE(KDbField::TypeGroup)
Q_DECLARE_METATYPE(KDbField::Type)
Q_DECLARE_METATYPE(KDb::Signedness)
Q_DECLARE_METATYPE(QList<KDbField::Type>)

void KDbTest::initTestCase()
{
}

void KDbTest::testVersionInfo()
{
    KDbVersionInfo info = KDb::version();
    KDbVersionInfo info2(KDb::version());
    QCOMPARE(info, info2);
    KDbVersionInfo info3(info.major(), info.minor(), info.release());
    QCOMPARE(info, info3);
    QVERIFY(KDbVersionInfo(0, 0, 0).isNull());
    QVERIFY(!info.isNull());
    QVERIFY(!info2.isNull());
    QVERIFY(!info3.isNull());
}

void KDbTest::testFieldTypes()
{
    QCOMPARE(KDbField::FirstType, KDbField::Byte);
    QCOMPARE(KDbField::LastType, KDbField::BLOB);
    QVERIFY(KDbField::FirstType < KDbField::LastType);
}

void KDbTest::testFieldTypesForGroup_data()
{
    QTest::addColumn<KDbField::TypeGroup>("typeGroup");
    QTest::addColumn<QList<KDbField::Type>>("types");

    QTest::newRow("invalid") << KDbField::InvalidGroup
        << (QList<KDbField::Type>() << KDbField::InvalidType);
    QTest::newRow("text") << KDbField::TextGroup << (QList<KDbField::Type>()
        << KDbField::Text << KDbField::LongText);
    QTest::newRow("integer") << KDbField::IntegerGroup
        << (QList<KDbField::Type>()
        << KDbField::Byte << KDbField::ShortInteger << KDbField::Integer << KDbField::BigInteger);
    QTest::newRow("float") << KDbField::FloatGroup
        << (QList<KDbField::Type>() << KDbField::Float << KDbField::Double);
    QTest::newRow("boolean") << KDbField::BooleanGroup
        << (QList<KDbField::Type>() << KDbField::Boolean);
    QTest::newRow("datetime") << KDbField::DateTimeGroup
        << (QList<KDbField::Type>() << KDbField::Date << KDbField::DateTime << KDbField::Time);
    QTest::newRow("blob") << KDbField::BLOBGroup
        << (QList<KDbField::Type>() << KDbField::BLOB);
}

void KDbTest::testFieldTypesForGroup()
{
    QFETCH(KDbField::TypeGroup, typeGroup);
    QFETCH(QList<KDbField::Type>, types);
    QCOMPARE(KDb::fieldTypesForGroup(typeGroup), types);
}

void KDbTest::testFieldTypeNamesAndStringsForGroup_data()
{
    QTest::addColumn<KDbField::TypeGroup>("typeGroup");
    QTest::addColumn<QList<QByteArray>>("typeNames");
    QTest::addColumn<QStringList>("typeStrings");

    QTest::newRow("invalid") << KDbField::InvalidGroup
        << (QList<QByteArray>() << "Invalid Type")
        << (QStringList() << "InvalidType");
    QTest::newRow("text") << KDbField::TextGroup << (QList<QByteArray>()
        << "Text" << "Long Text")
        << (QStringList() << "Text" << "LongText");
    QTest::newRow("integer") << KDbField::IntegerGroup
        << (QList<QByteArray>()
        << "Byte" << "Short Integer Number" << "Integer Number" << "Big Integer Number")
        << (QStringList() << "Byte" << "ShortInteger" << "Integer" << "BigInteger");
    QTest::newRow("float") << KDbField::FloatGroup
        << (QList<QByteArray>() << "Single Precision Number" << "Double Precision Number")
        << (QStringList() << "Float" << "Double");
    QTest::newRow("boolean") << KDbField::BooleanGroup
        << (QList<QByteArray>() << "Yes/No Value")
        << (QStringList() << "Boolean");
    QTest::newRow("datetime") << KDbField::DateTimeGroup
        << (QList<QByteArray>() << "Date" << "Date and Time" << "Time")
        << (QStringList() << "Date" << "DateTime" << "Time");
    QTest::newRow("blob") << KDbField::BLOBGroup
        << (QList<QByteArray>() << "Object")
        << (QStringList() << "BLOB");
}

void KDbTest::testFieldTypeNamesAndStringsForGroup()
{
    QFETCH(KDbField::TypeGroup, typeGroup);
    QFETCH(QList<QByteArray>, typeNames);
    QFETCH(QStringList, typeStrings);
    QStringList translatedNames;
    foreach(const QByteArray &name, typeNames) {
        translatedNames.append(KDbField::tr(name.constData()));
    }
    QCOMPARE(KDb::fieldTypeNamesForGroup(typeGroup), translatedNames);
    QCOMPARE(KDb::fieldTypeStringsForGroup(typeGroup), typeStrings);
}

void KDbTest::testDefaultFieldTypeForGroup()
{
    QCOMPARE(KDb::defaultFieldTypeForGroup(KDbField::InvalidGroup), KDbField::InvalidType);
    QCOMPARE(KDb::defaultFieldTypeForGroup(KDbField::TextGroup), KDbField::Text);
    QCOMPARE(KDb::defaultFieldTypeForGroup(KDbField::IntegerGroup), KDbField::Integer);
    QCOMPARE(KDb::defaultFieldTypeForGroup(KDbField::FloatGroup), KDbField::Double);
    QCOMPARE(KDb::defaultFieldTypeForGroup(KDbField::BooleanGroup), KDbField::Boolean);
    QCOMPARE(KDb::defaultFieldTypeForGroup(KDbField::DateTimeGroup), KDbField::Date);
    QCOMPARE(KDb::defaultFieldTypeForGroup(KDbField::BLOBGroup), KDbField::BLOB);
}

void KDbTest::testSimplifiedFieldTypeName()
{
    QCOMPARE(KDb::simplifiedFieldTypeName(KDbField::InvalidType), KDbField::tr("Invalid Group"));
    QCOMPARE(KDb::simplifiedFieldTypeName(KDbField::Byte), KDbField::tr("Number"));
    QCOMPARE(KDb::simplifiedFieldTypeName(KDbField::ShortInteger), KDbField::tr("Number"));
    QCOMPARE(KDb::simplifiedFieldTypeName(KDbField::Integer), KDbField::tr("Number"));
    QCOMPARE(KDb::simplifiedFieldTypeName(KDbField::BigInteger), KDbField::tr("Number"));
    QCOMPARE(KDb::simplifiedFieldTypeName(KDbField::Boolean), KDbField::tr("Yes/No"));
    QCOMPARE(KDb::simplifiedFieldTypeName(KDbField::Date), KDbField::tr("Date/Time"));
    QCOMPARE(KDb::simplifiedFieldTypeName(KDbField::DateTime), KDbField::tr("Date/Time"));
    QCOMPARE(KDb::simplifiedFieldTypeName(KDbField::Time), KDbField::tr("Date/Time"));
    QCOMPARE(KDb::simplifiedFieldTypeName(KDbField::Float), KDbField::tr("Number"));
    QCOMPARE(KDb::simplifiedFieldTypeName(KDbField::Double), KDbField::tr("Number"));
    QCOMPARE(KDb::simplifiedFieldTypeName(KDbField::Text), KDbField::tr("Text"));
    QCOMPARE(KDb::simplifiedFieldTypeName(KDbField::LongText), KDbField::tr("Text"));
    QCOMPARE(KDb::simplifiedFieldTypeName(KDbField::BLOB), KDbField::tr("Image"));
    QCOMPARE(KDb::simplifiedFieldTypeName(KDbField::Null), KDbField::tr("Invalid Group"));
    QCOMPARE(KDb::simplifiedFieldTypeName(KDbField::Asterisk), KDbField::tr("Invalid Group"));
    QCOMPARE(KDb::simplifiedFieldTypeName(KDbField::Enum), KDbField::tr("Invalid Group"));
    QCOMPARE(KDb::simplifiedFieldTypeName(KDbField::Map), KDbField::tr("Invalid Group"));
    QCOMPARE(KDb::simplifiedFieldTypeName(KDbField::Tuple), KDbField::tr("Invalid Group"));
}

void KDbTest::testCstringToVariant_data()
{
    QTest::addColumn<QString>("data"); // QString() -> 0, QString("") -> empty string ""
    QTest::addColumn<KDbField::Type>("type");
    QTest::addColumn<int>("length");
    QTest::addColumn<QVariant>("variant");
    QTest::addColumn<KDb::Signedness>("signedness");
    QTest::addColumn<bool>("okResult");

    QTest::newRow("invalid1") << QString() << KDbField::InvalidType << -1 << QVariant() << KDb::Signed << false;
    QTest::newRow("invalid2") << "" << KDbField::InvalidType << -1 << QVariant() << KDb::Signed << false;
    QTest::newRow("invalid3") << "abc" << KDbField::InvalidType << 3 << QVariant() << KDb::Signed << false;
    QTest::newRow("byte1") << "0" << KDbField::Byte << 1 << QVariant(0) << KDb::Signed << true;
    QTest::newRow("ubyte1") << "0" << KDbField::Byte << 1 << QVariant(0) << KDb::Unsigned << true;
    QTest::newRow("byte2") << "42" << KDbField::Byte << -1 << QVariant(42) << KDb::Signed << true;
    QTest::newRow("ubyte2") << "42" << KDbField::Byte << -1 << QVariant(42) << KDb::Unsigned << true;
    QTest::newRow("byte3") << "129" << KDbField::Byte << -1 << QVariant() << KDb::Signed << false;
    QTest::newRow("ubyte3") << "129" << KDbField::Byte << -1 << QVariant(129) << KDb::Unsigned << true;
    QTest::newRow("byte4") << "-128" << KDbField::Byte << -1 << QVariant(-128) << KDb::Signed << true;
    QTest::newRow("ubyte4") << "-128" << KDbField::Byte << -1 << QVariant() << KDb::Unsigned << false;
    QTest::newRow("short1") << "-123" << KDbField::ShortInteger << -1 << QVariant(-123) << KDb::Signed << true;
    QTest::newRow("short2") << "942" << KDbField::ShortInteger << -1 << QVariant(942) << KDb::Signed << true;
    QTest::newRow("short3") << "32767" << KDbField::ShortInteger << -1 << QVariant(32767) << KDb::Signed << true;
    QTest::newRow("short4") << "32768" << KDbField::ShortInteger << -1 << QVariant() << KDb::Signed << false;
    QTest::newRow("ushort4") << "32768" << KDbField::ShortInteger << -1 << QVariant(32768) << KDb::Unsigned << true;
    QTest::newRow("short5") << "-32768" << KDbField::ShortInteger << -1 << QVariant(-32768) << KDb::Signed << true;
    QTest::newRow("ushort5") << "-32768" << KDbField::ShortInteger << -1 << QVariant() << KDb::Unsigned << false;
    QTest::newRow("int1") << QString::number(0x07FFFFFFF) << KDbField::Integer << -1 << QVariant(0x07FFFFFFF) << KDb::Signed << true;
    QTest::newRow("uint1") << QString::number(0x07FFFFFFF) << KDbField::Integer << -1 << QVariant(0x07FFFFFFF) << KDb::Unsigned << true;
    QTest::newRow("int2") << QString::number(-0x07FFFFFFF) << KDbField::Integer << -1 << QVariant(-0x07FFFFFFF) << KDb::Signed << true;
    QTest::newRow("uint2") << QString::number(-0x07FFFFFFF) << KDbField::Integer << -1 << QVariant() << KDb::Unsigned << false;
    QTest::newRow("int3") << QString::number(-0x080000000) << KDbField::Integer << -1 << QVariant() << KDb::Signed << false;
    QTest::newRow("uint4") << "-1" << KDbField::Integer << -1 << QVariant() << KDb::Unsigned << false;
    QTest::newRow("int4") << "-1" << KDbField::Integer << -1 << QVariant(-1) << KDb::Signed << true;
    //!< @todo cannot be larger?
    QTest::newRow("bigint1") << QString::number(0x07FFFFFFF) << KDbField::BigInteger << -1 << QVariant(0x07FFFFFFF) << KDb::Signed << true;
    QTest::newRow("ubigint1") << QString::number(0x07FFFFFFF) << KDbField::BigInteger << -1 << QVariant(0x07FFFFFFF) << KDb::Unsigned << true;
    QTest::newRow("bigint2") << QString::number(-0x07FFFFFFF) << KDbField::BigInteger << -1 << QVariant(-0x07FFFFFFF) << KDb::Signed << true;
    QTest::newRow("ubigint2") << QString::number(-0x07FFFFFFF) << KDbField::BigInteger << -1 << QVariant() << KDb::Unsigned << false;
    QTest::newRow("bigint3") << QString::number(-0x080000000) << KDbField::BigInteger << -1 << QVariant() << KDb::Signed << false;
    QTest::newRow("ubigint4") << "-1" << KDbField::BigInteger << -1 << QVariant() << KDb::Unsigned << false;
    QTest::newRow("bigint4") << "-1" << KDbField::BigInteger << -1 << QVariant(-1) << KDb::Signed << true;
    QTest::newRow("bool0") << "0" << KDbField::Boolean << -1 << QVariant(false) << KDb::Signed << true;
    QTest::newRow("bool1") << "1" << KDbField::Boolean << -1 << QVariant(true) << KDb::Signed << true;
    QTest::newRow("bool-") << "-" << KDbField::Boolean << -1 << QVariant(true) << KDb::Signed << true;
    QTest::newRow("bool5") << "5" << KDbField::Boolean << -1 << QVariant(true) << KDb::Signed << true;
    QTest::newRow("bool false") << "false" << KDbField::Boolean << -1 << QVariant(false) << KDb::Signed << true;
    QTest::newRow("bool False") << "False" << KDbField::Boolean << -1 << QVariant(false) << KDb::Signed << true;
    QTest::newRow("bool TRUE") << "TRUE" << KDbField::Boolean << -1 << QVariant(true) << KDb::Signed << true;
    QTest::newRow("bool true") << "true" << KDbField::Boolean << -1 << QVariant(true) << KDb::Signed << true;
    QTest::newRow("bool no") << "no" << KDbField::Boolean << -1 << QVariant(true) << KDb::Signed << true; // surprised? See docs for QVariant::toBool().

    QTest::newRow("Null") << " " << KDbField::Null << -1 << QVariant() << KDb::Signed << false;
    QTest::newRow("Asterisk") << " " << KDbField::Asterisk << -1 << QVariant() << KDb::Signed << false;
    QTest::newRow("Enum") << " " << KDbField::Enum << -1 << QVariant() << KDb::Signed << false;
    QTest::newRow("Map") << " " << KDbField::Map << -1 << QVariant() << KDb::Signed << false;
    QTest::newRow("Tuple") << " " << KDbField::Tuple << -1 << QVariant() << KDb::Signed << false;

#if 0
    TODO: support:
    Date = 6,        /*!< */
    DateTime = 7,    /*!< */
    Time = 8,        /*!< */
    Float = 9,       /*!< 4 bytes */
    Double = 10,     /*!< 8 bytes */
    Text = 11,       /*!< Other name: Varchar */
    LongText = 12,   /*!< Other name: Memo */
    BLOB = 13,       /*!< Large binary object */
#endif
}

void KDbTest::testCstringToVariant()
{
    QFETCH(QString, data);
    QFETCH(KDbField::Type, type);
    QFETCH(int, length);
    QFETCH(QVariant, variant);
    QFETCH(KDb::Signedness, signedness);
    QFETCH(bool, okResult);
    bool ok;
    const QByteArray ba(data.toUtf8()); // to avoid pointer to temp.
    const char *realData = ba.isNull() ? 0 : ba.constData();
    QCOMPARE(KDb::cstringToVariant(realData, type, &ok, length, signedness), variant);
    QCOMPARE(ok, okResult);
    QCOMPARE(KDb::cstringToVariant(realData, type, 0, length, signedness), variant); // a case where ok == 0
    if (realData) {
        QCOMPARE(KDb::cstringToVariant(realData, type, &ok, data.length(), signedness), variant); // a case where length is set
        QCOMPARE(ok, okResult);
    }
    QCOMPARE(KDb::cstringToVariant(0, type, &ok, length, signedness), QVariant()); // a case where data == 0 (NULL)
    QVERIFY(ok || type < KDbField::Byte || type > KDbField::LastType); // fails for NULL if this type isn't allowed
    if (type != KDbField::Boolean) {
        QCOMPARE(KDb::cstringToVariant(realData, type, &ok, 0, signedness), QVariant()); // a case where length == 0
        QVERIFY(!ok);
    }
    if (KDbField::isTextType(type)) { // a case where data == ""
        QCOMPARE(KDb::cstringToVariant("", type, &ok, length, signedness), QVariant(""));
        QVERIFY(ok);
    }
    else if (type != KDbField::Boolean) {
        QCOMPARE(KDb::cstringToVariant("", type, &ok, length, signedness), QVariant());
        QVERIFY(!ok);
    }
}

#if 0
TODO:
// -- requiring a connection:
KDB_EXPORT bool deleteRecord(KDbConnection* conn, KDbTableSchema *table,
                                          const QString &keyname, const QString& keyval);
KDB_EXPORT bool deleteRecord(KDbConnection* conn, const QString &tableName,
                                          const QString &keyname, const QString &keyval);
KDB_EXPORT bool deleteRecord(KDbConnection* conn, KDbTableSchema *table,
                                          const QString& keyname, int keyval);
KDB_EXPORT bool deleteRecord(KDbConnection* conn, const QString &tableName,
                                          const QString &keyname, int keyval);
KDB_EXPORT bool deleteRecord(KDbConnection* conn, const QString &tableName,
                                   const QString &keyname1, KDbField::Type keytype1, const QVariant& keyval1,
                                   const QString &keyname2, KDbField::Type keytype2, const QVariant& keyval2);
KDB_EXPORT bool deleteRecord(KDbConnection* conn, const QString &tableName,
                                    const QString &keyname1, KDbField::Type keytype1, const QVariant& keyval1,
                                    const QString &keyname2, KDbField::Type keytype2, const QVariant& keyval2,
                                    const QString &keyname3, KDbField::Type keytype3, const QVariant& keyval3);

KDB_EXPORT void getHTMLErrorMesage(const KDbResultable& resultable, QString *msg, QString *details);
KDB_EXPORT void getHTMLErrorMesage(const KDbResultable& resultable, QString *msg);
KDB_EXPORT void getHTMLErrorMesage(const KDbResultable& resultable, KDbResultInfo *info);
KDB_EXPORT KDbEscapedString sqlWhere(KDbDriver *drv, KDbField::Type t,
                                        const QString& fieldName, const QVariant& value);
KDB_EXPORT int idForObjectName(KDbConnection* conn, const QString& objName, int objType);
int recordCount(KDbConnection* conn, const KDbEscapedString& sql);
KDB_EXPORT int recordCount(const KDbTableSchema& tableSchema);
KDB_EXPORT int recordCount(KDbQuerySchema* querySchema,
                           const QList<QVariant>& params = QList<QVariant>());
KDB_EXPORT int recordCount(KDbTableOrQuerySchema* tableOrQuery,
                           const QList<QVariant>& params = QList<QVariant>());
KDB_EXPORT int fieldCount(KDbTableOrQuerySchema* tableOrQuery);
KDB_EXPORT void connectionTestDialog(QWidget* parent, const KDbConnectionData& data,
        KDbMessageHandler* msgHandler);
KDB_EXPORT bool splitToTableAndFieldParts(const QString& string,
        QString *tableName, QString *fieldName,
        SplitToTableAndFieldPartsOptions option = FailIfNoTableOrFieldName);
KDB_EXPORT void getProperties(const KDbLookupFieldSchema *lookup, QMap<QByteArray, QVariant> *values);
KDB_EXPORT void getFieldProperties(const KDbField &field, QMap<QByteArray, QVariant> *values);
KDB_EXPORT bool setFieldProperties(KDbField *field, const QMap<QByteArray, QVariant>& values);
KDB_EXPORT bool setFieldProperty(KDbField *field, const QByteArray& propertyName,
                                       const QVariant& value);
KDB_EXPORT QVariant loadPropertyValueFromDom(const QDomNode& node, bool *ok);
KDB_EXPORT int loadIntPropertyValueFromDom(const QDomNode& node, bool* ok);
KDB_EXPORT QString loadStringPropertyValueFromDom(const QDomNode& node, bool* ok);
KDB_EXPORT QDomElement saveNumberElementToDom(QDomDocument *doc, QDomElement *parentEl,
        const QString& elementName, int value);
KDB_EXPORT QDomElement saveBooleanElementToDom(QDomDocument *doc, QDomElement *parentEl,
        const QString& elementName, bool value);
KDB_EXPORT bool isDefaultValueAllowed(const KDbField &field);
KDB_EXPORT QString temporaryTableName(KDbConnection *conn, const QString &baseName);
#ifdef KDB_DEBUG_GUI
typedef void(*DebugGUIHandler)(const QString&);
KDB_EXPORT void setDebugGUIHandler(DebugGUIHandler handler);
KDB_EXPORT void debugGUI(const QString& text);
KDB_EXPORT void setAlterTableActionDebugHandler(AlterTableActionDebugGUIHandler handler);
KDB_EXPORT void alterTableActionDebugGUI(const QString& text, int nestingLevel = 0);
#endif

// -- not requiring a connection:
KDB_EXPORT bool supportsVisibleDecimalPlacesProperty(KDbField::Type type);
KDB_EXPORT QString formatNumberForVisibleDecimalPlaces(double value, int decimalPlaces);
KDB_EXPORT bool isBuiltinTableFieldProperty(const QByteArray& propertyName);
KDB_EXPORT bool isExtendedTableFieldProperty(const QByteArray& propertyName);
KDB_EXPORT bool isLookupFieldSchemaProperty(const QByteArray& propertyName);
KDB_EXPORT KDbField::Type intToFieldType(int type);
KDB_EXPORT KDbField::TypeGroup intToFieldTypeGroup(int typeGroup);
KDB_EXPORT QVariant emptyValueForFieldType(KDbField::Type type);
KDB_EXPORT QVariant notEmptyValueForFieldType(KDbField::Type type);
KDB_EXPORT bool isKDbSQLKeyword(const QByteArray& word);
KDB_EXPORT QString escapeString(const QString& string);
KDB_EXPORT QString escapeBLOB(const QByteArray& array, BLOBEscapingType type);
KDB_EXPORT QByteArray pgsqlByteaToByteArray(const char* data, int length);
KDB_EXPORT QList<int> stringListToIntList(const QStringList &list, bool *ok);
KDB_EXPORT QString serializeList(const QStringList &list);
KDB_EXPORT QStringList deserializeList(const QString &data);
KDB_EXPORT QList<int> deserializeIntList(const QString &data, bool *ok);
KDB_EXPORT QString variantToString(const QVariant& v);
KDB_EXPORT QVariant stringToVariant(const QString& s, QVariant::Type type, bool* ok);

KDB_EXPORT void getLimitsForFieldType(KDbField::Type type, int *minValue, int *maxValue);
KDB_EXPORT KDbField::Type maximumForIntegerFieldTypes(KDbField::Type t1, KDbField::Type t2);
KDB_EXPORT QString defaultFileBasedDriverMimeType();
KDB_EXPORT QString defaultFileBasedDriverId();
KDB_EXPORT KDbEscapedString valueToSQL(KDbField::Type ftype, const QVariant& v);
KDB_EXPORT KDbEscapedString dateTimeToSQL(const QDateTime& v);
template<typename T>
T iifNotEmpty(const T &string, const T &stringIfEmpty)
{
    return string.isEmpty() ? stringIfEmpty : string;
}
template<typename T>
T iifNotEmpty(const QByteArray &string, const T &stringIfEmpty)
{
    return iifNotEmpty(QString(string), stringIfEmpty);
}
T iifNotEmpty(const T &string, const QByteArray &stringIfEmpty)
{
    return iifNotEmpty(string, QString(stringIfEmpty));
}
template<typename T>
T iif(bool ok, const T &value);
KDB_EXPORT QStringList libraryPaths();
KDB_EXPORT QString sqlite3ProgramPath();
KDB_EXPORT bool importSqliteFile(const QString &inputFileName, const QString &outputFileName);
#endif

void KDbTest::cleanupTestCase()
{
}

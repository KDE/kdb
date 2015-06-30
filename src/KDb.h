/* This file is part of the KDE project
   Copyright (C) 2004-2015 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_H
#define KDB_H

#include <QList>
#include <QVariant>
#include <QByteArray>

#include "config-kdb.h"
#include "KDbGlobal.h"
#include "KDbField.h"
#include "KDbVersionInfo.h"

class QDomNode;
class QDomElement;
class QDomDocument;

class KDbEscapedString;
class KDbConnection;
class KDbConnectionData;
class KDbMessageHandler;
class KDbQuerySchema;
class KDbResultable;
class KDbResultInfo;
class KDbTableSchema;
class KDbDriver;
class KDbLookupFieldSchema;
class KDbTableOrQuerySchema;

namespace KDb
{

//! @return KDb-specific information about version of the database.
KDB_EXPORT KDbVersionInfo version();

//! for convenience
KDB_EXPORT bool deleteRecord(KDbConnection* conn, KDbTableSchema *table,
                                          const QString &keyname, const QString& keyval);

KDB_EXPORT bool deleteRecord(KDbConnection* conn, const QString &tableName,
                                          const QString &keyname, const QString &keyval);

KDB_EXPORT bool deleteRecord(KDbConnection* conn, KDbTableSchema *table,
                                          const QString& keyname, int keyval);

KDB_EXPORT bool deleteRecord(KDbConnection* conn, const QString &tableName,
                                          const QString &keyname, int keyval);

/*! Deletes record with two generic criterias. */
KDB_EXPORT bool deleteRecord(KDbConnection* conn, const QString &tableName,
                                   const QString &keyname1, KDbField::Type keytype1, const QVariant& keyval1,
                                   const QString &keyname2, KDbField::Type keytype2, const QVariant& keyval2);

/*! Deletes record with three generic criterias. */
KDB_EXPORT bool deleteRecord(KDbConnection* conn, const QString &tableName,
                                    const QString &keyname1, KDbField::Type keytype1, const QVariant& keyval1,
                                    const QString &keyname2, KDbField::Type keytype2, const QVariant& keyval2,
                                    const QString &keyname3, KDbField::Type keytype3, const QVariant& keyval3);

/*! @return list of field types for field type group @a typeGroup. */
KDB_EXPORT const QList<KDbField::Type> fieldTypesForGroup(KDbField::TypeGroup typeGroup);

/*! @return list of translated field type names for field type group @a typeGroup. */
KDB_EXPORT QStringList fieldTypeNamesForGroup(KDbField::TypeGroup typeGroup);

/*! @return list of (nontranslated) field type names for field type group @a typeGroup. */
KDB_EXPORT QStringList fieldTypeStringsForGroup(KDbField::TypeGroup typeGroup);

/*! @return default field type for field type group @a typeGroup,
 for example, KDbField::Integer for KDbField::IntegerGroup.
 It is used e.g. in KexiAlterTableDialog, to properly fill
 'type' property when user selects type group for a field. */
KDB_EXPORT KDbField::Type defaultFieldTypeForGroup(KDbField::TypeGroup typeGroup);

/*! @return a slightly simplified field type name type @a type.
 For KDbField::BLOB type it returns a translated "Image" string or other, depending on the mime type.
 For numbers (either floating-point or integer) it returns a translated "Number" string.
 For other types KDbField::typeGroupName() is returned. */
//! @todo support names of other BLOB subtypes
KDB_EXPORT QString simplifiedFieldTypeName(KDbField::Type type);

/*! @return true if value @a v represents an empty (but not null) value.
 This depends on field type @a type. Values of some types (such as strings)
 can be both empty and not null. */
KDB_EXPORT bool isEmptyValue(KDbField::Type type, const QVariant &v);

/*! Sets string pointed by @a msg to an error message retrieved from @a resultable,
 and string pointed by @a details to details of this error (server message and result number).
 Does nothing if @a result is empty. In this case @a msg and @a details strings are not overwritten.
 If the string pointed by @a msg is not empty, @a result message is appended to the string
 pointed by @a details.
 */
KDB_EXPORT void getHTMLErrorMesage(const KDbResultable& resultable, QString *msg, QString *details);

/*! This methods works like above, but appends both a message and a description
 to string pointed by @a msg. */
KDB_EXPORT void getHTMLErrorMesage(const KDbResultable& resultable, QString *msg);

/*! This methods works like above, but works on @a result's  members instead. */
KDB_EXPORT void getHTMLErrorMesage(const KDbResultable& resultable, KDbResultInfo *info);

/*! Function useful for building WHERE parts of SQL statements.
 Constructs an SQL string like "fielname = value" for specific @a drv driver,
 field type @a t, @a fieldName and @a value. If @a value is null, "fieldname is NULL"
 string is returned. */
KDB_EXPORT KDbEscapedString sqlWhere(KDbDriver *drv, KDbField::Type t,
                                        const QString& fieldName, const QVariant& value);

/*! @return identifier for object @a objName of type @a objType
 or 0 if such object does not exist. */
KDB_EXPORT int idForObjectName(KDbConnection* conn, const QString& objName, int objType);

//! @todo perhaps use quint64 here?
/*! @return number of records that can be retrieved after executing @a sql statement
 within a connection @a conn. The statement should be of type SELECT.
 For SQL data sources it does not fetch any records, only "COUNT(*)"
 SQL aggregation is used at the backed.
 -1 is returned if error occurred. */
int recordCount(KDbConnection* conn, const KDbEscapedString& sql);

//! @todo perhaps use quint64 here?
/*! @return number of records that can be retrieved from @a tableSchema.
 The table must be created or retrieved using a KDbConnection object,
 i.e. tableSchema.connection() must not return 0.
 For SQL data sources it does not fetch any records, only "COUNT(*)"
 SQL aggregation is used at the backed.
 -1 is returned if error occurred. */
KDB_EXPORT int recordCount(const KDbTableSchema& tableSchema);

/*! @overload in rowCount(const KDbTableSchema& tableSchema)
 Operates on a query schema. @a params are optional values of parameters that will
 be inserted into places marked with [] before execution of the query. */
//! @todo perhaps use quint64 here?
KDB_EXPORT int recordCount(KDbQuerySchema* querySchema,
                           const QList<QVariant>& params = QList<QVariant>());

/*! @overload int rowCount(KDbQuerySchema& querySchema, const QList<QVariant>& params)
 Operates on a table or query schema. @a params are optional values of parameters that
 will be inserted into places marked with [] before execution of the query. */
//! @todo perhaps use quint64 here?
KDB_EXPORT int recordCount(KDbTableOrQuerySchema* tableOrQuery,
                           const QList<QVariant>& params = QList<QVariant>());

/*! @return a number of columns that can be retrieved from table or query schema.
 In case of query, expanded fields are counted. Can return -1 if @a tableOrQuery
 has neither table or query assigned. */
KDB_EXPORT int fieldCount(KDbTableOrQuerySchema* tableOrQuery);

/*! shows connection test dialog with a progress bar indicating connection testing
 (within a second thread).
 @a data is used to perform a (temporary) test connection. @a msgHandler is used to display errors.
 On successful connecting, a message is displayed. After testing, temporary connection is closed. */
KDB_EXPORT void connectionTestDialog(QWidget* parent, const KDbConnectionData& data,
        KDbMessageHandler* msgHandler);

//! Used in splitToTableAndFieldParts().
enum SplitToTableAndFieldPartsOptions {
    FailIfNoTableOrFieldName = 0, //!< default value for splitToTableAndFieldParts()
    SetFieldNameIfNoTableName = 1 //!< see splitToTableAndFieldParts()
};

/*! Splits @a string like "table.field" into "table" and "field" parts.
 On success, a table name is passed to @a tableName and a field name is passed to @a fieldName.
 The function fails if either:
 - @a string is empty, or
 - @a string does not contain '.' character and @a option is FailIfNoTableOrFieldName
    (the default), or
 - '.' character is the first of last character of @a string (in this case table name
   or field name could become empty what is not allowed).

 If @a option is SetFieldNameIfNoTableName and @a string does not contain '.',
 @a string is passed to @a fieldName and @a tableName is set to QString()
 without failure.

 If function fails, @a tableName and @a fieldName remain unchanged.
 @return true on success. */
KDB_EXPORT bool splitToTableAndFieldParts(const QString& string,
        QString *tableName, QString *fieldName,
        SplitToTableAndFieldPartsOptions option = FailIfNoTableOrFieldName);

/*! @return true if @a type supports "visibleDecimalPlaces" property. */
KDB_EXPORT bool supportsVisibleDecimalPlacesProperty(KDbField::Type type);

/*! @return string constructed by converting @a value.
 * If @a decimalPlaces is < 0, all meaningful fractional digits are returned.
 * If @a automatically is 0, just integer part is returned.
 * If @a automatically is > 0, fractional part should take exactly
   N digits: if the fractional part is shorter than N, additional zeros are appended.
   For example, "12.345" becomes "12.345000" if N=6.

 No rounding is actually performed.
 KLocale::formatNumber() and KLocale::decimalSymbol() are used to get locale settings.

 @see KDbField::visibleDecimalPlaces() */
KDB_EXPORT QString formatNumberForVisibleDecimalPlaces(double value, int decimalPlaces);

//! @return true if @a propertyName is a builtin field property.
KDB_EXPORT bool isBuiltinTableFieldProperty(const QByteArray& propertyName);

//! @return true if @a propertyName is an extended field property.
KDB_EXPORT bool isExtendedTableFieldProperty(const QByteArray& propertyName);

//! @return true if @a propertyName is belongs to lookup field's schema.
KDB_EXPORT bool isLookupFieldSchemaProperty(const QByteArray& propertyName);

/*! @return type of field for integer value @a type.
 If @a type cannot be casted to KDbField::Type, KDbField::InvalidType is returned.
 This can be used when type information is deserialized from a string or QVariant. */
KDB_EXPORT KDbField::Type intToFieldType(int type);

/*! @return type group of field for integer value @a typeGroup.
 If @a typeGroup cannot be casted to KDbField::TypeGroup, KDbField::InvalidGroup is returned.
 This can be used when type information is deserialized from a string or QVariant. */
KDB_EXPORT KDbField::TypeGroup intToFieldTypeGroup(int typeGroup);

/*! Gets property values for the lookup schema @a lookup.
 @a values is cleared before filling. This function is used e.g. for altering table design. */
KDB_EXPORT void getProperties(const KDbLookupFieldSchema *lookup, QMap<QByteArray, QVariant> *values);

/*! Gets property values for @a field.
 Properties from extended schema are included. @a values is cleared before filling.
 The same number of properties in the same order is returned.
 This function is used e.g. for altering table design.
 */
KDB_EXPORT void getFieldProperties(const KDbField &field, QMap<QByteArray, QVariant> *values);

/*! Sets property values for @a field. @return true if all the values are valid and allowed.
 On failure contents of @a field is undefined.
 Properties from extended schema are also supported.
 This function is used e.g. by KDbAlterTableHandler when property information comes in form of text.
 */
KDB_EXPORT bool setFieldProperties(KDbField *field, const QMap<QByteArray, QVariant>& values);

/*! Sets property value for @a field. @return true if the property has been found and
 the value is valid for this property. On failure contents of @a field is undefined.
 Properties from extended schema are also supported as well as
   QVariant customProperty(const QString& propertyName) const;

 This function is used e.g. by KDbAlterTableHandler when property information comes in form of text.
 */
KDB_EXPORT bool setFieldProperty(KDbField *field, const QByteArray& propertyName,
                                       const QVariant& value);

/*! @return property value loaded from a DOM @a node, written in a QtDesigner-like
 notation: &lt;number&gt;int&lt;/number&gt; or &lt;bool&gt;bool&lt;/bool&gt;, etc. Supported types are
 "string", "cstring", "bool", "number". For invalid values null QVariant is returned.
 You can check the validity of the returned value using QVariant::type(). */
KDB_EXPORT QVariant loadPropertyValueFromDom(const QDomNode& node, bool *ok);

/*! Convenience version of loadPropertyValueFromDom(). @return int value. */
KDB_EXPORT int loadIntPropertyValueFromDom(const QDomNode& node, bool* ok);

/*! Convenience version of loadPropertyValueFromDom(). @return QString value. */
KDB_EXPORT QString loadStringPropertyValueFromDom(const QDomNode& node, bool* ok);

/*! Saves integer element for value @a value to @a doc document within parent element
 @a parentEl. The value will be enclosed in "number" element and "elementName" element.
 Example: saveNumberElementToDom(doc, parentEl, "height", 15) will create
 @code
  <height><number>15</number></height>
 @endcode
 @return the reference to element created with tag elementName. */
KDB_EXPORT QDomElement saveNumberElementToDom(QDomDocument *doc, QDomElement *parentEl,
        const QString& elementName, int value);

/*! Saves boolean element for value @a value to @a doc document within parent element
 @a parentEl. Like saveNumberElementToDom() but creates "bool" tags. True/false values will be
 saved as "true"/"false" strings.
 @return the reference to element created with tag elementName. */
KDB_EXPORT QDomElement saveBooleanElementToDom(QDomDocument *doc, QDomElement *parentEl,
        const QString& elementName, bool value);

//! @return equivalent of empty (default) value that can be set for a database field of type @a type
/*! In particular returns:
 - empty string for text types,
 - 0 for integer and floating-point types,
 - false for boolean types,
 - a null byte array for BLOB type,
 - current date, time, date+time is returned (measured at client side) for date, time and
   date/time types respectively,
 - a null QVariant for unsupported values such as KDbField::InvalidType. */
KDB_EXPORT QVariant emptyValueForFieldType(KDbField::Type type);

//! @return a value that can be set for a database field of type @a type having "notEmpty" property set.
/*! It works in a similar way as @ref QVariant KDb::emptyValueForFieldType(KDbField::Type type)
 with the following differences:
 - " " string (a single space) is returned for Text and LongText types
 - a byte array with saved "filenew" PNG image (icon) for BLOB type
 Returns null QVariant for unsupported values like KDbField::InvalidType. */
KDB_EXPORT QVariant notEmptyValueForFieldType(KDbField::Type type);

/*! @return true if the @a word is an reserved KDbSQL keyword
 See generated/sqlkeywords.cpp.
 @todo add function returning list of keywords. */
KDB_EXPORT bool isKDbSQLKeyword(const QByteArray& word);

//! @return @a string string with applied KDbSQL identifier escaping
/*! This escaping can be used for field, table, database names, etc.
    Use it for user-visible backend-independent statements.
    @see KDb::escapeIdentifierAndAddQuotes() */
KDB_EXPORT QString escapeIdentifier(const QString& string);

//! @overload QString escapeIdentifier(const QString&)
KDB_EXPORT QByteArray escapeIdentifier(const QByteArray& string);

//! @return @a string string with applied KDbSQL identifier escaping and enclosed in " quotes
/*! This escaping can be used for field, table, database names, etc.
    Use it for user-visible backend-independent statements.
    @see KDb::escapeIdentifier */
KDB_EXPORT QString escapeIdentifierAndAddQuotes(const QString& string);

//! @overload QString escapeIdentifierAndAddQuotes(const QString&)
KDB_EXPORT QByteArray escapeIdentifierAndAddQuotes(const QByteArray& string);

/*! @return escaped string @a string w using KDbSQL dialect,
            i.e. doubles single quotes ("'") and inserts the string into single quotes.
    Quotes "'" are prepended and appended.
    Also escapes \\n, \\r, \\t, \\\\, \\0.
    Use it for user-visible backend-independent statements. */
KDB_EXPORT QString escapeString(const QString& string);

//! Escaping types for BLOCS. Used in escapeBLOB().
enum BLOBEscapingType {
    BLOBEscapeXHex = 1,        //!< escaping like X'1FAD', used by sqlite (hex numbers)
    BLOBEscape0xHex,           //!< escaping like 0x1FAD, used by mysql (hex numbers)
    BLOBEscapeHex,              //!< escaping like 1FAD without quotes or prefixes
    BLOBEscapeOctal           //!< escaping like 'zk\\000$x', used by pgsql
    //!< (only non-printable characters are escaped using octal numbers)
    //!< See http://www.postgresql.org/docs/8.1/interactive/datatype-binary.html
};

//! @todo reverse function for BLOBEscapeOctal is available: processBinaryData() in PostgresqlCursor.cpp - move it here
/*! @return a string containing escaped, printable representation of @a array.
 Escaping is controlled by @a type. For empty array, QString() is returned,
 so if you want to use this function in an SQL statement, empty arrays should be
 detected and "NULL" string should be put instead.
 This is helper, used in KDbDriver::escapeBLOB() and KDb::variantToString(). */
KDB_EXPORT QString escapeBLOB(const QByteArray& array, BLOBEscapingType type);

/*! @return byte array converted from @a data of length @a length.
 @a data is escaped in format used by PostgreSQL's bytea datatype
 described at http://www.postgresql.org/docs/8.1/interactive/datatype-binary.html
 This function is used by PostgreSQL KDb and migration drivers. */
KDB_EXPORT QByteArray pgsqlByteaToByteArray(const char* data, int length);

/*! @return int list converted from string list.
   If @a ok is not 0, *ok is set to result of the conversion. */
KDB_EXPORT QList<int> stringListToIntList(const QStringList &list, bool *ok);

/*! @return string converted from list @a list.
   Separators are ',' characters, "," and "\\" are escaped.
    @see KDb::deserializeList() */
KDB_EXPORT QString serializeList(const QStringList &list);

/*! @return string list converted from @a data which was built using serializeList().
   Separators are ',' characters, escaping is assumed as "\\,". */
KDB_EXPORT QStringList deserializeList(const QString &data);

/*! @return int list converted from @a data which was built using serializeList().
   Separators are ',' characters, escaping is assumed as "\\,".
   If @a ok is not 0, *ok is set to result of the conversion.
   @see KDb::stringListToIntList() */
KDB_EXPORT QList<int> deserializeIntList(const QString &data, bool *ok);

/*! @return string value serialized from a variant value @a v.
 This functions works like QVariant::toString() except the case when @a v is of type:
 - QByteArray - in this case KDb::escapeBLOB(v.toByteArray(), KDb::BLOBEscapeHex) is used.
 - QStringList - in this case KDb::serializeList(v.toStringList()) is used.

 This function is needed for handling values of random type, for example "defaultValue"
 property of table fields can contain value of any type.
 Note: the returned string is an unescaped string. */
KDB_EXPORT QString variantToString(const QVariant& v);

/*! @return variant value of type @a type for a string @a s that was previously serialized using
 @ref variantToString( const QVariant& v ) function.
 @a ok is set to the result of the operation. With exception for types mentioned in documentation
 of variantToString(), QVariant::convert() is used for conversion. */
KDB_EXPORT QVariant stringToVariant(const QString& s, QVariant::Type type, bool* ok);

/*! @return true if setting default value for @a field field is allowed. Fields with unique
 (and thus primary key) flags set do not accept  default values.
 False is returned also if @a field is 0. */
KDB_EXPORT bool isDefaultValueAllowed(KDbField* field);

//! Provides limits for values of type @a type
/*! The result is put into integers pointed by @a minValue and @a maxValue.
 The limits are machine-independent,. what is useful for format and protocol compatibility.
 Supported types are Byte, ShortInteger, Integer and BigInteger.
 The value of @a signedness controls the values; they can be limited to unsigned or not.
 Results for BigInteger or non-integer types are the same as for Integer due to limitation
 of int type. Signed integers are assumed. @a minValue and @a maxValue must not be 0. */
KDB_EXPORT void getLimitsForFieldType(KDbField::Type type, qlonglong *minValue, qlonglong *maxValue,
                                      KDb::Signedness signedness = KDb::Signed);

/*! @return type that's maximum of two integer types @a t1 and @a t2, e.g. Integer for (Byte, Integer).
 If one of the types is not of the integer group, KDbField::InvalidType is returned.
 Returned type may not fit to the result of evaluated expression that involves the arguments.
 For example, 100 is within Byte type, maximumForIntegerFieldTypes(Byte, Byte) is Byte but result
 of 100 * 100 exceeds the range of Byte. */
KDB_EXPORT KDbField::Type maximumForIntegerFieldTypes(KDbField::Type t1, KDbField::Type t2);

//! @return QVariant value converted from a @a data string
/*! Conversion is based on the information about type @a type.
 @a type has to be an element from KDbField::Type, not greater than KDbField::LastType.
 For unsupported type this function fails. @a length value controls number of characters
 used in the conversion. It is optional value for all cases but for the BLOB type because
 for it @a data is not null-terminated so the length cannot be measured.
 The value of @a signedness controls the conversion in case of integer types; numbers can be
 limited to unsigned or not.
 If @a ok is not 0 *ok is set to false on failure and to true on success. On failure a null
 QVariant is returned. The function fails if @a data is 0.
 For rules of conversion to the boolean type see the documentation of @ref QVariant::toBool(),
 QVariant::toDate() for date type, QVariant::toDateTime() for date+time type,
 QVariant::toTime() for time type. */
KDB_EXPORT QVariant cstringToVariant(const char* data, KDbField::Type type, bool *ok, int length = -1,
                                     KDb::Signedness signedness = KDb::Signed);

/*! @return default file-based driver MIME type
 (typically something like "application/x-kexiproject-sqlite") */
KDB_EXPORT QString defaultFileBasedDriverMimeType();

/*! @return default file-based driver ID (currently, "org.kde.kdb.sqlite"). */
KDB_EXPORT QString defaultFileBasedDriverId();

/*! Escapes and converts value @a v (for type @a ftype)
    to string representation required by KDbSQL commands.
    For Date/Time type KDb::dateTimeToSQL() is used.
    For BLOB type KDb::escapeBlob() with BLOBEscape0xHex conversion type is used. */
KDB_EXPORT KDbEscapedString valueToSQL(uint ftype, const QVariant& v);

/*! Converts value @a v to string representation required by KDbSQL commands:
    ISO 8601 DateTime format - with "T" delimiter/
    For specification see http://www.w3.org/TR/NOTE-datetime.
    Example: "1994-11-05T13:15:30" not "1994-11-05 13:15:30".
    @todo Add support for time zones */
KDB_EXPORT KDbEscapedString dateTimeToSQL(const QDateTime& v);

#ifdef KDB_DEBUG_GUI
//! A prototype of handler for GUI debugger
typedef void(*DebugGUIHandler)(const QString&);

//! Sets handler for GUI debugger
KDB_EXPORT void setDebugGUIHandler(DebugGUIHandler handler);

//! Outputs string @a text to the GUI debugger
KDB_EXPORT void debugGUI(const QString& text);

//! A prototype of handler for GUI debugger (specialized for the Alter Table feature)
typedef void(*AlterTableActionDebugGUIHandler)(const QString&, int);

//! Sets handler for GUI debugger (specialized for the Alter Table feature)
KDB_EXPORT void setAlterTableActionDebugHandler(AlterTableActionDebugGUIHandler handler);

//! Outputs string @a text to the GUI debugger (specialized for the Alter Table feature);
//! @a nestingLevel can be provided for nested outputs.
KDB_EXPORT void alterTableActionDebugGUI(const QString& text, int nestingLevel = 0);
#endif

//! @return @a string if it is not empty, else returns @a stringIfEmpty.
/*! This function is an optimization in cases when @a string is a result of expensive
 functioncall because any evaluation will be performed once, not twice. Another advantage
 is simpified code through the functional approach.
 The function expects bool isEmpty() method to be present in type T, so T can typically
 be QString or QByteArray. */
template<typename T>
T iifNotEmpty(const T &string, const T &stringIfEmpty)
{
    return string.isEmpty() ? stringIfEmpty : string;
}

//! @overload iifNotEmpty(const T &string, const T &stringIfEmpty)
template<typename T>
T iifNotEmpty(const QByteArray &string, const T &stringIfEmpty)
{
    return iifNotEmpty(QString(string), stringIfEmpty);
}

//! @overload iifNotEmpty(const T &string, const T &stringIfEmpty)
template<typename T>
T iifNotEmpty(const T &string, const QByteArray &stringIfEmpty)
{
    return iifNotEmpty(string, QString(stringIfEmpty));
}

//! @return @a value if @a ok is true, else returns default value T().
template<typename T>
T iif(bool ok, const T &value)
{
    if (ok) {
        return value;
    }
    return T();
}

/*! @return a list of paths that KDb will search when dynamically loading libraries (plugins)
 This is basicaly list of directories returned QCoreApplication::libraryPaths() that have readable
 subdirectory "kdb".
 @see QCoreApplication::libraryPaths() */
KDB_EXPORT QStringList libraryPaths();

/*! @return new temporary name suitable for creating new table.
 The name has mask tmp__{baseName}{rand} where baseName is passed as argument and {rand}
 is a 10 digits long hexadecimal number. @a baseName can be empty. It is adviced to use
 the returned name as quickly as possible for creating new physical table.
 It is not 100% guaranteed that table with this name will not exist at an attempt of creation
 but it is very unlikely. The function checks for existence of table in connection @a conn.*/
KDB_EXPORT QString temporaryTableName(KDbConnection *conn, const QString &baseName);

/*! @return absolute path to "sqlite3" program.
 Empty string is returned if the program was not found. */
KDB_EXPORT QString sqlite3ProgramPath();

/*! Imports file in SQL format from @a inputFileName into @a outputFileName.
 Works for any SQLite 3 dump file. Requires access to executing the "sqlite3" command.
 File named @a outputFileName will be silently overwritten with a new SQLite 3 database file.
 @return true on success. */
KDB_EXPORT bool importSqliteFile(const QString &inputFileName, const QString &outputFileName);

/*! @return true if @a s is a valid identifier, ie. starts with a letter or '_' character
 and contains only letters, numbers and '_' character. */
KDB_EXPORT bool isIdentifier(const QString& s);

/*! @return valid identifier based on @a s.
 Non-alphanumeric characters (or spaces) are replaced with '_'.
 If a number is at the beginning, '_' is added at start.
 Empty strings are not changed. Case remains unchanged. */
KDB_EXPORT QString stringToIdentifier(const QString &s);

/*! @return useful message "Value of "valueName" column must be an identifier.
  "v" is not a valid identifier.". It is also used by KDbIdentifierValidator.  */
KDB_EXPORT QString identifierExpectedMessage(const QString &valueName,
        const QVariant& v);

} // namespace KDb

#endif

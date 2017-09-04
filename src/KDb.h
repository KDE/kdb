/* This file is part of the KDE project
   Copyright (C) 2004-2017 Jarosław Staniek <staniek@kde.org>

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

#include "KDbField.h"
#include "KDbGlobal.h"
#include "KDbSqlResult.h"
#include "KDbTableSchema.h"

class QDomNode;
class QDomElement;
class QDomDocument;

class KDbConnection;
class KDbConnectionData;
class KDbDriver;
class KDbEscapedString;
class KDbLookupFieldSchema;
class KDbMessageHandler;
class KDbResultable;
class KDbResultInfo;
class KDbTableOrQuerySchema;
class KDbVersionInfo;
class tristate;

namespace KDb
{

//! @return runtime information about version of the KDb library.
//! @see KDbConnection::databaseVersion() KDbConnection::serverVersion() KDbDriverMetaData::version()
KDB_EXPORT KDbVersionInfo version();

/**
 * @brief Deletes records using one generic criteria.
 *
 * @return @c true on success and @c false on failure and if @a conn is @c nullptr
 */
KDB_EXPORT bool deleteRecords(KDbConnection* conn, const QString &tableName,
                              const QString &keyname, KDbField::Type keytype, const QVariant &keyval);

//! @overload
inline bool deleteRecords(KDbConnection* conn, const KDbTableSchema &table,
                              const QString &keyname, KDbField::Type keytype, const QVariant &keyval)
{
    return deleteRecords(conn, table.name(), keyname, keytype, keyval);
}

//! @overload
inline bool deleteRecords(KDbConnection* conn, const QString &tableName,
                              const QString &keyname, const QString &keyval)
{
    return deleteRecords(conn, tableName, keyname, KDbField::Text, keyval);
}

//! @overload
inline bool deleteRecords(KDbConnection* conn, const KDbTableSchema &table,
                              const QString &keyname, const QString &keyval)
{
    return deleteRecords(conn, table.name(), keyname, keyval);
}

//! @overload
inline bool deleteRecords(KDbConnection* conn, const KDbTableSchema &table,
                              const QString& keyname, int keyval)
{
    return deleteRecords(conn, table, keyname, KDbField::Integer, keyval);
}

//! @overload
inline bool deleteRecords(KDbConnection* conn, const QString &tableName,
                              const QString& keyname, int keyval)
{
    return deleteRecords(conn, tableName, keyname, KDbField::Integer, keyval);
}

//! Deletes records with two generic criterias.
KDB_EXPORT bool deleteRecords(KDbConnection* conn, const QString &tableName,
                             const QString &keyname1, KDbField::Type keytype1, const QVariant& keyval1,
                             const QString &keyname2, KDbField::Type keytype2, const QVariant& keyval2);

//! Deletes records with three generic criterias.
KDB_EXPORT bool deleteRecords(KDbConnection* conn, const QString &tableName,
                             const QString &keyname1, KDbField::Type keytype1, const QVariant& keyval1,
                             const QString &keyname2, KDbField::Type keytype2, const QVariant& keyval2,
                             const QString &keyname3, KDbField::Type keytype3, const QVariant& keyval3);

//! Deletes all records from table @a tableName.
KDB_EXPORT bool deleteAllRecords(KDbConnection* conn, const QString &tableName);

//! @overload bool deleteAllRecords(KDbConnection*, const QString&);
inline bool deleteAllRecords(KDbConnection* conn, const KDbTableSchema &table)
{
    return KDb::deleteAllRecords(conn, table.name());
}

/**
 * Returns value of last inserted record for an autoincrement field
 *
 * This method internally fetches values of the last inserted record for which @a result was
 * returned and returns selected field's value. The field belong to @a tableName table.
 * The field must be of integer type, there must be a record inserted using query for which the @a
 * result is returned.
 * std::numeric_limits<quint64>::max() is returned on error or if @a result is null.
 * The last inserted record is identified by a magical record identifier, usually called ROWID
 * (PostgreSQL has it as well as SQLite;
 * see KDbDriverBehavior::ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE).
 * ROWID's value will be assigned back to @a recordId if this pointer is not @c nullptr.
 */
KDB_EXPORT quint64 lastInsertedAutoIncValue(QSharedPointer<KDbSqlResult> result,
                                            const QString &autoIncrementFieldName,
                                            const QString &tableName, quint64 *recordId = nullptr);

/**
 * @overload
 *
 * Accepts @a recordId that can be obtained from KDbPreparedStatement::lastInsertRecordId()
 * or KDbSqlResult::lastInsertRecordId().
*/
KDB_EXPORT quint64 lastInsertedAutoIncValue(KDbConnection *conn, const quint64 recordId,
                                            const QString &autoIncrementFieldName,
                                            const QString &tableName);

/**
@overload
*/
inline quint64 lastInsertedAutoIncValue(QSharedPointer<KDbSqlResult> result,
                                        const QString &autoIncrementFieldName,
                                        const KDbTableSchema &table, quint64 *recordId = nullptr)
{
    return lastInsertedAutoIncValue(result, autoIncrementFieldName, table.name(), recordId);
}

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

/*! @return true if value @a value represents an empty (but not null) value.
  - Case 1: If field type @a type is of any text type (KDbField::isTextType(type) == true)
    then the function returns true if @a value casted to a QString value is empty and not null.
  - Case 2: If field type @a type is KDbField::BLOB then the function returns if @a value casted
    to a QByteArray value is empty and not null.
  - Case 3: If field type @a type is of any other type then the function returns true if value.isNull().
 @see KDbField::hasEmptyProperty() */
KDB_EXPORT bool isEmptyValue(KDbField::Type type, const QVariant &value);

/**
 * @brief Sets HTML-formatted error message with extra details obtained from result object
 *
 * Sets string pointed by @a msg to an error message retrieved from @a resultable,
 * and string pointed by @a details to details of this error (server message and result number).
 * Does nothing if there is no error (resultable.result().isError() == false) or if @a msg or
 * @a details is @c nullptr.
 * In this case strings pointer by @a msg and @a details strings are not changed.
 * If the string pointed by @a msg is not empty, it is not modified and message obtained from
 * @a resultable is appended to the string pointed by @a details instead.
 */
KDB_EXPORT void getHTMLErrorMesage(const KDbResultable& resultable, QString *msg, QString *details);

/**
 * @overload
 *
 * This methods works similarly but appends both a message and a description to string pointed by
 * @a msg.
 */
KDB_EXPORT void getHTMLErrorMesage(const KDbResultable& resultable, QString *msg);

/**
 * @overload
 *
 * This methods similarly but outputs message to @a info instead.
 */
KDB_EXPORT void getHTMLErrorMesage(const KDbResultable& resultable, KDbResultInfo *info);

/*! Function useful for building WHERE parts of SQL statements.
 Constructs an SQL string like "fielname = value" for specific @a drv driver,
 field type @a t, @a fieldName and @a value. If @a value is null, "fieldname IS NULL"
 string is returned. */
KDB_EXPORT KDbEscapedString sqlWhere(KDbDriver *drv, KDbField::Type t,
                                        const QString& fieldName, const QVariant& value);

/**
 * @brief Finds an identifier for object @a objName of type @a objType
 *
 * On success true is returned and *id is set to the value of the identifier.
 * On failure or if @a conn is @c nullptr, @c false is returned.
 * If there is no object with specified name and type, @c cancelled value is returned.
*/
KDB_EXPORT tristate idForObjectName(KDbConnection* conn, int *id, const QString& objName,
                                    int objType);

/**
 * @brief Shows connection test dialog
 *
 * Shows connection test dialog with a progress bar indicating connection testing
 * (within a separate thread). @a data is used to perform a (temporary) test connection.
 * @a msgHandler can be used for error handling. @a parent is used as dialog's parent widget.
 *
 * The dialog is modal so the call is blocking.
 *
 * On successful connecting, a successfull message of type KDbMessageHandler::Information is passed
 * to @a msgHandler. After testing, temporary connection is closed.
 *
 * @return @c true for successfull connecting, @c for failed connecting and @c cancelled if the test
 * has been cancelled.
 */
KDB_EXPORT tristate showConnectionTestDialog(QWidget* parent, const KDbConnectionData& data,
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
 - @a tableName or @a fieldName is @c nullptr

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
 * If @a decimalPlaces is < 0, all meaningful fractional digits are returned (up to 10).
 * If @a automatically is 0, just integer part is returned.
 * If @a automatically is > 0, fractional part should take exactly
   N digits: if the fractional part is shorter than N, additional zeros are appended.
   Examples:
   * numberToString(12.345, 6) == "12.345000"
   * numberToString(12.345, 0) == "12"
   * numberToString(12.345, -1) == "12.345"
   * numberToString(12.0, -1) == "12"
   * numberToString(0.0, -1) == "0"

 @note No rounding is performed
 @note No thousands group separator is used. Decimal symbol is '.'.

 @see KDb::numberToLocaleString() KDbField::visibleDecimalPlaces() */
KDB_EXPORT QString numberToString(double value, int decimalPlaces);

/**
 * @brief Returns number converted to string using default locale
 *
 * This method is similar to KDb::numberToString() but the string is formatted using QLocale::toString().
 *
 * @see KDb::numberToString() KDbField::visibleDecimalPlaces()
 */
KDB_EXPORT QString numberToLocaleString(double value, int decimalPlaces);

/**
 * @overload
 *
 * Returns number converted to string using specified locale.
 */
KDB_EXPORT QString numberToLocaleString(double value, int decimalPlaces, const QLocale &locale);

//! @return true if @a propertyName is a builtin field property.
KDB_EXPORT bool isBuiltinTableFieldProperty(const QByteArray& propertyName);

//! @return true if @a propertyName is an extended field property.
KDB_EXPORT bool isExtendedTableFieldProperty(const QByteArray& propertyName);

//! @return true if @a propertyName is belongs to lookup field's schema.
KDB_EXPORT bool isLookupFieldSchemaProperty(const QByteArray& propertyName);

/*! @return type of field for integer value @a type.
 If @a type cannot be casted to KDbField::Type or is not normal type,
 i.e. @a type > KDbField::LastType (e.g. KDbField::Null), KDbField::InvalidType is returned.
 This can be used when type information is deserialized from a string or QVariant.
 @see KDbField::typesCount() KDbField::specialTypesCount() */
KDB_EXPORT KDbField::Type intToFieldType(int type);

/*! @return type group of field for integer value @a typeGroup.
 If @a typeGroup cannot be casted to KDbField::TypeGroup, KDbField::InvalidGroup is returned.
 This can be used when type information is deserialized from a string or QVariant.
 @see KDbField::typeGroupsCount() */
KDB_EXPORT KDbField::TypeGroup intToFieldTypeGroup(int typeGroup);

/*! Gets property values for the lookup schema @a lookup.
 @a values is not cleared before filling. This function is used e.g. for altering table design.
 Nothing is performed if @a values is @c nullptr.
 If @a lookup is @c nullptr, all returned values are null.
*/
KDB_EXPORT void getProperties(const KDbLookupFieldSchema *lookup, QMap<QByteArray, QVariant> *values);

/*! Gets property values for @a field.
 Properties from extended schema are included. @a values is cleared before filling.
 The same number of properties in the same order is returned.
 This function is used e.g. for altering table design.
 Nothing is performed if @a values is @c nullptr.
 */
KDB_EXPORT void getFieldProperties(const KDbField &field, QMap<QByteArray, QVariant> *values);

/*! Sets property values for @a field. @return true if all the values are valid and allowed.
 On failure contents of @a field is undefined.
 Properties from extended schema are also supported.
 This function is used e.g. by KDbAlterTableHandler when property information comes in form of text.
 If @a field is @c nullptr nothing is performed and @c false is returned.
 */
KDB_EXPORT bool setFieldProperties(KDbField *field, const QMap<QByteArray, QVariant>& values);

/*! Sets value of a single property for @a field. @return true if the property has been found and
 the value is valid for this property. On failure contents of @a field is not modified.
 Properties from extended schema are also supported as well as custom properties
 (using KDbField::setCustomProperty()).

 This function is used e.g. by KDbAlterTableHandler when property information comes in form of text.
 If @a field is @c nullptr nothing is performed and @c false is returned.
 */
KDB_EXPORT bool setFieldProperty(KDbField *field, const QByteArray &propertyName,
                                 const QVariant &value);

/*! @return property value loaded from a DOM @a node, written in a QtDesigner-like
 notation: &lt;number&gt;int&lt;/number&gt; or &lt;bool&gt;bool&lt;/bool&gt;, etc. Supported types are
 "string", "cstring", "bool", "number". For invalid values null QVariant is returned.
 Validity of the returned value can be checked using the @a ok parameter and QVariant::type(). */
KDB_EXPORT QVariant loadPropertyValueFromDom(const QDomNode& node, bool *ok);

/*! Convenience version of loadPropertyValueFromDom(). @return int value. */
KDB_EXPORT int loadIntPropertyValueFromDom(const QDomNode& node, bool* ok);

/*! Convenience version of loadPropertyValueFromDom(). @return QString value. */
KDB_EXPORT QString loadStringPropertyValueFromDom(const QDomNode& node, bool* ok);

/*! Creates a new DOM element named @a elementName with numeric value @a value in @a doc document
 within parent element @a parentEl. The value will be enclosed in "number" element and
 "elementName" element.
 Example: saveNumberElementToDom(doc, parentEl, "height", 15) creates:
 @code
  <height><number>15</number></height>
 @endcode
 @return the reference to element created with tag elementName.
 Null element is returned if @a doc or @a parentEl is @c nullptr or if @a elementName is empty.
*/
KDB_EXPORT QDomElement saveNumberElementToDom(QDomDocument *doc, QDomElement *parentEl,
                                              const QString& elementName, int value);

/*! Creates a new DOM element named @a elementName with boolean value @a value in @a doc document
 within parent element @a parentEl.
 This method is like saveNumberElementToDom() but creates "bool" tags. True/false values will be
 saved as "true"/"false" strings.
 Example: saveBooleanElementToDom(doc, parentEl, "visible", true) creates:
 @code
  <visible><bool>true</bool></visible>
 @endcode
 @return the reference to element created with tag elementName.
 Null element is returned if @a doc or @a parentEl is @c nullptr or if @a elementName is empty.
*/
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
KDB_EXPORT bool isKDbSqlKeyword(const QByteArray& word);

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

/*! @return escaped string @a string for the KDbSQL dialect,
            i.e. doubles single quotes ("'") and inserts the string into single quotes.
    Quotes "'" are prepended and appended.
    Also escapes \\n, \\r, \\t, \\\\, \\0.
    Use it for user-visible backend-independent statements.
    @see unescapeString() */
KDB_EXPORT QString escapeString(const QString& string);

/**
 * @brief Returns escaped string @a string
 *
 * If @a drv driver is present, it is used to perform escaping, otherwise escapeString() is used
 * so the KDbSQL dialect-escaping is performed.
 *
 * @since 3.1.0
 */
KDB_EXPORT KDbEscapedString escapeString(KDbDriver *drv, const QString& string);

/**
 * @brief Returns escaped string @a string
 *
 * If @a conn is present, its driver is used to perform escaping, otherwise escapeString() is used
 * so the KDbSQL dialect-escaping is performed.
 *
 * @since 3.1.0
 */
KDB_EXPORT KDbEscapedString escapeString(KDbConnection *conn, const QString& string);

//! Unescapes characters in string @a string for the KDbSQL dialect.
/** The operation depends on @a quote character, which can be be ' or ".
 * @a string is assumed to be properly constructed. This is assured by the lexer's grammar.
 * Used by lexer to recognize the CHARACTER_STRING_LITERAL token.
 * @return unescaped string and sets value pointed by @a errorPosition (if any) to -1 on success;
 * and to index of problematic character on failure.
 * The function fails when unsupported @a quote character is passed or for unsupported sequences.
 *
 * Example sequences for ' character quote:
 * - \' -> ' (escaping)
 * - \" -> " (escaping)
 * - '' -> ' (repeated quote escapes too)
 * - "" -> "" (repeated but this is not the quote)
 * - ' -> (disallowed, escaping needed)
 * - " -> "
 * Example sequences for " character quote:
 * - \' -> ' (escaping)
 * - \" -> " (escaping)
 * - " -> " (disallowed, escaping needed)
 * - "" -> " (repeated quote escapes too)
 * - '' -> '' (repeated but this is not the quote)
 * - ' -> '
 *
 * Following sequences are always unescaped (selection based on a mix of MySQL and C/JavaScript):
 * - \0 -> NULL (QChar())
 * - \b -> backspace 0x8
 * - \f -> form feed 0xc
 * - \n -> new line 0xa
 * - \r -> carriage return 0xd
 * - \t -> horizontal tab 0x9
 * - \v -> vertical tab 0xb
 * - \\ -> backslash
 * - \? -> ? (useful when '?' placeholders are used)
 * - \% ->  (useful when '%' wildcards are used e.g. for the LIKE operator)
 * - \_ ->  (useful when '_' pattern is used e.g. for the LIKE operator)
 * - \xhh -> a character for which hh (exactly 2 digits) is interpreted as an hexadecimal
 *           number, 00 <= hh <= FF. Widely supported by programming languages.
 *           Can be also 00 <= hh <= ff.
 *           Example: \xA9 translates to "©".
 * - \uxxxx -> 16-bit unicode character, exactly 4 digits, each x is a hexadecimal digit,
 *           case insensitive; known from JavaScript, Java, C/C++. 0000 <= xxxxxx <= FFFF
 *           Example: \u2665 translates to "♥".
 * - \u{xxxxxx} -> 24-bit unicode "code point" character, each x is a hexadecimal digit,
 *           case insensitive; known from JavaScript (ECMAScript 6). 0 <= xxxxxx <= 10FFFF
 *           Example: \u{1D306} translates to "𝌆"
 *
 * @note Characters without special meaning can be escaped, but then the "\" character
 *       is skipped, e.g. "\a" == "a".
 * @note Trailing "\" character in @a string is ignored.
 * @note \nnn octal notation is not supported, it may be confusing and conflicting
 *       when combined with other characters (\0012 is not the same as \012).
 *       The industry is moving away from it and EcmaScript 5 deprecates it.
 *
 * See also:
 * - http://dev.mysql.com/doc/refman/5.7/en/string-literals.html
 * - https://en.wikipedia.org/wiki/Escape_sequences_in_C#Table_of_escape_sequences
 * - https://developer.mozilla.org/en-US/docs/Web/JavaScript/Guide/Grammar_and_types#Using_special_characters_in_strings
 * - https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Lexical_grammar#String_literals
 */
KDB_EXPORT QString unescapeString(const QString& string, char quote, int *errorPosition = nullptr);

//! Escaping types for BLOBS. Used in escapeBLOB().
enum class BLOBEscapingType {
    XHex = 1,        //!< Escaping like X'1FAD', used by sqlite (hex numbers)
    ZeroXHex,           //!< Escaping like 0x1FAD, used by mysql (hex numbers)
    Hex,             //!< Escaping like 1FAD without quotes or prefixes
    Octal,           //!< Escaping like 'zk\\000$x', used by PostgreSQL
                               //!< (only non-printable characters are escaped using octal numbers);
                               //!< see http://www.postgresql.org/docs/9.5/interactive/datatype-binary.html
    ByteaHex         //!< "bytea hex" escaping, e.g. E'\xDEADBEEF'::bytea used by PostgreSQL
                               //!< (only non-printable characters are escaped using octal numbers);
                               //!< see http://www.postgresql.org/docs/9.5/interactive/datatype-binary.html
};

/*! @return a string containing escaped, printable representation of @a array.
 Escaping is controlled by @a type. For empty array, QString() is returned,
 so if you want to use this function in an SQL statement, empty arrays should be
 detected and "NULL" string should be put instead.
 This is helper, used in KDbDriver::escapeBLOB() and KDb::variantToString(). */
KDB_EXPORT QString escapeBLOB(const QByteArray& array, BLOBEscapingType type);

/*! @return byte array converted from @a data of length @a length.
 If @a length is negative, the data is assumed to point to a null-terminated string
 and its length is determined dynamically.
 @a data is escaped in format used by PostgreSQL's bytea datatype
 described at http://www.postgresql.org/docs/8.1/interactive/datatype-binary.html
 This function is used by PostgreSQL KDb and migration drivers. */
KDB_EXPORT QByteArray pgsqlByteaToByteArray(const char* data, int length = -1);

/*! @return byte array converted from @a data of length @a length.
 If @a length is negative, the data is assumed to point to a null-terminated string
 and its length is determined dynamically.
 @a data is escaped in format X'*', where * is one or more hexadecimal digits.
 Both A-F and a-f letters are supported. Even and odd number of digits are supported.
 If @a ok is not 0, *ok is set to result of the conversion.
 See BLOBEscapingType::XHex. */
KDB_EXPORT QByteArray xHexToByteArray(const char* data, int length = -1, bool *ok = nullptr);

/*! @return byte array converted from @a data of length @a length.
 If @a length is negative, the data is assumed to point to a null-terminated string
 and its length is determined dynamically.
 @a data is escaped in format 0x*, where * is one or more hexadecimal digits.
 Both A-F and a-f letters are supported. Even and odd number of digits are supported.
 If @a ok is not 0, *ok is set to result of the conversion.
 See BLOBEscapingType::ZeroXHex. */
KDB_EXPORT QByteArray zeroXHexToByteArray(const char* data, int length = -1, bool *ok = nullptr);

/*! @return int list converted from string list.
   If @a ok is not 0, *ok is set to result of the conversion. */
KDB_EXPORT QList<int> stringListToIntList(const QStringList &list, bool *ok = nullptr);

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
 (and thus primary key) flags set do not accept  default values. */
KDB_EXPORT bool isDefaultValueAllowed(const KDbField &field);

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
    For Date/Time type KDb::dateTimeToSql() is used.
    For BLOB type KDb::escapeBlob() with BLOBEscapingType::ZeroXHex conversion type is used. */
KDB_EXPORT KDbEscapedString valueToSql(KDbField::Type ftype, const QVariant& v);

/*! Converts value @a v to string representation required by KDbSQL commands:
    ISO 8601 DateTime format - with "T" delimiter/
    For specification see http://www.w3.org/TR/NOTE-datetime.
    Example: "1994-11-05T13:15:30" not "1994-11-05 13:15:30".
    @todo Add support for time zones */
KDB_EXPORT KDbEscapedString dateTimeToSql(const QDateTime& v);

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
    return iifNotEmpty(QLatin1String(string), stringIfEmpty);
}

//! @overload iifNotEmpty(const T &string, const T &stringIfEmpty)
template<typename T>
T iifNotEmpty(const T &string, const QByteArray &stringIfEmpty)
{
    return iifNotEmpty(string, QLatin1String(stringIfEmpty));
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
 but it is very unlikely. The function checks for existence of a table with temporary name
 for connection @a conn. Empty string is returned if @a conn is @c nullptr or is not open
 or if checking for existence of table with temporary name failed.
*/
KDB_EXPORT QString temporaryTableName(KDbConnection *conn, const QString &baseName);

/*! @return absolute path to "sqlite3" program.
 Empty string is returned if the program was not found. */
KDB_EXPORT QString sqlite3ProgramPath();

/*! Imports file in SQL format from @a inputFileName into @a outputFileName.
 Works for any SQLite 3 dump file. Requires access to executing the "sqlite3" command.
 File named @a outputFileName will be silently overwritten with a new SQLite 3 database file.
 @return true on success. */
KDB_EXPORT bool importSqliteFile(const QString &inputFileName, const QString &outputFileName);

/*! @return @c true if @a s is a valid identifier, i.e. starts with a letter or '_' character
 and contains only letters, numbers and '_' character. */
KDB_EXPORT bool isIdentifier(const QString& s);

//! @overload isIdentifier(const QString& s)
//! @since 3.1
KDB_EXPORT bool isIdentifier(const QByteArray& s);

/*! @return valid identifier based on @a s.
 Non-alphanumeric characters (or spaces) are replaced with '_'.
 If a number is at the beginning, '_' is added at start.
 Empty strings are not changed. Case remains unchanged. */
KDB_EXPORT QString stringToIdentifier(const QString &s);

/*! @return useful message "Value of "valueName" field must be an identifier.
  "v" is not a valid identifier.". It is also used by KDbIdentifierValidator.  */
KDB_EXPORT QString identifierExpectedMessage(const QString &valueName,
        const QVariant& v);

} // namespace KDb

#endif

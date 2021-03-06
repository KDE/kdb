/* This file is part of the KDE project
   Copyright (C) 2002 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2003-2017 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_FIELD_H
#define KDB_FIELD_H

#include <QPair>
#include <QVector>
#include <QStringList>
#include <QHash>
#include <QCoreApplication>

#include "KDbUtils.h"

class KDbTableSchema;
class KDbQuerySchema;
class KDbFieldList;
class KDbExpression;

//! Meta-data for a field
/*! KDbField provides information about single database field.

 KDbField class has defined following members:
 - name
 - type
 - database constraints
 - additional options
 - maxLength (makes sense mostly for string types)
 - maxLengthStrategy (makes sense mostly for string types)
 - precision (for floating-point type)
 - defaultValue
 - caption (user readable name that can be e.g. translated)
 - description (user readable name additional text, can be useful for developers)
 - defaultWidth (a hint for displaying in tabular mode or as text box)

 KDbField can also have assigned expression (see KDbExpression class,
 and expression() method).

 Note that aliases for fields are defined within query, not in KDbField object,
 because the same field can be used in different queries with different alias.

 Notes for advanced use: KDbField obeject is designed to be owned by a parent object.
 Such a parent object can be KDbTableSchema, if the field defines single table column,
 or KDbQuerySchema, if the field defines an expression (KDbExpression class).

 Using expression class for fields allos to define expressions within queries like
 "SELECT AVG(price) FROM products"

 You can choose whether your field is owned by query or table,
 using appropriate constructor, or using parameterless constructor and
 calling setTable() or setQuery() later.
*/
class KDB_EXPORT KDbField
{
    Q_GADGET
    Q_FLAGS(Constraints Constraint Options Option)
    Q_DECLARE_TR_FUNCTIONS(KDbField)
public:
    typedef KDbUtils::AutodeletedList<KDbField*> List; //!< list of fields
    typedef QVector<KDbField*> Vector; //!< vector of fields
    typedef QList<KDbField*>::ConstIterator ListIterator; //!< iterator for list of fields
    typedef QPair<KDbField*, KDbField*> Pair; //!< fields pair
    typedef QList<Pair> PairList; //!< list of fields pair

    /*! Unified (most common used) types of fields. */
    enum Type {
        //-- Normal types:
        InvalidType = 0, /*!< Unsupported/Unimplemented type */
        Byte = 1,        /*!< 1 byte, signed or unsigned */
        FirstType = 1, /*! First type */
        ShortInteger = 2,/*!< 2 bytes, signed or unsigned */
        Integer = 3,     /*!< 4 bytes, signed or unsigned */
        BigInteger = 4,  /*!< 8 bytes, signed or unsigned */
        Boolean = 5,     /*!< 0 or 1 */
        Date = 6,        /*!< */
        DateTime = 7,    /*!< */
        Time = 8,        /*!< */
        Float = 9,       /*!< 4 bytes */
        Double = 10,     /*!< 8 bytes */
        Text = 11,       /*!< Other name: Varchar */
        LongText = 12,   /*!< Other name: Memo */
        BLOB = 13,       /*!< Large binary object */

        LastType = 13,   /*!< This line should be at the end of the list of types! */

        Null = 128,       /*!< Used for fields that are "NULL" expressions. */

        //-- Special, internal types:
        Asterisk = 129,  /*!< Used in KDbQueryAsterisk subclass objects only,
                             not used in table definitions,
                             but only in query definitions */
        Enum = 130,      /*!< An integer internal with a string list of hints */
        Map = 131,       /*!< Mapping from string to string list (more generic than Enum) */
        Tuple = 132,     /*!< A list of values (e.g. arguments of a function) */
        LastSpecialType = Tuple /*!< This line should be at the end of the list of special types! */
    };
    Q_ENUM(Type)

    /*! Type groups for fields. */
    enum TypeGroup {
        InvalidGroup = 0,
        TextGroup = 1,
        IntegerGroup = 2,
        FloatGroup = 3,
        BooleanGroup = 4,
        DateTimeGroup = 5,
        BLOBGroup = 6, /* large binary object */

        LastTypeGroup = 6 // This line should be at the end of the enum!
    };
    Q_ENUM(TypeGroup)

    /*! Possible constraints defined for a field. */
    enum Constraint {
        NoConstraints = 0,
        AutoInc = 1,
        Unique = 2,
        PrimaryKey = 4,
        ForeignKey = 8,
        NotNull = 16,
        NotEmpty = 32, //!< only legal for string-like and blob fields
        Indexed = 64
    };
    Q_DECLARE_FLAGS(Constraints, Constraint)

    /*! Possible options defined for a field. */
    enum Option {
        NoOptions = 0,
        Unsigned = 1
    };
    Q_DECLARE_FLAGS(Options, Option)

    /*! Creates a database field as a child of @a tableSchema table.
     No other properties are set (even the name), so these should be set later. */
    explicit KDbField(KDbTableSchema *tableSchema);

    /*! Creates a database field.
     maxLength property is set to 0 (unlimited length).
     No other properties are set (even the name), so these should be set later. */
    KDbField();

    /*! Creates a database field with specified properties.
     For meaning of @a maxLength argument please refer to setMaxLength(). */
    KDbField(const QString &name, Type type, Constraints constr = NoConstraints,
             Options options = NoOptions, int maxLength = 0, int precision = 0,
             const QVariant &defaultValue = QVariant(), const QString &caption = QString(),
             const QString &description = QString());

    /*! Constructs a deep copy of field @a f. */
    KDbField(const KDbField &f);

    virtual ~KDbField();

    //! @return parent for this field (table, query, etc.)
    KDbFieldList *parent();

    //! @overload
    const KDbFieldList *parent() const;

    //! @return number of normal types available, i.e. types > InvalidType and <= LastType.
    static int typesCount();

    //! @return number of special types available (Asterisk, Enum, etc.), that means
    static int specialTypesCount();

    //! @return number of type groups available
    static int typeGroupsCount();

    //! Converts type @a type to QVariant equivalent as accurate as possible
    /*! Only normal types are supported.
     @see typesCount() specialTypesCount() */
    static QVariant::Type variantType(Type type);

    //! Converts value @a value to variant corresponding to type @a type.
    /*! Only normal types are supported.
     If converting is not possible a null value is returned. */
    static QVariant convertToType(const QVariant &value, Type type);

    //! @return a translated type name for @a type
    /*! @a type has to be an element from KDbField::Type, not greater than KDbField::LastType.
     Only normal types and KDbField::Null are supported.
     For unsupported types empty string is returned. */
    //! @see typesCount() specialTypesCount()
    static QString typeName(Type type);

    //! @return list of all available translated names of normal types
    /*! The first element of the list is the name of KDbField::InvalidType, the last one
     is a name of KDbField::LastType.
     @see typesCount() specialTypesCount() */
    static QStringList typeNames();

    //! @return a nontranslated type string for @a type
    /*! For example returns "Integer" for KDbType::Integer.
        @a type has to be an element from KDbField::Type, not greater than KDbField::LastType;
        KDbField::Null is also supported.
        For unsupported types empty string is returned. */
    static QString typeString(Type type);

    //! @return type for a given nontranslated type string @a typeString
    /*! For example returns KDbType::Integer for "Integer".
        @a typeString has to be name of type not greater than KDbField::LastType;
        KDbField::Null is also supported.
        For unsupported value KDbField::InvalidType is returned. */
    static Type typeForString(const QString& typeString);

    //! @return type group for a given nontranslated type group @a typeGroupString
    /*! For example returns KDbField::TextGroup for "TextGroup" string.
        For unsupported value KDbField::InvalidGroup is returned. */
    static TypeGroup typeGroupForString(const QString& typeGroupString);

    //! @return group for @a type
    /*! For example returns KDbField::TextGroup for KDbField::Text type.
        @a type has to be an element from KDbField::Type, not greater than KDbField::LastType.
        For unsupported type KDbField::InvalidGroup is returned. */
    static TypeGroup typeGroup(Type type);

    //! @return a translated group name for @a typeGroup
    static QString typeGroupName(TypeGroup typeGroup);

    //! @return list of all available translated type group names
    /*! The first element of the list is the name of KDbField::InvalidGroup, the last one
        is a name of KDbField::LastTypeGroup. */
    static QStringList typeGroupNames();

    //! @return a nontranslated type group string for @a typeGroup, e.g. "IntegerGroup" for IntegerGroup type
    static QString typeGroupString(TypeGroup typeGroup);

    /*! @return the name of this field */
    QString name() const;

    /*! @return table schema of table that owns this field
     or null if it has no table assigned.
     @see query() */
    KDbTableSchema* table();

    //! @overload KDbTableSchema* table()
    const KDbTableSchema* table() const;

    /*! Sets @a table schema of table that owns this field.
     This does not adds the field to @a table object.
     You do not need to call this method by hand.
     Call KDbTableSchema::addField(KDbField *field) instead.
     @see setQuery() */
    void setTable(KDbTableSchema *table);

    /*! For special use when the field defines expression.
     @return query schema of query that owns this field
     or null if it has no query assigned.
     @see table() */
    KDbQuerySchema* query();

    //! @overload KDbQuerySchema* query()
    const KDbQuerySchema* query() const;

    /*! For special use when field defines expression.
     Sets @a query schema of query that owns this field.
     This does not adds the field to @a query object.
     You do not need to call this method by hand.
     Call KDbQuerySchema::addField() instead.
     @see setQuery() */
    void setQuery(KDbQuerySchema *query);

    /*! @return true if the field is autoincrement (e.g. integer/numeric) */
    inline bool isAutoIncrement() const {
        return constraints() & AutoInc;
    }

    /*! @return true if the field is member of single-field primary key */
    inline bool isPrimaryKey() const {
        return constraints() & PrimaryKey;
    }

    /*! @return true if the field is member of single-field unique key */
    inline bool isUniqueKey() const {
        return constraints() & Unique;
    }

    /*! @return true if the field is member of single-field foreign key */
    inline bool isForeignKey() const {
        return constraints() & ForeignKey;
    }

    /*! @return true if the field is not allowed to be null */
    inline bool isNotNull() const {
        return constraints() & NotNull;
    }

    /*! @return true if the field is not allowed to be null */
    inline bool isNotEmpty() const {
        return constraints() & NotEmpty;
    }

    /*! @return true if the field is indexed using single-field database index. */
    inline bool isIndexed() const {
        return constraints() & Indexed;
    }

    /*! @return true if the field is of any numeric type (integer or floating point) */
    inline bool isNumericType() const {
        return KDbField::isNumericType(type());
    }

    /*! Static version of isNumericType() method
     @return true if the field is of any numeric type (integer or floating point)*/
    static bool isNumericType(Type type);

    /*! @return true if the field is of any integer type */
    inline bool isIntegerType() const {
        return KDbField::isIntegerType(type());
    }

    /*! Static version of isIntegerType() method
     @return true if the field is of any integer type */
    static bool isIntegerType(Type type);

    /*! @return true if the field is of any floating point numeric type */
    inline bool isFPNumericType() const {
        return KDbField::isFPNumericType(type());
    }

    /*! static version of isFPNumericType() method
     @return true if the field is of any floating point numeric type */
    static bool isFPNumericType(Type type);

    /*! @return true if the field is of any date or time related type */
    inline bool isDateTimeType() const {
        return KDbField::isDateTimeType(type());
    }

    /*! Static version of isDateTimeType() method
     @return true if the field is of any date or time related type */
    static bool isDateTimeType(Type type);

    /*! @return true if the field is of any text type */
    inline bool isTextType() const {
        return KDbField::isTextType(type());
    }

    /*! Static version of isTextType() method
     @return true if the field is of any text type */
    static bool isTextType(Type type);

    /*! @return options defined for this field. */
    Options options() const;

    /*! Sets options for this field. */
    void setOptions(Options options);

    //! Converts field's type to QVariant equivalent as accurate as possible
    inline QVariant::Type variantType() const {
        return variantType(type());
    }

    /*! @return a type for this field. If there's expression assigned,
     type of the expression (after evaluation) is returned instead. */
    Type type() const;

    //! @return a translated type name for this field
    inline QString typeName() const {
        return KDbField::typeName(type());
    }

    //! @return type group for this field
    inline TypeGroup typeGroup() const {
        return KDbField::typeGroup(type());
    }

    //! @return a translated type group name for this field
    inline QString typeGroupName() const {
        return KDbField::typeGroupName(typeGroup());
    }

    //! @return a type string for this field,
    //! for example "Integer" string for KDbField::Integer type.
    inline QString typeString() const {
        return KDbField::typeString(type());
    }

    //! @return a type group string for this field,
    //! for example "Integer" string for KDbField::IntegerGroup.
    inline QString typeGroupString() const {
        return KDbField::typeGroupString(typeGroup());
    }

    /*! @return (optional) subtype for this field.
     Subtype is a string providing additional hint for field's type.
     E.g. for BLOB type, it can be a MIME type or certain QVariant type name,
     for example: "QPixmap", "QColor" or "QFont" */
    QString subType() const;

    /*! Sets (optional) subtype for this field.
     @see subType() */
    void setSubType(const QString& subType);

    //! @return default value for this field. Null value means there
    //! is no default value declared. The variant value is compatible with field's type.
    QVariant defaultValue() const;

    /*! @return default maximum length of text.
        Default is 0, i.e unlimited length (if the engine supports it). */
    static int defaultMaxLength();

    /*! Sets default maximum length of text. 0 means unlimited length,
        greater than 0 means specific maximum length. */
    static void setDefaultMaxLength(int maxLength);

    /*! Strategy for defining maximum length of text for this field.
      Only makes sense if the field type is of Text type.
      Default strategy is DefinedMaxLength.
     */
    enum MaxLengthStrategy {
        DefaultMaxLength,  //!< Default maximum text length defined globally by the application.
                           //!< @see defaultMaxLength()
        DefinedMaxLength   //!< Used if setMaxLength() was called to set specific maximum value
                           //!< or to unlimited (0).
    };
    Q_ENUM(MaxLengthStrategy)

    /*! @return a hint that indicates if the maximum length of text for this field is based on default setting
      (defaultMaxLength()) or was explicitly set.
      Only makes sense if the field type is Text. */
    MaxLengthStrategy maxLengthStrategy() const;

    /*! Sets strategy for defining maximum length of text for this field.
      Only makes sense if the field type is Text.
      Default strategy is DefinedMaxLength.
      Changing this value does not affect maxLength property.

      Fields with DefaultMaxLength strategy does not follow changes made by calling setDefaultMaxLength()
      so to update the default maximum lengths in fields, the app has to iterate over all fields of type Text,
      and reset to the new default as explained in setMaxLength() documentation.
      See documentation for setMaxLength() for information how to reset maxLength to default value.

      @see maxLengthStrategy(), setMaxLength() */
    void setMaxLengthStrategy(MaxLengthStrategy strategy);

    /*! @return maximum length of text allowed for this field. Only meaningful if the type is Text.
      @see setMaxLength() */
    int maxLength() const;

    /*! Sets maximum length for this field. Only works for Text type.
     It can be specific maximum value or 0 for unlimited length (which will work if engine supports).
     Resets maxLengthStrategy property to DefinedMaxLength.
     To reset to default maximum length, call setMaxLength(defaultMaxLength()) and then
     to indicate this is based on default setting, call setMaxLengthStrategy(DefaultMaxLength).
     @see maxLength(), maxLengthStrategy() */
    void setMaxLength(int maxLength);

    /*! @return precision for numeric and other fields that have both length (scale)
     and precision (floating point types). */
    int precision() const;

    /*! @return scale for numeric and other fields that have both length (scale)
     and precision (floating point types).
     The scale of a numeric is the count of decimal digits in the fractional part,
     to the right of the decimal point. The precision of a numeric is the total count
     of significant digits in the whole number, that is, the number of digits
     to both sides of the decimal point. So the number 23.5141 has a precision
     of 6 and a scale of 4. Integers can be considered to have a scale of zero. */
    int scale() const;

//! @todo should we keep extended properties here or move them to a QVariant dictionary?
    /*! @return number of decimal places that should be visible to the user,
     e.g. within table view widget, form or printout.
     Only meaningful if the field type is floating point or (in the future: decimal or currency).

     - Any value less than 0 (-1 is the default) means that there should be displayed all digits
       of the fractional part, except the ending zeros. This is known as "auto" mode.
       For example, 12.345000 becomes 12.345.

     - Value of 0 means that all the fractional part should be hidden (as well as the dot or comma).
       For example, 12.345000 becomes 12.

     - Value N > 0 means that the fractional part should take exactly N digits.
       If the fractional part is shorter than N, additional zeros are appended.
       For example, "12.345" becomes "12.345000" if N=6.
    */
    int visibleDecimalPlaces() const;

    /*! @return the constraints defined for this field. */
    Constraints constraints() const;

    /*! @return order of this field in containing table (counting starts from 0)
    (-1 if unspecified). */
    int order() const;

    /*! @return caption of this field. */
    QString caption() const;

    /*! @return caption of this field or - if empty - return its name. */
    QString captionOrName() const;

    /*! @return description text for this field. */
    QString description() const;

    //! if the type has the unsigned attribute
    inline bool isUnsigned() const {
        return options() & Unsigned;
    }

    /*! @return true if this field has EMPTY property (i.e. it is of type
    string or is a BLOB). */
    inline bool hasEmptyProperty() const {
        return KDbField::hasEmptyProperty(type());
    }

    /*! static version of hasEmptyProperty() method
     @return true if this field type has EMPTY property (i.e. it is string or BLOB type) */
    static bool hasEmptyProperty(Type type);

    /*! @return true if this field can be auto-incremented.
     Actually, returns true for integer field type. @see IntegerType, isAutoIncrement() */
    inline bool isAutoIncrementAllowed() const {
        return KDbField::isAutoIncrementAllowed(type());
    }

    /*! static version of isAutoIncrementAllowed() method
     @return true if this field type can be auto-incremented. */
    static bool isAutoIncrementAllowed(Type type);

    /*! Sets type @a t for this field.
     This does nothing if there's expression assigned.
     @see setExpression() */
    void setType(Type t);

    /*! Sets name @a name for this field. */
    void setName(const QString& name);

    /*! Sets constraints to @a c. If PrimaryKey is set in @a c, also
     constraits implied by being primary key are enforced (see setPrimaryKey()).
     If Indexed is not set in @a c, constraits implied by not being are
     enforced as well (see setIndexed()). */
    void setConstraints(Constraints c);

    /*! Sets scale for this field. Only works for floating-point types.
     @see scale() */
    void setScale(int s);

    /*! Sets number of decimal places that should be visible to the user.
     @see visibleDecimalPlaces() */
    void setVisibleDecimalPlaces(int p);

    /*! Sets scale for this field. Only works for floating-point types. */
    void setPrecision(int p);

    /*! Sets unsigned flag for this field. Only works for integer types. */
    void setUnsigned(bool u);

    /*! Sets default value for this field. Setting null value removes the default value.
     @see defaultValue() */
    void setDefaultValue(const QVariant& def);

    /*! Sets default value decoded from QByteArray.
      Decoding errors are detected (value is strictly checked against field type)
      - if one is encountered, default value is cleared (defaultValue()==QVariant()).
      @return true if given value was valid for field type. */
    bool setDefaultValue(const QByteArray& def);

    /*! Sets auto increment flag. Only available to set true,
     if isAutoIncrementAllowed() is true. */
    void setAutoIncrement(bool a);

    /*! Specifies whether the field is single-field primary key or not
     (KDb::PrimeryKey item).
     Use this with caution. Setting this to true implies setting:
     - setUniqueKey(true)
     - setNotNull(true)
     - setNotEmpty(true)
     - setIndexed(true)

     Setting this to false implies setting setAutoIncrement(false). */
    void setPrimaryKey(bool p);

    /*! Specifies whether the field has single-field unique constraint or not
     (KDb::Unique item). Setting this to true implies setting Indexed flag
     to true (setIndexed(true)), because index is required it control unique constraint. */
    void setUniqueKey(bool u);

    /*! Sets whether the field has to be declared with single-field foreign key.
     Used in KDbIndexSchema::setForeigKey(). */
    void setForeignKey(bool f);

    /*! Specifies whether the field has single-field unique constraint or not
     (KDb::NotNull item). Setting this to true implies setting Indexed flag
     to true (setIndexed(true)), because index is required it control
     not null constraint. */
    void setNotNull(bool n);

    /*! Specifies whether the field has single-field unique constraint or not
     (KDb::NotEmpty item). Setting this to true implies setting Indexed flag
     to true (setIndexed(true)), because index is required it control
     not empty constraint. */
    void setNotEmpty(bool n);

    /*! Specifies whether the field is indexed (KDb::Indexed item)
     (by single-field implicit index) or not.
     Use this with caution. Since index is used to control unique,
     not null/empty constratins, setting this to false implies setting:
     - setPrimaryKey(false)
     - setUniqueKey(false)
     - setNotNull(false)
     - setNotEmpty(false)
     because above flags need index to be present.
     Similarly, setting one of the above flags to true, will automatically
     do setIndexed(true) for the same reason. */
    void setIndexed(bool s);

    /*! Sets caption for this field to @a caption. */
    void setCaption(const QString& caption);

    /*! Sets description for this field to @a description. */
    void setDescription(const QString& description);

    /*! There can be added asterisks (KDbQueryAsterisk objects)
     to query schemas' field list. KDbQueryAsterisk subclasses KDbField class,
     and to check if the given object (pointed by KDbField*)
     is asterisk or just ordinary field definition,
     you can call this method. This is just effective version of QObject::isA().
     Every KDbQueryAsterisk object returns true here,
     and every KDbField object returns false.
    */
    inline bool isQueryAsterisk() const {
        return type() == KDbField::Asterisk;
    }

    /*! @return KDbExpression object if the field value is an
     expression.  Unless the expression is set with setExpression(), it is null.
    */
    KDbExpression expression();

    //! @overload KDbExpression expression()
    const KDbExpression expression() const;

    /*! Sets expression data @a expr. If there was already expression set, it is removed before new
     assignment.
     This KDbField object becames logical owner of @a expr object, so do not use the expression
     for other objects (you can call KDbExpression::clone()).
     Current field's expression is deleted, if exists.

     Because the field defines an expression, it should be assigned to a query, not to a table,
     otherwise this call will not have any effect.
    */
    void setExpression(const KDbExpression& expr);

    /*! @return true if there is expression defined for this field.
     This method is provided for better readibility
     - does the same as expression().isNull(). */
    bool isExpression() const;

//<TMP>
    /*! @return the hints for enum fields. */
    QVector<QString> enumHints() const;

    /*! @return hint name for enum value @a num. */
    QString enumHint(int num);

    /*! Sets the hint for enum fields */
    void setEnumHints(const QVector<QString> &hints);
//</TMP>

    /*! @return custom property @a propertyName.
     If there is no such a property, @a defaultValue is returned. */
    QVariant customProperty(const QByteArray& propertyName,
                            const QVariant& defaultValue = QVariant()) const;

    //! Sets value @a value for custom property @a propertyName
    void setCustomProperty(const QByteArray& propertyName, const QVariant& value);

    //! A data type used for handling custom properties of a field
    typedef QHash<QByteArray, QVariant> CustomPropertiesMap;

    //! @return all custom properties
    CustomPropertiesMap customProperties() const;

protected:
    explicit KDbField(KDbFieldList *aParent, int aOrder = -1);

    /*! Creates a database field as a child of @a querySchema table
     Assigns @a expr expression to this field, if present.
     Used internally by query schemas, e.g. to declare asterisks or
     to add expression columns.
     No other properties are set, so these should be set later. */
    KDbField(KDbQuerySchema *querySchema, const KDbExpression& expr);

    /*! @overload KDbField(KDbQuerySchema*, const KDbExpression&) */
    explicit KDbField(KDbQuerySchema *querySchema);

    //! Sets parent for this field.
    void setParent(KDbFieldList *parent);

    /*! Sets order of this field in containing table. Counting starts from 0. -1 if unspecified. */
    void setOrder(int order);

    //! @return a deep copy of this object. Used in @ref KDbFieldList(const KDbFieldList& fl).
    virtual KDbField* copy();

private:
    friend class KDbConnection;
    friend class KDbFieldList;
    friend class KDbTableSchema;
    friend class KDbQuerySchema;

    class Private;
    Private * const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KDbField::Constraints)
Q_DECLARE_OPERATORS_FOR_FLAGS(KDbField::Options)

//! Sends information about field @a field to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbField& field);

//! Sends information about field type @a type to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, KDbField::Type type);

//! Sends information about field type group @a typeGroup to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, KDbField::TypeGroup typeGroup);

#endif

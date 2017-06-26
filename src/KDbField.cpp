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

#include "KDbField.h"
#include "KDbField_p.h"
#include "KDbConnection.h"
#include "KDbDriver.h"
#include "KDbExpression.h"
#include "KDbQuerySchema.h"
#include "KDb.h"
#include "kdb_debug.h"

#include <QDateTime>

#include <assert.h>

//! @todo make this configurable
static int g_defaultMaxLength = 0; // unlimited

//-------------------------------------------------------
//! @internal Used in s_typeNames member to handle translated type names
class FieldTypeNames : public QVector<QString>
{
public:
    FieldTypeNames();
    QHash<QString, KDbField::Type> str2num;
    QStringList names;
private:
    Q_DISABLE_COPY(FieldTypeNames)
};

//! @internal Used in m_typeGroupNames member to handle translated type group names
class FieldTypeGroupNames : public QVector<QString>
{
public:
    FieldTypeGroupNames();
    QHash<QString, KDbField::TypeGroup> str2num;
    QStringList names;
private:
    Q_DISABLE_COPY(FieldTypeGroupNames)
};

//! real translated type names (and nontranslated type name strings)
Q_GLOBAL_STATIC(FieldTypeNames, s_typeNames)

//! real translated type group names (and nontranslated group name strings)
Q_GLOBAL_STATIC(FieldTypeGroupNames, s_typeGroupNames)

#define ADDTYPE(type, i18, str) \
    (*this)[KDbField::type] = i18; \
    (*this)[KDbField::type+KDbField::Null+1] = QLatin1String(str); \
    str2num[ QString::fromLatin1(str).toLower() ] = KDbField::type; \
    names.append(i18)
#define ADDGROUP(type, i18, str) \
    (*this)[KDbField::type] = i18; \
    (*this)[KDbField::type+KDbField::LastTypeGroup+1] = QLatin1String(str); \
    str2num[ QString::fromLatin1(str).toLower() ] = KDbField::type; \
    names.append(i18)

FieldTypeNames::FieldTypeNames()
        : QVector<QString>()
{
    resize((KDbField::Null + 1)*2);

    ADDTYPE(InvalidType, KDbField::tr("Invalid Type"), "InvalidType");
    ADDTYPE(Byte, KDbField::tr("Byte"), "Byte");
    ADDTYPE(ShortInteger, KDbField::tr("Short Integer Number"), "ShortInteger");
    ADDTYPE(Integer, KDbField::tr("Integer Number"), "Integer");
    ADDTYPE(BigInteger, KDbField::tr("Big Integer Number"), "BigInteger");
    ADDTYPE(Boolean, KDbField::tr("Yes/No Value"), "Boolean");
    ADDTYPE(Date, KDbField::tr("Date"), "Date");
    ADDTYPE(DateTime, KDbField::tr("Date and Time"), "DateTime");
    ADDTYPE(Time, KDbField::tr("Time"), "Time");
    ADDTYPE(Float, KDbField::tr("Single Precision Number"), "Float");
    ADDTYPE(Double, KDbField::tr("Double Precision Number"), "Double");
    ADDTYPE(Text, KDbField::tr("Text"), "Text");
    ADDTYPE(LongText, KDbField::tr("Long Text"), "LongText");
    ADDTYPE(BLOB, KDbField::tr("Object"), "BLOB");
    ADDTYPE(Null, QLatin1String("NULL")/*don't translate*/, "NULL");
}

//-------------------------------------------------------

FieldTypeGroupNames::FieldTypeGroupNames()
        : QVector<QString>()
{
    resize((KDbField::LastTypeGroup + 1)*2);

    ADDGROUP(InvalidGroup, KDbField::tr("Invalid Group"), "InvalidGroup");
    ADDGROUP(TextGroup, KDbField::tr("Text"), "TextGroup");
    ADDGROUP(IntegerGroup, KDbField::tr("Integer Number"), "IntegerGroup");
    ADDGROUP(FloatGroup, KDbField::tr("Floating Point Number"), "FloatGroup");
    ADDGROUP(BooleanGroup, KDbField::tr("Yes/No"), "BooleanGroup");
    ADDGROUP(DateTimeGroup, KDbField::tr("Date/Time"), "DateTimeGroup");
    ADDGROUP(BLOBGroup, KDbField::tr("Object"), "BLOBGroup");
}

//-------------------------------------------------------

class Q_DECL_HIDDEN KDbField::Private
{
public:
    Private(KDbFieldList *aParent = nullptr, int aOrder = -1)
        : parent(aParent)
        , type(KDbField::InvalidType)
        , precision(0)
        , options(KDbField::NoOptions)
        , defaultValue(QString())
        , order(aOrder)
    {
    }
    Private(const QString &aName, KDbField::Type aType, KDbField::Options aOptions, int aPrecision,
            const QVariant &aDefaultValue, const QString &aCaption, const QString &aDescription)
        : parent(nullptr)
        , type(aType)
        , name(aName)
        , caption(aCaption)
        , description(aDescription)
        , precision(aPrecision)
        , options(aOptions)
        , defaultValue(aDefaultValue)
        , order(-1)
    {
    }
    Private(const Private &o)
    {
        (*this) = o;
        if (o.customProperties) {
            customProperties = new CustomPropertiesMap(*o.customProperties);
        }
        if (!o.expr.isNull()) {//deep copy the expression
    //! @todo  expr = new KDbExpression(o.expr);
                expr = KDbExpression(o.expr.clone());
        } else {
            expr = KDbExpression();
        }
    }

    ~Private() {
        delete customProperties;
    }

    //! In most cases this points to a KDbTableSchema object that field is assigned
    KDbFieldList *parent;
    KDbField::Type type;
    QString name;
    QString caption;
    QString description;
    QString subType;
    KDbField::Constraints constraints;
    KDbField::MaxLengthStrategy maxLengthStrategy;
    int maxLength; //!< also used for storing scale for floating point types
    int precision;
    int visibleDecimalPlaces = -1; //!< used in visibleDecimalPlaces()
    KDbField::Options options;
    QVariant defaultValue;
    int order;
    KDbExpression expr;
    KDbField::CustomPropertiesMap* customProperties = nullptr;
    QVector<QString> hints;
};

//-------------------------------------------------------

KDbField::KDbField()
    : d(new Private)
{
    setMaxLength(0); // do not move this line up!
    setMaxLengthStrategy(DefinedMaxLength); // do not move this line up!
    setConstraints(NoConstraints);
}

KDbField::KDbField(KDbFieldList *aParent, int aOrder)
    : d(new Private(aParent, aOrder))
{
    setMaxLength(0); // do not move this line up!
    setMaxLengthStrategy(DefinedMaxLength); // do not move this line up!
    setConstraints(NoConstraints);
}

KDbField::KDbField(KDbTableSchema *tableSchema)
    : KDbField(tableSchema, tableSchema->fieldCount())
{
}

KDbField::KDbField(KDbQuerySchema *querySchema, const KDbExpression& expr)
    : KDbField(querySchema, querySchema->fieldCount())
{
    setExpression(expr);
}

KDbField::KDbField(KDbQuerySchema *querySchema)
    : KDbField(querySchema, querySchema->fieldCount())
{
}

KDbField::KDbField(const QString &name, Type type, Constraints constr, Options options,
                   int maxLength, int precision, const QVariant &defaultValue,
                   const QString &caption, const QString &description)
    : d(new Private(name, type, options, precision, defaultValue, caption, description))
{
    setMaxLength(maxLength);
    setConstraints(constr);
}

/*! Copy constructor. */
KDbField::KDbField(const KDbField &f)
    : d(new Private(*f.d))
{
}

KDbField::~KDbField()
{
    delete d;
}

KDbFieldList *KDbField::parent()
{
    return d->parent;
}

const KDbFieldList *KDbField::parent() const
{
    return d->parent;
}

void KDbField::setParent(KDbFieldList *parent)
{
    d->parent = parent;
}

QString KDbField::name() const
{
    return d->name;
}

KDbField::Options KDbField::options() const
{
    return d->options;
}

void KDbField::setOptions(Options options)
{
    d->options = options;
}

QString KDbField::subType() const
{
    return d->subType;
}

void KDbField::setSubType(const QString& subType)
{
    d->subType = subType;
}

QVariant KDbField::defaultValue() const
{
    return d->defaultValue;
}

int KDbField::precision() const
{
    return d->precision;
}

int KDbField::scale() const
{
    return d->maxLength;
}

int KDbField::visibleDecimalPlaces() const
{
    return d->visibleDecimalPlaces;
}

KDbField::Constraints KDbField::constraints() const
{
    return d->constraints;
}

int KDbField::order() const
{
    return d->order;
}

void KDbField::setOrder(int order)
{
    d->order = order;
}

QString KDbField::caption() const
{
    return d->caption;
}

void KDbField::setCaption(const QString& caption)
{
    d->caption = caption;
}

QString KDbField::captionOrName() const
{
    return d->caption.isEmpty() ? d->name : d->caption;
}

QString KDbField::description() const
{
    return d->description;
}

void KDbField::setDescription(const QString& description)
{
    d->description = description;
}

QVector<QString> KDbField::enumHints() const
{
    return d->hints;
}

QString KDbField::enumHint(int num)
{
    return (num < d->hints.size()) ? d->hints.at(num) : QString();
}

void KDbField::setEnumHints(const QVector<QString> &hints)
{
    d->hints = hints;
}

// static
int KDbField::typesCount()
{
    return LastType - InvalidType + 1;
}

// static
int KDbField::specialTypesCount()
{
    return LastSpecialType - Null + 1;
}

// static
int KDbField::typeGroupsCount()
{
    return LastTypeGroup - InvalidGroup + 1;
}

KDbField* KDbField::copy()
{
    return new KDbField(*this);
}

KDbField::Type KDbField::type() const
{
    if (!d->expr.isNull()) {
        return d->expr.type();
    }
    return d->type;
}

QVariant::Type KDbField::variantType(Type type)
{
    switch (type) {
    case Byte:
    case ShortInteger:
    case Integer:
    case BigInteger:
        return QVariant::Int;
    case Boolean:
        return QVariant::Bool;
    case Date:
        return QVariant::Date;
    case DateTime:
        return QVariant::DateTime;
    case Time:
        return QVariant::Time;
    case Float:
    case Double:
        return QVariant::Double;
    case Text:
    case LongText:
        return QVariant::String;
    case BLOB:
        return QVariant::ByteArray;
    default:
        break;
    }

    return QVariant::Invalid;
}

template <typename T>
static inline QVariant tryConvert(const QVariant &value)
{
    return value.canConvert<T>() ? value.value<T>() : value;
}

//static
//! @todo use an array of functions?
QVariant KDbField::convertToType(const QVariant &value, Type type)
{
    switch (type) {
    case Byte:
    case ShortInteger:
    case Integer:
        return tryConvert<int>(value);
    case BigInteger:
        return tryConvert<qlonglong>(value);
    case Boolean:
        return tryConvert<bool>(value);
    case Date:
        return tryConvert<QDate>(value);
    case DateTime:
        return tryConvert<QDateTime>(value);
    case Time:
        return tryConvert<QTime>(value);
    case Float:
        return tryConvert<float>(value);
    case Double:
        return tryConvert<double>(value);
    case Text:
    case LongText:
        return tryConvert<QString>(value);
    case BLOB:
        return tryConvert<QByteArray>(value);
    default:
        break;
    }
    return QVariant();
}

QString KDbField::typeName(Type type)
{
    return s_typeNames->value(type, QString::number(type));
}

QStringList KDbField::typeNames()
{
    return s_typeNames->names;
}

QString KDbField::typeString(Type type)
{
    return (type <= Null) ? s_typeNames->at(int(Null) + 1 + type)
                              : (QLatin1String("Type") + QString::number(type));
}

QString KDbField::typeGroupName(TypeGroup typeGroup)
{
    return (typeGroup <= LastTypeGroup) ? s_typeGroupNames->at(typeGroup) : typeGroupString(typeGroup);
}

QStringList KDbField::typeGroupNames()
{
    return s_typeGroupNames->names;
}

QString KDbField::typeGroupString(TypeGroup typeGroup)
{
    return s_typeGroupNames->value(int(LastTypeGroup) + 1 + typeGroup,
                                  QLatin1String("TypeGroup") + QString::number(typeGroup));
}

KDbField::Type KDbField::typeForString(const QString& typeString)
{
    return s_typeNames->str2num.value(typeString.toLower(), InvalidType);
}

KDbField::TypeGroup KDbField::typeGroupForString(const QString& typeGroupString)
{
    return s_typeGroupNames->str2num.value(typeGroupString.toLower(), InvalidGroup);
}

bool KDbField::isIntegerType(Type type)
{
    switch (type) {
    case KDbField::Byte:
    case KDbField::ShortInteger:
    case KDbField::Integer:
    case KDbField::BigInteger:
        return true;
    default:;
    }
    return false;
}

bool KDbField::isNumericType(Type type)
{
    switch (type) {
    case KDbField::Byte:
    case KDbField::ShortInteger:
    case KDbField::Integer:
    case KDbField::BigInteger:
    case KDbField::Float:
    case KDbField::Double:
        return true;
    default:;
    }
    return false;
}

bool KDbField::isFPNumericType(Type type)
{
    return type == KDbField::Float || type == KDbField::Double;
}

bool KDbField::isDateTimeType(Type type)
{
    switch (type) {
    case KDbField::Date:
    case KDbField::DateTime:
    case KDbField::Time:
        return true;
    default:;
    }
    return false;
}

bool KDbField::isTextType(Type type)
{
    switch (type) {
    case KDbField::Text:
    case KDbField::LongText:
        return true;
    default:;
    }
    return false;
}

bool KDbField::hasEmptyProperty(Type type)
{
    return KDbField::isTextType(type) || type == BLOB;
}

bool KDbField::isAutoIncrementAllowed(Type type)
{
    return KDbField::isIntegerType(type);
}

KDbField::TypeGroup KDbField::typeGroup(Type type)
{
    if (KDbField::isTextType(type))
        return TextGroup;
    else if (KDbField::isIntegerType(type))
        return IntegerGroup;
    else if (KDbField::isFPNumericType(type))
        return FloatGroup;
    else if (type == Boolean)
        return BooleanGroup;
    else if (KDbField::isDateTimeType(type))
        return DateTimeGroup;
    else if (type == BLOB)
        return BLOBGroup;

    return InvalidGroup; //unknown
}

KDbTableSchema* KDbField::table()
{
    return dynamic_cast<KDbTableSchema*>(d->parent);
}

const KDbTableSchema* KDbField::table() const
{
    return dynamic_cast<const KDbTableSchema*>(d->parent);
}

void KDbField::setTable(KDbTableSchema *tableSchema)
{
    d->parent = tableSchema;
}

KDbQuerySchema* KDbField::query()
{
    return dynamic_cast<KDbQuerySchema*>(d->parent);
}

const KDbQuerySchema* KDbField::query() const
{
    return dynamic_cast<const KDbQuerySchema*>(d->parent);
}

void KDbField::setQuery(KDbQuerySchema *querySchema)
{
    d->parent = querySchema;
}

void KDbField::setName(const QString& name)
{
    d->name = name.toLower();
}

void KDbField::setType(Type t)
{
    if (!d->expr.isNull()) {
        kdbWarning() << "Could not set type" << KDbField::typeName(t)
                     << "because the field has expression assigned!";
        return;
    }
    d->type = t;
}

void KDbField::setConstraints(Constraints c)
{
    d->constraints = c;
    //pkey must be unique notnull
    if (isPrimaryKey()) {
        setPrimaryKey(true);
    }
    if (isIndexed()) {
        setIndexed(true);
    }
    if (isAutoIncrement() && !isAutoIncrementAllowed()) {
        setAutoIncrement(false);
    }
}

int KDbField::defaultMaxLength()
{
    return g_defaultMaxLength;
}

void KDbField::setDefaultMaxLength(int maxLength)
{
    g_defaultMaxLength = maxLength;
}

KDbField::MaxLengthStrategy KDbField::maxLengthStrategy() const
{
    return d->maxLengthStrategy;
}

void KDbField::setMaxLengthStrategy(MaxLengthStrategy strategy)
{
    d->maxLengthStrategy = strategy;
}

int KDbField::maxLength() const
{
    return d->maxLength;
}

void KDbField::setMaxLength(int maxLength)
{
    d->maxLength = maxLength;
    d->maxLengthStrategy = DefinedMaxLength;
}

void KDbField::setPrecision(int p)
{
    if (!isFPNumericType()) {
        return;
    }
    d->precision = p;
}

void KDbField::setScale(int s)
{
    if (!isFPNumericType()) {
        return;
    }
    d->maxLength = s;
}

void KDbField::setVisibleDecimalPlaces(int p)
{
    if (!KDb::supportsVisibleDecimalPlacesProperty(type())) {
        return;
    }
    d->visibleDecimalPlaces = p < 0 ? -1 : p;
}

void KDbField::setUnsigned(bool u)
{
    if (!isIntegerType()) {
        return;
    }
    d->options |= Unsigned;
    if (!u) {
        d->options ^= Unsigned;
    }
}

void KDbField::setDefaultValue(const QVariant& def)
{
    d->defaultValue = def;
}

bool KDbField::setDefaultValue(const QByteArray& def)
{
    if (def.isNull()) {
        d->defaultValue = QVariant();
        return true;
    }

    bool ok;
    switch (type()) {
    case Byte: {
        unsigned int v = def.toUInt(&ok);
        if (!ok || v > 255) {
            d->defaultValue = QVariant();
        } else {
            d->defaultValue = QVariant(v);
        }
        break;
    }
    case ShortInteger: {
        int v = def.toInt(&ok);
        if (!ok || (!(d->options & Unsigned) && (v < -32768 || v > 32767))
            || ((d->options & Unsigned) && (v < 0 || v > 65535)))
        {
            d->defaultValue = QVariant();
        } else {
            d->defaultValue = QVariant(v);
        }
        break;
    }
    case Integer: {//4 bytes
        long v = def.toLong(&ok);
        //! @todo    if (!ok || (!(d->options & Unsigned) && (-v > 0x080000000 || v >
        //! (0x080000000-1))) || ((d->options & Unsigned) && (v < 0 || v > 0x100000000)))
        if (!ok || (!(d->options & Unsigned)
                    && (-v > (int)0x07FFFFFFF || v > (int)(0x080000000 - 1))))
        {
            d->defaultValue = QVariant();
        } else {
            d->defaultValue = QVariant((qint64)v);
        }
        break;
    }
    case BigInteger: {//8 bytes
//! @todo BigInteger support
        /*
              qint64 long v = def.toLongLong(&ok);
        //! @todo 2-part decoding
              if (!ok || (!(d->options & Unsigned) && (-v > 0x080000000 || v > (0x080000000-1))))
                d->defaultValue = QVariant();
              else
                if (d->options & Unsigned)
                  d->defaultValue=QVariant((quint64) v);
                else
                  d->defaultValue = QVariant((qint64)v);*/
        break;
    }
    case Boolean: {
        unsigned short v = def.toUShort(&ok);
        if (!ok || v > 1) {
            d->defaultValue = QVariant();
        } else {
            d->defaultValue = QVariant((bool)v);
        }
        break;
    }
    case Date: {//YYYY-MM-DD
        QDate date = QDate::fromString(QLatin1String(def), Qt::ISODate);
        if (!date.isValid()) {
            d->defaultValue = QVariant();
        } else {
            d->defaultValue = QVariant(date);
        }
        break;
    }
    case DateTime: {//YYYY-MM-DDTHH:MM:SS
        QDateTime dt = QDateTime::fromString(QLatin1String(def), Qt::ISODate);
        if (!dt.isValid()) {
            d->defaultValue = QVariant();
        } else {
            d->defaultValue = QVariant(dt);
        }
        break;
    }
    case Time: {//HH:MM:SS
        QTime time = QTime::fromString(QLatin1String(def), Qt::ISODate);
        if (!time.isValid()) {
            d->defaultValue = QVariant();
        } else {
            d->defaultValue = QVariant(time);
        }
        break;
    }
    case Float: {
        float v = def.toFloat(&ok);
        if (!ok || ((d->options & Unsigned) && (v < 0.0))) {
            d->defaultValue = QVariant();
        } else {
            d->defaultValue = QVariant(v);
        }
        break;
    }
    case Double: {
        double v = def.toDouble(&ok);
        if (!ok || ((d->options & Unsigned) && (v < 0.0))) {
            d->defaultValue = QVariant();
        } else {
            d->defaultValue = QVariant(v);
        }
        break;
    }
    case Text: {
        if (def.isNull() || def.length() > maxLength()) {
            d->defaultValue = QVariant();
        } else {
            d->defaultValue = QVariant(QLatin1String(def));
        }
        break;
    }
    case LongText: {
        if (def.isNull()) {
            d->defaultValue = QVariant();
        } else {
            d->defaultValue = QVariant(QLatin1String(def));
        }
        break;
    }
    case BLOB: {
//! @todo
        if (def.isNull()) {
            d->defaultValue = QVariant();
        } else {
            d->defaultValue = QVariant(def);
        }
        break;
    }
    default:
        d->defaultValue = QVariant();
    }
    return d->defaultValue.isNull();
}

void KDbField::setAutoIncrement(bool a)
{
    if (a && !isAutoIncrementAllowed()) {
        return;
    }
    if (isAutoIncrement() != a) {
        d->constraints ^= KDbField::AutoInc;
    }
}

void KDbField::setPrimaryKey(bool p)
{
    if (isPrimaryKey() != p) {
        d->constraints ^= KDbField::PrimaryKey;
    }
    if (p) {//also set implied constraints
        setUniqueKey(true);
        setNotNull(true);
        setNotEmpty(true);
        setIndexed(true);
    } else {
//! @todo is this ok for all engines?
        setAutoIncrement(false);
    }
}

void KDbField::setUniqueKey(bool u)
{
    if (isUniqueKey() != u) {
        d->constraints ^= KDbField::Unique;
        if (u) { //also set implied constraints
            setNotNull(true);
            setIndexed(true);
        }
    }
}

void KDbField::setForeignKey(bool f)
{
    if (isForeignKey() != f) {
        d->constraints ^= KDbField::ForeignKey;
    }
}

void KDbField::setNotNull(bool n)
{
    if (isNotNull() != n) {
        d->constraints ^=KDbField::NotNull;
    }
}

void KDbField::setNotEmpty(bool n)
{
    if (isNotEmpty() != n) {
        d->constraints ^= KDbField::NotEmpty;
    }
}

void KDbField::setIndexed(bool s)
{
    if (isIndexed() != s) {
        d->constraints ^= KDbField::Indexed;
    }
    if (!s) {//also set implied constraints
        setPrimaryKey(false);
        setUniqueKey(false);
        setNotNull(false);
        setNotEmpty(false);
    }
}

void debug(QDebug dbg, const KDbField& field, KDbFieldDebugOptions options)
{
    KDbConnection *conn = field.table() ? field.table()->connection() : nullptr;
    if (options & KDbFieldDebugAddName) {
        if (field.name().isEmpty()) {
            dbg.nospace() << "<NONAME> ";
        } else {
            dbg.nospace() << field.name() << ' ';
        }
    }
    if (field.options() & KDbField::Unsigned)
        dbg.nospace() << " UNSIGNED";
    dbg.nospace() << ' ' << qPrintable((conn && conn->driver())
        ? conn->driver()->sqlTypeName(field.type(), field) : KDbDriver::defaultSqlTypeName(field.type()));
    if (field.isFPNumericType() && field.precision() > 0) {
        if (field.scale() > 0)
            dbg.nospace() << QString::fromLatin1("(%1,%2)").arg(field.precision()).arg(field.scale());
        else
            dbg.nospace() << QString::fromLatin1("(%1)").arg(field.precision());
    }
    else if (field.type() == KDbField::Text && field.maxLength() > 0)
        dbg.space() << QString::fromLatin1("(%1)").arg(field.maxLength());

    if (field.constraints() & KDbField::AutoInc)
        dbg.nospace() << " AUTOINC";
    if (field.constraints() & KDbField::Unique)
        dbg.nospace() << " UNIQUE";
    if (field.constraints() & KDbField::PrimaryKey)
        dbg.nospace() << " PKEY";
    if (field.constraints() & KDbField::ForeignKey)
        dbg.nospace() << " FKEY";
    if (field.constraints() & KDbField::NotNull)
        dbg.nospace() << " NOTNULL";
    if (field.constraints() & KDbField::NotEmpty)
        dbg.nospace() << " NOTEMPTY";
    if (!field.defaultValue().isNull()) {
        dbg.nospace() << qPrintable(QString::fromLatin1(" DEFAULT=[%1]")
                           .arg(QLatin1String(field.defaultValue().typeName())));
        dbg.nospace() << qPrintable(KDb::variantToString(field.defaultValue()));
    }
    if (field.isExpression()) {
        dbg.nospace() << " EXPRESSION=";
        dbg.nospace() << field.expression();
    }
    const KDbField::CustomPropertiesMap customProperties(field.customProperties());
    if (!customProperties.isEmpty()) {
        dbg.space() << QString::fromLatin1("CUSTOM PROPERTIES (%1): ").arg(customProperties.count());
        bool first = true;
        for (KDbField::CustomPropertiesMap::ConstIterator it(customProperties.constBegin());
                it != customProperties.constEnd(); ++it)
        {
            if (first)
                first = false;
            else
                dbg.nospace() << ',';
            dbg.space() << qPrintable(QString::fromLatin1("%1 = %2 (%3)")
                .arg(QLatin1String(it.key()), it.value().toString(),
                     QLatin1String(it.value().typeName())));
        }
    }
}

KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbField& field)
{
    debug(dbg, field, KDbFieldDebugAddName);
    return dbg.space();
}

KDB_EXPORT QDebug operator<<(QDebug dbg, KDbField::Type type)
{
    return dbg.space() << qPrintable(KDbField::typeString(type));
}

KDB_EXPORT QDebug operator<<(QDebug dbg, KDbField::TypeGroup typeGroup)
{
    return dbg.space() << qPrintable(KDbField::typeGroupString(typeGroup));
}

bool KDbField::isExpression() const
{
    return !d->expr.isNull();
}

KDbExpression KDbField::expression()
{
    return d->expr;
}

const KDbExpression KDbField::expression() const
{
    return d->expr;
}

void KDbField::setExpression(const KDbExpression& expr)
{
    Q_ASSERT(!d->parent || dynamic_cast<KDbQuerySchema*>(d->parent));
    if (d->expr == expr) {
        return;
    }
    d->expr = expr;
}

QVariant KDbField::customProperty(const QByteArray& propertyName,
                               const QVariant& defaultValue) const
{
    if (!d->customProperties) {
        return defaultValue;
    }
    return d->customProperties->value(propertyName, defaultValue);
}

void KDbField::setCustomProperty(const QByteArray& propertyName, const QVariant& value)
{
    if (propertyName.isEmpty()) {
        return;
    }
    if (!d->customProperties) {
        d->customProperties = new CustomPropertiesMap();
    }
    d->customProperties->insert(propertyName, value);
}

KDbField::CustomPropertiesMap KDbField::customProperties() const
{
    return d->customProperties ? *d->customProperties : CustomPropertiesMap();
}

/* This file is part of the KDE project
   Copyright (C) 2002 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2003-2015 Jarosław Staniek <staniek@kde.org>

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
#include "KDbConnection.h"
#include "KDbDriver.h"
#include "KDbExpression.h"
#include "KDb.h"
#include "kdb_debug.h"

#include <QDateTime>

#include <assert.h>

KDbField::FieldTypeNames KDbField::m_typeNames;
KDbField::FieldTypeGroupNames KDbField::m_typeGroupNames;

//! @todo make this configurable
static int m_defaultMaxLength = 0; // unlimited

KDbField::KDbField()
{
    init();
    setConstraints(NoConstraints);
}


KDbField::KDbField(KDbTableSchema *tableSchema)
{
    init();
    m_parent = tableSchema;
    m_order = tableSchema->fieldCount();
    setConstraints(NoConstraints);
}

KDbField::KDbField(KDbQuerySchema *querySchema, const KDbExpression& expr)
{
    init();
    m_parent = querySchema;
    m_order = querySchema->fieldCount();
    setConstraints(NoConstraints);
    setExpression(expr);
}

KDbField::KDbField(KDbQuerySchema *querySchema)
{
    init();
    m_parent = querySchema;
    m_order = querySchema->fieldCount();
    setConstraints(NoConstraints);
}

KDbField::KDbField(const QString& name, Type type,
             Constraints constr, Options options, int maxLength, int precision,
             QVariant defaultValue, const QString& caption, const QString& description)
        : m_parent(0)
        , m_name(name.toLower())
        , m_precision(precision)
        , m_visibleDecimalPlaces(-1)
        , m_options(options)
        , m_defaultValue(defaultValue)
        , m_order(-1)
        , m_caption(caption)
        , m_desc(description)
        , m_customProperties(0)
        , m_type(type)
{
    m_expr = new KDbExpression();
    setMaxLength(maxLength);
    setConstraints(constr);
}

/*! Copy constructor. */
KDbField::KDbField(const KDbField& f)
{
    (*this) = f;
    if (f.m_customProperties)
        m_customProperties = new CustomPropertiesMap(f.customProperties());

    if (!f.m_expr->isNull()) {//deep copy the expression
//! @todo  m_expr = new KDbExpression(*f.m_expr);
            m_expr = new KDbExpression(f.m_expr->clone());
    }
    else
        m_expr = new KDbExpression();
}

KDbField::~KDbField()
{
    delete m_customProperties;
    delete m_expr;
}

KDbField* KDbField::copy() const
{
    return new KDbField(*this);
}

void KDbField::init()
{
    m_parent = 0;
    m_type = InvalidType;
    m_precision = 0;
    m_visibleDecimalPlaces = -1;
    m_options = NoOptions;
    m_defaultValue = QVariant(QString());
    m_order = -1;
    m_customProperties = 0;
    m_expr = new KDbExpression();
    setMaxLength(0); // do not move this line up!
    setMaxLengthStrategy(DefinedMaxLength); // do not move this line up!
}

KDbField::Type KDbField::type() const
{
    if (!m_expr->isNull())
        return m_expr->type();
    return m_type;
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
        return QVariant::Invalid;
    }

    return QVariant::Invalid;
}

QString KDbField::typeName(Type type)
{
    m_typeNames.init();
    return m_typeNames.value(type, QString::number(type));
}

QStringList KDbField::typeNames()
{
    m_typeNames.init();
    return m_typeNames.names;
}

QString KDbField::typeString(Type type)
{
    m_typeNames.init();
    return (type <= Null) ? m_typeNames.at(int(Null) + 1 + type)
                              : (QLatin1String("Type") + QString::number(type));
}

QString KDbField::typeGroupName(TypeGroup typeGroup)
{
    m_typeGroupNames.init();
    return (typeGroup <= LastTypeGroup) ? m_typeGroupNames.at(typeGroup) : typeGroupString(typeGroup);
}

QStringList KDbField::typeGroupNames()
{
    m_typeGroupNames.init();
    return m_typeGroupNames.names;
}

QString KDbField::typeGroupString(TypeGroup typeGroup)
{
    m_typeGroupNames.init();
    return m_typeGroupNames.value(int(LastTypeGroup) + 1 + typeGroup,
                                  QLatin1String("TypeGroup") + QString::number(typeGroup));
}

KDbField::Type KDbField::typeForString(const QString& typeString)
{
    m_typeNames.init();
    return m_typeNames.str2num.value(typeString.toLower(), InvalidType);
}

KDbField::TypeGroup KDbField::typeGroupForString(const QString& typeGroupString)
{
    m_typeGroupNames.init();
    return m_typeGroupNames.str2num.value(typeGroupString.toLower(), InvalidGroup);
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

KDbTableSchema*
KDbField::table() const
{
    return dynamic_cast<KDbTableSchema*>(m_parent);
}

void
KDbField::setTable(KDbTableSchema *tableSchema)
{
    m_parent = tableSchema;
}

KDbQuerySchema*
KDbField::query() const
{
    return dynamic_cast<KDbQuerySchema*>(m_parent);
}

void
KDbField::setQuery(KDbQuerySchema *querySchema)
{
    m_parent = querySchema;
}

void
KDbField::setName(const QString& n)
{
    m_name = n.toLower();
}

void
KDbField::setType(Type t)
{
    if (!m_expr->isNull()) {
        kdbWarning() << "could not set type" << KDbField::typeName(t)
                << "because the field has expression assigned!";
        return;
    }
    m_type = t;
}

void KDbField::setConstraints(Constraints c)
{
    m_constraints = c;
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
    return m_defaultMaxLength;
}

void KDbField::setDefaultMaxLength(int maxLength)
{
    m_defaultMaxLength = maxLength;
}

KDbField::MaxLengthStrategy KDbField::maxLengthStrategy() const
{
    return m_maxLengthStrategy;
}

void KDbField::setMaxLengthStrategy(MaxLengthStrategy strategy)
{
    m_maxLengthStrategy = strategy;
}

int KDbField::maxLength() const
{
    return m_maxLength;
}

void
KDbField::setMaxLength(int maxLength)
{
    m_maxLength = maxLength;
    m_maxLengthStrategy = DefinedMaxLength;
}

void
KDbField::setPrecision(int p)
{
    if (!isFPNumericType())
        return;
    m_precision = p;
}

void
KDbField::setScale(int s)
{
    if (!isFPNumericType())
        return;
    m_maxLength = s;
}

void
KDbField::setVisibleDecimalPlaces(int p)
{
    if (!KDb::supportsVisibleDecimalPlacesProperty(type()))
        return;
    m_visibleDecimalPlaces = p < 0 ? -1 : p;
}

void
KDbField::setUnsigned(bool u)
{
    m_options |= Unsigned;
    if (!u)
        m_options ^= Unsigned;
}

void
KDbField::setDefaultValue(const QVariant& def)
{
    m_defaultValue = def;
}

bool
KDbField::setDefaultValue(const QByteArray& def)
{
    if (def.isNull()) {
        m_defaultValue = QVariant();
        return true;
    }

    bool ok;
    switch (type()) {
    case Byte: {
        unsigned int v = def.toUInt(&ok);
        if (!ok || v > 255)
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant(v);
        break;
    }
    case ShortInteger: {
        int v = def.toInt(&ok);
        if (!ok || (!(m_options & Unsigned) && (v < -32768 || v > 32767)) || ((m_options & Unsigned) && (v < 0 || v > 65535)))
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant(v);
        break;
    }
    case Integer: {//4 bytes
        long v = def.toLong(&ok);
//! @todo    if (!ok || (!(m_options & Unsigned) && (-v > 0x080000000 || v > (0x080000000-1))) || ((m_options & Unsigned) && (v < 0 || v > 0x100000000)))
        if (!ok || (!(m_options & Unsigned) && (-v > (int)0x07FFFFFFF || v > (int)(0x080000000 - 1))))
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant((qint64)v);
        break;
    }
    case BigInteger: {//8 bytes
//! @todo BigInteger support
        /*
              qint64 long v = def.toLongLong(&ok);
        //! @todo 2-part decoding
              if (!ok || (!(m_options & Unsigned) && (-v > 0x080000000 || v > (0x080000000-1))))
                m_defaultValue = QVariant();
              else
                if (m_options & Unsigned)
                  m_defaultValue=QVariant((quint64) v);
                else
                  m_defaultValue = QVariant((qint64)v);*/
        break;
    }
    case Boolean: {
        unsigned short v = def.toUShort(&ok);
        if (!ok || v > 1)
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant((bool)v);
        break;
    }
    case Date: {//YYYY-MM-DD
        QDate date = QDate::fromString(QLatin1String(def), Qt::ISODate);
        if (!date.isValid())
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant(date);
        break;
    }
    case DateTime: {//YYYY-MM-DDTHH:MM:SS
        QDateTime dt = QDateTime::fromString(QLatin1String(def), Qt::ISODate);
        if (!dt.isValid())
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant(dt);
        break;
    }
    case Time: {//HH:MM:SS
        QTime time = QTime::fromString(QLatin1String(def), Qt::ISODate);
        if (!time.isValid())
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant(time);
        break;
    }
    case Float: {
        float v = def.toFloat(&ok);
        if (!ok || ((m_options & Unsigned) && (v < 0.0)))
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant(v);
        break;
    }
    case Double: {
        double v = def.toDouble(&ok);
        if (!ok || ((m_options & Unsigned) && (v < 0.0)))
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant(v);
        break;
    }
    case Text: {
        if (def.isNull() || def.length() > maxLength())
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant(QLatin1String(def));
        break;
    }
    case LongText: {
        if (def.isNull())
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant(QLatin1String(def));
        break;
    }
    case BLOB: {
//! @todo
        if (def.isNull())
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant(def);
        break;
    }
    default:
        m_defaultValue = QVariant();
    }
    return m_defaultValue.isNull();
}

void
KDbField::setAutoIncrement(bool a)
{
    if (a && !isAutoIncrementAllowed())
        return;
    if (isAutoIncrement() != a)
        m_constraints ^= KDbField::AutoInc;
}

void
KDbField::setPrimaryKey(bool p)
{
    if (isPrimaryKey() != p)
        m_constraints ^= KDbField::PrimaryKey;
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

void
KDbField::setUniqueKey(bool u)
{
    if (isUniqueKey() != u) {
        m_constraints ^= KDbField::Unique;
        if (u) { //also set implied constraints
            setNotNull(true);
            setIndexed(true);
        }
    }
}

void
KDbField::setForeignKey(bool f)
{
    if (isForeignKey() != f)
        m_constraints ^= KDbField::ForeignKey;
}

void
KDbField::setNotNull(bool n)
{
    if (isNotNull() != n)
        m_constraints ^=KDbField::NotNull;
}

void KDbField::setNotEmpty(bool n)
{
    if (isNotEmpty() != n)
        m_constraints ^= KDbField::NotEmpty;
}

void KDbField::setIndexed(bool s)
{
    if (isIndexed() != s)
        m_constraints ^= KDbField::Indexed;
    if (!s) {//also set implied constraints
        setPrimaryKey(false);
        setUniqueKey(false);
        setNotNull(false);
        setNotEmpty(false);
    }
}

QDebug operator<<(QDebug dbg, const KDbField& field)
{
    KDbConnection *conn = field.table() ? field.table()->connection() : 0;
    if (field.name().isEmpty()) {
        dbg.nospace() << "<NONAME>";
    } else {
        dbg.nospace() << field.name();
    }
    if (field.options() & KDbField::Unsigned)
        dbg.nospace() << " UNSIGNED";
    dbg.nospace() << ' ' << qPrintable((conn && conn->driver())
        ? conn->driver()->sqlTypeName(field.type()) : KDbDriver::defaultSQLTypeName(field.type()));
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
            dbg.space() << QString::fromLatin1("%1 = %2 (%3)")
                .arg(QLatin1String(it.key())).arg(it.value().toString())
                .arg(QLatin1String(it.value().typeName()));
        }
    }
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
    return !m_expr->isNull();
}

KDbExpression KDbField::expression()
{
    return *m_expr;
}

const KDbExpression KDbField::expression() const {
    return *m_expr;
}

void KDbField::setExpression(const KDbExpression& expr)
{
    assert(!m_parent || dynamic_cast<KDbQuerySchema*>(m_parent));
    if (*m_expr == expr)
        return;
    *m_expr = expr;
}

QVariant KDbField::customProperty(const QByteArray& propertyName,
                               const QVariant& defaultValue) const
{
    if (!m_customProperties)
        return defaultValue;
    return m_customProperties->value(propertyName, defaultValue);
}

void KDbField::setCustomProperty(const QByteArray& propertyName, const QVariant& value)
{
    if (propertyName.isEmpty())
        return;
    if (!m_customProperties)
        m_customProperties = new CustomPropertiesMap();
    m_customProperties->insert(propertyName, value);
}

//-------------------------------------------------------
#define ADDTYPE(type, i18, str) \
    (*this)[KDbField::type] = i18; \
    (*this)[KDbField::type+KDbField::Null+1] = QLatin1String(str); \
    str2num[ QString::fromLatin1(str).toLower() ] = type; \
    names.append(i18)
#define ADDGROUP(type, i18, str) \
    (*this)[KDbField::type] = i18; \
    (*this)[KDbField::type+KDbField::LastTypeGroup+1] = QLatin1String(str); \
    str2num[ QString::fromLatin1(str).toLower() ] = type; \
    names.append(i18)

KDbField::FieldTypeNames::FieldTypeNames()
        : QVector<QString>()
        , m_initialized(false)
{
}

void KDbField::FieldTypeNames::init()
{
    if (m_initialized)
        return;
    m_initialized = true;
    resize((KDbField::Null + 1)*2);

    ADDTYPE(InvalidType, tr("Invalid Type"), "InvalidType");
    ADDTYPE(Byte, tr("Byte"), "Byte");
    ADDTYPE(ShortInteger, tr("Short Integer Number"), "ShortInteger");
    ADDTYPE(Integer, tr("Integer Number"), "Integer");
    ADDTYPE(BigInteger, tr("Big Integer Number"), "BigInteger");
    ADDTYPE(Boolean, tr("Yes/No Value"), "Boolean");
    ADDTYPE(Date, tr("Date"), "Date");
    ADDTYPE(DateTime, tr("Date and Time"), "DateTime");
    ADDTYPE(Time, tr("Time"), "Time");
    ADDTYPE(Float, tr("Single Precision Number"), "Float");
    ADDTYPE(Double, tr("Double Precision Number"), "Double");
    ADDTYPE(Text, tr("Text"), "Text");
    ADDTYPE(LongText, tr("Long Text"), "LongText");
    ADDTYPE(BLOB, tr("Object"), "BLOB");
    ADDTYPE(Null, QLatin1String("NULL")/*don't translate*/, "NULL");
}

//-------------------------------------------------------

KDbField::FieldTypeGroupNames::FieldTypeGroupNames()
        : QVector<QString>()
        , m_initialized(false)
{
}

void KDbField::FieldTypeGroupNames::init()
{
    if (m_initialized)
        return;
    m_initialized = true;
    resize((KDbField::LastTypeGroup + 1)*2);

    ADDGROUP(InvalidGroup, KDbField::tr("Invalid Group"), "InvalidGroup");
    ADDGROUP(TextGroup, KDbField::tr("Text"), "TextGroup");
    ADDGROUP(IntegerGroup, KDbField::tr("Integer Number"), "IntegerGroup");
    ADDGROUP(FloatGroup, KDbField::tr("Floating Point Number"), "FloatGroup");
    ADDGROUP(BooleanGroup, KDbField::tr("Yes/No"), "BooleanGroup");
    ADDGROUP(DateTimeGroup, KDbField::tr("Date/Time"), "DateTimeGroup");
    ADDGROUP(BLOBGroup, KDbField::tr("Object"), "BLOBGroup");
}

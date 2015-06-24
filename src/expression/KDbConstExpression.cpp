/* This file is part of the KDE project
   Copyright (C) 2003-2012 Jarosław Staniek <staniek@kde.org>

   Based on nexp.cpp : Parser module of Python-like language
   (C) 2001 Jarosław Staniek, MIMUW (www.mimuw.edu.pl)

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

#include "KDbExpression.h"
#include "KDb.h"
#include "KDbQuerySchema.h"
#include "KDbDriver.h"
#include "kdb_debug.h"
#include "generated/sqlparser.h"

#include <QPoint>

KDbConstExpressionData::KDbConstExpressionData(const QVariant& aValue)
 : KDbExpressionData()
 , value(aValue)
{
    ExpressionDebug << "ConstExpressionData" << ref;
}

KDbConstExpressionData::~KDbConstExpressionData()
{
    ExpressionDebug << "~ConstExpressionData" << ref;
}

KDbConstExpressionData* KDbConstExpressionData::clone()
{
    ExpressionDebug << "ConstExpressionData::clone" << *this;
    return new KDbConstExpressionData(*this);
}

KDbField::Type KDbConstExpressionData::typeInternal(KDb::ExpressionCallStack* callStack) const
{
    Q_UNUSED(callStack);
    switch (token.value()) {
    case SQL_NULL:
        return KDbField::Null;
    case INTEGER_CONST:
//! @todo ok?
//! @todo add sign info?
        if (value.type() == QVariant::Int || value.type() == QVariant::UInt) {
            qint64 v = value.toInt();
            if (v <= 0xff && v > -0x80)
                return KDbField::Byte;
            if (v <= 0xffff && v > -0x8000)
                return KDbField::ShortInteger;
            return KDbField::Integer;
        }
        return KDbField::BigInteger;
    case CHARACTER_STRING_LITERAL:
        if (KDbField::defaultMaxLength() > 0
            && uint(value.toString().length()) > KDbField::defaultMaxLength())
        {
            return KDbField::LongText;
        }
        else {
            return KDbField::Text;
        }
    case SQL_TRUE:
    case SQL_FALSE:
        return KDbField::Boolean;
    case REAL_CONST:
        return KDbField::Double;
    case DATE_CONST:
        return KDbField::Date;
    case DATETIME_CONST:
        return KDbField::DateTime;
    case TIME_CONST:
        return KDbField::Time;
    }
    return KDbField::InvalidType;
}

void KDbConstExpressionData::debugInternal(QDebug dbg, KDb::ExpressionCallStack* callStack) const
{
    Q_UNUSED(callStack);
    QString res = QLatin1String("ConstExp(")
        + token.name()
        + QLatin1String(",") + toString().toString()
        + QString::fromLatin1(",type=%1").arg(KDbDriver::defaultSQLTypeName(type()));
    if (value.type() == QVariant::Point && token.value() == REAL_CONST) {
        res += QLatin1String(",DECIMAL");
    }
    res += QLatin1String(")");
    dbg.nospace() << res.toLocal8Bit().constData();
}

KDbEscapedString KDbConstExpressionData::toStringInternal(KDbQuerySchemaParameterValueListIterator* params,
                                                    KDb::ExpressionCallStack* callStack) const
{
    Q_UNUSED(params);
    Q_UNUSED(callStack);
    switch (token.value()) {
    case SQL_NULL:
        return KDbEscapedString("NULL");
    case CHARACTER_STRING_LITERAL:
//! @todo better escaping!
        return KDbEscapedString('\'') + value.toString() + '\'';
    case SQL_TRUE:
        return KDbEscapedString("TRUE");
    case SQL_FALSE:
        return KDbEscapedString("FALSE");
    case REAL_CONST: {
        if (value.type() == QVariant::Point) {
            return KDbEscapedString("%1.%2").arg(value.toPoint().x()).arg(qAbs(value.toPoint().y()));
        }
        if (!value.canConvert<qreal>()) {
            return KDbEscapedString("0.0");
        }
        break;
    }
    case DATE_CONST:
        return KDbEscapedString('\'') + value.toDate().toString(Qt::ISODate) + '\'';
    case DATETIME_CONST:
        return KDbEscapedString('\'')
                + KDbEscapedString(value.toDateTime().date().toString(Qt::ISODate))
                + ' ' + value.toDateTime().time().toString(Qt::ISODate) + '\'';
    case TIME_CONST:
        return KDbEscapedString('\'') + value.toTime().toString(Qt::ISODate) + '\'';
    case INTEGER_CONST:
    default:
        break;
    }
    return KDbEscapedString(value.toString());
}

void KDbConstExpressionData::getQueryParameters(QList<KDbQuerySchemaParameter>& params)
{
    Q_UNUSED(params);
}

bool KDbConstExpressionData::validateInternal(KDbParseInfo *parseInfo, KDb::ExpressionCallStack* callStack)
{
    Q_UNUSED(parseInfo);
    return typeInternal(callStack) != KDbField::InvalidType;
}

//=========================================

KDbConstExpression::KDbConstExpression()
 : KDbExpression(new KDbConstExpressionData(QVariant()))
{
    ExpressionDebug << "KDbConstExpression() ctor" << *this;
}

KDbConstExpression::KDbConstExpression(KDbToken token, const QVariant& value)
        : KDbExpression(new KDbConstExpressionData(value), KDb::ConstExpression, token)
{
}

KDbConstExpression::KDbConstExpression(KDbExpressionData* data, KDb::ExpressionClass aClass,
                                       KDbToken token)
        : KDbExpression(data, aClass, token)
{
}

KDbConstExpression::KDbConstExpression(KDbExpressionData* data)
    : KDbExpression(data)
{
}

KDbConstExpression::KDbConstExpression(const ExplicitlySharedExpressionDataPointer &ptr)
    : KDbExpression(ptr)
{
}

KDbConstExpression::KDbConstExpression(const KDbConstExpression& expr)
        : KDbExpression(expr)
{
}

KDbConstExpression::~KDbConstExpression()
{
}

QVariant KDbConstExpression::value() const
{
    return d->convert<const KDbConstExpressionData>()->value;
}

void KDbConstExpression::setValue(const QVariant& value)
{
    d->convert<KDbConstExpressionData>()->value = value;
}

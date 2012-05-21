/* This file is part of the KDE project
   Copyright (C) 2003-2011 Jarosław Staniek <staniek@kde.org>

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

#include <Predicate/Expression>
#include <Predicate/Utils>
#include <Predicate/QuerySchema>
#include <Predicate/Tools/Static>
#include "Expression_p.h"
#include "parser/SqlParser.h"

#include <ctype.h>

using namespace Predicate;

ConstExpressionData::ConstExpressionData(const QVariant& aValue)
 : ExpressionData()
 , value(aValue)
{
    qDebug() << "ConstExpressionData" << ref;
}

ConstExpressionData::~ConstExpressionData()
{
    qDebug() << "~ConstExpressionData" << ref;
}

ConstExpressionData* ConstExpressionData::clone()
{
    qDebug() << "ConstExpressionData::clone" << *this;
    return new ConstExpressionData(*this);
}

Field::Type ConstExpressionData::type() const
{
    switch (token) {
    case SQL_NULL:
        return Field::Null;
    case INTEGER_CONST:
//! @todo ok?
//! @todo add sign info?
        if (value.type() == QVariant::Int || value.type() == QVariant::UInt) {
            qint64 v = value.toInt();
            if (v <= 0xff && v > -0x80)
                return Field::Byte;
            if (v <= 0xffff && v > -0x8000)
                return Field::ShortInteger;
            return Field::Integer;
        }
        return Field::BigInteger;
    case CHARACTER_STRING_LITERAL:
//! @todo Field::defaultTextLength() is hardcoded now!
        if (value.toString().length() > (int)Field::defaultTextLength())
            return Field::LongText;
        else
            return Field::Text;
    case REAL_CONST:
        return Field::Double;
    case DATE_CONST:
        return Field::Date;
    case DATETIME_CONST:
        return Field::DateTime;
    case TIME_CONST:
        return Field::Time;
    }
    return Field::InvalidType;
}

QDebug ConstExpressionData::debug(QDebug dbg) const
{
    dbg.nospace() << "ConstExp(" << Expression::tokenToDebugString(token)
           << QLatin1Char(',') << toString().toString()
           << QString::fromLatin1(",type=%1)").arg(Driver::defaultSQLTypeName(type()));
    return dbg.space();
}

EscapedString ConstExpressionData::toString(QuerySchemaParameterValueListIterator* params) const
{
    Q_UNUSED(params);
    switch (token) {
    case SQL_NULL:
        return EscapedString("NULL");
    case CHARACTER_STRING_LITERAL:
//! @todo better escaping!
        return EscapedString('\'') + value.toString() + '\'';
    case REAL_CONST:
        return EscapedString::number(value.toPoint().x()) + '.'
                + EscapedString::number(value.toPoint().y());
    case DATE_CONST:
        return EscapedString('\'') + value.toDate().toString(Qt::ISODate) + '\'';
    case DATETIME_CONST:
        return EscapedString('\'')
                + EscapedString(value.toDateTime().date().toString(Qt::ISODate))
                + ' ' + value.toDateTime().time().toString(Qt::ISODate) + '\'';
    case TIME_CONST:
        return EscapedString('\'') + value.toTime().toString(Qt::ISODate) + '\'';
    }
    return EscapedString(value.toString());
}

void ConstExpressionData::getQueryParameters(QuerySchemaParameterList& params)
{
    Q_UNUSED(params);
}

bool ConstExpressionData::validate(ParseInfo& parseInfo)
{
    if (!ExpressionData::validate(parseInfo))
        return false;

    return type() != Field::InvalidType;
}

//=========================================

ConstExpression::ConstExpression()
 : Expression(new ConstExpressionData(QVariant()))
{
    qDebug() << "ConstExpression() ctor" << *this;
}

ConstExpression::ConstExpression(int token, const QVariant& value)
        : Expression(new ConstExpressionData(value), ConstExpressionClass, token)
{
}

ConstExpression::ConstExpression(ExpressionData* data, ExpressionClass aClass,
                                 int token)
        : Expression(data, aClass, token)
{
}

ConstExpression::ConstExpression(ExpressionData* data)
    : Expression(data)
{
}

ConstExpression::ConstExpression(const ConstExpression& expr)
        : Expression(expr)
{
}

ConstExpression::~ConstExpression()
{
}

QVariant ConstExpression::value() const
{
    return d->convert<const ConstExpressionData>()->value;
}

void ConstExpression::setValue(const QVariant& value)
{
    d->convert<ConstExpressionData>()->value = value;
}

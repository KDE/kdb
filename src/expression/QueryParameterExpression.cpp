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
#include "Expression_p.h"
#include <Predicate/Utils>
#include <Predicate/QuerySchema>
#include "parser/SqlParser.h"
#include "parser/Parser_p.h"
#include <Predicate/Tools/Static>

#include <ctype.h>

using namespace Predicate;

QueryParameterExpressionData::QueryParameterExpressionData()
 : ConstExpressionData()
 , m_type(Field::InvalidType)
{
    qDebug() << "QueryParameterExpressionData" << ref;
}

QueryParameterExpressionData::QueryParameterExpressionData(
    Field::Type type, const QVariant& value)
 : ConstExpressionData(value)
 , m_type(type)
{
   qDebug() << "QueryParameterExpressionData" << ref;
}

QueryParameterExpressionData::~QueryParameterExpressionData()
{
    qDebug() << "~QueryParameterExpressionData" << ref;
}

QueryParameterExpressionData* QueryParameterExpressionData::clone()
{
    qDebug() << "QueryParameterExpressionData::clone" << *this;
    return new QueryParameterExpressionData(*this);
}

QDebug QueryParameterExpressionData::debug(QDebug dbg) const
{
    dbg.nospace() << "QueryParExp("
        + QString::fromLatin1("[%2]").arg(value.toString())
        + QString(",type=%1)").arg(Driver::defaultSQLTypeName(type()));
    return dbg.space();
}

EscapedString QueryParameterExpressionData::toString(
    QuerySchemaParameterValueListIterator* params) const
{
    return params ? params->getPreviousValueAsString(type())
           : EscapedString("[%1]").arg(EscapedString(value.toString()));
}

void QueryParameterExpressionData::getQueryParameters(QuerySchemaParameterList& params)
{
    QuerySchemaParameter param;
    param.message = value.toString();
    param.type = type();
    params.append(param);
}

bool QueryParameterExpressionData::validate(ParseInfo& parseInfo)
{
    Q_UNUSED(parseInfo);
    return type() != Field::InvalidType;
}

Field::Type QueryParameterExpressionData::type() const
{
    return m_type;
}

//=========================================

QueryParameterExpression::QueryParameterExpression()
 : ConstExpression(new QueryParameterExpressionData)
{
    qDebug() << "QueryParameterExpression() ctor" << *this;
}

QueryParameterExpression::QueryParameterExpression(const QString& message)
        : ConstExpression(new QueryParameterExpressionData(Field::Text, message),
              QueryParameterExpressionClass, QUERY_PARAMETER)
{
}

QueryParameterExpression::QueryParameterExpression(const QueryParameterExpression& expr)
        : ConstExpression(expr)
{
}

QueryParameterExpression::QueryParameterExpression(ExpressionData* data)
    : ConstExpression(data)
{
    qDebug() << "QueryParameterExpression ctor (ExpressionData*)" << *this;
}

QueryParameterExpression::~QueryParameterExpression()
{
}

void QueryParameterExpression::setType(Field::Type type)
{
    d->convert<QueryParameterExpressionData>()->m_type = type;
}

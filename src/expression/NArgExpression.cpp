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
#include "parser/Parser_p.h"

#include <ctype.h>

using namespace Predicate;

NArgExpressionData::NArgExpressionData()
 : ExpressionData()
{
    qDebug() << "NArgExpressionData" << ref;
}

NArgExpressionData::~NArgExpressionData()
{
    qDebug() << "~NArgExpressionData" << ref;
}

NArgExpressionData* NArgExpressionData::clone()
{
    qDebug() << "NArgExpressionData::clone" << *this;
    return new NArgExpressionData(*this);
}

bool NArgExpressionData::validate(ParseInfo& parseInfo)
{
    if (!ExpressionData::validate(parseInfo))
        return false;

    foreach(ExplicitlySharedExpressionDataPointer data, children) {
        if (!data->validate(parseInfo))
            return false;
    }
    return true;
}

QDebug NArgExpressionData::debug(QDebug dbg) const
{
    dbg.nospace() << "NArgExp(class="
        << expressionClassName(expressionClass);
    foreach(ExplicitlySharedExpressionDataPointer data, children) {
        dbg.nospace() << ", ";
        dbg.nospace() << *data;
    }
    dbg.nospace() << ")";
    return dbg.space();
}

EscapedString NArgExpressionData::toString(QuerySchemaParameterValueListIterator* params) const
{
    EscapedString s;
    s.reserve(256);
    foreach(ExplicitlySharedExpressionDataPointer data, children) {
        if (!s.isEmpty())
            s += ", ";
        s += data->toString(params);
    }
    return s;
}

void NArgExpressionData::getQueryParameters(QuerySchemaParameterList& params)
{
    foreach(ExplicitlySharedExpressionDataPointer data, children) {
        data->getQueryParameters(params);
    }
}

//=========================================

NArgExpression::NArgExpression()
 : Expression(new NArgExpressionData)
{
    qDebug() << "NArgExpression() ctor" << *this;
}

NArgExpression::NArgExpression(ExpressionData* data)
 : Expression(data)
{
    qDebug() << "NArgExpression(ExpressionData*) ctor" << *this;
}

NArgExpression::NArgExpression(ExpressionClass aClass, int token)
        : Expression(new NArgExpressionData, aClass, token)
{
    qDebug() << "NArgExpression(ExpressionClass, int) ctor" << *this;
}

NArgExpression::NArgExpression(const NArgExpression& expr)
        : Expression(expr)
{
    //list = expr.list;
/*    foreach(Expression* e, expr.list) {
        add(e->copy());
    }*/
}

NArgExpression::~NArgExpression()
{
    //qDeleteAll(list);
}

// NArgExpression* NArgExpression::copy() const
// {
//     return new NArgExpression(*this);
// }

void NArgExpression::append(const Expression& expr)
{
    appendChild(expr);
}

void NArgExpression::prepend(const Expression& expr)
{
    prependChild(expr);
}

Expression NArgExpression::arg(int n) const
{
    d->children.at(n);
}

int NArgExpression::argCount() const
{
    return d->children.count();
}

int NArgExpression::isEmpty() const
{
    return d->children.isEmpty();
}

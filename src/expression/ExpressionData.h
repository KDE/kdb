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

#ifndef PREDICATE_EXPRESSION_P_H
#define PREDICATE_EXPRESSION_P_H

#include "Utils.h"
#include "QuerySchema.h"
#include "Tools/Static.h"

#include <ctype.h>

namespace Predicate {
class ExpressionData;
class UnaryExpressionData;
class ConstExpressionData;
class QueryParameterExpressionData;
class ParseInfo;

//! Classes of expressions
enum ExpressionClass {
    UnknownExpressionClass,
    UnaryExpressionClass,
    ArithmeticExpressionClass,
    LogicalExpressionClass,
    RelationalExpressionClass,
    SpecialBinaryExpressionClass,
    ConstExpressionClass,
    VariableExpressionClass,
    FunctionExpressionClass,
    AggregationExpressionClass,
    TableListExpressionClass,
    QueryParameterExpressionClass
};

typedef QExplicitlySharedDataPointer<ExpressionData> ExplicitlySharedExpressionDataPointer;

//! Internal data class used to implement implicitly shared class Expression.
//! Provides thread-safe reference counting.
class ExpressionData : public QSharedData
{
public:
    ExpressionData();

    /*Data(const Data& other)
    : QSharedData(other)
    , token(other.token)
    , expressionClass(other.expressionClass)
    , parent(other.parent)
    , children(other.children)
    {
        qDebug() << "ExpressionData" << ref;
    }*/

    virtual ~ExpressionData();

    //! @see Expression::token()
    int token;
    //! @see Expression::expressionClass()
    ExpressionClass expressionClass;
    ExplicitlySharedExpressionDataPointer parent;
    QList<ExplicitlySharedExpressionDataPointer> children;
    virtual Field::Type type() const; //!< @return type of this expression;
    virtual EscapedString toString(QuerySchemaParameterValueListIterator* params = 0) const;
    virtual void getQueryParameters(QuerySchemaParameterList& params);
    virtual bool validate(ParseInfo& parseInfo);
    virtual QString tokenToString() const; //!< @return string for token, like "<=" or ">";
    virtual ExpressionData* clone();
    //! Sends information about this expression  to debug output @a dbg.
    virtual QDebug debug(QDebug dbg) const;

    template <typename T>
    const T* convertConst() const { return dynamic_cast<const T*>(this); }

    template <typename T>
    T* convert() { return dynamic_cast<T*>(this); }
};

//! Internal data class used to implement implicitly shared class NArgExpression.
//! Provides thread-safe reference counting.
class NArgExpressionData : public ExpressionData
{
public:
    NArgExpressionData();
    virtual ~NArgExpressionData();

    //virtual Field::Type type() const; //!< @return type of this expression;
    virtual EscapedString toString(QuerySchemaParameterValueListIterator* params = 0) const;
    virtual void getQueryParameters(QuerySchemaParameterList& params);
    virtual bool validate(ParseInfo& parseInfo);
//    virtual QString tokenToString() const; //!< @return string for token, like "<=" or ">";
    virtual NArgExpressionData* clone();
    //! Sends information about this expression  to debug output @a dbg.
    virtual QDebug debug(QDebug dbg) const;
};

//! Internal data class used to implement implicitly shared class UnaryExpression.
//! Provides thread-safe reference counting.
class UnaryExpressionData : public ExpressionData
{
public:
    UnaryExpressionData();
    virtual ~UnaryExpressionData();

    virtual Field::Type type() const; //!< @return type of this expression;
    virtual EscapedString toString(QuerySchemaParameterValueListIterator* params = 0) const;
    virtual void getQueryParameters(QuerySchemaParameterList& params);
    virtual bool validate(ParseInfo& parseInfo);
//    virtual QString tokenToString() const; //!< @return string for token, like "<=" or ">";
    virtual UnaryExpressionData* clone();
    //! Sends information about this expression  to debug output @a dbg.
    virtual QDebug debug(QDebug dbg) const;

    inline ExplicitlySharedExpressionDataPointer arg() const {
        if (children.isEmpty())
            return ExplicitlySharedExpressionDataPointer();
        return children.first();
    }
};

//! Internal data class used to implement implicitly shared class BinaryExpression.
//! Provides thread-safe reference counting.
class BinaryExpressionData : public ExpressionData
{
public:
    BinaryExpressionData();
    virtual ~BinaryExpressionData();

    virtual Field::Type type() const; //!< @return type of this expression;
    virtual EscapedString toString(QuerySchemaParameterValueListIterator* params = 0) const;
    virtual void getQueryParameters(QuerySchemaParameterList& params);
    virtual bool validate(ParseInfo& parseInfo);
    virtual QString tokenToString() const; //!< @return string for token, like "<=" or ">";
    virtual BinaryExpressionData* clone();
    //! Sends information about this expression  to debug output @a dbg.
    virtual QDebug debug(QDebug dbg) const;

    inline ExplicitlySharedExpressionDataPointer left() const { return children[0]; }
    inline ExplicitlySharedExpressionDataPointer right() const { return children[1]; }
};

//! Internal data class used to implement implicitly shared class ConstExpression.
//! Provides thread-safe reference counting.
class ConstExpressionData : public ExpressionData
{
public:
    ConstExpressionData(const QVariant& aValue = QVariant());
    virtual ~ConstExpressionData();

    QVariant value;
    virtual Field::Type type() const; //!< @return type of this expression;
    virtual EscapedString toString(QuerySchemaParameterValueListIterator* params = 0) const;
    virtual void getQueryParameters(QuerySchemaParameterList& params);
    virtual bool validate(ParseInfo& parseInfo);
    //virtual QString tokenToString() const;
    virtual ConstExpressionData* clone();
    //! Sends information about this expression  to debug output @a dbg.
    virtual QDebug debug(QDebug dbg) const;
};

//! Internal data class used to implement implicitly shared class QueryParameterExpression.
//! Provides thread-safe reference counting.
class QueryParameterExpressionData : public ConstExpressionData
{
public:
    QueryParameterExpressionData();
    QueryParameterExpressionData(Field::Type type, const QVariant& value);
    virtual ~QueryParameterExpressionData();

    Field::Type m_type;
    virtual Field::Type type() const; //!< @return type of this expression;
    virtual EscapedString toString(QuerySchemaParameterValueListIterator* params = 0) const;
    virtual void getQueryParameters(QuerySchemaParameterList& params);
    virtual bool validate(ParseInfo& parseInfo);
    //virtual QString tokenToString() const;
    virtual QueryParameterExpressionData* clone();
    //! Sends information about this expression  to debug output @a dbg.
    virtual QDebug debug(QDebug dbg) const;
};

//! Internal data class used to implement implicitly shared class VariableExpression.
//! Provides thread-safe reference counting.
class VariableExpressionData : public ExpressionData
{
public:
    VariableExpressionData();
    VariableExpressionData(const QString& aName);
    virtual ~VariableExpressionData();

    /*! Verbatim name as returned by scanner. */
    QString name;

    /*! 0 by default. After successful validate() it will point to a field,
     if the variable is of a form "tablename.fieldname" or "fieldname",
     otherwise (eg. for asterisks) still 0.
     Only meaningful for column expressions within a query. */
    Field *field;

    /*! -1 by default. After successful validate() it will contain a position of a table
     within query that needs to be bound to the field.
     This value can be either be -1 if no binding is needed.
     This value is used in the Parser to call
      QuerySchema::addField(Field* field, int bindToTable);
     Only meaningful for column expressions within a query. */
    int tablePositionForField;

    /*! 0 by default. After successful validate() it will point to a table
     that is referenced by asterisk, i.e. "*.tablename".
     This is set to NULL if this variable is not an asterisk of that form. */
    TableSchema *tableForQueryAsterisk;

    virtual Field::Type type() const; //!< @return type of this expression;
    virtual EscapedString toString(QuerySchemaParameterValueListIterator* params = 0) const;
    virtual void getQueryParameters(QuerySchemaParameterList& params);
    /*! Validation. Sets field, tablePositionForField
     and tableForQueryAsterisk members.
     See addColumn() in parse.y to see how it's used on column adding. */
    virtual bool validate(ParseInfo& parseInfo);
    //virtual QString tokenToString() const;
    virtual VariableExpressionData* clone();
    //! Sends information about this expression  to debug output @a dbg.
    virtual QDebug debug(QDebug dbg) const;
};

//! Internal data class used to implement implicitly shared class FunctionExpression.
//! Provides thread-safe reference counting.
class FunctionExpressionData : public ExpressionData
{
public:
    FunctionExpressionData();
    FunctionExpressionData(const QString& aName);
    virtual ~FunctionExpressionData();

    QString name;
    ExplicitlySharedExpressionDataPointer args;

    virtual Field::Type type() const; //!< @return type of this expression;
    virtual EscapedString toString(QuerySchemaParameterValueListIterator* params = 0) const;
    virtual void getQueryParameters(QuerySchemaParameterList& params);
    /*! Validation. Sets field, tablePositionForField
     and tableForQueryAsterisk members.
     See addColumn() in parse.y to see how it's used on column adding. */
    virtual bool validate(ParseInfo& parseInfo);
    //virtual QString tokenToString() const;
    virtual FunctionExpressionData* clone();
    //! Sends information about this expression  to debug output @a dbg.
    virtual QDebug debug(QDebug dbg) const;
};

} // namespace Predicate

QDebug operator<<(QDebug dbg, const Predicate::ExpressionData& expr);

#endif

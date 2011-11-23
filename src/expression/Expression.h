/* This file is part of the KDE project
   Copyright (C) 2003-2010 Jarosław Staniek <staniek@kde.org>

   Design based on nexp.h : Parser module of Python-like language
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

#ifndef PREDICATE_EXPRESSION_H
#define PREDICATE_EXPRESSION_H

#include <Predicate/Global>
#include <Predicate/Field>
#include <Predicate/QuerySchemaParameter>
#include <Predicate/EscapedString>

#include <QtDebug>

#include "Expression_p.h"

namespace Predicate
{

//! Custom tokens are not used in parser but used as extension in expression classes.
//#define PREDICATE_CUSTOM_TOKEN 0x1000

//! @return class name of class @a c
PREDICATE_EXPORT QString expressionClassName(ExpressionClass c);

class ParseInfo;
class NArgExpression;
class UnaryExpression;
class BinaryExpression;
class ConstExpression;
class VariableExpression;
class FunctionExpression;
class QueryParameterExpression;
class QuerySchemaParameterValueListIterator;

//class ExpressionData;

//! A base class for all expressions
class PREDICATE_EXPORT Expression
{
public:
    Expression();

    /*Expression(const QExplicitlySharedDataPointer<Data> &data)
     : d(data)
    {
    }*/
// 
    virtual ~Expression();

    //! @return true if this expression is null. 
    //! Equivalent of expressionClass() == Predicate::UnknownExpressionClass.
    bool isNull() const;

    //! Creates a deep (not shallow) copy of the Expression.
    Expression clone() const;

    /*!
    @return the token for this expression. Tokens are characters (e.g. '+', '-')
    or identifiers (e.g. SQL_NULL) of elements used by the PredicateSQL parser.
    */
    int token() const;

    /*!
    @return class identifier of this expression.
    */
    ExpressionClass expressionClass() const;

    // //! @return a deep copy of this object.
    // //! @todo a nonpointer will be returned here when we move to implicit data sharing
    //virtual Expression* copy() const = 0;

    /*! @return type of this expression, based on effect of its evaluation. */
    Field::Type type() const;

    /*! @return true if evaluation of this expression succeeded. */
    bool validate(ParseInfo& parseInfo);

    /*! @return string as a representation of this expression element
                by running recursive calls.
     @a param, if not 0, points to a list item containing value
     of a query parameter (used in QueryParameterExpr). */
    EscapedString toString(QuerySchemaParameterValueListIterator* params = 0) const;

    /*! Collects query parameters (messages and types) reculsively and saves them to params.
     The leaf nodes are objects of QueryParameterExpr class. */
    void getQueryParameters(QuerySchemaParameterList& params);

    //! @return debug string, used in QDebug operator<<(QDebug, const Expression&)
//    virtual QString debugString() const;

    static QString tokenToDebugString(int token);

    /*! @return single character if the token is < 256
     or token name, e.g. LESS_OR_EQUAL (for debugging). */
    inline QString tokenToDebugString() const {
        return tokenToDebugString(token());
    }

    //! @return string for token, like "<=" or ">"
    QString tokenToString() const;

    //! Convenience type casts.
    NArgExpression toNArg() const;
    UnaryExpression toUnary() const;
    BinaryExpression toBinary() const;
    ConstExpression toConst() const;
    VariableExpression toVariable() const;
    FunctionExpression toFunction() const;
    QueryParameterExpression toQueryParameter() const;

    bool isNArg() const;
    bool isUnary() const;
    bool isBinary() const;
    bool isConst() const;
    bool isVariable() const;
    bool isFunction() const;
    bool isQueryParameter() const;

    QDebug debug(QDebug dbg) const;

    bool operator==(const Expression& e) const;

    bool operator!=(const Expression& e) const;
    
protected:
    /*! @return the parent expression. */
    Expression parent() const;

    /*! @return the list of children expressions. */
    QList<ExplicitlySharedExpressionDataPointer> children() const;

    /*! Sets the parent expression. */
    void setParent(const Expression& parent);
    
    void appendChild(const Expression& child);

    void prependChild(const Expression& child);

    void removeChild(const Expression& child);

    void appendChild(const ExplicitlySharedExpressionDataPointer& child) const;

    Expression(ExpressionData* data);

    Expression(ExpressionData* data, ExpressionClass aClass, int token);

    //friend QDebug Predicate::operator<<(QDebug, const Expression&);

    //! @internal
    ExplicitlySharedExpressionDataPointer d;

    friend class NArgExpression;
    friend class UnaryExpression;
    friend class BinaryExpression;
    friend class ConstExpression;
    friend class QueryParameterExpression;
    friend class VariableExpression;
    friend class FunctionExpression;
};

//! A base class N-argument operation
class PREDICATE_EXPORT NArgExpression : public Expression
{
public:
    NArgExpression();
    NArgExpression(ExpressionClass aClass, int token);
    NArgExpression(const NArgExpression& expr);
    virtual ~NArgExpression();
    //! @return a deep copy of this object.
    //virtual NArgExpression* copy() const;
    void append(const Expression& expr);
    void prepend(const Expression& expr);
    Expression arg(int n) const;
    int argCount() const;
    int isEmpty() const;
protected:
    explicit NArgExpression(ExpressionData* data);
    
    friend class Expression;
    friend class FunctionExpression;
};

//! An unary argument operation: + - NOT (or !) ~ "IS NULL" "IS NOT NULL"
class PREDICATE_EXPORT UnaryExpression : public Expression
{
public:
    UnaryExpression();
    UnaryExpression(int token, const Expression& arg);
    UnaryExpression(const UnaryExpression& expr);
    virtual ~UnaryExpression();
    // //! @return a deep copy of this object.
    //virtual UnaryExpression* copy() const;
    Expression arg() const;

protected:
    explicit UnaryExpression(ExpressionData* data);

    friend class Expression;
};

/*! A base class for binary operation
 - arithmetic operations: + - / * % << >> & | ||
 - relational operations: = (or ==) < > <= >= <> (or !=) LIKE IN 'SIMILAR TO' 'NOT SIMILAR TO'
 - logical operations: OR (or ||) AND (or &&) XOR
 - SpecialBinary "pseudo operators":
    * e.g. "f1 f2" : token == 0
    * e.g. "f1 AS f2" : token == AS
*/
class PREDICATE_EXPORT BinaryExpression : public Expression
{
public:
    BinaryExpression();
    BinaryExpression(ExpressionClass aClass, const Expression& leftExpr,
                     int token, const Expression& rightExpr);
    BinaryExpression(const BinaryExpression& expr);
    virtual ~BinaryExpression();
    // //! @return a deep copy of this object.
    //virtual BinaryExpression* copy() const;
    Expression left() const;
    void setLeft(const Expression& leftExpr);
    Expression right() const;
    void setRight(const Expression& rightExpr);

protected:
    explicit BinaryExpression(ExpressionData* data);

    friend class Expression;
};

/*! String, integer, float constants also includes NULL value.
 token can be: IDENTIFIER, SQL_NULL, CHARACTER_STRING_LITERAL,
 INTEGER_CONST, REAL_CONST */
class PREDICATE_EXPORT ConstExpression : public Expression
{
public:
    ConstExpression();
    ConstExpression(int token, const QVariant& value);
    ConstExpression(const ConstExpression& expr);
    virtual ~ConstExpression();
    // //! @return a deep copy of this object.
    //virtual ConstExpression* copy() const;

    QVariant value() const;
    void setValue(const QVariant& value);

protected:
    ConstExpression(ExpressionData* data, ExpressionClass aClass,
                    int token);
    explicit ConstExpression(ExpressionData* data);

    friend class Expression;
};

//! Query parameter used to getting user input of constant values.
//! It contains a message that is displayed to the user.
class PREDICATE_EXPORT QueryParameterExpression : public ConstExpression
{
public:
    QueryParameterExpression();
    explicit QueryParameterExpression(const QString& message);
    QueryParameterExpression(const QueryParameterExpression& expr);
    virtual ~QueryParameterExpression();
    // //! @return a deep copy of this object.
    //virtual QueryParameterExpression* copy() const;  
//    virtual Field::Type type() const;
    /*! Sets expected type of the parameter. The default is String.
     This method is called from parent's expression validate().
     This depends on the type of the related expression.
     For instance: query "SELECT * FROM cars WHERE name=[enter name]",
     "[enter name]" has parameter of the same type as "name" field.
     "=" binary expression's validate() will be called for the left side
     of the expression and then the right side will have type set to String.
    */
    void setType(Field::Type type);
protected:
    explicit QueryParameterExpression(ExpressionData* data);

    friend class Expression;
};

//! Variables like <i>fieldname</i> or <i>tablename</i>.<i>fieldname</i>
class PREDICATE_EXPORT VariableExpression : public Expression
{
public:
    VariableExpression();
    explicit VariableExpression(const QString& name);
    VariableExpression(const VariableExpression& expr);
    virtual ~VariableExpression();
    // //! @return a deep copy of this object.
    //virtual VariableExpression* copy() const;

    /*! Verbatim name as returned by scanner. */
    QString name() const;

    /*! 0 by default. After successful validate() it returns a field,
     if the variable is of a form "tablename.fieldname" or "fieldname",
     otherwise (eg. for asterisks) still 0.
     Only meaningful for column expressions within a query. */
    Field *field() const;

    /*! -1 by default. After successful validate() it returns a position of a table
     within query that needs to be bound to the field.
     This value can be either be -1 if no binding is needed.
     This value is used in the Parser to call
      QuerySchema::addField(Field* field, int bindToTable);
     Only meaningful for column expressions within a query. */
    int tablePositionForField() const;

    /*! 0 by default. After successful validate() it returns table that
     is referenced by asterisk, i.e. "*.tablename".
     It is 0 if this variable is not an asterisk of that form. */
    TableSchema *tableForQueryAsterisk() const;

protected:
    explicit VariableExpression(ExpressionData* data);

    friend class Expression;
};

//! - aggregation functions like SUM, COUNT, MAX, ...
//! - builtin functions like CURRENT_TIME()
//! - user defined functions
class PREDICATE_EXPORT FunctionExpression : public Expression
{
public:
    FunctionExpression();
    explicit FunctionExpression(const QString& name);
    FunctionExpression(const QString& name, NArgExpression& args);
    FunctionExpression(const FunctionExpression& expr);
    virtual ~FunctionExpression();
    // //! @return a deep copy of this obect.
    // virtual FunctionExpression* copy() const;

    QString name() const;
    NArgExpression arguments() const;

    static QStringList builtInAggregates();
    static bool isBuiltInAggregate(const QString& function);
protected:
    explicit FunctionExpression(ExpressionData* data);

    friend class Expression;
};

} //namespace Predicate

//! Sends information about expression  @a expr to debug output @a dbg.
PREDICATE_EXPORT QDebug operator<<(QDebug dbg, const Predicate::Expression& expr);

#endif

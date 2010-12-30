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

#include <Predicate/Global.h>
#include <Predicate/Field.h>
#include <Predicate/QuerySchema.h>

#include <QtDebug>
#include <QList>
#include <QByteArray>

namespace Predicate
{

//! classes
//todo: enum ExpressionClass
//todo:     UnknownExpression = 0,
#define PredicateExpr_Unknown 0
#define PredicateExpr_Unary 1
#define PredicateExpr_Arithm 2
#define PredicateExpr_Logical 3
#define PredicateExpr_Relational 4
#define PredicateExpr_SpecialBinary 5
#define PredicateExpr_Const 6
#define PredicateExpr_Variable 7
#define PredicateExpr_Function 8
#define PredicateExpr_Aggregation 9
#define PredicateExpr_TableList 10
#define PredicateExpr_QueryParameter 11

//! Custom tokens are not used in parser but used as extension in expression classes.
//#define PREDICATE_CUSTOM_TOKEN 0x1000

//! @return class name of class @a c
PREDICATE_EXPORT QString exprClassName(int c);

class ParseInfo;
class NArgExpression;
class UnaryExpression;
class BinaryExpression;
class ConstExpression;
class VariableExpression;
class FunctionExpression;
class QueryParameterExpression;
class QuerySchemaParameterValueListIterator;
//class QuerySchemaParameterList;

//! A base class for all expressions
class PREDICATE_EXPORT Expression
{
public:
    typedef QList<Expression*> List;
    typedef QList<Expression*>::ConstIterator ListIterator;

    explicit Expression(int token);
    virtual ~Expression();

    //! @return a deep copy of this object.
//! @todo a nonpointer will be returned here when we move to implicit data sharing
    virtual Expression* copy() const = 0;

    int token() const {
        return m_token;
    }

    virtual Field::Type type() const;

    Expression* parent() const {
        return m_par;
    }

    virtual void setParent(Expression *p) {
        m_par = p;
    }

    virtual bool validate(ParseInfo& parseInfo);

    /*! @return string as a representation of this expression element by running recursive calls.
     @a param, if not 0, points to a list item containing value of a query parameter
     (used in QueryParameterExpr). */
    virtual EscapedString toString(QuerySchemaParameterValueListIterator* params = 0) const = 0;

    /*! Collects query parameters (messages and types) reculsively and saves them to params.
     The leaf nodes are objects of QueryParameterExpr class. */
    virtual void getQueryParameters(QuerySchemaParameterList& params) = 0;

    /*! @return single character if the token is < 256
     or token name, e.g. LESS_OR_EQUAL (for debugging). */
    inline QString tokenToDebugString() const {
        return tokenToDebugString(m_token);
    }

    //! @return debug string, used in QDebug operator<<(QDebug, const Expression&)
    virtual QString debugString() const;

    static QString tokenToDebugString(int token);

    /*! @return string for token, like "<=" or ">" */
    virtual QString tokenToString() const;

    int exprClass() const {
        return m_cl;
    }

    /*! Convenience type casts. */
    NArgExpression* toNArg();
    UnaryExpression* toUnary();
    BinaryExpression* toBinary();
    ConstExpression* toConst();
    VariableExpression* toVariable();
    FunctionExpression* toFunction();
    QueryParameterExpression* toQueryParameter();

protected:
    int m_cl; //!< class
    Expression *m_par; //!< parent expression
    int m_token;
};

//! A base class N-argument operation
class PREDICATE_EXPORT NArgExpression : public Expression
{
public:
    NArgExpression(int aClass, int token);
    NArgExpression(const NArgExpression& expr);
    virtual ~NArgExpression();
    //! @return a deep copy of this object.
    virtual NArgExpression* copy() const;
    void add(Expression *expr);
    void prepend(Expression *expr);
    Expression *arg(int n);
    int args();
    virtual QString debugString() const;
    virtual EscapedString toString(QuerySchemaParameterValueListIterator* params = 0) const;
    virtual void getQueryParameters(QuerySchemaParameterList& params);
    virtual bool validate(ParseInfo& parseInfo);
    Expression::List list;
};

//! An unary argument operation: + - NOT (or !) ~ "IS NULL" "IS NOT NULL"
class PREDICATE_EXPORT UnaryExpression : public Expression
{
public:
    UnaryExpression(int token, Expression *arg);
    UnaryExpression(const UnaryExpression& expr);
    virtual ~UnaryExpression();
    //! @return a deep copy of this object.
    virtual UnaryExpression* copy() const;
    virtual Field::Type type() const;
    virtual QString debugString() const;
    virtual EscapedString toString(QuerySchemaParameterValueListIterator* params = 0) const;
    virtual void getQueryParameters(QuerySchemaParameterList& params);
    Expression *arg() const {
        return m_arg;
    }
    virtual bool validate(ParseInfo& parseInfo);

    Expression *m_arg;
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
    BinaryExpression(int aClass, Expression *left_expr, int token, Expression *right_expr);
    BinaryExpression(const BinaryExpression& expr);
    virtual ~BinaryExpression();
    //! @return a deep copy of this object.
    virtual BinaryExpression* copy() const;
    virtual Field::Type type() const;
    virtual QString debugString() const;
    virtual EscapedString toString(QuerySchemaParameterValueListIterator* params = 0) const;
    virtual void getQueryParameters(QuerySchemaParameterList& params);
    Expression *left() const {
        return m_larg;
    }
    Expression *right() const {
        return m_rarg;
    }
    virtual bool validate(ParseInfo& parseInfo);
    virtual QString tokenToString() const;

    Expression *m_larg;
    Expression *m_rarg;
};

/*! String, integer, float constants also includes NULL value.
 token can be: IDENTIFIER, SQL_NULL, CHARACTER_STRING_LITERAL,
 INTEGER_CONST, REAL_CONST */
class PREDICATE_EXPORT ConstExpression : public Expression
{
public:
    ConstExpression(int token, const QVariant& val);
    ConstExpression(const ConstExpression& expr);
    virtual ~ConstExpression();
    //! @return a deep copy of this object.
    virtual ConstExpression* copy() const;
    virtual Field::Type type() const;
    virtual QString debugString() const;
    virtual EscapedString toString(QuerySchemaParameterValueListIterator* params = 0) const;
    virtual void getQueryParameters(QuerySchemaParameterList& params);
    virtual bool validate(ParseInfo& parseInfo);
    QVariant value;
};

//! Query parameter used to getting user input of constant values.
//! It contains a message that is displayed to the user.
class PREDICATE_EXPORT QueryParameterExpression : public ConstExpression
{
public:
    explicit QueryParameterExpression(const QString& message);
    QueryParameterExpression(const QueryParameterExpression& expr);
    virtual ~QueryParameterExpression();
    //! @return a deep copy of this object.
    virtual QueryParameterExpression* copy() const;
    virtual Field::Type type() const;
    /*! Sets expected type of the parameter. The default is String.
     This method is called from parent's expression validate().
     This depends on the type of the related expression.
     For instance: query "SELECT * FROM cars WHERE name=[enter name]",
     "[enter name]" has parameter of the same type as "name" field.
     "=" binary expression's validate() will be called for the left side
     of the expression and then the right side will have type set to String.
    */
    void setType(Field::Type type);
    virtual QString debugString() const;
    virtual EscapedString toString(QuerySchemaParameterValueListIterator* params = 0) const;
    virtual void getQueryParameters(QuerySchemaParameterList& params);
    virtual bool validate(ParseInfo& parseInfo);
protected:
    Field::Type m_type;
};

//! Variables like <i>fieldname</i> or <i>tablename</i>.<i>fieldname</i>
class PREDICATE_EXPORT VariableExpression : public Expression
{
public:
    explicit VariableExpression(const QString& _name);
    VariableExpression(const VariableExpression& expr);
    virtual ~VariableExpression();
    //! @return a deep copy of this object.
    virtual VariableExpression* copy() const;
    virtual Field::Type type() const;
    virtual QString debugString() const;
    virtual EscapedString toString(QuerySchemaParameterValueListIterator* params = 0) const;
    virtual void getQueryParameters(QuerySchemaParameterList& params);

    /*! Validation. Sets field, tablePositionForField
     and tableForQueryAsterisk members.
     See addColumn() in parse.y to see how it's used on column adding. */
    virtual bool validate(ParseInfo& parseInfo);

    /*! Verbatim name as returned by scanner. */
    QString name;

    /*! NULL by default. After successful validate() it will point to a field,
     if the variable is of a form "tablename.fieldname" or "fieldname",
     otherwise (eg. for asterisks) -still NULL.
     Only meaningful for column expressions within a query. */
    Field *field;

    /*! -1 by default. After successful validate() it will contain a position of a table
     within query that needs to be bound to the field.
     This value can be either be -1 if no binding is needed.
     This value is used in the Parser to call
      QuerySchema::addField(Field* field, int bindToTable);
     Only meaningful for column expressions within a query. */
    int tablePositionForField;

    /*! NULL by default. After successful validate() it will point to a table
     that is referenced by asterisk, i.e. "*.tablename".
     This is set to NULL if this variable is not an asterisk of that form. */
    TableSchema *tableForQueryAsterisk;
};

//! - aggregation functions like SUM, COUNT, MAX, ...
//! - builtin functions like CURRENT_TIME()
//! - user defined functions
class PREDICATE_EXPORT FunctionExpression : public Expression
{
public:
    explicit FunctionExpression(const QString& _name, NArgExpression* args_ = 0);
    FunctionExpression(const FunctionExpression& expr);
    virtual ~FunctionExpression();
    //! @return a deep copy of this obect.
    virtual FunctionExpression* copy() const;
    virtual Field::Type type() const;
    virtual QString debugString() const;
    virtual EscapedString toString(QuerySchemaParameterValueListIterator* params = 0) const;
    virtual void getQueryParameters(QuerySchemaParameterList& params);
    virtual bool validate(ParseInfo& parseInfo);

    static QList<QByteArray> builtInAggregates();
    static bool isBuiltInAggregate(const QByteArray& fname);

    QString name;
    NArgExpression* args;
};

} //namespace Predicate

//! Sends information about expression  @a expr to debug output @a dbg.
PREDICATE_EXPORT QDebug operator<<(QDebug dbg, const Predicate::Expression& expr);

#endif

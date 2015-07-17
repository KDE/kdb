/* This file is part of the KDE project
   Copyright (C) 2003-2015 Jarosław Staniek <staniek@kde.org>
   Copyright (C) 2014 Radoslaw Wicik <radoslaw@wicik.pl>

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

#ifndef KDB_EXPRESSION_H
#define KDB_EXPRESSION_H

#include "KDbGlobal.h"
#include "KDbField.h"
#include "KDbQuerySchemaParameter.h"
#include "KDbEscapedString.h"
#include "KDbExpressionData.h"
#include "KDbToken.h"

//! @return class name of class @a c
KDB_EXPORT QString expressionClassName(KDb::ExpressionClass c);

class KDbParseInfo;
class KDbNArgExpression;
class KDbUnaryExpression;
class KDbBinaryExpression;
class KDbConstExpression;
class KDbVariableExpression;
class KDbFunctionExpression;
class KDbQueryParameterExpression;
class KDbQuerySchemaParameterValueListIterator;

//! The KDbExpression class represents a base class for all expressions.
class KDB_EXPORT KDbExpression
{
    Q_DECLARE_TR_FUNCTIONS(KDbExpression)
public:
    /*! Constructs a null expression.
      @see KDbExpression::isNull() */
    KDbExpression();

    virtual ~KDbExpression();

    //! @return true if this expression is null.
    //! Equivalent of expressionClass() == KDb::UnknownExpression.
    //! @note Returns false for expressions of type KDbField::Null (SQL's NULL).
    bool isNull() const;

    //! Creates a deep (not shallow) copy of the KDbExpression.
    KDbExpression clone() const;

    /*!
    @return the token for this expression. Tokens are characters (e.g. '+', '-')
    or identifiers (e.g. SQL_NULL) of elements used by the KDbSQL parser.
    By default token is 0.
    */
    KDbToken token() const;

    /*! Sets token @a token for this expression. */
    void setToken(KDbToken token);

    /*!
    @return class identifier of this expression.
    Default expressionClass is KDb::UnknownExpression.
    */
    KDb::ExpressionClass expressionClass() const;

    /*! Sets expression class @a aClass for this expression. */
    void setExpressionClass(KDb::ExpressionClass aClass);

    /*! @return type of this expression, based on effect of its evaluation.
     Default type is KDbField::InvalidType. @see isValid() */
    KDbField::Type type() const;

    //! @return true if type of this object is not KDbField::InvalidType.
    /*! A covenience method. @see type() */
    bool isValid() const;

    //! @return true if type of this object belong to a group of text types.
    /*! A covenience method. @see type() */
    bool isTextType() const;

    //! \return true if type of this object belong to a group of integer types.
    /*! A covenience method. @see type() */
    bool isIntegerType() const;

    //! @return true if type of this object belong to a group of numeric types.
    /*! A covenience method. @see type() */
    bool isNumericType() const;

    //! @return true if type of this object belong to a group of floating-point numeric types.
    /*! A covenience method. @see type() */
    bool isFPNumericType() const;

    //! @return true if type of this object belong to a group of time, date and date/time types.
    /*! A covenience method. @see type() */
    bool isDateTimeType() const;

    /*! @return true if evaluation of this expression succeeded. */
    bool validate(KDbParseInfo *parseInfo);

    /*! @return string as a representation of this expression element
                by running recursive calls.
     @a param, if not 0, points to a list item containing value
     of a query parameter (used in QueryParameterExpr).
     The result may depend on the optional @a driver parameter.
     If @a driver is 0, representation for portable KDbSQL dialect is returned. */
    KDbEscapedString toString(const KDbDriver *driver, KDbQuerySchemaParameterValueListIterator* params = 0) const;

    /*! Collects query parameters (messages and types) recursively and saves them to @a params.
     The leaf nodes are objects of QueryParameterExpr class.
     @note @a params must not be 0. */
    void getQueryParameters(QList<KDbQuerySchemaParameter>* params);

    //! @return expression class for token @a token.
    //! @todo support more tokens
    static KDb::ExpressionClass classForToken(KDbToken token);

    //! Convenience type casts.
    KDbNArgExpression toNArg() const;
    KDbUnaryExpression toUnary() const;
    KDbBinaryExpression toBinary() const;
    KDbConstExpression toConst() const;
    KDbVariableExpression toVariable() const;
    KDbFunctionExpression toFunction() const;
    KDbQueryParameterExpression toQueryParameter() const;

    bool isNArg() const;
    bool isUnary() const;
    bool isBinary() const;
    bool isConst() const;
    bool isVariable() const;
    bool isFunction() const;
    bool isQueryParameter() const;

    QDebug debug(QDebug dbg, KDb::ExpressionCallStack* callStack) const;

    bool operator==(const KDbExpression& e) const;

    bool operator!=(const KDbExpression& e) const;

    /*! @return the parent expression. */
    KDbExpression parent() const;

protected:
    /*! @return the list of children expressions. */
    QList<ExplicitlySharedExpressionDataPointer> children() const;

    void appendChild(const KDbExpression& child);

    void prependChild(const KDbExpression& child);

    KDbExpression takeChild(int i);

    bool removeChild(const KDbExpression& child);

    void removeChild(int i);

    void insertChild(int i, const KDbExpression& child);

    //! Used for inserting placeholders, e.g. in KDbBinaryExpression::KDbBinaryExpression()
    void insertEmptyChild(int i);

    void appendChild(const ExplicitlySharedExpressionDataPointer& child);

    int indexOfChild(const KDbExpression& child, int from = 0) const;

    int lastIndexOfChild(const KDbExpression& child, int from = -1) const;

    bool checkBeforeInsert(const ExplicitlySharedExpressionDataPointer& child);

    //! Only for KDbBinaryExpression::setLeft() and KDbBinaryExpression::setRight()
    void setLeftOrRight(const KDbExpression& right, int index);

    explicit KDbExpression(KDbExpressionData* data);

    KDbExpression(KDbExpressionData* data, KDb::ExpressionClass aClass, KDbToken token);

    explicit KDbExpression(const ExplicitlySharedExpressionDataPointer &ptr);

    //! @internal
    ExplicitlySharedExpressionDataPointer d;

    friend class KDbNArgExpression;
    friend class KDbUnaryExpression;
    friend class KDbBinaryExpression;
    friend class KDbConstExpression;
    friend class KDbQueryParameterExpression;
    friend class KDbVariableExpression;
    friend class KDbFunctionExpression;
};

//! The KDbNArgExpression class represents a base class N-argument expression.
class KDB_EXPORT KDbNArgExpression : public KDbExpression
{
public:
    /*! Constructs a null N-argument expression.
      @see KDbExpression::isNull() */
    KDbNArgExpression();

    //! Constructs an N-argument expression of class @a aClass and token @a token.
    KDbNArgExpression(KDb::ExpressionClass aClass, KDbToken token);

    /*! Constructs a copy of other N-argument expression @a expr.
     Resulting object is not a deep copy but rather a reference to the object @a expr. */
    KDbNArgExpression(const KDbNArgExpression& expr);

    //! Destroys the expression.
    virtual ~KDbNArgExpression();

    //! Inserts expression argument @a expr at the end of this expression.
    void append(const KDbExpression& expr);

    //! Inserts expression argument @a expr at the beginning of this expression.
    void prepend(const KDbExpression& expr);

    /*! Inserts expression argument @a expr at index position @a i in this expression.
     If @a i is 0, the expression is prepended to the list of arguments.
     If @a i is argCount(), the value is appended to the list of arguments.
     @a i must be a valid index position in the list (i.e., 0 <= i < argCount()). */
    void insert(int i, const KDbExpression& expr);

    //! Replaces expression argument at index @a i with expression @a expr.
    //! @a i must be a valid index position in the list (i.e., 0 <= i < argCount()). */
    void replace(int i, const KDbExpression& expr);

    /*! Removes the expression argument @a expr and returns true on success;
        otherwise returns false. */
    bool remove(const KDbExpression& expr);

    /*! Removes the expression at index position @a i.
     @a i must be a valid index position in the list (i.e., 0 <= i < argCount()). */
    void removeAt(int i);

    /*! Removes the expression at index position @a i and returns it.
      @a i must be a valid index position in the list (i.e., 0 <= i < argCount()).
      If you don't use the return value, removeAt() is more efficient. */
    KDbExpression takeAt(int i);

    /*! @return the index position of the first occurrence of expression argument
      @a expr in this expression, searching forward from index position @a from.
      @return -1 if no argument matched.
      @see lastIndexOf() */
    int indexOf(const KDbExpression& expr, int from = 0) const;

    /*! @return the index position of the last occurrence of expression argument
      @a expr in this expression, searching backward from index position @a from.
      If from is -1 (the default), the search starts at the last item.
      Returns -1 if no argument matched.
      @see indexOf() */
    int lastIndexOf(const KDbExpression& expr, int from = -1) const;

    //! @return expression index @a i in the list of arguments.
    //! If the index @a i is out of bounds, the function returns null expression.
    KDbExpression arg(int i) const;

    //! @return the number of expression arguments in this expression.
    int argCount() const;

    //! @return true if the expression contains no arguments; otherwise returns false.
    bool isEmpty() const;

    //! @return true if any argument is invalid (!KDbExpression::isValid()).
    bool containsInvalidArgument() const;

    //! @return true if any argument is NULL (type KDbField::Null).
    bool containsNullArgument() const;

protected:
    explicit KDbNArgExpression(KDbExpressionData* data);

    explicit KDbNArgExpression(const ExplicitlySharedExpressionDataPointer &ptr);

    friend class KDbExpression;
    friend class KDbFunctionExpression;
};

//! The KDbUnaryExpression class represents unary expression (with a single argument).
/*! operation: + - NOT (or !) ~ "IS NULL" "IS NOT NULL"
  */
class KDB_EXPORT KDbUnaryExpression : public KDbExpression
{
public:
    /*! Constructs a null unary expression.
      @see KDbExpression::isNull() */
    KDbUnaryExpression();

    //! Constructs unary expression with token @a token and argument @a arg.
    KDbUnaryExpression(KDbToken token, const KDbExpression& arg);

    /*! Constructs a copy of other unary expression @a expr.
     Resulting object is not a deep copy but rather a reference to the object @a expr. */
    KDbUnaryExpression(const KDbUnaryExpression& expr);

    virtual ~KDbUnaryExpression();

    //! @return expression that is argument for this unary expression
    KDbExpression arg() const;

    //! Sets expression argument @a expr for this unary expression.
    void setArg(const KDbExpression &arg);

protected:
    explicit KDbUnaryExpression(KDbExpressionData* data);

    explicit KDbUnaryExpression(const ExplicitlySharedExpressionDataPointer &ptr);

    friend class KDbExpression;
};

//! The KDbBinaryExpression class represents binary operation.
/*
 - arithmetic operations: + - / * % << >> & | ||
 - relational operations: = (or ==) < > <= >= <> (or !=) LIKE 'NOT LIKE' IN 'SIMILAR TO'
                          'NOT SIMILAR TO'
 - logical operations: OR (or ||) AND (or &&) XOR
 - SpecialBinary "pseudo operators":
    * e.g. "f1 f2" : token == 0
    * e.g. "f1 AS f2" : token == AS
*/
class KDB_EXPORT KDbBinaryExpression : public KDbExpression
{
public:
    /*! Constructs a null binary expression.
      @see KDbExpression::isNull() */
    KDbBinaryExpression();

    /*! Constructs binary expression with left expression @a leftExpr,
     token @a token, and right expression @a rightExpr. */
    KDbBinaryExpression(const KDbExpression& leftExpr, KDbToken token, const KDbExpression& rightExpr);

    /*! Constructs a copy of other unary expression @a expr.
     Resulting object is not a deep copy but rather a reference to the object @a expr. */
    KDbBinaryExpression(const KDbBinaryExpression& expr);

    virtual ~KDbBinaryExpression();

    KDbExpression left() const;

    void setLeft(const KDbExpression& leftExpr);

    KDbExpression right() const;

    void setRight(const KDbExpression& rightExpr);

protected:
    explicit KDbBinaryExpression(KDbExpressionData* data);

    explicit KDbBinaryExpression(const ExplicitlySharedExpressionDataPointer &ptr);

    friend class KDbExpression;
};


//! The KDbConstExpression class represents const expression.
/*! Types are string, integer, float constants. Also includes NULL value.
 Token can be: IDENTIFIER, SQL_NULL, SQL_TRUE, SQL_FALSE, CHARACTER_STRING_LITERAL,
 INTEGER_CONST, REAL_CONST, DATE_CONST, DATETIME_CONST, TIME_CONST.

 @note For REAL_CONST accepted values can be of type qreal, double and QPoint.
       In the case of QPoint, integer value (with a sign) is stored in QPoint::x
       and the fraction part (that should be always positive) is stored in QPoint::y.
       This gives 31 bits for the integer part (10 decimal digits) and 31 bits for the part
       (10 decimal digits).
*/
class KDB_EXPORT KDbConstExpression : public KDbExpression
{
public:
    /*! Constructs a null const expression.
      @see KDbExpression::isNull() */
    KDbConstExpression();

    /*! Constructs const expression token @a token and value @a value. */
    KDbConstExpression(KDbToken token, const QVariant& value);

    /*! Constructs a copy of other const expression @a expr.
     Resulting object is not a deep copy but rather a reference to the object @a expr. */
    KDbConstExpression(const KDbConstExpression& expr);

    virtual ~KDbConstExpression();

    QVariant value() const;

    void setValue(const QVariant& value);

protected:
    //! Internal, used by KDbQueryParameterExpression(const QString& message).
    KDbConstExpression(KDbExpressionData* data, KDb::ExpressionClass aClass, KDbToken token);
    explicit KDbConstExpression(KDbExpressionData* data);
    explicit KDbConstExpression(const ExplicitlySharedExpressionDataPointer &ptr);

    friend class KDbExpression;
};

//! The KDbQueryParameterExpression class represents query parameter expression.
/*! Query parameter is used to getting user input of constant values.
 It contains a message that is displayed to the user.
*/
class KDB_EXPORT KDbQueryParameterExpression : public KDbConstExpression
{
public:
    /*! Constructs a null query parameter expression.
      @see KDbExpression::isNull() */
    KDbQueryParameterExpression();

    /*! Constructs query parameter expression with message @a message. */
    explicit KDbQueryParameterExpression(const QString& message);

    /*! Constructs a copy of other query parameter expression @a expr.
     Resulting object is not a deep copy but rather a reference to the object @a expr. */
    KDbQueryParameterExpression(const KDbQueryParameterExpression& expr);

    virtual ~KDbQueryParameterExpression();

    /*! Sets expected type of the parameter. The default is String.
     This method is called from parent's expression validate().
     This depends on the type of the related expression.
     For instance: query "SELECT * FROM cars WHERE name=[enter name]",
     "[enter name]" has parameter of the same type as "name" field.
     "=" binary expression's validate() will be called for the left side
     of the expression and then the right side will have type set to String. */
    void setType(KDbField::Type type);

protected:
    explicit KDbQueryParameterExpression(KDbExpressionData* data);
    explicit KDbQueryParameterExpression(const ExplicitlySharedExpressionDataPointer &ptr);

    friend class KDbExpression;
};

//! The KDbVariableExpression class represents variables such as <i>fieldname</i> or <i>tablename</i>.<i>fieldname</i>
class KDB_EXPORT KDbVariableExpression : public KDbExpression
{
public:
    /*! Constructs a null variable expression.
      @see KDbExpression::isNull() */
    KDbVariableExpression();

    /*! Constructs variable expression with name @a name. */
    explicit KDbVariableExpression(const QString& name);

    /*! Constructs a copy of other variable expression @a expr.
     Resulting object is not a deep copy but rather a reference to the object @a expr. */
    KDbVariableExpression(const KDbVariableExpression& expr);

    virtual ~KDbVariableExpression();

    /*! Verbatim name as returned by scanner. */
    QString name() const;

    /*! 0 by default. After successful validate() it returns a field,
     if the variable is of a form "tablename.fieldname" or "fieldname",
     otherwise (eg. for asterisks) still 0.
     Only meaningful for column expressions within a query. */
    KDbField *field() const;

    /*! -1 by default. After successful validate() it returns a position of a table
     within query that needs to be bound to the field.
     This value can be either be -1 if no binding is needed.
     This value is used in the Parser to call
      KDbQuerySchema::addField(KDbField* field, int bindToTable);
     Only meaningful for column expressions within a query. */
    int tablePositionForField() const;

    /*! 0 by default. After successful validate() it returns table that
     is referenced by asterisk, i.e. "*.tablename".
     It is 0 if this variable is not an asterisk of that form. */
    KDbTableSchema *tableForQueryAsterisk() const;

protected:
    explicit KDbVariableExpression(KDbExpressionData* data);
    explicit KDbVariableExpression(const ExplicitlySharedExpressionDataPointer &ptr);

    friend class KDbExpression;
};

//! The KDbFunctionExpression class represents expression that use functional notation F(x, ...)
/*! The functions list include:
 - aggregation functions like SUM, COUNT, MAX, ...
 - builtin functions like CURRENT_TIME()
 - user defined functions */
class KDB_EXPORT KDbFunctionExpression : public KDbExpression
{
public:
    /*! Constructs a null function expression.
      @see KDbExpression::isNull() */
    KDbFunctionExpression();

    /*! Constructs function expression with name @a name, without arguments. */
    explicit KDbFunctionExpression(const QString& name);

    /*! Constructs function expression with name @a name and arguments @a arguments. */
    KDbFunctionExpression(const QString& name, const KDbNArgExpression &arguments);

    /*! Constructs a copy of other function expression @a expr.
     Resulting object is not a deep copy but rather a reference to the object @a expr. */
    KDbFunctionExpression(const KDbFunctionExpression& expr);

    virtual ~KDbFunctionExpression();

    //! @return name of the function.
    QString name() const;

    //! Sets name of the function to @a name.
    void setName(const QString &name);

    //! @return list of arguments of the function.
    KDbNArgExpression arguments();

    //! Sets the list of arguments to @a arguments.
    void setArguments(const KDbNArgExpression &arguments);

    /*! Constructs function expression with name @a name and arguments @a arguments. */
    static QStringList builtInAggregates();

    static bool isBuiltInAggregate(const QString& function);

protected:
    explicit KDbFunctionExpression(KDbExpressionData* data);
    explicit KDbFunctionExpression(const ExplicitlySharedExpressionDataPointer &ptr);

    friend class KDbExpression;
};

//! Sends information about expression  @a expr to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbExpression& expr);

#endif

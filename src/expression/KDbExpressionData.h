/* This file is part of the KDE project
   Copyright (C) 2003-2015 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_EXPRESSION_P_H
#define KDB_EXPRESSION_P_H

#include "config-kdb.h"
#include "KDb.h"
#include "KDbQuerySchema.h"

class KDbExpressionData;
class KDbUnaryExpressionData;
class KDbConstExpressionData;
class KDbQueryParameterExpressionData;
class KDbParseInfo;

namespace KDb
{
typedef QList<const KDbExpressionData*> ExpressionCallStack;

//! Classes of expressions
enum ExpressionClass {
    UnknownExpression,
    UnaryExpression,
    ArithmeticExpression,
    LogicalExpression,
    RelationalExpression,
    SpecialBinaryExpression,
    ConstExpression,
    VariableExpression,
    FunctionExpression,
    AggregationExpression,
    FieldListExpression,
    TableListExpression,
    ArgumentListExpression,
    QueryParameterExpression
};
}

typedef QExplicitlySharedDataPointer<KDbExpressionData> ExplicitlySharedExpressionDataPointer;

//! Internal data class used to implement implicitly shared class KDbExpression.
//! Provides thread-safe reference counting.
class KDB_TESTING_EXPORT KDbExpressionData : public QSharedData
{
public:
    KDbExpressionData();

    /*Data(const Data& other)
    : QSharedData(other)
    , token(other.token)
    , expressionClass(other.expressionClass)
    , parent(other.parent)
    , children(other.children)
    {
        kdbDebug() << "KDbExpressionData" << ref;
    }*/

    virtual ~KDbExpressionData();

    //! @see KDbExpression::token()
    KDbToken token;
    //! @see KDbExpression::expressionClass()
    KDb::ExpressionClass expressionClass;
    ExplicitlySharedExpressionDataPointer parent;
    QList<ExplicitlySharedExpressionDataPointer> children;
    KDbField::Type type() const; //!< @return type of this expression;
    bool isValid() const;
    bool isTextType() const;
    bool isIntegerType() const;
    bool isNumericType() const;
    bool isFPNumericType() const;
    bool isDateTimeType() const;
    KDbEscapedString toString(const KDbDriver *driver,
                              KDbQuerySchemaParameterValueListIterator* params = 0) const;
    virtual void getQueryParameters(QList<KDbQuerySchemaParameter>& params);
    bool validate(KDbParseInfo *parseInfo);
    virtual KDbExpressionData* clone();

    template <typename T>
    const T* convertConst() const { return dynamic_cast<const T*>(this); }

    template <typename T>
    T* convert() { return dynamic_cast<T*>(this); }

    //! Sends information about this expression  to debug output @a dbg.
    QDebug debug(QDebug dbg, KDb::ExpressionCallStack* callStack) const;

    KDbField::Type type(KDb::ExpressionCallStack* callStack) const;

    KDbEscapedString toString(const KDbDriver *driver,
                              KDbQuerySchemaParameterValueListIterator* params,
                              KDb::ExpressionCallStack* callStack) const;

    bool validate(KDbParseInfo *parseInfo, KDb::ExpressionCallStack* callStack);

protected:
    //! Sends information about this expression  to debug output @a dbg (internal).
    virtual void debugInternal(QDebug dbg, KDb::ExpressionCallStack* callStack) const;

    virtual KDbField::Type typeInternal(KDb::ExpressionCallStack* callStack) const;

    virtual KDbEscapedString toStringInternal(const KDbDriver *driver,
                                              KDbQuerySchemaParameterValueListIterator* params,
                                              KDb::ExpressionCallStack* callStack) const;

    virtual bool validateInternal(KDbParseInfo *parseInfo, KDb::ExpressionCallStack* callStack);

    bool addToCallStack(QDebug *dbg, QList<const KDbExpressionData*>* callStack) const;
};

//! Internal data class used to implement implicitly shared class KDbNArgExpression.
//! Provides thread-safe reference counting.
class KDbNArgExpressionData : public KDbExpressionData
{
public:
    KDbNArgExpressionData();
    virtual ~KDbNArgExpressionData();

    virtual void getQueryParameters(QList<KDbQuerySchemaParameter>& params);
    virtual KDbNArgExpressionData* clone();
    bool containsInvalidArgument() const;
    bool containsNullArgument() const;

protected:
    //! Sends information about this expression  to debug output @a dbg (internal).
    virtual void debugInternal(QDebug dbg, KDb::ExpressionCallStack* callStack) const;

    virtual KDbField::Type typeInternal(KDb::ExpressionCallStack* callStack) const;

    virtual KDbEscapedString toStringInternal(const KDbDriver *driver,
                                              KDbQuerySchemaParameterValueListIterator* params,
                                              KDb::ExpressionCallStack* callStack) const;

    virtual bool validateInternal(KDbParseInfo *parseInfo, KDb::ExpressionCallStack* callStack);
};

//! Internal data class used to implement implicitly shared class KDbUnaryExpression.
//! Provides thread-safe reference counting.
class KDbUnaryExpressionData : public KDbExpressionData
{
public:
    KDbUnaryExpressionData();
    virtual ~KDbUnaryExpressionData();

    virtual void getQueryParameters(QList<KDbQuerySchemaParameter>& params);
    virtual KDbUnaryExpressionData* clone();
    inline ExplicitlySharedExpressionDataPointer arg() const {
        return children.isEmpty() ? ExplicitlySharedExpressionDataPointer() : children.first();
    }

protected:
    //! Sends information about this expression  to debug output @a dbg (internal).
    virtual void debugInternal(QDebug dbg, KDb::ExpressionCallStack* callStack) const;

    virtual KDbEscapedString toStringInternal(const KDbDriver *driver,
                                              KDbQuerySchemaParameterValueListIterator* params,
                                              KDb::ExpressionCallStack* callStack) const;

    virtual KDbField::Type typeInternal(KDb::ExpressionCallStack* callStack) const;

    virtual bool validateInternal(KDbParseInfo *parseInfo, KDb::ExpressionCallStack* callStack);
};

//! Internal data class used to implement implicitly shared class KDbBinaryExpression.
//! Provides thread-safe reference counting.
class KDbBinaryExpressionData : public KDbExpressionData
{
public:
    KDbBinaryExpressionData();
    virtual ~KDbBinaryExpressionData();

    virtual void getQueryParameters(QList<KDbQuerySchemaParameter>& params);
    virtual KDbBinaryExpressionData* clone();
    ExplicitlySharedExpressionDataPointer left() const;
    ExplicitlySharedExpressionDataPointer right() const;

protected:
    void setLeft(const KDbExpressionData& left);

    //! Sends information about this expression  to debug output @a dbg (internal).
    virtual void debugInternal(QDebug dbg, KDb::ExpressionCallStack* callStack) const;

    virtual KDbEscapedString toStringInternal(const KDbDriver *driver,
                                              KDbQuerySchemaParameterValueListIterator* params,
                                              KDb::ExpressionCallStack* callStack) const;

    virtual KDbField::Type typeInternal(KDb::ExpressionCallStack* callStack) const;

    virtual bool validateInternal(KDbParseInfo *parseInfo, KDb::ExpressionCallStack* callStack);
};

//! Internal data class used to implement implicitly shared class KDbConstExpression.
//! Provides thread-safe reference counting.
class KDbConstExpressionData : public KDbExpressionData
{
public:
    explicit KDbConstExpressionData(const QVariant& aValue = QVariant());
    virtual ~KDbConstExpressionData();

    QVariant value;
    virtual void getQueryParameters(QList<KDbQuerySchemaParameter>& params);
    virtual KDbConstExpressionData* clone();

protected:
    //! Sends information about this expression  to debug output @a dbg (internal).
    virtual void debugInternal(QDebug dbg, KDb::ExpressionCallStack* callStack) const;

    virtual KDbEscapedString toStringInternal(const KDbDriver *driver,
                                              KDbQuerySchemaParameterValueListIterator* params,
                                              KDb::ExpressionCallStack* callStack) const;

    virtual KDbField::Type typeInternal(KDb::ExpressionCallStack* callStack) const;

    virtual bool validateInternal(KDbParseInfo *parseInfo, KDb::ExpressionCallStack* callStack);
};

//! Internal data class used to implement implicitly shared class KDbQueryParameterExpression.
//! Provides thread-safe reference counting.
class KDbQueryParameterExpressionData : public KDbConstExpressionData
{
public:
    KDbQueryParameterExpressionData();
    KDbQueryParameterExpressionData(KDbField::Type type, const QVariant& value);
    virtual ~KDbQueryParameterExpressionData();

    KDbField::Type m_type;
    virtual void getQueryParameters(QList<KDbQuerySchemaParameter>& params);
    virtual KDbQueryParameterExpressionData* clone();

protected:
    //! Sends information about this expression  to debug output @a dbg (internal).
    virtual void debugInternal(QDebug dbg, KDb::ExpressionCallStack* callStack) const;

    virtual KDbEscapedString toStringInternal(const KDbDriver *driver,
                                              KDbQuerySchemaParameterValueListIterator* params,
                                              KDb::ExpressionCallStack* callStack) const;

    virtual KDbField::Type typeInternal(KDb::ExpressionCallStack* callStack) const;

    virtual bool validateInternal(KDbParseInfo *parseInfo, KDb::ExpressionCallStack* callStack);
};

//! Internal data class used to implement implicitly shared class KDbVariableExpression.
//! Provides thread-safe reference counting.
class KDbVariableExpressionData : public KDbExpressionData
{
public:
    KDbVariableExpressionData();
    explicit KDbVariableExpressionData(const QString& aName);
    virtual ~KDbVariableExpressionData();

    /*! Verbatim name as returned by scanner. */
    QString name;

    /*! 0 by default. After successful validate() it will point to a field,
     if the variable is of a form "tablename.fieldname" or "fieldname",
     otherwise (eg. for asterisks) still 0.
     Only meaningful for column expressions within a query. */
    KDbField *field;

    /*! -1 by default. After successful validate() it will contain a position of a table
     within query that needs to be bound to the field.
     This value can be either be -1 if no binding is needed.
     This value is used in the Parser to call
      KDbQuerySchema::addField(KDbField* field, int bindToTable);
     Only meaningful for column expressions within a query. */
    int tablePositionForField;

    /*! 0 by default. After successful validate() it will point to a table
     that is referenced by asterisk, i.e. "*.tablename".
     This is set to NULL if this variable is not an asterisk of that form. */
    KDbTableSchema *tableForQueryAsterisk;

    virtual void getQueryParameters(QList<KDbQuerySchemaParameter>& params);
    virtual KDbVariableExpressionData* clone();

protected:
    //! Sends information about this expression  to debug output @a dbg (internal).
    virtual void debugInternal(QDebug dbg, KDb::ExpressionCallStack* callStack) const;

    virtual KDbEscapedString toStringInternal(const KDbDriver *driver,
                                              KDbQuerySchemaParameterValueListIterator* params,
                                              KDb::ExpressionCallStack* callStack) const;

    virtual KDbField::Type typeInternal(KDb::ExpressionCallStack* callStack) const;

    /*! Validation. Sets field, tablePositionForField
     and tableForQueryAsterisk members.
     See addColumn() in parse.y to see how it's used on column adding. */
    virtual bool validateInternal(KDbParseInfo *parseInfo, KDb::ExpressionCallStack* callStack);
};

//! Internal data class used to implement implicitly shared class KDbFunctionExpression.
//! Provides thread-safe reference counting.
class KDbFunctionExpressionData : public KDbExpressionData
{
public:
    KDbFunctionExpressionData();
    explicit KDbFunctionExpressionData(const QString& aName,
                                       ExplicitlySharedExpressionDataPointer arguments = ExplicitlySharedExpressionDataPointer());
    virtual ~KDbFunctionExpressionData();

    QString name;
    ExplicitlySharedExpressionDataPointer args;

    virtual void getQueryParameters(QList<KDbQuerySchemaParameter>& params);
    virtual KDbFunctionExpressionData* clone();

    void setArguments(ExplicitlySharedExpressionDataPointer arguments);

protected:
    //! Sends information about this expression  to debug output @a dbg (internal).
    virtual void debugInternal(QDebug dbg, KDb::ExpressionCallStack* callStack) const;

    virtual KDbEscapedString toStringInternal(const KDbDriver *driver,
                                              KDbQuerySchemaParameterValueListIterator* params,
                                              KDb::ExpressionCallStack* callStack) const;

    virtual KDbField::Type typeInternal(KDb::ExpressionCallStack* callStack) const;

    /*! Validation. Sets field, tablePositionForField
     and tableForQueryAsterisk members.
     See addColumn() in parse.y to see how it's used on column adding. */
    virtual bool validateInternal(KDbParseInfo *parseInfo, KDb::ExpressionCallStack* callStack);
};

QDebug operator<<(QDebug dbg, const KDbExpressionData& expr);

#endif

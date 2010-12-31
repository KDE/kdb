/* This file is part of the KDE project
   Copyright (C) 2003-2010 Jarosław Staniek <staniek@kde.org>

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

#include "Expression.h"
#include "Utils.h"
#include "QuerySchema.h"
#include "parser/SqlParser.h"
#include "parser/Parser_p.h"
#include "tools/Static.h"

#include <ctype.h>

PREDICATE_EXPORT QString Predicate::expressionClassName(ExpressionClass c)
{
    if (c == UnaryExpressionClass)
        return "Unary";
    else if (c == ArithmeticExpressionClass)
        return "Arithm";
    else if (c == LogicalExpressionClass)
        return "Logical";
    else if (c == RelationalExpressionClass)
        return "Relational";
    else if (c == SpecialBinaryExpressionClass)
        return "SpecialBinary";
    else if (c == ConstExpressionClass)
        return "Const";
    else if (c == VariableExpressionClass)
        return "Variable";
    else if (c == FunctionExpressionClass)
        return "Function";
    else if (c == AggregationExpressionClass)
        return "Aggregation";
    else if (c == TableListExpressionClass)
        return "TableList";
    else if (c == QueryParameterExpressionClass)
        return "QueryParameter";

    return "Unknown";
}

using namespace Predicate;

//=========================================

Expression::Expression(ExpressionClass aClass, int token)
        : m_cl(aClass)
        , m_par(0)
        , m_token(token)
{
}

Expression::~Expression()
{
}

Field::Type Expression::type() const
{
    return Field::InvalidType;
}

QString Expression::debugString() const
{
    return QString("Expression(%1,type=%1)")
                   .arg(m_token).arg(Driver::defaultSQLTypeName(type()));
}

//! Sends information about expression  @a expr to debug output @a dbg.
PREDICATE_EXPORT QDebug operator<<(QDebug dbg, const Expression& expr)
{
    dbg.nospace() << expr.debugString();
    return dbg.space();
}

bool Expression::validate(ParseInfo& /*parseInfo*/)
{
    return true;
}

extern const char* tname(int offset);
#define safe_tname(token) ((token>=255 && token<=__LAST_TOKEN) ? tname(token-255) : "")

QString Expression::tokenToDebugString(int token)
{
    if (token < 254) {
        if (isprint(token))
            return QString(QChar(uchar(token)));
        else
            return QString::number(token);
    }
    return QString(safe_tname(token));
}

QString Expression::tokenToString() const
{
    if (m_token < 255 && isprint(m_token))
        return tokenToDebugString();
    return QString();
}

NArgExpression* Expression::toNArg()
{
    return dynamic_cast<NArgExpression*>(this);
}
UnaryExpression* Expression::toUnary()
{
    return dynamic_cast<UnaryExpression*>(this);
}
BinaryExpression* Expression::toBinary()
{
    return dynamic_cast<BinaryExpression*>(this);
}
ConstExpression* Expression::toConst()
{
    return dynamic_cast<ConstExpression*>(this);
}
VariableExpression* Expression::toVariable()
{
    return dynamic_cast<VariableExpression*>(this);
}
FunctionExpression* Expression::toFunction()
{
    return dynamic_cast<FunctionExpression*>(this);
}
QueryParameterExpression* Expression::toQueryParameter()
{
    return dynamic_cast<QueryParameterExpression*>(this);
}

//=========================================

NArgExpression::NArgExpression(ExpressionClass aClass, int token)
        : Expression(aClass, token)
{
}

NArgExpression::NArgExpression(const NArgExpression& expr)
        : Expression(expr)
{
    foreach(Expression* e, expr.list) {
        add(e->copy());
    }
}

NArgExpression::~NArgExpression()
{
}

NArgExpression* NArgExpression::copy() const
{
    return new NArgExpression(*this);
}

QString NArgExpression::debugString() const
{
    QString s = QString("NArgExpression(")
                + "class=" + expressionClassName(expressionClass());
    foreach(Expression *expr, list) {
        s += ", ";
        s += expr->debugString();
    }
    s += ")";
    return s;
}

EscapedString NArgExpression::toString(QuerySchemaParameterValueListIterator* params) const
{
    EscapedString s;
    s.reserve(256);
    foreach(Expression* e, list) {
        if (!s.isEmpty())
            s += ", ";
        s += e->toString(params);
    }
    return s;
}

void NArgExpression::getQueryParameters(QuerySchemaParameterList& params)
{
    foreach(Expression *e, list) {
        e->getQueryParameters(params);
    }
}

Expression* NArgExpression::arg(int nr)
{
    return list.at(nr);
}

void NArgExpression::add(Expression *expr)
{
    list.append(expr);
    expr->setParent(this);
}

void NArgExpression::prepend(Expression *expr)
{
    list.prepend(expr);
    expr->setParent(this);
}

int NArgExpression::args()
{
    return list.count();
}

bool NArgExpression::validate(ParseInfo& parseInfo)
{
    if (!Expression::validate(parseInfo))
        return false;

    foreach(Expression *e, list) {
        if (!e->validate(parseInfo))
            return false;
    }
    return true;
}

//=========================================
UnaryExpression::UnaryExpression(int token, Expression *arg)
        : Expression(UnaryExpressionClass, token)
        , m_arg(arg)
{
    if (m_arg)
        m_arg->setParent(this);
}

UnaryExpression::UnaryExpression(const UnaryExpression& expr)
        : Expression(expr)
        , m_arg(expr.m_arg ? expr.m_arg->copy() : 0)
{
    if (m_arg)
        m_arg->setParent(this);
}

UnaryExpression::~UnaryExpression()
{
    delete m_arg;
}

UnaryExpression* UnaryExpression::copy() const
{
    return new UnaryExpression(*this);
}

QString UnaryExpression::debugString() const
{
    return "UnaryExpression('"
           + tokenToDebugString() + "', "
           + (m_arg ? m_arg->debugString() : QString("<NONE>"))
           + QString(",type=%1)").arg(Driver::defaultSQLTypeName(type()));
}

EscapedString UnaryExpression::toString(QuerySchemaParameterValueListIterator* params) const
{
    if (token() == '(') //parentheses (special case)
        return "(" + (m_arg ? m_arg->toString(params) : EscapedString("<NULL>")) + ")";
    if (token() < 255 && isprint(token()))
        return tokenToDebugString() + (m_arg ? m_arg->toString(params) : EscapedString("<NULL>"));
    if (token() == NOT)
        return "NOT " + (m_arg ? m_arg->toString(params) : EscapedString("<NULL>"));
    if (token() == SQL_IS_NULL)
        return (m_arg ? m_arg->toString(params) : EscapedString("<NULL>")) + " IS NULL";
    if (token() == SQL_IS_NOT_NULL)
        return (m_arg ? m_arg->toString(params) : EscapedString("<NULL>")) + " IS NOT NULL";
    return EscapedString("{INVALID_OPERATOR#%1} ").arg(token()) + (m_arg ? m_arg->toString(params) : EscapedString("<NULL>"));
}

void UnaryExpression::getQueryParameters(QuerySchemaParameterList& params)
{
    if (m_arg)
        m_arg->getQueryParameters(params);
}

Field::Type UnaryExpression::type() const
{
    //NULL IS NOT NULL : BOOLEAN
    //NULL IS NULL : BOOLEAN
    switch (token()) {
    case SQL_IS_NULL:
    case SQL_IS_NOT_NULL:
        return Field::Boolean;
    }
    const Field::Type t = m_arg->type();
    if (t == Field::Null)
        return Field::Null;
    if (token() == NOT)
        return Field::Boolean;

    return t;
}

bool UnaryExpression::validate(ParseInfo& parseInfo)
{
    if (!Expression::validate(parseInfo))
        return false;

    if (!m_arg->validate(parseInfo))
        return false;

//! @todo compare types... e.g. NOT applied to Text makes no sense...

    // update type
    if (m_arg->toQueryParameter()) {
        m_arg->toQueryParameter()->setType(type());
    }

    return true;
#if 0
    Expression *n = l.at(0);

    n->check();
    /*typ wyniku:
        const bool dla "NOT <bool>" (negacja)
        int dla "# <str>" (dlugosc stringu)
        int dla "+/- <int>"
        */
    if (is(NOT) && n->nodeTypeIs(TYP_BOOL)) {
        node_type = new NConstType(TYP_BOOL);
    } else if (is('#') && n->nodeTypeIs(TYP_STR)) {
        node_type = new NConstType(TYP_INT);
    } else if ((is('+') || is('-')) && n->nodeTypeIs(TYP_INT)) {
        node_type = new NConstType(TYP_INT);
    } else {
        ERR("Niepoprawny argument typu '%s' dla operatora '%s'",
            n->nodeTypeName(), is(NOT) ? QString("not") : QChar(typ()));
    }
#endif
}

//=========================================
BinaryExpression::BinaryExpression(ExpressionClass aClass, Expression *left_expr, int token, Expression *right_expr)
        : Expression(aClass, token)
        , m_larg(left_expr)
        , m_rarg(right_expr)
{
    if (m_larg)
        m_larg->setParent(this);
    if (m_rarg)
        m_rarg->setParent(this);
}

BinaryExpression::BinaryExpression(const BinaryExpression& expr)
        : Expression(expr)
        , m_larg(expr.m_larg ? expr.m_larg->copy() : 0)
        , m_rarg(expr.m_rarg ? expr.m_rarg->copy() : 0)
{
}

BinaryExpression::~BinaryExpression()
{
    delete m_larg;
    delete m_rarg;
}

BinaryExpression* BinaryExpression::copy() const
{
    return new BinaryExpression(*this);
}

bool BinaryExpression::validate(ParseInfo& parseInfo)
{
    if (!Expression::validate(parseInfo))
        return false;

    if (!m_larg->validate(parseInfo))
        return false;
    if (!m_rarg->validate(parseInfo))
        return false;

//! @todo compare types..., BITWISE_SHIFT_RIGHT requires integers, etc...

    //update type for query parameters
    QueryParameterExpression * queryParameter = m_larg->toQueryParameter();
    if (queryParameter)
        queryParameter->setType(m_rarg->type());
    queryParameter = m_rarg->toQueryParameter();
    if (queryParameter)
        queryParameter->setType(m_larg->type());

    return true;
}

Field::Type BinaryExpression::type() const
{
    const Field::Type lt = m_larg->type(), rt = m_rarg->type();
    if (lt == Field::InvalidType || rt == Field::InvalidType)
        return Field::InvalidType;
    if (lt == Field::Null || rt == Field::Null) {
        if (token() != OR) //note that NULL OR something   != NULL
            return Field::Null;
    }

    switch (token()) {
    case BITWISE_SHIFT_RIGHT:
    case BITWISE_SHIFT_LEFT:
    case CONCATENATION:
        return lt;
    }

    const bool ltInt = Field::isIntegerType(lt);
    const bool rtInt = Field::isIntegerType(rt);
    if (ltInt && rtInt)
        return Predicate::maximumForIntegerTypes(lt, rt);

    if (Field::isFPNumericType(lt) && (rtInt || lt == rt))
        return lt;
    if (Field::isFPNumericType(rt) && (ltInt || lt == rt))
        return rt;

    return Field::Boolean;
}

QString BinaryExpression::debugString() const
{
    return QString("BinaryExpression(")
           + "class=" + expressionClassName(expressionClass())
           + "," + (m_larg ? m_larg->debugString() : QString("<NONE>"))
           + ",'" + tokenToDebugString() + "',"
           + (m_rarg ? m_rarg->debugString() : QString("<NONE>"))
           + QString(",type=%1)").arg(Driver::defaultSQLTypeName(type()));
}

QString BinaryExpression::tokenToString() const
{
    if (token() < 255 && isprint(token()))
        return tokenToDebugString();
    // other arithmetic operations: << >>
    switch (token()) {
    case BITWISE_SHIFT_RIGHT: return ">>";
    case BITWISE_SHIFT_LEFT: return "<<";
        // other relational operations: <= >= <> (or !=) LIKE IN
    case NOT_EQUAL: return "<>";
    case NOT_EQUAL2: return "!=";
    case LESS_OR_EQUAL: return "<=";
    case GREATER_OR_EQUAL: return ">=";
    case LIKE: return "LIKE";
    case SQL_IN: return "IN";
        // other logical operations: OR (or ||) AND (or &&) XOR
    case SIMILAR_TO: return "SIMILAR TO";
    case NOT_SIMILAR_TO: return "NOT SIMILAR TO";
    case OR: return "OR";
    case AND: return "AND";
    case XOR: return "XOR";
        // other string operations: || (as CONCATENATION)
    case CONCATENATION: return "||";
        // SpecialBinary "pseudo operators":
        /* not handled here */
    default:;
    }
    return QString("{INVALID_BINARY_OPERATOR#%1} ").arg(token());
}

EscapedString BinaryExpression::toString(QuerySchemaParameterValueListIterator* params) const
{
#define INFIX(a) \
    (m_larg ? m_larg->toString(params) : EscapedString("<NULL>")) + " " + a + " " + (m_rarg ? m_rarg->toString(params) : EscapedString("<NULL>"))
    return INFIX(tokenToString());
#undef INFIX
}

void BinaryExpression::getQueryParameters(QuerySchemaParameterList& params)
{
    if (m_larg)
        m_larg->getQueryParameters(params);
    if (m_rarg)
        m_rarg->getQueryParameters(params);
}

//=========================================
ConstExpression::ConstExpression(int token, const QVariant& val)
        : Expression(ConstExpressionClass, token)
        , value(val)
{
}

ConstExpression::ConstExpression(ExpressionClass aClass, int token, const QVariant& val)
        : Expression(aClass, token)
        , value(val)
{
}

ConstExpression::ConstExpression(const ConstExpression& expr)
        : Expression(expr)
        , value(expr.value)
{
}

ConstExpression::~ConstExpression()
{
}

ConstExpression* ConstExpression::copy() const
{
    return new ConstExpression(*this);
}

Field::Type ConstExpression::type() const
{
    if (token() == SQL_NULL)
        return Field::Null;
    else if (token() == INTEGER_CONST) {
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
    } else if (token() == CHARACTER_STRING_LITERAL) {
//! @todo Field::defaultTextLength() is hardcoded now!
        if (value.toString().length() > (int)Field::defaultTextLength())
            return Field::LongText;
        else
            return Field::Text;
    } else if (token() == REAL_CONST)
        return Field::Double;
    else if (token() == DATE_CONST)
        return Field::Date;
    else if (token() == DATETIME_CONST)
        return Field::DateTime;
    else if (token() == TIME_CONST)
        return Field::Time;

    return Field::InvalidType;
}

QString ConstExpression::debugString() const
{
    return QLatin1String("ConstExpression('") + tokenToDebugString() + "'," + toString().toString()
           + QString(",type=%1)").arg(Driver::defaultSQLTypeName(type()));
}

EscapedString ConstExpression::toString(QuerySchemaParameterValueListIterator* params) const
{
    Q_UNUSED(params);
    if (token() == SQL_NULL)
        return EscapedString("NULL");
    else if (token() == CHARACTER_STRING_LITERAL)
//! @todo better escaping!
        return EscapedString("'") + value.toString() + "'";
    else if (token() == REAL_CONST)
        return EscapedString::number(value.toPoint().x()) + "." + EscapedString::number(value.toPoint().y());
    else if (token() == DATE_CONST)
        return EscapedString("'") + value.toDate().toString(Qt::ISODate) + "'";
    else if (token() == DATETIME_CONST)
        return EscapedString("'") + EscapedString(value.toDateTime().date().toString(Qt::ISODate))
               + " " + value.toDateTime().time().toString(Qt::ISODate) + "'";
    else if (token() == TIME_CONST)
        return EscapedString("'") + value.toTime().toString(Qt::ISODate) + "'";

    return EscapedString(value.toString());
}

void ConstExpression::getQueryParameters(QuerySchemaParameterList& params)
{
    Q_UNUSED(params);
}

bool ConstExpression::validate(ParseInfo& parseInfo)
{
    if (!Expression::validate(parseInfo))
        return false;

    return type() != Field::InvalidType;
}

//=========================================
QueryParameterExpression::QueryParameterExpression(const QString& message)
        : ConstExpression(QueryParameterExpressionClass, QUERY_PARAMETER, message)
        , m_type(Field::Text)
{
}

QueryParameterExpression::QueryParameterExpression(const QueryParameterExpression& expr)
        : ConstExpression(expr)
        , m_type(expr.m_type)
{
}

QueryParameterExpression::~QueryParameterExpression()
{
}

QueryParameterExpression* QueryParameterExpression::copy() const
{
    return new QueryParameterExpression(*this);
}

Field::Type QueryParameterExpression::type() const
{
    return m_type;
}

void QueryParameterExpression::setType(Field::Type type)
{
    m_type = type;
}

QString QueryParameterExpression::debugString() const
{
    return QString("QueryParameterExpression('") + QString::fromLatin1("[%2]").arg(value.toString())
           + QString("',type=%1)").arg(Driver::defaultSQLTypeName(type()));
}

EscapedString QueryParameterExpression::toString(QuerySchemaParameterValueListIterator* params) const
{
    return params ? params->getPreviousValueAsString(type())
                                : EscapedString("[%1]").arg(EscapedString(value.toString()));
}

void QueryParameterExpression::getQueryParameters(QuerySchemaParameterList& params)
{
    QuerySchemaParameter param;
    param.message = value.toString();
    param.type = type();
    params.append(param);
}

bool QueryParameterExpression::validate(ParseInfo& parseInfo)
{
    Q_UNUSED(parseInfo);
    return type() != Field::InvalidType;
}

//=========================================
VariableExpression::VariableExpression(const QString& _name)
        : Expression(VariableExpressionClass, 0/*undefined*/)
        , name(_name)
        , field(0)
        , tablePositionForField(-1)
        , tableForQueryAsterisk(0)
{
}

VariableExpression::VariableExpression(const VariableExpression& expr)
        : Expression(expr)
        , name(expr.name)
        , field(expr.field)
        , tablePositionForField(expr.tablePositionForField)
        , tableForQueryAsterisk(expr.tableForQueryAsterisk)
{
}

VariableExpression::~VariableExpression()
{
}

VariableExpression* VariableExpression::copy() const
{
    return new VariableExpression(*this);
}

QString VariableExpression::debugString() const
{
    return QString("VariableExpression(") + name
           + QString(",type=%1)").arg(field ? Driver::defaultSQLTypeName(type()) : QString("FIELD NOT DEFINED YET"));
}

EscapedString VariableExpression::toString(QuerySchemaParameterValueListIterator* params) const
{
    Q_UNUSED(params);
    return EscapedString(name);
}

void VariableExpression::getQueryParameters(QuerySchemaParameterList& params)
{
    Q_UNUSED(params);
}

//! We're assuming it's called after VariableExpr::validate()
Field::Type VariableExpression::type() const
{
    if (field)
        return field->type();

    //BTW, asterisks are not stored in VariableExpr outside of parser, so ok.
    return Field::InvalidType;
}

#define IMPL_ERROR(errmsg) parseInfo.errMsg = "Implementation error"; parseInfo.errDescr = errmsg

bool VariableExpression::validate(ParseInfo& parseInfo)
{
    if (!Expression::validate(parseInfo))
        return false;
    field = 0;
    tablePositionForField = -1;
    tableForQueryAsterisk = 0;

    /* taken from parser's addColumn(): */
    PreDbg << "checking variable name: " << name;
    int dotPos = name.indexOf('.');
    QString tableName, fieldName;
//! @todo shall we also support db name?
    if (dotPos > 0) {
        tableName = name.left(dotPos);
        fieldName = name.mid(dotPos + 1);
    }
    if (tableName.isEmpty()) {//fieldname only
        fieldName = name;
        if (fieldName == "*") {
//   querySchema->addAsterisk( new QueryAsterisk(querySchema) );
            return true;
        }

        //find first table that has this field
        Field *firstField = 0;
        foreach(TableSchema *table, *parseInfo.querySchema->tables()) {
            Field *f = table->field(fieldName);
            if (f) {
                if (!firstField) {
                    firstField = f;
                } else if (f->table() != firstField->table()) {
                    //ambiguous field name
                    parseInfo.errMsg = QObject::tr("Ambiguous field name");
                    parseInfo.errDescr = QObject::tr("Both table \"%1\" and \"%2\" have defined \"%3\" field. "
                                              "Use \"<tableName>.%4\" notation to specify table name.")
                                              .arg(firstField->table()->name(), f->table()->name(),
                                              fieldName, fieldName);
                    return false;
                }
            }
        }
        if (!firstField) {
            parseInfo.errMsg = QObject::tr("Field not found");
            parseInfo.errDescr = QObject::tr("Table containing \"%1\" field not found").arg(fieldName);
            return false;
        }
        //ok
        field = firstField; //store
//  querySchema->addField(firstField);
        return true;
    }

    //table.fieldname or tableAlias.fieldname
    tableName = tableName.toLower();
    TableSchema *ts = parseInfo.querySchema->table(tableName);
    if (ts) {//table.fieldname
        //check if "table" is covered by an alias
        const QList<int> tPositions = parseInfo.querySchema->tablePositions(tableName);
        QByteArray tableAlias;
        bool covered = true;
        foreach(int position, tPositions) {
            tableAlias = parseInfo.querySchema->tableAlias(position);
            if (tableAlias.isEmpty() || tableAlias.toLower() == tableName.toLatin1()) {
                covered = false; //uncovered
                break;
            }
            PreDbg << " --" << "covered by " << tableAlias << " alias";
        }
        if (covered) {
            parseInfo.errMsg = QObject::tr("Could not access the table directly using its name");
            parseInfo.errDescr = QObject::tr("Table \"%1\" is covered by aliases. Instead of \"%2\", "
                                      "you can write \"%3\"").arg(tableName, tableName + "." + fieldName, tableAlias + "." + QString(fieldName));
            return false;
        }
    }

    int tablePosition = -1;
    if (!ts) {//try to find tableAlias
        tablePosition = parseInfo.querySchema->tablePositionForAlias(tableName.toLatin1());
        if (tablePosition >= 0) {
            ts = parseInfo.querySchema->tables()->at(tablePosition);
            if (ts) {
//    PreDbg << " --it's a tableAlias.name";
            }
        }
    }

    if (!ts) {
        parseInfo.errMsg = QObject::tr("Table not found");
        parseInfo.errDescr = QObject::tr("Unknown table \"%1\"").arg(tableName);
        return false;
    }

    if (!parseInfo.repeatedTablesAndAliases.contains(tableName)) {  //for sanity
        IMPL_ERROR(tableName + "." + fieldName + ", !positionsList ");
        return false;
    }
    const QList<int> positionsList(parseInfo.repeatedTablesAndAliases.value(tableName));

    //it's a table.*
    if (fieldName == "*") {
        if (positionsList.count() > 1) {
            parseInfo.errMsg = QObject::tr("Ambiguous \"%1.*\" expression").arg(tableName);
            parseInfo.errDescr = QObject::tr("More than one \"%1\" table or alias defined").arg(tableName);
            return false;
        }
        tableForQueryAsterisk = ts;
//   querySchema->addAsterisk( new QueryAsterisk(querySchema, ts) );
        return true;
    }

// PreDbg << " --it's a table.name";
    Field *realField = ts->field(fieldName);
    if (!realField) {
        parseInfo.errMsg = QObject::tr("Field not found");
        parseInfo.errDescr = QObject::tr("Table \"%1\" has no \"%2\" field").arg(tableName, fieldName);
        return false;
    }

    // check if table or alias is used twice and both have the same column
    // (so the column is ambiguous)
    int numberOfTheSameFields = 0;
    foreach(int position, positionsList) {
        TableSchema *otherTS = parseInfo.querySchema->tables()->at(position);
        if (otherTS->field(fieldName))
            numberOfTheSameFields++;
        if (numberOfTheSameFields > 1) {
            parseInfo.errMsg = QObject::tr("Ambiguous \"%1.%2\" expression").arg(tableName, fieldName);
            parseInfo.errDescr = QObject::tr("More than one \"%1\" table or alias defined containing \"%2\" field")
                                      .arg(tableName, fieldName);
            return false;
        }
    }
    field = realField; //store
    tablePositionForField = tablePosition;
//    querySchema->addField(realField, tablePosition);

    return true;
}

//=========================================

static const char* FunctionExpr_builtIns_[] = {"SUM", "MIN", "MAX", "AVG", "COUNT", "STD", "STDDEV", "VARIANCE", 0 };

class BuiltInAggregates : public QSet<QByteArray>
{
public:
    BuiltInAggregates() : QSet<QByteArray>() {
        for (const char **p = FunctionExpr_builtIns_; *p; p++)
            insert(QByteArray::fromRawData(*p, qstrlen(*p)));
    }
};

PREDICATE_GLOBAL_STATIC(BuiltInAggregates, _builtInAggregates)

//=========================================

inline ExpressionClass classForFunctionName(const QString& name)
{
    if (FunctionExpression::isBuiltInAggregate(name))
        return AggregationExpressionClass;
    else
        return FunctionExpressionClass;
}

FunctionExpression::FunctionExpression(const QString& _name, NArgExpression* args_)
        : Expression(classForFunctionName(_name), 0/*undefined*/)
        , name(_name)
        , args(args_)
{
    if (args)
        args->setParent(this);
}

FunctionExpression::FunctionExpression(const FunctionExpression& expr)
        : Expression(expr.expressionClass(), 0/*undefined*/)
        , name(expr.name)
        , args(expr.args ? args->copy() : 0)
{
    if (args)
        args->setParent(this);
}

FunctionExpression::~FunctionExpression()
{
    delete args;
}

FunctionExpression* FunctionExpression::copy() const
{
    return new FunctionExpression(*this);
}

QString FunctionExpression::debugString() const
{
    QString res;
    res.append(QString("FunctionExpression(") + name);
    if (args)
        res.append(QString(",") + args->debugString());
    res.append(QString(",type=%1)").arg(Driver::defaultSQLTypeName(type())));
    return res;
}

EscapedString FunctionExpression::toString(QuerySchemaParameterValueListIterator* params) const
{
    return name + "(" + (args ? args->toString(params) : EscapedString()) + ")";
}

void FunctionExpression::getQueryParameters(QuerySchemaParameterList& params)
{
    args->getQueryParameters(params);
}

Field::Type FunctionExpression::type() const
{
//! @todo
    return Field::InvalidType;
}

bool FunctionExpression::validate(ParseInfo& parseInfo)
{
    if (!Expression::validate(parseInfo))
        return false;

    return args ? args->validate(parseInfo) : true;
}

bool FunctionExpression::isBuiltInAggregate(const QString& fname)
{
    return _builtInAggregates->contains(fname.toLatin1().toUpper());
}

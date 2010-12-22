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
#include "parser/SqlParser.h"
#include "parser/Parser_p.h"
#include "tools/Static.h"

#include <ctype.h>

#include <QtDebug>


PREDICATE_EXPORT QString Predicate::exprClassName(int c)
{
    if (c == PredicateExpr_Unary)
        return "Unary";
    else if (c == PredicateExpr_Arithm)
        return "Arithm";
    else if (c == PredicateExpr_Logical)
        return "Logical";
    else if (c == PredicateExpr_Relational)
        return "Relational";
    else if (c == PredicateExpr_SpecialBinary)
        return "SpecialBinary";
    else if (c == PredicateExpr_Const)
        return "Const";
    else if (c == PredicateExpr_Variable)
        return "Variable";
    else if (c == PredicateExpr_Function)
        return "Function";
    else if (c == PredicateExpr_Aggregation)
        return "Aggregation";
    else if (c == PredicateExpr_TableList)
        return "TableList";
    else if (c == PredicateExpr_QueryParameter)
        return "QueryParameter";

    return "Unknown";
}

using namespace Predicate;

//=========================================

BaseExpr::BaseExpr(int token)
        : m_cl(PredicateExpr_Unknown)
        , m_par(0)
        , m_token(token)
{
}

BaseExpr::~BaseExpr()
{
}

Field::Type BaseExpr::type() const
{
    return Field::InvalidType;
}

QString BaseExpr::debugString() const
{
    return QString("BaseExpr(%1,type=%1)").arg(m_token).arg(Driver::defaultSQLTypeName(type()));
}

//! Sends information about expression  @a expr to debug output @a dbg.
PREDICATE_EXPORT QDebug operator<<(QDebug dbg, const BaseExpr& expr)
{
    dbg.nospace() << expr.debugString();
    return dbg.space();
}

bool BaseExpr::validate(ParseInfo& /*parseInfo*/)
{
    return true;
}

extern const char* tname(int offset);
#define safe_tname(token) ((token>=255 && token<=__LAST_TOKEN) ? tname(token-255) : "")

QString BaseExpr::tokenToDebugString(int token)
{
    if (token < 254) {
        if (isprint(token))
            return QString(QChar(uchar(token)));
        else
            return QString::number(token);
    }
    return QString(safe_tname(token));
}

QString BaseExpr::tokenToString() const
{
    if (m_token < 255 && isprint(m_token))
        return tokenToDebugString();
    return QString();
}

NArgExpr* BaseExpr::toNArg()
{
    return dynamic_cast<NArgExpr*>(this);
}
UnaryExpr* BaseExpr::toUnary()
{
    return dynamic_cast<UnaryExpr*>(this);
}
BinaryExpr* BaseExpr::toBinary()
{
    return dynamic_cast<BinaryExpr*>(this);
}
ConstExpr* BaseExpr::toConst()
{
    return dynamic_cast<ConstExpr*>(this);
}
VariableExpr* BaseExpr::toVariable()
{
    return dynamic_cast<VariableExpr*>(this);
}
FunctionExpr* BaseExpr::toFunction()
{
    return dynamic_cast<FunctionExpr*>(this);
}
QueryParameterExpr* BaseExpr::toQueryParameter()
{
    return dynamic_cast<QueryParameterExpr*>(this);
}

//=========================================

NArgExpr::NArgExpr(int aClass, int token)
        : BaseExpr(token)
{
    m_cl = aClass;
//Qt 4 list.setAutoDelete(true);
}

NArgExpr::NArgExpr(const NArgExpr& expr)
        : BaseExpr(expr)
{
    foreach(BaseExpr* e, expr.list) {
        add(e->copy());
    }
}

NArgExpr::~NArgExpr()
{
}

NArgExpr* NArgExpr::copy() const
{
    return new NArgExpr(*this);
}

QString NArgExpr::debugString() const
{
    QString s = QString("NArgExpr(")
                + "class=" + exprClassName(m_cl);
    foreach(BaseExpr *expr, list) {
        s += ", ";
        s += expr->debugString();
    }
    s += ")";
    return s;
}

EscapedString NArgExpr::toString(QuerySchemaParameterValueListIterator* params) const
{
    EscapedString s;
    s.reserve(256);
    foreach(BaseExpr* e, list) {
        if (!s.isEmpty())
            s += ", ";
        s += e->toString(params);
    }
    return s;
}

void NArgExpr::getQueryParameters(QuerySchemaParameterList& params)
{
    foreach(BaseExpr *e, list) {
        e->getQueryParameters(params);
    }
}

BaseExpr* NArgExpr::arg(int nr)
{
    return list.at(nr);
}

void NArgExpr::add(BaseExpr *expr)
{
    list.append(expr);
    expr->setParent(this);
}

void NArgExpr::prepend(BaseExpr *expr)
{
    list.prepend(expr);
    expr->setParent(this);
}

int NArgExpr::args()
{
    return list.count();
}

bool NArgExpr::validate(ParseInfo& parseInfo)
{
    if (!BaseExpr::validate(parseInfo))
        return false;

    foreach(BaseExpr *e, list) {
        if (!e->validate(parseInfo))
            return false;
    }
    return true;
}

//=========================================
UnaryExpr::UnaryExpr(int token, BaseExpr *arg)
        : BaseExpr(token)
        , m_arg(arg)
{
    m_cl = PredicateExpr_Unary;
    if (m_arg)
        m_arg->setParent(this);
}

UnaryExpr::UnaryExpr(const UnaryExpr& expr)
        : BaseExpr(expr)
        , m_arg(expr.m_arg ? expr.m_arg->copy() : 0)
{
    if (m_arg)
        m_arg->setParent(this);
}

UnaryExpr::~UnaryExpr()
{
    delete m_arg;
}

UnaryExpr* UnaryExpr::copy() const
{
    return new UnaryExpr(*this);
}

QString UnaryExpr::debugString() const
{
    return "UnaryExpr('"
           + tokenToDebugString() + "', "
           + (m_arg ? m_arg->debugString() : QString("<NONE>"))
           + QString(",type=%1)").arg(Driver::defaultSQLTypeName(type()));
}

EscapedString UnaryExpr::toString(QuerySchemaParameterValueListIterator* params) const
{
    if (m_token == '(') //parentheses (special case)
        return "(" + (m_arg ? m_arg->toString(params) : EscapedString("<NULL>")) + ")";
    if (m_token < 255 && isprint(m_token))
        return tokenToDebugString() + (m_arg ? m_arg->toString(params) : EscapedString("<NULL>"));
    if (m_token == NOT)
        return "NOT " + (m_arg ? m_arg->toString(params) : EscapedString("<NULL>"));
    if (m_token == SQL_IS_NULL)
        return (m_arg ? m_arg->toString(params) : EscapedString("<NULL>")) + " IS NULL";
    if (m_token == SQL_IS_NOT_NULL)
        return (m_arg ? m_arg->toString(params) : EscapedString("<NULL>")) + " IS NOT NULL";
    return EscapedString("{INVALID_OPERATOR#%1} ").arg(m_token) + (m_arg ? m_arg->toString(params) : EscapedString("<NULL>"));
}

void UnaryExpr::getQueryParameters(QuerySchemaParameterList& params)
{
    if (m_arg)
        m_arg->getQueryParameters(params);
}

Field::Type UnaryExpr::type() const
{
    //NULL IS NOT NULL : BOOLEAN
    //NULL IS NULL : BOOLEAN
    switch (m_token) {
    case SQL_IS_NULL:
    case SQL_IS_NOT_NULL:
        return Field::Boolean;
    }
    const Field::Type t = m_arg->type();
    if (t == Field::Null)
        return Field::Null;
    if (m_token == NOT)
        return Field::Boolean;

    return t;
}

bool UnaryExpr::validate(ParseInfo& parseInfo)
{
    if (!BaseExpr::validate(parseInfo))
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
    BaseExpr *n = l.at(0);

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
BinaryExpr::BinaryExpr(int aClass, BaseExpr *left_expr, int token, BaseExpr *right_expr)
        : BaseExpr(token)
        , m_larg(left_expr)
        , m_rarg(right_expr)
{
    m_cl = aClass;
    if (m_larg)
        m_larg->setParent(this);
    if (m_rarg)
        m_rarg->setParent(this);
}

BinaryExpr::BinaryExpr(const BinaryExpr& expr)
        : BaseExpr(expr)
        , m_larg(expr.m_larg ? expr.m_larg->copy() : 0)
        , m_rarg(expr.m_rarg ? expr.m_rarg->copy() : 0)
{
}

BinaryExpr::~BinaryExpr()
{
    delete m_larg;
    delete m_rarg;
}

BinaryExpr* BinaryExpr::copy() const
{
    return new BinaryExpr(*this);
}

bool BinaryExpr::validate(ParseInfo& parseInfo)
{
    if (!BaseExpr::validate(parseInfo))
        return false;

    if (!m_larg->validate(parseInfo))
        return false;
    if (!m_rarg->validate(parseInfo))
        return false;

//! @todo compare types..., BITWISE_SHIFT_RIGHT requires integers, etc...

    //update type for query parameters
    QueryParameterExpr * queryParameter = m_larg->toQueryParameter();
    if (queryParameter)
        queryParameter->setType(m_rarg->type());
    queryParameter = m_rarg->toQueryParameter();
    if (queryParameter)
        queryParameter->setType(m_larg->type());

    return true;
}

Field::Type BinaryExpr::type() const
{
    const Field::Type lt = m_larg->type(), rt = m_rarg->type();
    if (lt == Field::InvalidType || rt == Field::InvalidType)
        return Field::InvalidType;
    if (lt == Field::Null || rt == Field::Null) {
        if (m_token != OR) //note that NULL OR something   != NULL
            return Field::Null;
    }

    switch (m_token) {
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

QString BinaryExpr::debugString() const
{
    return QString("BinaryExpr(")
           + "class=" + exprClassName(m_cl)
           + "," + (m_larg ? m_larg->debugString() : QString("<NONE>"))
           + ",'" + tokenToDebugString() + "',"
           + (m_rarg ? m_rarg->debugString() : QString("<NONE>"))
           + QString(",type=%1)").arg(Driver::defaultSQLTypeName(type()));
}

QString BinaryExpr::tokenToString() const
{
    if (m_token < 255 && isprint(m_token))
        return tokenToDebugString();
    // other arithmetic operations: << >>
    switch (m_token) {
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
    return QString("{INVALID_BINARY_OPERATOR#%1} ").arg(m_token);
}

EscapedString BinaryExpr::toString(QuerySchemaParameterValueListIterator* params) const
{
#define INFIX(a) \
    (m_larg ? m_larg->toString(params) : EscapedString("<NULL>")) + " " + a + " " + (m_rarg ? m_rarg->toString(params) : EscapedString("<NULL>"))
    return INFIX(tokenToString());
#undef INFIX
}

void BinaryExpr::getQueryParameters(QuerySchemaParameterList& params)
{
    if (m_larg)
        m_larg->getQueryParameters(params);
    if (m_rarg)
        m_rarg->getQueryParameters(params);
}

//=========================================
ConstExpr::ConstExpr(int token, const QVariant& val)
        : BaseExpr(token)
        , value(val)
{
    m_cl = PredicateExpr_Const;
}

ConstExpr::ConstExpr(const ConstExpr& expr)
        : BaseExpr(expr)
        , value(expr.value)
{
}

ConstExpr::~ConstExpr()
{
}

ConstExpr* ConstExpr::copy() const
{
    return new ConstExpr(*this);
}

Field::Type ConstExpr::type() const
{
    if (m_token == SQL_NULL)
        return Field::Null;
    else if (m_token == INTEGER_CONST) {
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
    } else if (m_token == CHARACTER_STRING_LITERAL) {
//! @todo Field::defaultTextLength() is hardcoded now!
        if (value.toString().length() > (int)Field::defaultTextLength())
            return Field::LongText;
        else
            return Field::Text;
    } else if (m_token == REAL_CONST)
        return Field::Double;
    else if (m_token == DATE_CONST)
        return Field::Date;
    else if (m_token == DATETIME_CONST)
        return Field::DateTime;
    else if (m_token == TIME_CONST)
        return Field::Time;

    return Field::InvalidType;
}

QString ConstExpr::debugString() const
{
    return QLatin1String("ConstExpr('") + tokenToDebugString() + "'," + toString().toString()
           + QString(",type=%1)").arg(Driver::defaultSQLTypeName(type()));
}

EscapedString ConstExpr::toString(QuerySchemaParameterValueListIterator* params) const
{
    Q_UNUSED(params);
    if (m_token == SQL_NULL)
        return EscapedString("NULL");
    else if (m_token == CHARACTER_STRING_LITERAL)
//! @todo better escaping!
        return EscapedString("'") + value.toString() + "'";
    else if (m_token == REAL_CONST)
        return EscapedString::number(value.toPoint().x()) + "." + EscapedString::number(value.toPoint().y());
    else if (m_token == DATE_CONST)
        return EscapedString("'") + value.toDate().toString(Qt::ISODate) + "'";
    else if (m_token == DATETIME_CONST)
        return EscapedString("'") + EscapedString(value.toDateTime().date().toString(Qt::ISODate))
               + " " + value.toDateTime().time().toString(Qt::ISODate) + "'";
    else if (m_token == TIME_CONST)
        return EscapedString("'") + value.toTime().toString(Qt::ISODate) + "'";

    return EscapedString(value.toString());
}

void ConstExpr::getQueryParameters(QuerySchemaParameterList& params)
{
    Q_UNUSED(params);
}

bool ConstExpr::validate(ParseInfo& parseInfo)
{
    if (!BaseExpr::validate(parseInfo))
        return false;

    return type() != Field::InvalidType;
}

//=========================================
QueryParameterExpr::QueryParameterExpr(const QString& message)
        : ConstExpr(QUERY_PARAMETER, message)
        , m_type(Field::Text)
{
    m_cl = PredicateExpr_QueryParameter;
}

QueryParameterExpr::QueryParameterExpr(const QueryParameterExpr& expr)
        : ConstExpr(expr)
        , m_type(expr.m_type)
{
}

QueryParameterExpr::~QueryParameterExpr()
{
}

QueryParameterExpr* QueryParameterExpr::copy() const
{
    return new QueryParameterExpr(*this);
}

Field::Type QueryParameterExpr::type() const
{
    return m_type;
}

void QueryParameterExpr::setType(Field::Type type)
{
    m_type = type;
}

QString QueryParameterExpr::debugString() const
{
    return QString("QueryParameterExpr('") + QString::fromLatin1("[%2]").arg(value.toString())
           + QString("',type=%1)").arg(Driver::defaultSQLTypeName(type()));
}

EscapedString QueryParameterExpr::toString(QuerySchemaParameterValueListIterator* params) const
{
    return params ? params->getPreviousValueAsString(type())
                                : EscapedString("[%1]").arg(EscapedString(value.toString()));
}

void QueryParameterExpr::getQueryParameters(QuerySchemaParameterList& params)
{
    QuerySchemaParameter param;
    param.message = value.toString();
    param.type = type();
    params.append(param);
}

bool QueryParameterExpr::validate(ParseInfo& parseInfo)
{
    Q_UNUSED(parseInfo);
    return type() != Field::InvalidType;
}

//=========================================
VariableExpr::VariableExpr(const QString& _name)
        : BaseExpr(0/*undefined*/)
        , name(_name)
        , field(0)
        , tablePositionForField(-1)
        , tableForQueryAsterisk(0)
{
    m_cl = PredicateExpr_Variable;
}

VariableExpr::VariableExpr(const VariableExpr& expr)
        : BaseExpr(expr)
        , name(expr.name)
        , field(expr.field)
        , tablePositionForField(expr.tablePositionForField)
        , tableForQueryAsterisk(expr.tableForQueryAsterisk)
{
}

VariableExpr::~VariableExpr()
{
}

VariableExpr* VariableExpr::copy() const
{
    return new VariableExpr(*this);
}

QString VariableExpr::debugString() const
{
    return QString("VariableExpr(") + name
           + QString(",type=%1)").arg(field ? Driver::defaultSQLTypeName(type()) : QString("FIELD NOT DEFINED YET"));
}

EscapedString VariableExpr::toString(QuerySchemaParameterValueListIterator* params) const
{
    Q_UNUSED(params);
    return EscapedString(name);
}

void VariableExpr::getQueryParameters(QuerySchemaParameterList& params)
{
    Q_UNUSED(params);
}

//! We're assuming it's called after VariableExpr::validate()
Field::Type VariableExpr::type() const
{
    if (field)
        return field->type();

    //BTW, asterisks are not stored in VariableExpr outside of parser, so ok.
    return Field::InvalidType;
}

#define IMPL_ERROR(errmsg) parseInfo.errMsg = "Implementation error"; parseInfo.errDescr = errmsg

bool VariableExpr::validate(ParseInfo& parseInfo)
{
    if (!BaseExpr::validate(parseInfo))
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

FunctionExpr::FunctionExpr(const QString& _name, NArgExpr* args_)
        : BaseExpr(0/*undefined*/)
        , name(_name)
        , args(args_)
{
    if (isBuiltInAggregate(name.toLatin1()))
        m_cl = PredicateExpr_Aggregation;
    else
        m_cl = PredicateExpr_Function;
    if (args)
        args->setParent(this);
}

FunctionExpr::FunctionExpr(const FunctionExpr& expr)
        : BaseExpr(0/*undefined*/)
        , name(expr.name)
        , args(expr.args ? args->copy() : 0)
{
    if (args)
        args->setParent(this);
}

FunctionExpr::~FunctionExpr()
{
    delete args;
}

FunctionExpr* FunctionExpr::copy() const
{
    return new FunctionExpr(*this);
}

QString FunctionExpr::debugString() const
{
    QString res;
    res.append(QString("FunctionExpr(") + name);
    if (args)
        res.append(QString(",") + args->debugString());
    res.append(QString(",type=%1)").arg(Driver::defaultSQLTypeName(type())));
    return res;
}

EscapedString FunctionExpr::toString(QuerySchemaParameterValueListIterator* params) const
{
    return name + "(" + (args ? args->toString(params) : EscapedString()) + ")";
}

void FunctionExpr::getQueryParameters(QuerySchemaParameterList& params)
{
    args->getQueryParameters(params);
}

Field::Type FunctionExpr::type() const
{
//! @todo
    return Field::InvalidType;
}

bool FunctionExpr::validate(ParseInfo& parseInfo)
{
    if (!BaseExpr::validate(parseInfo))
        return false;

    return args ? args->validate(parseInfo) : true;
}

bool FunctionExpr::isBuiltInAggregate(const QByteArray& fname)
{
    return _builtInAggregates->contains(fname.toUpper());
}

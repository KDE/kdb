/* This file is part of the KDE project
   Copyright (C) 2004 Lucijan Busch <lucijan@kde.org>
   Copyright (C) 2004-2018 Jarosław Staniek <staniek@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

%{
#include "sqlparser.h"
#include "KDb.h"
#include "KDbExpression.h"
#include "KDbParser.h"
#include "KDbSqlTypes.h"
#include "kdb_debug.h"

extern int globalCurrentPos;
extern QByteArray globalToken;

#define YY_NO_UNPUT
#define ECOUNT globalCurrentPos += yyleng; globalToken = yytext

extern void setError(const QString& errDesc);
extern void setError(const QString& errName, const QString& errDesc);

/* Only quotes the input if it does not start with a quote character, otherwise
 it would be too hard to read with some fonts. */
static QString maybeQuote(const QString& string)
{
    QString first(string.left(1));
    if (first == QLatin1Char('\'') || first == QLatin1Char('"') || first == QLatin1Char('`')) {
        return string;
    }
    return QStringLiteral("\"%1\"").arg(string);
}
%}

/* *** Please reflect changes to this file in ../driver_p.cpp *** */

%option case-insensitive
%option noyywrap
%option never-interactive

%x DATE_OR_TIME

whitespace         [ \t\n]
whitespace_1_line  [ \t]
digit              [0-9]
e_str1             (\\\'|\'\'|[^'])
e_str2             (\\\"|\"\"|[^"])
string             (\'{e_str1}*\'|\"{e_str2}*\")
/*identifier       [a-zA-Z_][a-zA-Z_0-9]* */
identifier         [a-zA-Z_0-9]+
/* quoted_identifier (\"[a-zA-Z_0-9]+\") */
query_parameter    \[[^\[\]]+\]

integer            {digit}+
decimal            (({digit}*\.{digit}+)|({digit}+\.{digit}*))
real               ((({digit}*\.{digit}+)|({digit}+\.{digit}*)|({digit}+))([Ee][-+]?{digit}+))

%%

    int DATE_OR_TIME_caller = 0;

"<>" {
    ECOUNT;
    return NOT_EQUAL;
}

"!=" {
    ECOUNT;
    return NOT_EQUAL2;
}

"==" {
    ECOUNT;
    return '=';
}

"<=" {
    ECOUNT;
    return LESS_OR_EQUAL;
}

">=" {
    ECOUNT;
    return GREATER_OR_EQUAL;
}

"IN" {
    ECOUNT;
    return SQL_IN;
}

{integer} {
//! @todo what about hex or octal values?
    //we're using QString:toLongLong() here because atoll() is not so portable:
    ECOUNT;
    bool ok;
    yylval.integerValue = QByteArray(yytext).toLongLong(&ok);
    if (!ok) {
        setError(KDbParser::tr("Invalid integer number"), KDbParser::tr("This integer number may be too large."));
        return SCAN_ERROR;
    }
    return INTEGER_CONST;
}

{decimal} {
    ECOUNT;
    yylval.binaryValue = new QByteArray(yytext, yyleng);
    return REAL_CONST;
}

  /* --- DATE_OR_TIME --- */
# {
    ECOUNT;
    sqlParserDebug() << "### begin DATE_OR_TIME" << yytext << "(" << yyleng << ")";
    DATE_OR_TIME_caller = YYSTATE;
    BEGIN(DATE_OR_TIME);
    return '#';
}

<DATE_OR_TIME>{

[-+/:] { // year prefix or / or - or : separator
    ECOUNT;
    return yytext[0];
}

{integer} { // year, month, day, hour, minute or second
    ECOUNT;
    yylval.binaryValue = new QByteArray(yytext, yyleng);
    return DATE_TIME_INTEGER;
}

{whitespace_1_line}+ {
    ECOUNT;
    return TABS_OR_SPACES;
}

{whitespace_1_line}*(?i:AM) {
    ECOUNT;
    return TIME_AM;
}

{whitespace_1_line}*(?i:PM) {
    ECOUNT;
    return TIME_PM;
}

# {
    ECOUNT;
    sqlParserDebug() << "### end DATE_OR_TIME" << yytext << "(" << yyleng << ")";
    BEGIN(DATE_OR_TIME_caller);
    return '#';
}


. { // fallback rule to avoid flex's default action that prints the character to stdout
    // without notifying the scanner.
    ECOUNT;
    const QString string(QString::fromUtf8(yytext, yyleng));
    setError(KDbParser::tr("Unexpected character %1 in date/time").arg(maybeQuote(string)));
    return SCAN_ERROR;
}

}
  /* -- end of DATE_OR_TIME --- */

("AND"|"&&") {
    ECOUNT;
    return AND;
}

"AS" {
    ECOUNT;
    return AS;
}

"CREATE" {
    ECOUNT;
    return CREATE;
}

"FROM" {
    ECOUNT;
    return FROM;
}

"INTEGER" {
    ECOUNT;
    return SQL_TYPE;
}

"JOIN" {
    ECOUNT;
    return JOIN;
}

"LEFT" {
    ECOUNT;
    return LEFT;
}

"LIKE" {
    ECOUNT;
    return LIKE;
}

"NOT"{whitespace}+"LIKE" {
    ECOUNT;
    return NOT_LIKE;
}

"BETWEEN" {
    ECOUNT;
    return BETWEEN;
}

"NOT"{whitespace}+"BETWEEN" {
    ECOUNT;
    return NOT_BETWEEN;
}

"NOT"{whitespace}+"SIMILAR"{whitespace}+"TO" {
    ECOUNT;
    return NOT_SIMILAR_TO;
}

"SIMILAR"{whitespace}+"TO" {
    ECOUNT;
    return SIMILAR_TO;
}

"IS"{whitespace}+"NOT"{whitespace}+"NULL" {
    ECOUNT;
    return SQL_IS_NOT_NULL;
}

"IS"{whitespace}+"NULL" {
    ECOUNT;
    return SQL_IS_NULL;
}

"NOT" {
    ECOUNT;
    return NOT;
}

"IS" {
    ECOUNT;
    return SQL_IS;
}

"NULL" {
    ECOUNT;
    return SQL_NULL;
}

"TRUE" {
        ECOUNT;
        return SQL_TRUE;
}

"FALSE" {
        ECOUNT;
        return SQL_FALSE;
}

"ON" {
    ECOUNT;
    return SQL_ON;
}

"OR" {
    ECOUNT;
    return OR;
}

"||" { /* also means OR for numbers (mysql) */
    ECOUNT;
    return CONCATENATION;
}

"<<" {
    ECOUNT;
    return BITWISE_SHIFT_LEFT;
}

">>" {
    ECOUNT;
    return BITWISE_SHIFT_RIGHT;
}

"XOR" {
    ECOUNT;
    return XOR;
}

"RIGHT" {
    ECOUNT;
    return RIGHT;
}

"SELECT" {
    ECOUNT;
    return SELECT;
}

"TABLE" {
    ECOUNT;
    return TABLE;
}

"WHERE" {
    ECOUNT;
    return WHERE;
}

"ORDER" {
    ECOUNT;
    return ORDER;
}

"BY" {
    ECOUNT;
    return BY;
}

"ASC" {
    ECOUNT;
    return ASC;
}

"DESC" {
    ECOUNT;
    return DESC;
}

{string} {
    ECOUNT;
    sqlParserDebug() << "{string} yytext: '" << yytext << "' (" << yyleng << ")";
    int errorPosition;
    const QString unescaped(
        KDb::unescapeString(QString::fromUtf8(yytext+1, yyleng-2), yytext[0], &errorPosition));
    if (errorPosition >= 0) { // sanity check
        setError(KDbParser::tr("Invalid string"),
                 KDbParser::tr("Invalid character in string"));
        return SCAN_ERROR;
    }
    yylval.stringValue = new QString(unescaped);
    return CHARACTER_STRING_LITERAL;

/* "ZZZ" sentinel for script */
}

{identifier} {
    sqlParserDebug() << "{identifier} yytext: '" << yytext << "' (" << yyleng << ")";
    ECOUNT;
    if (yytext[0]>='0' && yytext[0]<='9') {
        setError(KDbParser::tr("Invalid identifier"),
                 KDbParser::tr("Identifiers should start with a letter or '_' character"));
        return SCAN_ERROR;
    }
    yylval.stringValue = new QString(QString::fromUtf8(yytext, yyleng));
    return IDENTIFIER;
}

{query_parameter} {
    sqlParserDebug() << "{query_parameter} yytext: '" << yytext << "' (" << yyleng << ")";
    ECOUNT;
    yylval.stringValue = new QString(QString::fromUtf8(yytext+1, yyleng-2));
    return QUERY_PARAMETER;
}

{whitespace}+ {
    ECOUNT;
}

[\~\!\@\#\^\&\|\`\?,()\[\]\.;\:\+\-\*\/\%\^\<\>\=] {
    sqlParserDebug() << "char: '" << yytext[0] << "'";
    ECOUNT;
    return yytext[0];
}

. { // fallback rule to avoid flex's default action that prints the character to stdout
    // without notifying the scanner.
    ECOUNT;
    const QString string(QString::fromUtf8(yytext, yyleng));
    setError(KDbParser::tr("Unexpected character %1").arg(maybeQuote(string)));
    return SCAN_ERROR;
}

%%

void tokenize(const char *data)
{
    yy_switch_to_buffer(yy_scan_string(data));
    globalToken.clear();
    globalCurrentPos = 0;
}


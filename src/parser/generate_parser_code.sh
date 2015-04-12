#!/bin/sh
# generates parser and lexer code using bison and flex

builddir=$PWD
srcdir=`dirname $0`
cd $srcdir
flex -ogenerated/sqlscanner.cpp KDbSqlScanner.l
bison -d KDbSqlParser.y -Wall -fall -rall -t --report-file=$builddir/KDbSqlParser.output

echo '#ifndef KDBSQLPARSER_H
#define KDBSQLPARSER_H
#include "KDbField.h"
#include "KDbParser.h"
#include "KDbSqlTypes.h"

bool parseData(KDbParser *p, const char *data);
const char* tokenName(unsigned int offset);
unsigned int maxToken();' > generated/sqlparser.h

cat KDbSqlParser.tab.h >> generated/sqlparser.h
echo '#endif' >> generated/sqlparser.h
sed --in-place 's/[[:space:]]\+$//;s/\t/        /g' generated/sqlparser.h

cat KDbSqlParser.tab.c | sed -e "s/KDbSqlParser\.tab\.c/KDbSqlParser.cpp/g" > generated/sqlparser.cpp
echo 'const char* tokenName(unsigned int offset) { return yytname[YYTRANSLATE(offset)]; }
unsigned int maxToken() { return YYMAXUTOK; }' >> generated/sqlparser.cpp
sed --in-place 's/[[:space:]]\+$//;s/\t/        /g' generated/sqlparser.cpp

./extract_tokens.sh > generated/tokens.cpp
rm -f KDbSqlParser.tab.h KDbSqlParser.tab.c

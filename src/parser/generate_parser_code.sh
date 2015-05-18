#!/bin/bash
# generates parser and lexer code using bison and flex

builddir=$PWD
srcdir=`dirname $0`
cd $srcdir

# check version of bison
read a b c <<< $(bison --version | head -n 1 | sed "s/.* \([0-9]\.[0-9].*\)/\\1/;s/\./\n/g")
v=$(expr $a \* 10000 + $b \* 100 + $c)
if [ $v -lt 30000 ] ; then echo "Error: bison >= 3.0.0 is required, found $a.$b.$c" ; exit 1; fi

# generate lexer and parser
flex -ogenerated/sqlscanner.cpp KDbSqlScanner.l
bison -d KDbSqlParser.y -Wall -fall -rall -t --report-file=$builddir/KDbSqlParser.output

# postprocess
echo '#ifndef KDBSQLPARSER_H
#define KDBSQLPARSER_H
#include "KDbField.h"
#include "KDbParser.h"
#include "KDbSqlTypes.h"

bool parseData(KDbParser *p, const char *data);
KDB_TESTING_EXPORT const char* tokenName(unsigned int offset);
KDB_TESTING_EXPORT unsigned int maxToken();' > generated/sqlparser.h

cat KDbSqlParser.tab.h >> generated/sqlparser.h
echo '#endif' >> generated/sqlparser.h
sed --in-place 's/[[:space:]]\+$//;s/\t/        /g' generated/sqlparser.h

cat KDbSqlParser.tab.c | sed -e "s/KDbSqlParser\.tab\.c/KDbSqlParser.cpp/g" > generated/sqlparser.cpp
echo 'KDB_TESTING_EXPORT const char* tokenName(unsigned int offset) { return yytname[YYTRANSLATE(offset)]; }
KDB_TESTING_EXPORT unsigned int maxToken() { return YYMAXUTOK; }' >> generated/sqlparser.cpp
sed --in-place 's/[[:space:]]\+$//;s/\t/        /g' generated/sqlparser.cpp

./extract_tokens.sh > generated/tokens.cpp
rm -f KDbSqlParser.tab.h KDbSqlParser.tab.c

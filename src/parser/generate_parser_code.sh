#!/bin/sh
# generates parser and lexer code using bison and flex

builddir=$PWD
srcdir=`dirname $0`
cd $srcdir
flex -oSqlScanner.cpp SqlScanner.l
bison -d SqlParser.y -Wall -fall -rall -t --report-file=$builddir/sqlparser.output

echo '#ifndef _SQLPARSER_H_
#define _SQLPARSER_H_
#include <Predicate/Field.h>
#include "Parser.h"
#include "SqlTypes.h"

bool parseData(Predicate::Parser *p, const char *data);' > SqlParser.h

cat SqlParser.tab.h >> SqlParser.h
echo '#endif' >> SqlParser.h

cat SqlParser.tab.c | sed -e "s/SqlParser\.tab\.c/SqlParser.cpp/g" > SqlParser.cpp
echo "const char* tname(int offset) { return yytname[offset]; }" >> SqlParser.cpp

./extract_tokens.sh > tokens.cpp
rm -f SqlParser.tab.h SqlParser.tab.c

#!/bin/sh
# generates parser and lexer code using bison and flex

dir=`dirname $0`
cd $dir
flex -oSqlScanner.cpp SqlScanner.l
bison -dv SqlParser.y
echo '#ifndef _SQLPARSER_H_
#define _SQLPARSER_H_
#include "Field.h"
#include "Parser.h"
#include "SqlTypes.h"

bool parseData(KexiDB::Parser *p, const char *data);' > SqlParser.h

cat SqlParser.tab.h >> SqlParser.h
echo '#endif' >> SqlParser.h

cat SqlParser.tab.c > SqlParser.cpp
echo "const char * const tname(int offset) { return yytname[offset]; }" >> SqlParser.cpp

./extract_tokens.sh > tokens.cpp
rm -f SqlParser.tab.h SqlParser.tab.c

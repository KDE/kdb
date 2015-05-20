#!/bin/bash
# Generates parser and lexer code using bison and flex

BISON_MIN=3.0.4      # keep updated for best results
BISON_MIN_NUM=30004  # keep updated for best results
FLEX_MIN=2.5.37      # keep updated for best results
FLEX_MIN_NUM=20537   # keep updated for best results

# Check minimum version of bison
bisonv=`bison --version | head -n 1| cut -f4 -d" "`
bisonv1=`echo $bisonv | cut -f1 -d.`
bisonv2=`echo $bisonv | cut -f2 -d.`
bisonv3=`echo $bisonv | cut -f3 -d.`
if [ -z $bisonv3 ] ; then bisonv3=0; fi
bisonvnum=`expr $bisonv1 \* 10000 + $bisonv2 \* 100 + $bisonv3`
if [ $bisonvnum -lt $BISON_MIN_NUM ] ; then
    echo "$bisonv is too old bison version, the minimum is $BISON_MIN."
    exit 1
fi

# Check minimum version of flex
flexv=`flex --version | head -n 1| cut -f2 -d" "`
flexv1=`echo $flexv | cut -f1 -d.`
flexv2=`echo $flexv | cut -f2 -d.`
flexv3=`echo $flexv | cut -f3 -d.`
flexvnum=`expr $flexv1 \* 10000 + $flexv2 \* 100 + $flexv3`
if [ $flexvnum -lt $FLEX_MIN_NUM ] ; then
    echo "$flexv is too old flex version, the minimum is $FLEX_MIN."
    exit 1
fi

# Generate lexer and parser
builddir=$PWD
srcdir=`dirname $0`
cd $srcdir

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

# Fine-tune the code: extra functions and remove trailing white space
cat KDbSqlParser.tab.h >> generated/sqlparser.h
echo '#endif' >> generated/sqlparser.h
sed --in-place 's/[[:space:]]\+$//;s/\t/        /g' generated/sqlparser.h

cat KDbSqlParser.tab.c | sed -e "s/KDbSqlParser\.tab\.c/KDbSqlParser.cpp/g" > generated/sqlparser.cpp
echo 'KDB_TESTING_EXPORT const char* tokenName(unsigned int offset) { return yytname[YYTRANSLATE(offset)]; }
KDB_TESTING_EXPORT unsigned int maxToken() { return YYMAXUTOK; }' >> generated/sqlparser.cpp
sed --in-place 's/[[:space:]]\+$//;s/\t/        /g' generated/sqlparser.cpp

# Extract table of SQL tokens
./extract_tokens.sh > generated/tokens.cpp
rm -f KDbSqlParser.tab.h KDbSqlParser.tab.c

#!/bin/sh

echo "/* WARNING! All changes made in this file will be lost! Run 'generate_parser_code.sh' instead. */
static const char* const _tokens[] = {"
for t in `grep  "\"[a-zA-Z_]*\"" KDbSqlScanner.l | sed -e 's/{.*}//g;s/{.*//g;s/[()|+ ]//g;s/\/\*.*\///g;s/""/"\n"/g;' | grep '^\".*\"$' | sort | uniq` ; do
    if [ "$t" = "ZZZ" ] ; then break ; fi
    echo '    '$t',';
done

echo "    0
};"

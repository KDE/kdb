#!/bin/sh

# Extract strings from all source files.
# EXTRACT_TR_STRINGS extracts strings with lupdate and convert them to .pot with
# lconvert.
LIST=`find . \( -path ./drivers/\* -o -path ./src/tools/KDbSimpleCommandLineApp.cpp \) -prune \
-o \( -name \*.cpp -o -name \*.h -o -name \*.ui -o -name \*.qml \) -type f -print`

$EXTRACT_TR_STRINGS $LIST -o $podir/kdb_qt.pot

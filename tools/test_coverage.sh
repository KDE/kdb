#!/bin/bash

# Inspiration from http://itk.org/gitweb?p=ITK.git;a=blob;f=Utilities/Maintenance/computeCodeCoverageLocally.sh;h=539e6c62c2dc2ba9f7c16b915120915fe192f67a;hb=HEAD

CMAKE_BINARY_DIR=`grep ^CMAKE_BINARY_DIR Makefile | sed 's/.*\= //'`
CMAKE_SOURCE_DIR=`grep ^CMAKE_SOURCE_DIR Makefile | sed 's/.*\= //'`

echo CMAKE_BINARY_DIR = $CMAKE_BINARY_DIR
echo CMAKE_SOURCE_DIR = $CMAKE_SOURCE_DIR

cd $CMAKE_BINARY_DIR/Predicate

rm -f app.info app.info2

lcov --directory . --zerocounters 

ctest || exit 1

lcov --directory . -b $CMAKE_SOURCE_DIR/Predicate/parser --capture --output-file app.info || exit 1
lcov --remove app.info 'Predicate/tests*' '*ThirdParty*' '/usr/*' --output-file  app.info2 || exit 1

genhtml --legend -f --demangle-cpp -o gcov_report app.info2

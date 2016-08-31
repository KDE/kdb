#!/bin/sh
# Invokes cppclean for the project.
# Run after successful building.
# Arguments: <build_dir>

set -e

build=$1
cd $build

src=$(grep SOURCE_DIR CMakeCache.txt | sed -e "s/.*=//")
sqlite_inc=$(grep SQLITE_INCLUDE_PATH CMakeCache.txt | sed -e "s/.*=//")

cppclean \
-I$src/autotests \
-I$src/src \
-I$src/src/expression \
-I$src/src/generated \
-I$src/src/parser/generated \
-I$src/src/interfaces \
-I$src/src/parser \
-I$src/src/sql \
-I$src/src/tools \
-I$src/src/views \
-I$build/src \
-I$build/autotests \
-I$build/src/drivers/mysql \
-I$build/src/drivers/postgresql \
-I$build/src/drivers/sqlite \
-I$sqlite_inc \
$src >&2 || true

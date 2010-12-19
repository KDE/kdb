#!/bin/sh
#
# Generates build files using CMake with Debug enabled and re-using
# KDE 4 directory $KDEDIRS.
#
# Usage:
#  0. Set $KDEDIRS
#  1. mkdir -p {predicate-build-dir}
#  2. cd {predicate-build-dir}
#  3. {predicate-source-dir}/tools/cmakepredicate.sh {predicate-source-dir}
#
# You need to type the above command only once to configure the CMake-based
# buildsystem.
#
# To build and install type:
#  make
#  make install
#

if [ -z "$KDEDIRS" ] ; then
    echo "Please set \$KDEDIRS"
    exit 1
fi

export PREDICATE_INSTALL_PREFIX=$KDEDIRS

cmd="cmake $1 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$PREDICATE_INSTALL_PREFIX \
    -DLIB_INSTALL_DIR=$PREDICATE_INSTALL_PREFIX/lib -DPLUGIN_INSTALL_DIR=lib/kde4/plugins \
    -DQT_PLUGINS_DIR=$PREDICATE_INSTALL_PREFIX/lib/kde4/plugins"

echo "------------------------------------------"
echo "Generating CMake files using this command:"
echo $cmd
echo
$cmd

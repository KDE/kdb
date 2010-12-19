#!/bin/sh
#
# Generates build files using CMake with Debug enabled and local PREFIX.
#
# Usage:
#  0. Set $PREDICATE_INSTALL_PREFIX, e.g. to $HOME/predicate/install
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

if [ -z "$PREDICATE_INSTALL_PREFIX" ] ; then
    echo "Please set \$PREDICATE_INSTALL_PREFIX"
    exit 1
fi

if [ -d "/usr/lib64" ] ; then
    c=lib64
else
    _libdir=lib
fi

mkdir -p $PREDICATE_INSTALL_PREFIX

cmd="cmake $1 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$PREDICATE_INSTALL_PREFIX \
    -DLIB_INSTALL_DIR=$PREDICATE_INSTALL_PREFIX/$_libdir -DPLUGIN_INSTALL_DIR=plugins \
    -DQT_PLUGINS_DIR=$PREDICATE_INSTALL_PREFIX/$_libdir/plugins"

echo "------------------------------------------"
echo "Generating CMake files using this command:"
echo $cmd
echo
$cmd

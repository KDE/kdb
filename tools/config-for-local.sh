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
# "Dry run" mode:
#  Add --dry-run parameter. This only displays the resulting cmake command
#  and exists without executing cmake. Useful when cmake arguments
#  are needed e.g. for configuring a Qt Creator project.
#
# You need to type the above command only once to configure the CMake-based
# buildsystem.
#
# To build and install type:
#  make
#  make install
#

if [ $# -lt 1 ] ; then
    echo "Usage: $0 {predicate-source-dir}"
    exit 1
fi

if [ -z "$PREDICATE_INSTALL_PREFIX" ] ; then
    echo "Please set \$PREDICATE_INSTALL_PREFIX"
    exit 1
fi

if [ `getconf LONG_BIT` -eq "64" ] ; then
    _libdir=lib64
else
    _libdir=lib
fi

mkdir -p $PREDICATE_INSTALL_PREFIX

cmd="cmake $1 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$PREDICATE_INSTALL_PREFIX \
    -DLIB_INSTALL_DIR=$PREDICATE_INSTALL_PREFIX/$_libdir -DPLUGIN_INSTALL_DIR=plugins \
    -DQT_PLUGINS_DIR=$PREDICATE_INSTALL_PREFIX/$_libdir/plugins"

echo "------------------------------------------"
echo "CMake will be executed using this command:"
echo
echo $cmd
if [ "$1" == "--dry-run" -o "$2" == "--dry-run" ] ; then
    exit 0
fi
echo
$cmd

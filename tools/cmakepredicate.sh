#!/bin/sh

# Generates build files using CMake, useful for installing predicate library and plugins
# to custom $KDEDIR location rather than global /usr/local.
# Usage:
#  cd {predicate-build-dir}
#  path/to/cmakepredicate.sh path/to/predicate
#
# then just type:
#  make
#  make install

cmd="cmake $1 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$KDEDIR \
	-DKDE4_BUILD_TESTS=ON \
	-DLIB_INSTALL_DIR=$KDEDIR/lib -DPLUGIN_INSTALL_DIR=lib/kde4/plugins \
	-DQT_PLUGINS_DIR=$KDEDIR/lib/kde4/plugins"

echo "------------------------------------------"
echo "Generating CMake files using this command:"
echo $cmd
echo
$cmd

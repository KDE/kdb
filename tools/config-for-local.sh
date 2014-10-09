#!/bin/sh
#
# Generates build files using CMake with Debug enabled and local PREFIX.
#
# Usage:
#  0. Set $PREDICATE_INSTALL_PREFIX environment variable to directory that
#      is indended PREFIX for the local Predicate installation,
#      e.g. to $HOME/predicate/inst
#  1. Pick location for your Predicate build directory {predicate-build-dir} and:
#     % mkdir -p {predicate-build-dir}
#  2. % cd {predicate-build-dir}
#  3. % {predicate-source-dir}/tools/config-for-local.sh {cmake-options}
#     ({predicate-source-dir} is your Predicate source code directory,
#      and {cmake-options} are extra cmake options as documented at 
#      http://www.cmake.org/cmake/help/v2.8.8/cmake.html#section_Options)
#
# "Dry run" mode:
#  Add --dry-run option. This only displays the resulting cmake command
#  and exists without executing cmake. Useful when cmake arguments
#  are needed e.g. for configuring a Qt Creator project.
#
# You need to type the above commands only once to configure the CMake-based
# buildsystem. However the script can be re-run safely.
# To completely reconfigure, remove the {predicate-build-dir} completely.
#
# To build and install type:
#  make
#  make install
#
# Detailed configuration and build instructions: https://community.kde.org/Predicate/Build
#

SOURCE_DIR=$(readlink -f $(dirname $0)/..)

if [ ! -d "${SOURCE_DIR}" ] ; then
    echo "Predicate source directory not found"
    exit 1
fi

if [ -z "${PREDICATE_INSTALL_PREFIX}" ] ; then
    echo "Please set \$PREDICATE_INSTALL_PREFIX, e.g. to \$HOME/predicate/inst"
    exit 1
fi

if [ `getconf LONG_BIT` -eq "64" ] ; then
    _libdir=lib64
else
    _libdir=lib
fi

mkdir -p "${PREDICATE_INSTALL_PREFIX}"

cmd="cmake $* -DCMAKE_INSTALL_PREFIX=${PREDICATE_INSTALL_PREFIX} \
    -DLIB_INSTALL_DIR=${PREDICATE_INSTALL_PREFIX}/${_libdir} \
    -DPLUGIN_INSTALL_DIR=${PREDICATE_INSTALL_PREFIX}/${_libdir}/plugins \
    -DQT_PLUGINS_DIR=${PREDICATE_INSTALL_PREFIX}/${_libdir}/plugins ${SOURCE_DIR}"

echo "------------------------------------------"
echo "CMake will be executed using this command:"
echo
echo $cmd
if [ "$1" = "--dry-run" ] ; then
    exit 0
fi
echo
$cmd

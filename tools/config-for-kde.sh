#!/bin/sh
#
# Generates build files using CMake with Debug enabled and re-using
# KDE 4 directory pointed by the $KDEDIRS environment variable.
#
# Usage:
#  0. Set $KDEDIRS environment variable to directory of your local
#     KDE installation (by convention it is $HOME/kde4/inst)
#  1. Pick location for your Predicate build directory {predicate-build-dir} and:
#     % mkdir -p {predicate-build-dir}
#  2. % cd {predicate-build-dir}
#  3. % {predicate-source-dir}/tools/config-for-kde.sh {cmake-options}
#     ({predicate-source-dir} is your Predicate source code directory,
#      and {cmake-options} are extra cmake options as documented at 
#      http://www.cmake.org/cmake/help/v2.8.8/cmake.html#section_Options)

# "Dry run" mode:
#  Add --dry-run option. This only displays the resulting cmake command
#  and exists without executing cmake. Useful when cmake arguments
#  are needed e.g. for configuring a Qt Creator project.
#
# You need to type the above commands only once to configure the CMake-based
# buildsystem. However the script can be re-run safely.
# To completely reconfigure, remove the {predicate-build-dir} completely before
# running the script or cmake again.
#
# To build and install type:
#  make
#  make install
#
# Detailed configuration and build instructions: https://community.kde.org/Predicate/Build
#

SOURCE_DIR=$(readlink -f $(dirname $0)/..)

if [ ! -d "$SOURCE_DIR" ] ; then
    echo "Predicate source directory not found"
    exit 1
fi

if [ -z "$KDEDIRS" ] ; then
    echo "Please set \$KDEDIRS"
    exit 1
fi

if [ `getconf LONG_BIT` -eq "64" ] ; then
    _libdir=lib64
else
    _libdir=lib
fi

export PREDICATE_INSTALL_PREFIX=${KDEDIRS}

cmd="cmake $* -DCMAKE_INSTALL_PREFIX=${PREDICATE_INSTALL_PREFIX} \
    -DLIB_INSTALL_DIR=${PREDICATE_INSTALL_PREFIX}/${_libdir} \
    -DPLUGIN_INSTALL_DIR=${PREDICATE_INSTALL_PREFIX}/${_libdir}/kde4/plugins \
    -DQT_PLUGINS_DIR=$PREDICATE_INSTALL_PREFIX/${_libdir}/kde4/plugins ${SOURCE_DIR}"

echo "------------------------------------------"
echo "CMake will be executed using this command:"
echo
echo $cmd
if [ "$1" = "--dry-run" ] ; then
    exit 0
fi
echo
$cmd

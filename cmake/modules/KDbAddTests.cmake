# Additional CMake macros
#
# Copyright (C) 2015-2017 Jarosław Staniek <staniek@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Note: the file must be included before KDEInstallDirs or add_tests() won't fully work

if(__kdb_add_tests)
  return()
endif()
set(__kdb_add_tests YES)

include(KDbAddSimpleOption)

# Adds BUILD_TESTING option to enable all kinds of tests. If enabled, build in autotests/
# and tests/ subdirectory is enabled. If optional argument ARG1 is ON, building tests will
# be ON by default. Otherwise building tests will be OFF. ARG1 is OFF by default.
# If tests are OFF, BUILD_COVERAGE is set to OFF.
# If tests are on BUILD_TESTING macro is defined.
macro(kdb_add_tests)
  if(KDE_INSTALL_TARGETS_DEFAULT_ARGS)
      message(FATAL_ERROR "Include before KDEInstallDirs!")
  endif()
  if (NOT "${ARG1}" STREQUAL "ON")
    set(_SET OFF)
  endif()
  simple_option(BUILD_TESTING "Build tests" ${_SET}) # override default from CTest.cmake
  if(BUILD_TESTING)
    add_definitions(-DBUILD_TESTING)
    include(CTest)
  else()
    if (BUILD_COVERAGE)
        set(BUILD_COVERAGE OFF)
        message(STATUS "Building with gcov support disabled because BUILD_TESTING is OFF")
    endif()
  endif()
endmacro()

# Additional CMake macros
#
# Copyright (C) 2015-2017 Jarosław Staniek <staniek@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(__kdb_macros)
  return()
endif()
set(__kdb_macros YES)

include(GenerateExportHeader)
include(GetGitRevisionDescription)
include(KDbAddSimpleOption)

string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)
string(COMPARE EQUAL "${CMAKE_CXX_COMPILER_ID}" "Clang" CMAKE_COMPILER_IS_CLANG)

# x.81.z or larger means test release, so the stable major version is x+1
if(PROJECT_VERSION_MINOR GREATER 80)
    set(PROJECT_UNSTABLE ON)
    math(EXPR PROJECT_STABLE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR} + 1")
    set(PROJECT_STABLE_VERSION_MINOR 0)
    set(PROJECT_STABLE_VERSION_PATCH 0)
# x.y.81 or larger means test release, so the stable minor version is y+1
elseif(PROJECT_VERSION_PATCH GREATER 80)
    set(PROJECT_STABLE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    math(EXPR PROJECT_STABLE_VERSION_MINOR "${PROJECT_VERSION_MINOR} + 1")
    set(PROJECT_STABLE_VERSION_PATCH 0)
else()
    set(PROJECT_STABLE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    set(PROJECT_STABLE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(PROJECT_STABLE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
endif()

# Fetches git revision and branch from the source dir of the current build if possible.
# Sets ${PROJECT_NAME_UPPER}_GIT_SHA1_STRING and ${PROJECT_NAME_UPPER}_GIT_BRANCH_STRING variables.
# If git information is not available but ${CMAKE_SOURCE_DIR}/GIT_VERSION file exists,
# it is parsed. This file can be created by scripts while preparing tarballs and is
# supposed to contain two lines: hash and branch.
macro(get_git_revision_and_branch)
  set(${PROJECT_NAME_UPPER}_GIT_SHA1_STRING "")
  set(${PROJECT_NAME_UPPER}_GIT_BRANCH_STRING "")
  get_git_head_revision(GIT_REFSPEC ${PROJECT_NAME_UPPER}_GIT_SHA1_STRING)
  get_git_branch(${PROJECT_NAME_UPPER}_GIT_BRANCH_STRING)
  if(NOT ${PROJECT_NAME_UPPER}_GIT_SHA1_STRING OR NOT ${PROJECT_NAME_UPPER}_GIT_BRANCH_STRING)
    if(EXISTS "${CMAKE_SOURCE_DIR}/GIT_VERSION")
      file(READ "${CMAKE_SOURCE_DIR}/GIT_VERSION" _ver)
      string(REGEX REPLACE "\n" ";" _ver "${_ver}")
      list(GET _ver 0 ${PROJECT_NAME_UPPER}_GIT_SHA1_STRING)
      list(GET _ver 1 ${PROJECT_NAME_UPPER}_GIT_BRANCH_STRING)
    endif()
  endif()
  if(${PROJECT_NAME_UPPER}_GIT_SHA1_STRING OR ${PROJECT_NAME_UPPER}_GIT_BRANCH_STRING)
    string(SUBSTRING ${${PROJECT_NAME_UPPER}_GIT_SHA1_STRING} 0 7 ${PROJECT_NAME_UPPER}_GIT_SHA1_STRING)
  else()
    set(${PROJECT_NAME_UPPER}_GIT_SHA1_STRING "")
    set(${PROJECT_NAME_UPPER}_GIT_BRANCH_STRING "")
  endif()
endmacro()

# Adds ${PROJECT_NAME_UPPER}_UNFINISHED option. If it is ON, unfinished features
# (useful for testing but may confuse end-user) are compiled-in.
# This option is OFF by default.
macro(add_unfinished_features_option)
  simple_option(${PROJECT_NAME_UPPER}_UNFINISHED
                "Include unfinished features (useful for testing but may confuse end-user)" OFF)
endmacro()

# Adds commands that generate ${_filename}${PROJECT_STABLE_VERSION_MAJOR}.pc file
# out of ${_filename}.pc.cmake file and installs the .pc file to ${LIB_INSTALL_DIR}/pkgconfig.
# These commands are not executed for WIN32.
# ${CMAKE_SOURCE_DIR}/${_filename}.pc.cmake should exist.
macro(add_pc_file _filename)
  if (NOT WIN32)
    set(_name ${_filename}${PROJECT_STABLE_VERSION_MAJOR})
    configure_file(${CMAKE_SOURCE_DIR}/${_filename}.pc.cmake ${CMAKE_BINARY_DIR}/${_name}.pc @ONLY)
    install(FILES ${CMAKE_BINARY_DIR}/${_name}.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
  endif()
endmacro()

# Sets detailed version information for library co-installability.
# - adds PROJECT_VERSION_MAJOR to the lib name
# - sets VERSION and SOVERSION to PROJECT_VERSION_MAJOR.PROJECT_VERSION_MINOR
# - sets ${_target_upper}_BASE_NAME variable to the final lib name
# - sets ${_target_upper}_BASE_NAME_LOWER variable to the final lib name, lowercase
# - sets ${_target_upper}_INCLUDE_INSTALL_DIR to include dir for library headers
# - (where _target_upper is uppercase ${_target}
macro(set_coinstallable_lib_version _target)
    set(_name ${_target}${PROJECT_STABLE_VERSION_MAJOR})
    set_target_properties(${_target}
        PROPERTIES VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
                   SOVERSION ${PROJECT_VERSION_MAJOR}
                   EXPORT_NAME ${_target}
                   OUTPUT_NAME ${_name}
    )
    string(TOUPPER ${_target} _target_upper)
    string(TOUPPER ${_target_upper}_BASE_NAME _var)
    set(${_var} ${_name})
    string(TOLOWER ${_name} ${_var}_LOWER)
    set(${_target_upper}_INCLUDE_INSTALL_DIR ${INCLUDE_INSTALL_DIR}/${_name})
    unset(_target_upper)
    unset(_var)
endmacro()

#set(CMAKE_DISABLE_SOURCE_CHANGES ON)

cmake_minimum_required(VERSION 2.8)

project( Predicate )

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

include(MacroEnsureOutOfSourceBuild)
MACRO_ENSURE_OUT_OF_SOURCE_BUILD()

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

# define the generic version of the Predicate libraries here
# this makes it easy to advance it when the next Predicate release comes
set(PREDICATE_VERSION_MAJOR "00")
set(GENERIC_PREDICATE_LIB_SOVERSION "0") # like PREDICATE_VERSION_MAJOR
set(PREDICATE_VERSION_MINOR "00")
set(PREDICATE_VERSION_RELEASE "1")
set(PREDICATE_VERSION "0.0.1")
set(PREDICATE_VERSION_STRING "${PREDICATE_VERSION}")

set(GENERIC_PREDICATE_LIB_VERSION ${PREDICATE_VERSION})

# If Git is installed and a '.git' directory is found,
# we append the Git revision to AKONADI_VERSION_STRING
if(EXISTS "${Predicate_SOURCE_DIR}/.git")
  find_package(Git)
  if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                    WORKING_DIRECTORY ${Predicate_SOURCE_DIR}
                    OUTPUT_VARIABLE predicate_git_revision)
    string(REGEX REPLACE "\n" "" predicate_git_revision "${predicate_git_revision}")
    set(PREDICATE_VERSION_STRING "${PREDICATE_VERSION_STRING} (revision: ${predicate_git_revision})")
  endif()
endif()

SET(BIN_INSTALL_DIR
    "${CMAKE_INSTALL_PREFIX}/bin"
    CACHE PATH "The subdirectory relative to the install prefix for executables (default is bin)"
)

SET(LIB_INSTALL_DIR
    "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}"
    CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is lib${LIB_SUFFIX})."
)

if(WIN32)
  SET(PLUGIN_INSTALL_BASE_DIR ${BIN_INSTALL_DIR})
else(WIN32)
  SET(PLUGIN_INSTALL_BASE_DIR lib${LIB_SUFFIX})
endif(WIN32)

SET(PLUGIN_INSTALL_DIR
    "${PLUGIN_INSTALL_BASE_DIR}/Predicate/plugins"
    CACHE PATH "The subdirectory relative to the install prefix where plugins will be installed (default is ${PLUGIN_INSTALL_BASE_DIR}/Predicate/plugins)."
)

if (NOT WIN32)
  configure_file(${Predicate_SOURCE_DIR}/predicate.pc.cmake ${Predicate_BINARY_DIR}/predicate.pc @ONLY)
endif (NOT WIN32)

include(MacroOptionalFindPackage)
include(MacroOptionalAddSubdirectory)
include(InstallSettings)
include(MacroLogFeature)
include(FeatureSummary)
include(CreateSharedDataClasses)

macro(add_simple_feature_info _NAME _DESCRIPTION _DEFAULT)
  if ("${_DEFAULT}" STREQUAL "${${_NAME}}")
    set(_STATUS " (default value)")
  else ()
    set(_STATUS "")
  endif ()
  add_feature_info(${_NAME} ${_NAME} ${_DESCRIPTION}${_STATUS})
endmacro(add_simple_feature_info)

macro(simple_option _NAME _DESCRIPTION _DEFAULT)
  option(${_NAME} ${_DESCRIPTION} ${_DEFAULT})
  add_simple_feature_info(${_NAME} ${_DESCRIPTION} ${_DEFAULT})
endmacro(simple_option)

# Qt string handling optimizations: http://qt-project.org/doc/qt-4.8/qstring.html#more-efficient-string-construction
add_definitions(-DQT_USE_QSTRINGBUILDER -DQT_NO_CAST_TO_ASCII -DQT_NO_CAST_FROM_ASCII)

# Increase compatbility with Qt
add_definitions(-DQT_NO_KEYWORD)

#### Qt4 ####
set(QT_MIN_VERSION 4.7.0)
find_package(Qt4 REQUIRED)

include_directories(
  ${Predicate_SOURCE_DIR}
  ${Predicate_SOURCE_DIR}/Predicate
  ${Predicate_SOURCE_DIR}/Predicate/includes
  ${Predicate_SOURCE_DIR}/Predicate/expression
  ${Predicate_SOURCE_DIR}/Predicate/parser
  ${Predicate_SOURCE_DIR}/Predicate/views
  ${Predicate_BINARY_DIR}
  ${Predicate_BINARY_DIR}/Predicate
  ${QT_INCLUDES}
)

#### Subdirs ####
add_subdirectory( Predicate )
add_subdirectory( Drivers )

simple_option(PREDICATE_UNFINISHED "Include unfinished features (useful for testing but may confuse end-user)" OFF)

simple_option(BUILD_TESTING "Building of tests" OFF) # override default from CTest.cmake
if (BUILD_TESTING)
  include(CTest)
  add_definitions(-DBUILD_TESTING)
  simple_option(BUILD_TEST_COVERAGE "Building of test coverage" OFF)
  if (BUILD_TEST_COVERAGE)
    set(CMAKE_CXX_FLAGS "-g -O0 -Wall -W -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage")
  endif (BUILD_TEST_COVERAGE)
  add_subdirectory( tests )
else ()
  set(BUILD_TEST_COVERAGE OFF)
  simple_option(BUILD_TEST_COVERAGE "Building of test coverage (disabled because BUILD_TESTING is OFF)" OFF)
endif ()

macro_display_feature_log()

############### install stuff ###############

if(NOT WIN32)
  install(FILES ${Predicate_BINARY_DIR}/predicate.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
endif(NOT WIN32)

feature_summary(WHAT ALL)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
project(KDb)

# ECM setup
find_package(ECM 1.8.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDbGenerateHeaders)
include(ECMGeneratePriFile)
include(ECMPackageConfigHelpers)
include(ECMPoQmTools)
include(ECMSetupVersion)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

include(FeatureSummary)
include(GenerateExportHeader)

include(MacroOptionalFindPackage)
include(MacroOptionalAddSubdirectory)
include(InstallSettings)
include(MacroLogFeature)
include(KDbCreateSharedDataClasses)

set(KDB_VERSION "2.96.0") # Update this

# define the generic version of the KDb framework here
# this makes it easy to advance for the next KDb release
set(KDB_VERSION_STRING "3.0 Pre-Alpha")
set(KDB_VERSION_MAJOR 2)
set(KDB_VERSION_MINOR 96)
set(KDB_VERSION_RELEASE 0)
set(GENERIC_KDB_LIB_VERSION_MAJOR "2")

set(GENERIC_KDB_LIB_VERSION "${GENERIC_KDB_LIB_VERSION_MAJOR}.96.0")
set(GENERIC_KDB_LIB_SOVERSION "${GENERIC_KDB_LIB_VERSION_MAJOR}")

# If Git is installed and a '.git' directory is found,
# we append the Git revision to AKONADI_VERSION_STRING
set(KDB_GIT_SHA1_STRING "")
if(EXISTS "${KDb_SOURCE_DIR}/.git")
  find_package(Git)
  if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                    WORKING_DIRECTORY ${KDb_SOURCE_DIR}
                    OUTPUT_VARIABLE KDB_GIT_SHA1_STRING)
    string(REGEX REPLACE "\n" "" KDB_GIT_SHA1_STRING "${KDB_GIT_SHA1_STRING}")
  endif()
endif()

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

# Dependencies
set(REQUIRED_QT_VERSION "5.2.0")

# Required Qt5 components to build this framework
find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Core Gui Widgets)

#set(CMAKE_AUTOMOC ON)
#set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(tests)

# SET(BIN_INSTALL_DIR
#     "${CMAKE_INSTALL_PREFIX}/bin"
#     CACHE PATH "The subdirectory relative to the install prefix for executables (default is bin)"
# )

# SET(LIB_INSTALL_DIR
#     "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}"
#     CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is lib${LIB_SUFFIX})."
# )

# if(WIN32)
#   SET(PLUGIN_INSTALL_BASE_DIR ${BIN_INSTALL_DIR})
# else(WIN32)
#   SET(PLUGIN_INSTALL_BASE_DIR lib${LIB_SUFFIX})
# endif(WIN32)

# SET(PLUGIN_INSTALL_DIR
#     "${PLUGIN_INSTALL_BASE_DIR}/KDb/plugins"
#     CACHE PATH "The subdirectory relative to the install prefix where plugins will be installed (default is ${PLUGIN_INSTALL_BASE_DIR}/KDb/plugins)."
# )

if (NOT WIN32)
  configure_file(${KDb_SOURCE_DIR}/kdb.pc.cmake ${KDb_BINARY_DIR}/kdb.pc @ONLY)
endif (NOT WIN32)

simple_option(KDB_UNFINISHED
              "Include unfinished features (useful for testing but may confuse end-user)" OFF)

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
else ()
  set(BUILD_TEST_COVERAGE OFF)
  simple_option(BUILD_TEST_COVERAGE "Building of test coverage (disabled because BUILD_TESTING is OFF)" OFF)
endif ()

# Install
if(NOT WIN32)
  install(FILES ${KDb_BINARY_DIR}/kdb.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
endif(NOT WIN32)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

macro_display_feature_log()

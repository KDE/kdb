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
include(ECMInstallIcons)
include(ECMOptionalAddSubdirectory)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

include(FeatureSummary)
include(GenerateExportHeader)

include(InstallSettings)
include(MacroLogFeature)
include(KDbCreateSharedDataClasses)

set(KDB_VERSION "2.96.0") # Update this

# Dependencies
set(REQUIRED_QT_VERSION "5.2.0")

# Required components to build this framework
# TODO move Widgets-dependent part to a libKDbWidgets
find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Core Widgets Xml)
find_package(KF5 5.7.0 REQUIRED CoreAddons)

find_package(ICU REQUIRED)

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

# If Git is installed and a '.git' directory is found set KDB_GIT_SHA1_STRING
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

if (BUILD_TESTING)
  add_definitions(-DBUILD_TESTING)
  include(CTest)
  add_subdirectory(autotests)
  add_subdirectory(tests)
else ()
  set(BUILD_COVERAGE OFF)
  simple_option(BUILD_COVERAGE "Build test coverage (disabled because BUILD_TESTING is OFF)" OFF)
endif ()

simple_option(BUILD_EXAMPLES "Build example applications" OFF)
if (BUILD_EXAMPLES)
    #add_subdirectory(examples)
endif()

add_subdirectory(src)

simple_option(KDB_UNFINISHED
              "Include unfinished features (useful for testing but may confuse end-user)" OFF)

simple_option(BUILD_TESTING "Build tests" OFF) # override default from CTest.cmake

if (NOT WIN32)
  configure_file(${KDb_SOURCE_DIR}/KDb.pc.cmake ${KDb_BINARY_DIR}/KDb.pc @ONLY)
  install(FILES ${KDb_BINARY_DIR}/KDb.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

macro_display_feature_log()

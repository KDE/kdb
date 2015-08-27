cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
project(KDb)

# ECM setup
find_package(ECM 1.8.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

if(POLICY CMP0063) # Honor visibility properties for all target types (since cmake 3.3)
    cmake_policy(SET CMP0063 OLD)
endif()

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

include(GenerateExportHeader)
include(MacroLogFeature)
include(KDbCreateSharedDataClasses)
include(KDbMacros)
string(COMPARE EQUAL "${CMAKE_CXX_COMPILER_ID}" "Clang" CMAKE_COMPILER_IS_CLANG)

set(KDB_VERSION "2.96.0") # Update this

# Dependencies
set(REQUIRED_QT_VERSION "5.2.0")

# Required components to build this framework
# TODO move Widgets-dependent part to a libKDbWidgets
find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Core Widgets Xml)
find_package(KF5 5.7.0 REQUIRED CoreAddons)
find_package(ICU REQUIRED)

get_git_revision_and_branch()
add_tests()
add_examples()
add_unfinished_features_option()
add_pc_file()

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
macro_display_feature_log()

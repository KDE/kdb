cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
cmake_policy(SET CMP0048 NEW)
project(KDb VERSION 2.96.7) # Update this

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

# Dependencies
set(REQUIRED_QT_VERSION 5.4.0)
set(REQUIRED_KF5_VERSION 5.16.0)

# Required components to build this framework
# TODO move Widgets-dependent part to a libKDbWidgets
find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Core Widgets Xml)
find_package(KF5 ${REQUIRED_KF5_VERSION} REQUIRED CoreAddons)
find_package(ICU REQUIRED)

get_git_revision_and_branch()
add_tests()
add_examples()
add_unfinished_features_option()
add_pc_file()

add_subdirectory(src)

add_custom_target(cppclean
    COMMAND ${CMAKE_SOURCE_DIR}/tools/cppclean-kdb.sh ${CMAKE_BINARY_DIR}
    COMMENT "Running CPPCLEAN"
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
macro_display_feature_log()

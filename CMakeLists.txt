cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
find_package(ECM 1.8.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
include(SetKDbCMakePolicies NO_POLICY_SCOPE)

project(KDb VERSION 3.0.91) # Update this

# CMake
include(CMakePackageConfigHelpers)
include(GenerateExportHeader)

# ECM
include(ECMGeneratePriFile)
include(ECMInstallIcons)
include(ECMOptionalAddSubdirectory)
include(ECMPoQmTools)
include(ECMSetupVersion)
include(KDEInstallDirs)
include(KDECMakeSettings NO_POLICY_SCOPE)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

# Own
include(KDbCreateSharedDataClasses)
include(KDbGenerateHeaders)
include(KDbMacros)
include(KDbAddQCH)

simple_option(BUILD_QCH "Build API documentation in QCH format" OFF)

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
add_pc_file(${PROJECT_NAME})

add_subdirectory(src)
add_subdirectory(tools)

add_custom_target(cppclean
    COMMAND ${CMAKE_SOURCE_DIR}/tools/cppclean-kdb.sh ${CMAKE_BINARY_DIR}
    COMMENT "Running CPPCLEAN"
)

if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ecm_install_po_files_as_qm(po)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

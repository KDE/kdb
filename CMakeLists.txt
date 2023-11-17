cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# Dependencies
set(REQUIRED_QT_VERSION 5.15.0)
set(REQUIRED_KF_VERSION 5.100.0)

find_package(ECM ${REQUIRED_KF_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
include(SetKDbCMakePolicies NO_POLICY_SCOPE)

project(KDb VERSION 3.2.90) # Update this: Alpha = 90, Beta 1 = 91, RC 1 = 94

include(KDbAddTests)
include(KDbAddExamples)
kdb_add_tests(OFF)
kdb_add_examples(OFF)

# ECM
include(KDEInstallDirs)
include(KDECMakeSettings NO_POLICY_SCOPE)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(ECMGeneratePriFile)
include(ECMInstallIcons)
include(ECMOptionalAddSubdirectory)
include(ECMPoQmTools)
include(ECMSetupVersion)
include(ECMCoverageOption)
include(ECMSetupQtPluginMacroNames)
include(ECMAddQch)

# Own
include(KDbCreateSharedDataClasses)
include(KDbGenerateHeaders)
include(KDbMacros)

simple_option(BUILD_QCH "Build API documentation in QCH format" OFF)


# Required components to build this framework
# TODO move Widgets-dependent part to a libKDbWidgets
find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Core Widgets Xml)
find_package(KF${QT_MAJOR_VERSION} ${REQUIRED_KF_VERSION} REQUIRED CoreAddons)
find_package(ICU REQUIRED i18n uc)
set_package_properties(ICU PROPERTIES
                       PURPOSE "Required by KDb for unicode-aware string comparisons")

get_git_revision_and_branch()
add_unfinished_features_option()

add_subdirectory(src)
add_subdirectory(tools)

if(BUILD_TESTING)
  add_subdirectory(autotests)
  add_subdirectory(tests)
endif()

add_custom_target(cppclean
    COMMAND ${CMAKE_SOURCE_DIR}/tools/cppclean-kdb.sh ${CMAKE_BINARY_DIR}
    COMMENT "Running CPPCLEAN"
)

if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/poqm")
    ecm_install_po_files_as_qm(poqm)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

project( Predicate )

cmake_minimum_required(VERSION 2.6)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_MODULE_PATH ${Predicate_SOURCE_DIR}/cmake/modules)

# define the generic version of the Predicate libraries here
# this makes it easy to advance it when the next Predicate release comes
set(PREDICATE_VERSION_MAJOR "1")
set(PREDICATE_VERSION_MINOR "00")
set(PREDICATE_VERSION_RELEASE "00")
set(PREDICATE_VERSION "1.0.0")
#todo add build date as in kdepim
set(PREDICATE_VERSION_STRING "${PREDICATE_VERSION}")
set(GENERIC_PREDICATE_LIB_VERSION ${PREDICATE_VERSION})
set(GENERIC_PREDICATE_LIB_SOVERSION ${PREDICATE_VERSION_MAJOR})

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
include(InstallSettings)
include(MacroLogFeature)
include(CreateSharedDataClasses)

#### Qt4 ####
set(QT_MIN_VERSION 4.5.0)
find_package(Qt4 REQUIRED)

#### Subdirs ####
add_subdirectory( Predicate )
add_subdirectory( Drivers )
add_subdirectory( tests )

macro_display_feature_log()

############### install stuff ###############

if(NOT WIN32)
  install(FILES ${Predicate_BINARY_DIR}/predicate.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
endif(NOT WIN32)

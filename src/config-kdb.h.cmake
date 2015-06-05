#ifndef KDB_CONFIG_H
#define KDB_CONFIG_H

/* config-kdb.h. Generated by cmake from config-kdb.h.cmake */

/*! @file config-kdb.h
    Global KDb configuration (build time)
*/

//! @def KDB_GIT_SHA1_STRING
//! @brief Indicates the git sha1 commit which was used for compilation of KDb
#cmakedefine KDB_GIT_SHA1_STRING "@KDB_GIT_SHA1_STRING@"

//! @def BIN_INSTALL_DIR
//! @brief The subdirectory relative to the install prefix for executables.
#define BIN_INSTALL_DIR "${BIN_INSTALL_DIR}"

//! @def KDB_TESTING_EXPORT
//! @brief Export symbols for testing
#ifdef BUILD_TESTING
#  define KDB_TESTING_EXPORT KDB_EXPORT
#else
#  define KDB_TESTING_EXPORT
#endif

//! @def KDB_EXPRESSION_DEBUG
//! @brief Defined if debugging for expressions is enabled
#cmakedefine KDB_EXPRESSION_DEBUG

//! @def KDB_DRIVERMANAGER_DEBUG
//! @brief Defined if debugging for the driver manager is enabled
#cmakedefine KDB_DRIVERMANAGER_DEBUG

//! @def KDB_DEBUG_GUI
//! @brief Defined if a GUI for debugging is enabled
#cmakedefine KDB_DEBUG_GUI

//! @def KDB_UNFINISHED
//! @brief Defined if unfinished features of KDb are enabled
#cmakedefine KDB_UNFINISHED

#endif

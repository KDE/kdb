# - Try to find MySQL / MySQL Embedded library
# Find the MySQL includes and client library
# This module defines
#  MYSQL_INCLUDE_DIR, where to find mysql.h
#  MYSQL_LIBRARIES, the libraries needed to use MySQL.
#  MYSQL_LIB_DIR, path to the MYSQL_LIBRARIES
#  MYSQL_EMBEDDED_LIBRARIES, the libraries needed to use MySQL Embedded.
#  MYSQL_EMBEDDED_LIB_DIR, path to the MYSQL_EMBEDDED_LIBRARIES
#  MySQL_FOUND, If false, do not try to use MySQL.
#  MySQL_Embedded_FOUND, If false, do not try to use MySQL Embedded.
#  MYSQL_USING_MARIADB, If true MariaDB has been found and will be used as a replacement for MySQL

# Copyright (c) 2006-2018, Jarosław Staniek <staniek@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(CheckCXXSourceCompiles)
include(MacroPushRequiredVars)
include(FeatureSummary)
set_package_properties(MySQL PROPERTIES
    DESCRIPTION "MySQL Client Library (libmysqlclient)" URL "http://www.mysql.com")

set(MYSQL_USING_MARIADB FALSE)

if(WIN32)
   set(ProgramFilesX "ProgramFiles(x86)")
   set(MYSQL_INCLUDE_PATHS
      $ENV{MYSQL_INCLUDE_DIR}
      $ENV{MYSQL_DIR}/include
      $ENV{ProgramW6432}/MySQL/*/include
      $ENV{ProgramFiles}/MySQL/*/include
      $ENV{${ProgramFilesX}}/MySQL/*/include
      $ENV{SystemDrive}/MySQL/*/include
      $ENV{ProgramW6432}/*/include
      $ENV{ProgramFiles}/*/include # MariaDB
      $ENV{${ProgramFilesX}}/*/include # MariaDB
   )
   set(MYSQL_INCLUDE_PATH_SUFFIXES mysql)
   # First try to identify MariaDB.
   # Once we find headers of MariaDB we will look for MariaDB and mot MySQL.
   # This way we avoid mixing MariaDB and MySQL files.
   find_path(_MARIADB_INCLUDE_DIR mariadb_version.h
      PATHS ${MYSQL_INCLUDE_PATHS}
      PATH_SUFFIXES ${MYSQL_INCLUDE_PATH_SUFFIXES}
   )
   if(_MARIADB_INCLUDE_DIR)
      set(MYSQL_USING_MARIADB TRUE)
      unset(_MARIADB_INCLUDE_DIR)
   endif()
   # now a real find
   find_path(MYSQL_INCLUDE_DIR mysql.h
      PATHS ${MYSQL_INCLUDE_PATHS}
      PATH_SUFFIXES ${MYSQL_INCLUDE_PATH_SUFFIXES}
   )
else()
   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   find_package(PkgConfig)
   pkg_check_modules(PC_MYSQL QUIET mysql mariadb)
   if(PC_MYSQL_VERSION)
       set(MySQL_VERSION_STRING ${PC_MYSQL_VERSION})
   endif()

   find_path(MYSQL_INCLUDE_DIR mysql.h
      PATHS
      $ENV{MYSQL_INCLUDE_DIR}
      $ENV{MYSQL_DIR}/include
      ${PC_MYSQL_INCLUDEDIR}
      ${PC_MYSQL_INCLUDE_DIRS}
      /usr/local/mysql/include
      /opt/mysql/mysql/include
      PATH_SUFFIXES
      mysql
   )
endif()

if(WIN32)
   string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOLOWER)

   # path suffix for debug/release mode
   # binary_dist: mysql binary distribution
   # build_dist: custom build
   if(CMAKE_BUILD_TYPE_TOLOWER MATCHES "debug")
      set(binary_dist debug)
      set(build_dist Debug)
   else()
      add_definitions(-DDBUG_OFF)
      set(binary_dist opt)
      set(build_dist Release)
   endif()

   if(MYSQL_USING_MARIADB)
       set(MYSQL_LIB_PATHS
          $ENV{ProgramW6432}/*/lib # MariaDB
          $ENV{ProgramFiles}/*/lib # MariaDB
          $ENV{${ProgramFilesX}}/*/lib # MariaDB
       )
      find_library(_LIBMYSQL_LIBRARY NAMES libmariadb
         PATHS ${MYSQL_LIB_PATHS}
      )
      find_library(_MYSQLCLIENT_LIBRARY NAMES mariadbclient
         PATHS ${MYSQL_LIB_PATHS}
      )
      if(_LIBMYSQL_LIBRARY AND _MYSQLCLIENT_LIBRARY)
         # once we find one MariaDB component, always search for MariaDB, not MySQL components
         set(MYSQL_LIBRARIES ${_LIBMYSQL_LIBRARY} ${_MYSQLCLIENT_LIBRARY})
      endif()
   else() # mysql
       set(MYSQL_LIB_PATHS
          $ENV{MYSQL_DIR}/lib/${binary_dist}
          $ENV{MYSQL_DIR}/libmysql/${build_dist}
          $ENV{MYSQL_DIR}/client/${build_dist}
          $ENV{ProgramW6432}/MySQL/*/lib/${binary_dist}
          $ENV{ProgramFiles}/MySQL/*/lib/${binary_dist}
          $ENV{${ProgramFilesX}}/MySQL/*/lib/${binary_dist}
          $ENV{SystemDrive}/MySQL/*/lib/${binary_dist}
       )
       find_library(_LIBMYSQL_LIBRARY NAMES libmysql
          PATHS ${MYSQL_LIB_PATHS}
       )
       find_library(_MYSQLCLIENT_LIBRARY NAMES mysqlclient
          PATHS ${MYSQL_LIB_PATHS}
       )
       if(_LIBMYSQL_LIBRARY AND _MYSQLCLIENT_LIBRARY)
          set(MYSQL_LIBRARIES ${_LIBMYSQL_LIBRARY} ${_MYSQLCLIENT_LIBRARY})
       endif()
   endif()
else() # !win32
   find_library(_MYSQLCLIENT_LIBRARY NAMES mysqlclient
      PATHS
      $ENV{MYSQL_DIR}/libmysql_r/.libs
      $ENV{MYSQL_DIR}/lib
      $ENV{MYSQL_DIR}/lib/mysql
      ${PC_MYSQL_LIBDIR}
      ${PC_MYSQL_LIBRARY_DIRS}
      PATH_SUFFIXES
      mysql
   )
   set(MYSQL_LIBRARIES ${_MYSQLCLIENT_LIBRARY})
   # TODO: set MYSQL_USING_MARIADB if MariaDB found
endif()

if(_LIBMYSQL_LIBRARY)
   get_filename_component(MYSQL_LIB_DIR ${_LIBMYSQL_LIBRARY} PATH)
   unset(_LIBMYSQL_LIBRARY)
endif()
if(_MYSQLCLIENT_LIBRARY)
    if(NOT MYSQL_LIB_DIR)
        get_filename_component(MYSQL_LIB_DIR ${_MYSQLCLIENT_LIBRARY} PATH)
    endif()
    unset(_MYSQLCLIENT_LIBRARY)
endif()

find_library(MYSQL_EMBEDDED_LIBRARIES NAMES mysqld
   PATHS
   ${MYSQL_LIB_PATHS}
)

if(MYSQL_EMBEDDED_LIBRARIES)
   get_filename_component(MYSQL_EMBEDDED_LIB_DIR ${MYSQL_EMBEDDED_LIBRARIES} PATH)

    macro_push_required_vars()
    set( CMAKE_REQUIRED_INCLUDES ${MYSQL_INCLUDE_DIR} )
    set( CMAKE_REQUIRED_LIBRARIES ${MYSQL_EMBEDDED_LIBRARIES} )
    check_cxx_source_compiles( "#include <mysql.h>\nint main() { int i = MYSQL_OPT_USE_EMBEDDED_CONNECTION; }" HAVE_MYSQL_OPT_EMBEDDED_CONNECTION )
    macro_pop_required_vars()
endif()

# Did we find anything?
include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(MySQL
    REQUIRED_VARS MYSQL_LIBRARIES MYSQL_INCLUDE_DIR MYSQL_LIB_DIR
    VERSION_VAR MySQL_VERSION_STRING)
if(MYSQL_EMBEDDED_LIBRARIES AND MYSQL_EMBEDDED_LIB_DIR AND HAVE_MYSQL_OPT_EMBEDDED_CONNECTION)
    find_package_handle_standard_args(MySQL_Embedded
                                  REQUIRED_VARS MYSQL_EMBEDDED_LIBRARIES MYSQL_INCLUDE_DIR
                                                MYSQL_EMBEDDED_LIB_DIR
                                                HAVE_MYSQL_OPT_EMBEDDED_CONNECTION)
endif()

mark_as_advanced(MYSQL_INCLUDE_DIR MYSQL_LIBRARIES MYSQL_LIB_DIR
                 MYSQL_EMBEDDED_LIBRARIES MYSQL_EMBEDDED_LIB_DIR HAVE_MYSQL_OPT_EMBEDDED_CONNECTION
                 MYSQL_USING_MARIADB)

if(MySQL_FOUND)
   set(MySQL_FOUND 1 CACHE INTERNAL "" FORCE)
    if(NOT MYSQL_FIND_QUIETLY)
       if(MYSQL_USING_MARIADB)
          message(STATUS "Found MariaDB, using as replacement for MySQL")
       endif()
    endif()
else()
   unset(MySQL_FOUND CACHE)
   set(MySQL_FOUND FALSE)
   unset(MYSQL_LIBRARIES)
   unset(MYSQL_LIBRARIES CACHE)
   unset(MYSQL_INCLUDE_DIR)
   unset(MYSQL_INCLUDE_DIR CACHE)
   unset(MYSQL_LIB_DIR)
   unset(MYSQL_LIB_DIR CACHE)
   unset(MySQL_VERSION_STRING)
   unset(MySQL_VERSION_STRING CACHE)
endif()

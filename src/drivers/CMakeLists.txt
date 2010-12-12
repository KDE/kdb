if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

#TODO  add_definitions(-DKDE_DEFAULT_DEBUG_AREA=44001)

set(PREDICATE_PLUGINS_DIR ${QT_PLUGINS_DIR}/predicate)
include_directories(
  ${CMAKE_SOURCE_DIR}
  ${Predicate_BINARY_DIR}
  ${QT_INCLUDES}
)

# find SQLite:
set(SQLITE_RECOMMENDED_VERSION "3.7.3")
find_package(Sqlite 3.6.16)
macro_log_feature(SQLITE_FOUND "SQLite" "Embedded SQL database engine" "http://www.sqlite.org"
                  TRUE ${SQLITE_MIN_VERSION} "Required for Predicate's default database driver.")
if(NOT SQLITE_FOUND)
    message(FATAL_ERROR "Required package SQLite NOT found")
else(NOT SQLITE_FOUND)
    add_subdirectory( sqlite )
endif(NOT SQLITE_FOUND)

INCLUDE (FindMySQL)
IF (MYSQL_FOUND)
   add_subdirectory( mysql )
ENDIF (MYSQL_FOUND)

INCLUDE (FindPkgConfig)
INCLUDE (FindPostgreSQL)
INCLUDE (FindPqxx)
set (PQXX_MIN_VERSION "3.0.0")
set (PQXX_MAX_VERSION "4.0.0")

macro_optional_find_package(PostgreSQL)
pkg_check_modules (PQXX libpqxx)

IF (POSTGRESQL_FOUND AND PQXX_FOUND AND PQXX_VERSION VERSION_GREATER PQXX_MIN_VERSION AND PQXX_VERSION VERSION_LESS PQXX_MAX_VERSION)
   add_subdirectory(pqxx)
ENDIF (POSTGRESQL_FOUND AND PQXX_FOUND AND PQXX_VERSION VERSION_GREATER PQXX_MIN_VERSION AND PQXX_VERSION VERSION_LESS PQXX_MAX_VERSION)

IF (POSTGRESQL_FOUND)
   add_subdirectory(postgresql)
ENDIF (POSTGRESQL_FOUND)

INCLUDE(FindFreeTDS)
IF (FreeTDS_FOUND)
   add_subdirectory(sybase)
ENDIF (FreeTDS_FOUND)

INCLUDE(FindXBase)
IF(XBASE_FOUND)
#todo   add_subdirectory(xbase)
ENDIF(XBASE_FOUND)

#message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(pgsql_dir)")
#message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(mysql_dir)")

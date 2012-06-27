if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

#TODO  add_definitions(-DKDE_DEFAULT_DEBUG_AREA=44001)

set(PREDICATE_PLUGINS_DIR ${QT_PLUGINS_DIR}/predicate)

# find SQLite:
set(SQLITE_MIN_VERSION 3.6.16)
set(SQLITE_RECOMMENDED_VERSION 3.7.12)
set(SQLITE_LOAD_EXTENSION_REQUIRED ON)
find_package(Sqlite 3.6.16 REQUIRED)
macro_log_feature(SQLITE_FOUND "SQLite" "Embedded SQL database engine with enabled extensions loading"
                  "http://www.sqlite.org" FALSE "${SQLITE_MIN_VERSION}"
                  "Recommended as Predicate's default database driver.")

if(SQLITE_FOUND)
    ##
    ## Test for ICU
    ##
    find_package(ICU)
    macro_log_feature(ICU_FOUND "ICU" "International Components for Unicode (ICU) Library"
                      "http://icu-project.org" FALSE "" "Required by SQLite database driver")
    if(ICU_FOUND)
        add_subdirectory( sqlite )
    else(ICU_FOUND)
        message(STATUS "WARNING: Because of that, SQLite driver will not be built.")
    endif(ICU_FOUND)
else(SQLITE_FOUND)
    message(STATUS "Recommended package SQLite NOT found.")
endif(SQLITE_FOUND)

INCLUDE (FindMySQL)
IF (MYSQL_FOUND)
   add_subdirectory( mysql )
ENDIF (MYSQL_FOUND)

INCLUDE (FindPkgConfig)
INCLUDE (FindPostgreSQL)

#INCLUDE (FindPqxx)
#set (PQXX_MIN_VERSION "3.0.0")
#set (PQXX_MAX_VERSION "4.0.0")
#
#macro_optional_find_package(PostgreSQL)
#pkg_check_modules (PQXX libpqxx)
#
#IF (POSTGRESQL_FOUND AND PQXX_FOUND AND PQXX_VERSION VERSION_GREATER PQXX_MIN_VERSION AND PQXX_VERSION VERSION_LESS PQXX_MAX_VERSION)
#   add_subdirectory(pqxx)
#ENDIF (POSTGRESQL_FOUND AND PQXX_FOUND AND PQXX_VERSION VERSION_GREATER PQXX_MIN_VERSION AND PQXX_VERSION VERSION_LESS PQXX_MAX_VERSION)

IF (POSTGRESQL_FOUND)
   add_subdirectory(postgresql)
ENDIF (POSTGRESQL_FOUND)

#INCLUDE(FindFreeTDS)
#IF (FreeTDS_FOUND)
#todo   add_subdirectory(sybase)
#ENDIF (FreeTDS_FOUND)

#INCLUDE(FindXBase)
#IF(XBASE_FOUND)
#todo   add_subdirectory(xbase)
#ENDIF(XBASE_FOUND)

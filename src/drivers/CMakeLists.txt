if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

#TODO  add_definitions(-DKDE_DEFAULT_DEBUG_AREA=44001)

set(PREDICATE_PLUGINS_DIR ${QT_PLUGINS_DIR}/predicate)

# find SQLite:
set(SQLITE_MIN_VERSION 3.6.16)
set(SQLITE_RECOMMENDED_VERSION 3.8.5)
set(SQLITE_LOAD_EXTENSION_REQUIRED ON)
find_package(Sqlite ${SQLITE_MIN_VERSION})
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
        macro_optional_add_subdirectory(sqlite)
    else(ICU_FOUND)
        message(STATUS "WARNING: Because of that, SQLite driver will not be built.")
    endif(ICU_FOUND)
else(SQLITE_FOUND)
    message(STATUS "Recommended package SQLite NOT found.")
endif(SQLITE_FOUND)

macro_optional_find_package(MySQL)
macro_log_feature(MYSQL_FOUND "libmysqlclient" "MySQL Client Library"
                  "http://www.mysql.com" FALSE "" "Required by MySQL database driver")
IF (MYSQL_FOUND)
   macro_optional_add_subdirectory(mysql)
ENDIF (MYSQL_FOUND)

set (PQXX_MIN_VERSION "3.0.0")
set (PQXX_MAX_VERSION "4.0.0")
macro_optional_find_package(PostgreSQL)
macro_log_feature(POSTGRESQL_FOUND "libpq" "C API to PostgreSQL"
                  "http://www.postgresql.org" FALSE "" "Required by PostgreSQL database driver")
IF (POSTGRESQL_FOUND)
   macro_optional_add_subdirectory(postgresql)
ENDIF (POSTGRESQL_FOUND)

#macro_optional_find_package(FreeTDS)
#macro_log_feature(FREETDS_FOUND "FreeTDS" "Open source implementation of the TDS (Tabular Data Stream) protocol"
#                  "http://www.freetds.org" FALSE "" "Required by Sybase database driver")
#IF (FreeTDS_FOUND)
#todo   macro_optional_add_subdirectory(sybase)
#ENDIF (FreeTDS_FOUND)

#macro_optional_find_package(XBase)
#macro_log_feature(XBASE_FOUND "XBase" "XBase compatible C++ class library"
#                  "http://linux.techass.com/projects/xdb" FALSE "" "Required by XBase database driver")
#INCLUDE(FindXBase)
#IF(XBASE_FOUND)
#todo   macro_optional_add_subdirectory(xbase)
#ENDIF(XBASE_FOUND)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0005 NEW)
endif()

#TODO  add_definitions(-DKDE_DEFAULT_DEBUG_AREA=44001)

set(KDB_PLUGIN_INSTALL_DIR ${PLUGIN_INSTALL_DIR}/kdb)

# -----------------------
macro(build_and_install_kdb_driver _name _srcs _extra_libs)
    set(_target kdb_${_name}driver)
    ecm_create_qm_loader(_srcs ${_target}_qt)
    add_library(${_target} MODULE ${_srcs})

    target_link_libraries(${_target}
        PUBLIC
            KDb
        PRIVATE
            ${_extra_libs}
    )
    # Needed for examples and autotests:
    set_target_properties(${_target}
                          PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/plugins/kdb")
    install(TARGETS ${_target} DESTINATION ${KDB_PLUGIN_INSTALL_DIR})
endmacro()
# -----------------------

# find SQLite:
set(SQLITE_MIN_VERSION 3.6.16)
set(SQLITE_RECOMMENDED_VERSION 3.14.1)
set(SQLITE_LOAD_EXTENSION_REQUIRED ON)
find_package(Sqlite ${SQLITE_MIN_VERSION})
macro_log_feature(SQLITE_FOUND "SQLite3" "Embedded SQL database engine with enabled extensions loading"
                  "http://www.sqlite.org" FALSE "${SQLITE_MIN_VERSION}"
                  "Required by SQLite database driver")

set(BUILD_SQLITE_DB_DRIVER_DESC "SQLite database driver")
if(SQLITE_FOUND)
    ##
    ## Test for ICU
    ##
    find_package(ICU)
    macro_log_feature(ICU_FOUND "ICU" "International Components for Unicode (ICU) Library"
                      "http://icu-project.org" FALSE "" "Required by SQLite database driver")
    if(ICU_FOUND)
        ecm_optional_add_subdirectory(sqlite)
    else()
        add_feature_info(BUILD_SQLITE_DB_DRIVER FALSE "${BUILD_SQLITE_DB_DRIVER_DESC} (because ICU not found)")
        message(STATUS "WARNING: Because of that ${BUILD_SQLITE_DB_DRIVER_DESC} will not be built.")
    endif()
else()
    add_feature_info(BUILD_SQLITE_DB_DRIVER FALSE "${BUILD_SQLITE_DB_DRIVER_DESC} (because SQLite3 not found)")
    message(STATUS "WARNING: Because of that ${BUILD_SQLITE_DB_DRIVER_DESC} will not be built.")
endif()

# -----------------------

set(BUILD_MYSQL_DB_DRIVER_DESC "MySQL database driver")
find_package(MySQL)
macro_log_feature(MYSQL_FOUND "libmysqlclient" "MySQL Client Library"
                  "http://www.mysql.com" FALSE "" "Required by ${BUILD_MYSQL_DB_DRIVER_DESC}")
if(MYSQL_FOUND)
   ecm_optional_add_subdirectory(mysql)
else()
   add_feature_info(BUILD_MYSQL_DB_DRIVER FALSE "${BUILD_MYSQL_DB_DRIVER_DESC} (because MySQL Client Library not found)")
endif()

# -----------------------

set(BUILD_POSTGRESQL_DB_DRIVER_DESC "PostgreSQL database driver")
find_package(PostgreSQL)
macro_log_feature(POSTGRESQL_FOUND "libpq" "C API to PostgreSQL"
                  "http://www.postgresql.org" FALSE "" "Required by ${BUILD_POSTGRESQL_DB_DRIVER_DESC}")
if(POSTGRESQL_FOUND)
   ecm_optional_add_subdirectory(postgresql)
else()
   add_feature_info(BUILD_POSTGRESQL_DB_DRIVER FALSE "${BUILD_POSTGRESQL_DB_DRIVER_DESC} (because C API to PostgreSQL not found)")
endif()

#set(BUILD_SYBASE_DB_DRIVER_DESC "Sybase/MSSQL database driver")
#find_package(FreeTDS)
#macro_log_feature(FREETDS_FOUND "FreeTDS" "Open source implementation of the TDS (Tabular Data Stream) protocol"
#                  "http://www.freetds.org" FALSE "" "Required by ${BUILD_SYBASE_DB_DRIVER_DESC}")
#if(FreeTDS_FOUND)
#   ecm_optional_add_subdirectory(sybase)
#else()
#   add_feature_info(BUILD_SYBASE_DB_DRIVER FALSE "${BUILD_SYBASE_DB_DRIVER_DESC} (because FreeTDS protocol implementation not found)")
#endif()

#set(BUILD_XBASE_DB_DRIVER_DESC "xBase database driver")
#find_package(XBase)
#macro_log_feature(XBASE_FOUND "XBase" "XBase compatible C++ class library"
#                  "http://linux.techass.com/projects/xdb" FALSE "" "Required by ${BUILD_XBASE_DB_DRIVER_DESC}")
#if(XBASE_FOUND)
#   ecm_optional_add_subdirectory(xbase)
#else()
#   add_feature_info(BUILD_XBASE_DB_DRIVER FALSE "${BUILD_XBASE_DB_DRIVER_DESC} (because XBase compatible C++ class library not found)")
#endif()

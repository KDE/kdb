if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0005 NEW)
endif()

#TODO  add_definitions(-DKDE_DEFAULT_DEBUG_AREA=44001)

set(KDB_PLUGIN_INSTALL_DIR ${PLUGIN_INSTALL_DIR}/${KDB_BASE_NAME_LOWER})

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
                          PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/plugins/${KDB_BASE_NAME_LOWER}")
    install(TARGETS ${_target} DESTINATION ${KDB_PLUGIN_INSTALL_DIR})
endmacro()
# -----------------------

# find SQLite:
set(SQLITE_MIN_VERSION 3.6.16)
set(SQLITE_RECOMMENDED_VERSION 3.22.0)
set(SQLITE_LOAD_EXTENSION_REQUIRED ON)
set(BUILD_SQLITE_DB_DRIVER_DESC "SQLite database driver")
find_package(Sqlite ${SQLITE_MIN_VERSION})
set_package_properties(Sqlite PROPERTIES TYPE RECOMMENDED
                       PURPOSE "Required by ${BUILD_SQLITE_DB_DRIVER_DESC}")

if(Sqlite_FOUND)
    ecm_optional_add_subdirectory(sqlite)
else()
    add_feature_info(BUILD_SQLITE_DB_DRIVER FALSE "${BUILD_SQLITE_DB_DRIVER_DESC} (because SQLite3 not found)")
    message(STATUS "WARNING: Because of that ${BUILD_SQLITE_DB_DRIVER_DESC} will not be built.")
endif()

# -----------------------

set(BUILD_MYSQL_DB_DRIVER_DESC "MySQL database driver")
find_package(MySQL)
set_package_properties(MySQL PROPERTIES TYPE RECOMMENDED
                       PURPOSE "Required by ${BUILD_MYSQL_DB_DRIVER_DESC}")
if(MySQL_FOUND)
   ecm_optional_add_subdirectory(mysql)
else()
   add_feature_info(BUILD_MYSQL_DB_DRIVER FALSE "${BUILD_MYSQL_DB_DRIVER_DESC} (because MySQL Client Library not found)")
endif()

# -----------------------

set(BUILD_POSTGRESQL_DB_DRIVER_DESC "PostgreSQL database driver")
find_package(PostgreSQL)
set_package_properties(PostgreSQL PROPERTIES TYPE RECOMMENDED
                       PURPOSE "Required by ${BUILD_POSTGRESQL_DB_DRIVER_DESC}")
if(PostgreSQL_FOUND)
   ecm_optional_add_subdirectory(postgresql)
else()
   add_feature_info(BUILD_POSTGRESQL_DB_DRIVER FALSE "${BUILD_POSTGRESQL_DB_DRIVER_DESC} (because C API to PostgreSQL not found)")
endif()

#set(BUILD_SYBASE_DB_DRIVER_DESC "Sybase/MSSQL database driver")
#find_package(FreeTDS)
#set_package_properties(FreeTDS PROPERTIES
#                       DESCRIPTION "Open source implementation of the TDS (Tabular Data Stream) protocol"
#                       URL "http://www.freetds.org"
#                       TYPE OPTIONAL
#                       PURPOSE "Required by ${BUILD_SYBASE_DB_DRIVER_DESC}"
#)
#if(FreeTDS_FOUND)
#   ecm_optional_add_subdirectory(sybase)
#else()
#   add_feature_info(BUILD_SYBASE_DB_DRIVER FALSE "${BUILD_SYBASE_DB_DRIVER_DESC} (because FreeTDS protocol implementation not found)")
#endif()

#set(BUILD_XBASE_DB_DRIVER_DESC "xBase database driver")
#find_package(XBase)
#set_package_properties(XBase PROPERTIES
#                       DESCRIPTION "XBase compatible C++ class library"
#                       URL "http://linux.techass.com/projects/xdb"
#                       TYPE OPTIONAL
#                       PURPOSE "Required by ${BUILD_XBASE_DB_DRIVER_DESC}"
#)
#if(XBASE_FOUND)
#   ecm_optional_add_subdirectory(xbase)
#else()
#   add_feature_info(BUILD_XBASE_DB_DRIVER FALSE "${BUILD_XBASE_DB_DRIVER_DESC} (because XBase compatible C++ class library not found)")
#endif()

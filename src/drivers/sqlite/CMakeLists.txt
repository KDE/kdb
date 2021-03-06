add_feature_info(BUILD_SQLITE_DB_DRIVER TRUE ${BUILD_SQLITE_DB_DRIVER_DESC})

# Generate SqliteGlobal.h
configure_file(SqliteGlobal.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/SqliteGlobal.h)

simple_option(KDB_SQLITE_VACUUM "Support for SQLite VACUUM (compacting)" ON)

if (KDB_SQLITE_VACUUM)
  set(KDB_SQLITE_DUMP_TOOL ${KDB_BASE_NAME_LOWER}_sqlite3_dump)
  add_definitions(-DKDB_SQLITE_DUMP_TOOL=\"${KDB_SQLITE_DUMP_TOOL}\")
endif ()

# Definitions used for the sqlite driver and the shell
add_definitions(
    # sqlite compile-time options, https://sqlite.org/compile.html
    -DSQLITE_SECURE_DELETE
    -DSQLITE_ENABLE_COLUMN_METADATA # Some additional APIs that provide convenient access to meta-data
                                    # about tables and queries
    -DSQLITE_ENABLE_FTS3 # Version 3 of the full-text search engine
    -DSQLITE_ENABLE_FTS3_PARENTHESIS # Modifies the query pattern parser in FTS3 such that it supports
                                     # operators AND and NOT (in addition to the usual OR and NEAR)
                                     # and also allows query expressions to contain nested parenthesesis.
    -DSQLITE_ENABLE_MEMORY_MANAGEMENT # Extra logic to SQLite that allows it to release unused memory upon request
    -DSQLITE_ENABLE_RTREE # Support for the R*Tree index extension
    -DSQLITE_ENABLE_STAT2 # Additional logic to the ANALYZE command and to the query planner that can help SQLite
                          # to chose a better query plan under certain situations
    -DSQLITE_ENABLE_UPDATE_DELETE_LIMIT # Optional ORDER BY and LIMIT clause on UPDATE and DELETE statements
    -DSQLITE_ENABLE_UNLOCK_NOTIFY # Enables the sqlite3_unlock_notify() interface and its associated functionality
                                  # (https://sqlite.org/unlock_notify.html)
    -DSQLITE_SOUNDEX # Enables the soundex() SQL function (https://sqlite.org/lang_corefunc.html#soundex)

# todo -DSQLITE_OMIT_DEPRECATED
)

include_directories(
  ${SQLITE_INCLUDE_DIR}
)

if(MSVC)
  # avoid deprecated warnings
  add_definitions(
    -D_CRT_SECURE_NO_DEPRECATE
    -D_CRT_NONSTDC_NO_DEPRECATE
  )
endif()

set(kdb_sqlite_SRCS
   sqlite_debug.cpp
   SqliteConnection.cpp
   SqliteConnection_p.cpp
   SqliteDriver.cpp
   SqliteCursor.cpp
   SqliteKeywords.cpp
   SqlitePreparedStatement.cpp
   SqliteAdmin.cpp
   SqliteAlter.cpp
   SqliteFunctions.cpp
   kdb_sqlitedriver.json
)

if (KDB_SQLITE_VACUUM)
  list(APPEND kdb_sqlite_SRCS SqliteVacuum.cpp)
  list(APPEND kdb_sqlite_MOC_SRCS SqliteVacuum.h)
endif ()

build_and_install_kdb_driver(sqlite "${kdb_sqlite_SRCS}" "${SQLITE_LIBRARIES}")

add_subdirectory(icu)

if (KDB_SQLITE_VACUUM)
  add_subdirectory(dump)
endif ()

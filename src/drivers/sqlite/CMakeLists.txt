add_feature_info(BUILD_SQLITE_DB_DRIVER TRUE ${BUILD_SQLITE_DB_DRIVER_DESC})

# Generate SqliteGlobal.h
configure_file(SqliteGlobal.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/SqliteGlobal.h )

simple_option(KDB_SQLITE_VACUUM "Support for SQLite VACUUM (compacting)" ON)

if (KDB_SQLITE_VACUUM)
  set(KDB_SQLITE_DUMP_TOOL kdb_sqlite3_dump)
  ADD_DEFINITIONS(-DKDB_SQLITE_DUMP_TOOL=\"${KDB_SQLITE_DUMP_TOOL}\")
endif (KDB_SQLITE_VACUUM)

# definitins used for the sqlite driver and the shell
ADD_DEFINITIONS(
    # sqlite compile-time options, http://sqlite.org/compile.html
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
                                  # (http://sqlite.org/unlock_notify.html)
    -DSQLITE_SOUNDEX # Enables the soundex() SQL function (http://sqlite.org/lang_corefunc.html#soundex)

# todo -DSQLITE_OMIT_DEPRECATED
)

include_directories(
  ${KDb_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QT_INCLUDES}
)

ADD_DEFINITIONS(
  -D__KDB__=
  -DKDB_DRIVER_NAME="sqlite"
)

if(MSVC)
  # avoid deprecated warnings
  ADD_DEFINITIONS(
    -D_CRT_SECURE_NO_DEPRECATE
    -D_CRT_NONSTDC_NO_DEPRECATE
  )
endif(MSVC)

########### next target ###############

set(kdb_sqlite_SRCS
   SqliteConnection.cpp 
   SqliteDriver.cpp 
   SqliteCursor.cpp 
   SqliteKeywords.cpp 
   SqlitePreparedStatement.cpp
   SqliteAdmin.cpp
   SqliteAlter.cpp
)

set(kdb_sqlite_MOC_SRCS
   SqliteDriver.h
)

if (KDB_SQLITE_VACUUM)
  list(APPEND kdb_sqlite_SRCS SqliteVacuum.cpp)
  list(APPEND kdb_sqlite_MOC_SRCS SqliteVacuum.h)
endif (KDB_SQLITE_VACUUM)

qt_wrap_cpp(kdb_sqlite kdb_sqlite_SRCS ${kdb_sqlite_MOC_SRCS})

add_library(kdb_sqlite SHARED ${kdb_sqlite_SRCS})

target_link_libraries(kdb_sqlite ${SQLITE_LIBRARIES} kdb)
if(DEFINED CMAKE_DL_LIBS)
  target_link_libraries(kdb_sqlite ${CMAKE_DL_LIBS})
endif(DEFINED CMAKE_DL_LIBS)
if(DEFINED CMAKE_THREAD_LIBS)
  target_link_libraries(kdb_sqlite ${CMAKE_THREAD_LIBS})
endif(DEFINED CMAKE_THREAD_LIBS)
set_target_properties(kdb_sqlite PROPERTIES PREFIX "")

install(TARGETS kdb_sqlite DESTINATION ${KDB_PLUGINS_DIR})
install(FILES kdb_sqlite.desktop DESTINATION ${KDB_PLUGINS_DIR})

add_subdirectory(icu)

if (KDB_SQLITE_VACUUM)
  add_subdirectory(dump)
endif (KDB_SQLITE_VACUUM)

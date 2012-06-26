# Generate SqliteGlobal.h
configure_file(SqliteGlobal.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/SqliteGlobal.h )

option(WITH_PREDICATE_SQLITE_VACUUM "Enable support for SQLite vacuum (compacting) feature" ON)
add_feature_info(PREDICATE_SQLITE_VACUUM WITH_PREDICATE_SQLITE_VACUUM "Enable support for SQLite vacuum (compacting) feature")
if (PREDICATE_SQLITE_VACUUM)
  set(PREDICATE_SQLITE_DUMP_TOOL predicate_sqlite3_dump)
  ADD_DEFINITIONS(-DPREDICATE_SQLITE_DUMP_TOOL=\"${PREDICATE_SQLITE_DUMP_TOOL}\")
endif (PREDICATE_SQLITE_VACUUM)

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
  ${Predicate_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QT_INCLUDES}
)

ADD_DEFINITIONS(
  -D__PREDICATE__=
  -DPREDICATE_DRIVER_NAME="sqlite"
)

if(MSVC)
  # avoid deprecated warnings
  ADD_DEFINITIONS(
    -D_CRT_SECURE_NO_DEPRECATE
    -D_CRT_NONSTDC_NO_DEPRECATE
  )
endif(MSVC)

########### next target ###############

set(predicate_sqlite_SRCS
   SqliteConnection.cpp 
   SqliteDriver.cpp 
   SqliteCursor.cpp 
   SqliteKeywords.cpp 
   SqlitePreparedStatement.cpp
   SqliteAdmin.cpp
   SqliteAlter.cpp
)

set(predicate_sqlite_MOC_SRCS
   SqliteDriver.h
)

if (PREDICATE_SQLITE_VACUUM)
  list(APPEND predicate_sqlite_SRCS SqliteVacuum.cpp)
  list(APPEND predicate_sqlite_MOC_SRCS SqliteVacuum.h)
endif (PREDICATE_SQLITE_VACUUM)

qt_wrap_cpp(predicate_sqlite predicate_sqlite_SRCS ${predicate_sqlite_MOC_SRCS})

add_library(predicate_sqlite SHARED ${predicate_sqlite_SRCS})

target_link_libraries(predicate_sqlite ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS} ${SQLITE_LIBRARIES} predicate)
set_target_properties(predicate_sqlite PROPERTIES PREFIX "")

install(TARGETS predicate_sqlite DESTINATION ${PREDICATE_PLUGINS_DIR})
install(FILES predicate_sqlite.desktop DESTINATION ${PREDICATE_PLUGINS_DIR})

add_subdirectory(icu)

if (PREDICATE_SQLITE_VACUUM)
  add_subdirectory(dump)
endif (PREDICATE_SQLITE_VACUUM)

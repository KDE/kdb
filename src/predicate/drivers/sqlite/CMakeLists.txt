include_directories(
  ${SQLITE_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}
  ${QT_INCLUDES}
)

ADD_DEFINITIONS(
  -D__PREDICATE__=
  -DPREDICATE_DRIVER_NAME="sqlite3"
  -DPREDICATE_SQLITE3_NO_VACUUM=1 #TODO remove
)

if(MSVC)
  # avoid deprecated warnings
  ADD_DEFINITIONS(
    -D_CRT_SECURE_NO_DEPRECATE
    -D_CRT_NONSTDC_NO_DEPRECATE
  )
endif(MSVC)

########### next target ###############

set(predicate_sqlite3_SRCS 
   SqliteConnection.cpp 
   SqliteDriver.cpp 
   SqliteCursor.cpp 
   SqliteKeywords.cpp 
   SqlitePreparedStatement.cpp
   SqliteAdmin.cpp
#todo   SqliteVacuum.cpp
   SqliteAlter.cpp
)

set(predicate_sqlite3_MOC_SRCS
   SqliteConnection.h
   SqliteDriver.h
#todo   SqliteVacuum.h
)

qt_wrap_cpp(predicate_sqlite3 predicate_sqlite3_SRCS ${predicate_sqlite3_MOC_SRCS})

add_library(predicate_sqlite3 SHARED ${predicate_sqlite3_SRCS})

target_link_libraries(predicate_sqlite3 ${SQLITE_LIBRARIES} predicate )

install(TARGETS predicate_sqlite3 DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

#TODO install( FILES  kexidb_sqlite3driver.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

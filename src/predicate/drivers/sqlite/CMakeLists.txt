include_directories(
  ${CMAKE_SOURCE_DIR}/3rdparty/kexisql3/src
  ${CMAKE_SOURCE_DIR}
  ${QT_INCLUDES}
)

ADD_DEFINITIONS( -D__PREDICATE__= )

########### next target ###############

set(predicate_sqlite3_SRCS 
   SqliteConnection.cpp 
   SqliteDriver.cpp 
   SqliteCursor.cpp 
   SqliteKeywords.cpp 
   SqlitePreparedStatement.cpp
   SqliteAdmin.cpp
   SqliteVacuum.cpp
   SqliteAlter.cpp
)

set(predicate_sqlite3_MOC_SRCS
   SqliteConnection.h
   SqliteDriver.h
   SqliteVacuum.h
)

qt_wrap_cpp(predicate_sqlite3 predicate_sqlite3_SRCS ${predicate_sqlite3_MOC_SRCS})

add_library(predicate_sqlite3 SHARED ${predicate_sqlite3_SRCS})

target_link_libraries(predicate_sqlite3 kexisql3 predicate )

install(TARGETS predicate_sqlite3 DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

#TODO install( FILES  kexidb_sqlite3driver.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

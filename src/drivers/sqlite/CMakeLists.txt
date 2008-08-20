include_directories(
  ${CMAKE_SOURCE_DIR}/3rdparty/kexisql3/src
  ${CMAKE_SOURCE_DIR}/src
  ${QT_INCLUDES}
)

ADD_DEFINITIONS( -D__PREDICATE__= )

########### next target ###############

set(predicate_sqlite3_SRCS 
   sqliteconnection.cpp 
   sqlitedriver.cpp 
   sqlitecursor.cpp 
   sqlitekeywords.cpp 
   sqlitepreparedstatement.cpp
   sqliteadmin.cpp
   sqlitevacuum.cpp
   sqlitealter.cpp
)


add_library(predicate_sqlite3 SHARED ${predicate_sqlite3_SRCS})

target_link_libraries(predicate_sqlite3 kexisql3 predicate )

install(TARGETS predicate_sqlite3 DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

#TODO install( FILES  kexidb_sqlite3driver.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

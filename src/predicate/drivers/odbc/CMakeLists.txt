include_directories(
  /usr/include/ #TODO
  ${CMAKE_SOURCE_DIR}
  ${QT_INCLUDES}
)

ADD_DEFINITIONS(
  -D__PREDICATE__= 
  -DPREDICATE_DRIVER_NAME="odbc"
)

########### next target ###############

set(predicate_odbcdriver_PART_SRCS 
  kodbcdriver.cpp
  odbcdriver.cpp 
  odbcconnection_p.cpp 
  odbcconnection.cpp 
  odbccursor.cpp 
  odbcpreparedstatement.cpp
  odbcqueryunit.cpp
  odbcsqlqueryunit.cpp
  odbctypeinfoqueryunit.cpp
  odbctablesqueryunit.cpp
  odbcspecialcolumnsqueryunit.cpp
)

add_library(predicate_odbc SHARED ${predicate_odbcdriver_SRCS})

target_link_libraries(predicate_odbc
  predicate
)

install(TARGETS predicate_odbc DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

#TODO install( FILES  kexidb_odbcdriver.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

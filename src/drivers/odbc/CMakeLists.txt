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
set_target_properties(predicate_odbc PROPERTIES PREFIX "")

install(TARGETS predicate_odbc DESTINATION ${PREDICATE_PLUGINS_DIR})
install(FILES predicate_odbc.desktop DESTINATION ${PREDICATE_PLUGINS_DIR})

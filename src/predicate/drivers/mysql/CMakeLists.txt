include_directories(
  ${MYSQL_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${QT_INCLUDES}
)

ADD_DEFINITIONS( -D__PREDICATE__= )

########### next target ###############

set(predicate_mysql_SRCS 
   MysqlDriver.cpp 
   MysqlConnection.cpp 
   MysqlConnection_p.cpp 
   MysqlCursor.cpp 
   MysqlKeywords.cpp 
   MysqlPreparedStatement.cpp )


add_library(predicate_mysql SHARED ${predicate_mysql_SRCS})

target_link_libraries(predicate_mysql
  ${MYSQL_LIBRARIES}
  predicate
)

install(TARGETS predicate_mysql DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

#TODO install( FILES predicate_mysql.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

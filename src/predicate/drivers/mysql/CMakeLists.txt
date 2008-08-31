include_directories(
  ${MYSQL_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}
  ${QT_INCLUDES}
)

ADD_DEFINITIONS(
  -D__PREDICATE__= 
  -DPREDICATE_DRIVER_NAME="mysql"
)

########### next target ###############

set(predicate_mysql_SRCS 
   MysqlDriver.cpp 
   MysqlConnection.cpp 
   MysqlConnection_p.cpp 
   MysqlCursor.cpp 
   MysqlKeywords.cpp 
   MysqlPreparedStatement.cpp
)

set(predicate_mysql_MOC_SRCS
   MysqlConnection.h
   MysqlDriver.h
)

qt_wrap_cpp(predicate_mysql predicate_mysql_SRCS ${predicate_mysql_MOC_SRCS})

add_library(predicate_mysql SHARED ${predicate_mysql_SRCS})

target_link_libraries(predicate_mysql
  ${MYSQL_LIBRARIES}
  ws2_32
  msvcrtd
  predicate
)

if(MSVC)
  # avoid LNK2005 errors
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FORCE:MULTIPLE")
SET_TARGET_PROPERTIES(predicate_mysql PROPERTIES
    LINK_FLAGS "
    /NODEFAULTLIB:\"libcmtd.lib\"
    ")
endif(MSVC)

install(TARGETS predicate_mysql DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

#TODO install( FILES predicate_mysql.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

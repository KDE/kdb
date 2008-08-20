include_directories(
  ${ORACLE_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${QT_INCLUDES}
)

ADD_DEFINITIONS( -D__PREDICATE__= )

########### next target ###############

set(predicate_oracle_SRCS 
   oracledriver.cpp 
   oracleconnection.cpp 
   oracleconnection_p.cpp 
   oraclecursor.cpp 
   oraclekeywords.cpp 
   oraclepreparedstatement.cpp )

#TODO: detect this:
INCLUDE(/usr/include/oracle/11.1.0.1/client)

add_library(predicate_oracle SHARED ${predicate_oracle_SRCS})

target_link_libraries(predicate_oracle
  ${ORACLE_LIBRARIES} 
  predicate
  occi
  clntsh
)

install(TARGETS predicate_oracle DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

#TODO install( FILES  predicate_oracle.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

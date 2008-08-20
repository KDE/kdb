include_directories(
  ${FreeTDS_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${QT_INCLUDES}
)

ADD_DEFINITIONS( -D__PREDICATE__= )

########### next target ###############

set(predicate_sybase_SRCS 
   sybasedriver.cpp 
   sybaseconnection.cpp 
   sybaseconnection_p.cpp 
   sybasecursor.cpp 
   sybasekeywords.cpp 
   sybasepreparedstatement.cpp )


add_library(predicate_sybase SHARED ${predicate_sybase_SRCS})

target_link_libraries(predicate_sybase
  ${FreeTDS_LIBRARIES}
  predicate
)

install(TARGETS predicate_sybase DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

#TODO install( FILES predicate_sybase.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

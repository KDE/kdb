include_directories(
  ${XBase_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${QT_INCLUDES}
)

ADD_DEFINITIONS( -D__PREDICATE__= )

########### next target ###############

set(predicate_xbase_SRCS 
   xbasedriver.cpp 
   xbaseconnection.cpp 
   xbaseconnection_p.cpp 
   xbasecursor.cpp 
   xbasekeywords.cpp 
   xbaseexport.cpp
   )


add_library(predicate_xbase SHARED ${predicate_xbase_SRCS})

target_link_libraries(predicate_xbase
  ${XBase_LIBRARIES} 
  predicate
#TODO  keximigrate
)

install(TARGETS predicate_xbase DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

#TODO install( FILES predicate_xbase.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

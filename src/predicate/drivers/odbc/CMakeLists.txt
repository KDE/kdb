include_directories( ${CMAKE_SOURCE_DIR} ${QT_INCLUDES}  )

########### next target ###############

set(predicate_odbc_SRCS odbcdriver.cpp odbcconnection.cpp )


add_library(predicate_odbc SHARED ${predicate_odbc_SRCS})

target_link_libraries(predicate_odbc odbc predicate )

install(TARGETS predicate_odbc DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

#TODO install( FILES  predicate_odbc.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

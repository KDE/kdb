include_directories(
  ${POSTGRESQL_INCLUDE_DIR}
  ${PQXX_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${QT_INCLUDES}
)

ADD_DEFINITIONS(
  -D__PREDICATE__=
)

if (MSVC)
  ADD_DEFINITIONS(-EHsc)
elif (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_C_COMPILER MATCHES "icc")
  ADD_DEFINITIONS(-fexceptions)
endif (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_C_COMPILER MATCHES "icc")

########### next target ###############

set(predicate_pqxx_SRCS 
   pqxxdriver.cpp 
   pqxxcursor.cpp 
   pqxxconnection.cpp 
   pqxxkeywords.cpp 
   pqxxconnection_p.cpp 
   pqxxpreparedstatement.cpp )


add_library(predicate_pqxx SHARED ${predicate_pqxx_SRCS})

target_link_libraries(predicate_pqxx
  ${POSTGRESQL_LIBRARIES}
  ${PQXX_LIBRARIES}
  predicate
)

install(TARGETS predicate_pqxx DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

#TODO install( FILES predicate_pqxx.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

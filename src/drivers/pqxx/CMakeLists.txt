include_directories(
  ${POSTGRESQL_INCLUDE_DIR}
  ${PQXX_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}
  ${QT_INCLUDES}
)

ADD_DEFINITIONS(
  -D__PREDICATE__=
  -DPREDICATE_DRIVER_NAME="pqxx"
)

if (MSVC)
  ADD_DEFINITIONS(-EHsc)
elif (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_C_COMPILER MATCHES "icc")
  ADD_DEFINITIONS(-fexceptions)
endif (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_C_COMPILER MATCHES "icc")

########### next target ###############

set(predicate_pqxx_SRCS 
   PqxxDriver.cpp 
   PqxxCursor.cpp 
   PqxxConnection.cpp 
   PqxxKeywords.cpp 
   PqxxConnection_p.cpp 
   PqxxPreparedStatement.cpp
)

set(predicate_pqxx_MOC_SRCS
   PqxxDriver.h
)

qt_wrap_cpp(predicate_pqxx predicate_pqxx_SRCS ${predicate_pqxx_MOC_SRCS})

add_library(predicate_pqxx SHARED ${predicate_pqxx_SRCS})

target_link_libraries(predicate_pqxx
  ${POSTGRESQL_LIBRARIES}
  ${PQXX_LIBRARIES}
  predicate
)
set_target_properties(predicate_pqxx PROPERTIES PREFIX "")

install(TARGETS predicate_pqxx DESTINATION ${PREDICATE_PLUGINS_DIR})
install(FILES predicate_pqxx.desktop DESTINATION ${PREDICATE_PLUGINS_DIR})

include_directories(
  ${POSTGRESQL_INCLUDE_DIR}
)

ADD_DEFINITIONS(
  -D__PREDICATE__=
  -DPREDICATE_DRIVER_NAME="postgresql"
)

########### next target ###############

set(predicate_postgresql_SRCS
   PostgresqlDriver.cpp
   PostgresqlCursor.cpp
   PostgresqlConnection.cpp
   PostgresqlKeywords.cpp
   PostgresqlConnection_p.cpp
   PostgresqlPreparedStatement.cpp
)

set(predicate_postgresql_MOC_SRCS
   PostgresqlDriver.h
)

qt_wrap_cpp(predicate_postgresql predicate_postgresql_SRCS ${predicate_postgresql_MOC_SRCS})

add_library(predicate_postgresql SHARED ${predicate_postgresql_SRCS})

target_link_libraries(predicate_postgresql
  ${POSTGRESQL_LIBRARIES}
  predicate
)
set_target_properties(predicate_postgresql PROPERTIES PREFIX "")

install(TARGETS predicate_postgresql DESTINATION ${PREDICATE_PLUGINS_DIR})
install(FILES predicate_postgresql.desktop DESTINATION ${PREDICATE_PLUGINS_DIR})

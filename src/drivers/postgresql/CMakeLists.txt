add_feature_info(BUILD_POSTGRESQL_DB_DRIVER TRUE ${BUILD_POSTGRESQL_DB_DRIVER_DESC})

include_directories(
  ${PostgreSQL_INCLUDE_DIRS}
)

add_definitions(
  -D__KDB__=
)

########### next target ###############

set(kdb_postgresqldriver_SRCS
   PostgresqlDriver.cpp
   PostgresqlConnection.cpp
   PostgresqlCursor.cpp
   PostgresqlKeywords.cpp
   PostgresqlConnection_p.cpp
   PostgresqlPreparedStatement.cpp
)

set(kdb_postgresqldriver_MOC_SRCS
   PostgresqlDriver.h
)

add_library(kdb_postgresqldriver SHARED ${kdb_postgresqldriver_SRCS})
kcoreaddons_desktop_to_json(kdb_postgresqldriver kdb_postgresqldriver.desktop)

target_link_libraries(kdb_postgresqldriver
  ${PostgreSQL_LIBRARIES}
  kdb
  Qt5::Network
)
set_target_properties(kdb_postgresqldriver PROPERTIES PREFIX "")

install(TARGETS kdb_postgresqldriver DESTINATION ${KDB_PLUGIN_INSTALL_DIR})

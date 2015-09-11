add_feature_info(BUILD_POSTGRESQL_DB_DRIVER TRUE ${BUILD_POSTGRESQL_DB_DRIVER_DESC})

include_directories(
  ${PostgreSQL_INCLUDE_DIRS}
)

set(kdb_postgresqldriver_SRCS
   PostgresqlDriver.cpp
   PostgresqlConnection.cpp
   PostgresqlCursor.cpp
   PostgresqlKeywords.cpp
   PostgresqlConnection_p.cpp
   PostgresqlPreparedStatement.cpp
)

build_and_install_kdb_driver(postgresql "${kdb_postgresqldriver_SRCS}" "${PostgreSQL_LIBRARIES}")

add_feature_info(BUILD_POSTGRESQL_DB_DRIVER TRUE ${BUILD_POSTGRESQL_DB_DRIVER_DESC})

include_directories(
  ${PostgreSQL_INCLUDE_DIRS}
)

set(kdb_postgresqldriver_SRCS
   postgresql_debug.cpp
   PostgresqlTypes.cpp
   PostgresqlDriver.cpp
   PostgresqlConnection.cpp
   PostgresqlCursor.cpp
   PostgresqlKeywords.cpp
   PostgresqlConnection_p.cpp
   PostgresqlPreparedStatement.cpp
   kdb_postgresqldriver.json
   README
)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANG)
   set_source_files_properties(PostgresqlTypes.cpp
      PROPERTIES
         COMPILE_FLAGS " -Wno-undef " # needed because server/c.h uses #if _MSC_VER >= 1400
   )
endif()

find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Network)

build_and_install_kdb_driver(postgresql "${kdb_postgresqldriver_SRCS}" "${PostgreSQL_LIBRARIES};Qt${QT_MAJOR_VERSION}::Network")

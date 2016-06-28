add_feature_info(BUILD_MYSQL_DB_DRIVER TRUE ${BUILD_MYSQL_DB_DRIVER_DESC})

include_directories(
  ${MYSQL_INCLUDE_DIR}
)

set(kdb_mysqldriver_SRCS
    mysql_debug.cpp
    MysqlDriver.cpp
    MysqlConnection.cpp
    MysqlConnection_p.cpp
    MysqlCursor.cpp
    MysqlKeywords.cpp
    MysqlPreparedStatement.cpp
    kdb_mysqldriver.json
)

build_and_install_kdb_driver(mysql "${kdb_mysqldriver_SRCS}" "${MYSQL_LIBRARIES}")

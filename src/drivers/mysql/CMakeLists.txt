add_feature_info(BUILD_MYSQL_DB_DRIVER TRUE ${BUILD_MYSQL_DB_DRIVER_DESC})

include_directories(
  ${MYSQL_INCLUDE_DIR}
)

add_definitions(
  -D__KDB__=
)

set(kdb_mysqldriver_SRCS
    mysql_debug.cpp
    MysqlDriver.cpp
    MysqlConnection.cpp
    MysqlConnection_p.cpp
    MysqlCursor.cpp
    MysqlKeywords.cpp
    MysqlPreparedStatement.cpp
)


ecm_create_qm_loader(kdb_mysqldriver_SRCS kdb_mysqldriver_qt)

add_library(kdb_mysqldriver SHARED ${kdb_mysqldriver_SRCS})
kcoreaddons_desktop_to_json(kdb_mysqldriver kdb_mysqldriver.desktop)

target_link_libraries(kdb_mysqldriver
    PUBLIC
        KDb
    PRIVATE
        ${MYSQL_LIBRARIES}
)

if(MSVC)
  target_link_libraries(kdb_mysqldriver ws2_32 msvcrtd)
  # avoid LNK2005 errors
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FORCE:MULTIPLE")
  set_target_properties(kdb_mysqldriver PROPERTIES LINK_FLAGS "/NODEFAULTLIB:\"libcmtd.lib\"")
endif()

install(TARGETS kdb_mysqldriver DESTINATION ${KDB_PLUGIN_INSTALL_DIR})

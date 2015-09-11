add_feature_info(BUILD_SYBASE_DB_DRIVER TRUE ${BUILD_SYBASE_DB_DRIVER_DESC})

include_directories(
  ${FreeTDS_INCLUDE_DIR}
)

set(kdb_sybasedriver_SRCS
   SybaseDriver.cpp
   SybaseConnection.cpp
   SybaseConnection_p.cpp
   SybaseCursor.cpp
   SybaseKeywords.cpp
   SybasePreparedStatement.cpp
)

build_and_install_kdb_driver(sybase "${kdb_sybasedriver_SRCS}" "${FreeTDS_LIBRARIES}")

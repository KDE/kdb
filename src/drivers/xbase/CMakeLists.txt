add_feature_info(BUILD_XBASE_DB_DRIVER TRUE ${BUILD_XBASE_DB_DRIVER_DESC})

include_directories(
  ${XBase_INCLUDE_DIR}
)

set(kdb_xbasedriver_SRCS
   XbaseDriver.cpp
   XbaseConnection.cpp
   XbaseConnection_p.cpp
   XbaseCursor.cpp
   XbaseKeywords.cpp
   XbaseExport.cpp
)

build_and_install_kdb_driver(xbase "${kdb_xbasedriver_SRCS}" "${XBase_LIBRARIES}")

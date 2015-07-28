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

ecm_create_qm_loader(kdb_xbasedriver_SRCS kdb_xbasedriver_qt)

add_library(kdb_xbasedriver SHARED ${kdb_xbasedriver_SRCS})
kcoreaddons_desktop_to_json(kdb_xbasedriver kdb_xbasedriver.desktop)

target_link_libraries(kdb_xbasedriver
  ${XBase_LIBRARIES}
  kdb
#TODO  keximigrate
)
set_target_properties(kdb_xbasedriver PROPERTIES PREFIX "")

install(TARGETS kdb_xbasedriver DESTINATION ${KDB_PLUGIN_INSTALL_DIR})

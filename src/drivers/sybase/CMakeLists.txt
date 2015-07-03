include_directories(
  ${FreeTDS_INCLUDE_DIR}
)

add_definitions(
  -D__KDB__=
)

########### next target ###############

set(kdb_sybasedriver_SRCS
   SybaseDriver.cpp
   SybaseConnection.cpp
   SybaseConnection_p.cpp
   SybaseCursor.cpp
   SybaseKeywords.cpp
   SybasePreparedStatement.cpp )


add_library(kdb_sybasedriver SHARED ${kdb_sybasedriver_SRCS})
kcoreaddons_desktop_to_json(kdb_sybasedriver kdb_sybasedriver.desktop)

target_link_libraries(kdb_sybasedriver
  ${FreeTDS_LIBRARIES}
  kdb
)
set_target_properties(kdb_sybasedriver PROPERTIES PREFIX "")

install(TARGETS kdb_sybasedriver DESTINATION ${KDB_PLUGIN_INSTALL_DIR})

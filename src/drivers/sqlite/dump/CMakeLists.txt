include(ECMMarkNonGuiExecutable)

set(kdb_sqlite_dump_tool_SRCS main.cpp shell.c README)

add_executable(${KDB_SQLITE_DUMP_TOOL} ${kdb_sqlite_dump_tool_SRCS})
ecm_mark_nongui_executable(${KDB_SQLITE_DUMP_TOOL})

target_link_libraries(${KDB_SQLITE_DUMP_TOOL}
    PRIVATE
        KDb
        ${SQLITE_LIBRARIES}
)
if(WIN32)
    set_target_properties(${KDB_SQLITE_DUMP_TOOL}
                          PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
endif()

install(TARGETS ${KDB_SQLITE_DUMP_TOOL} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

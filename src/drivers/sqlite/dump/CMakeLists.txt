set(kdb_sqlite_dump_tool_SRCS main.cpp shell.c)

add_executable(${KDB_SQLITE_DUMP_TOOL} ${kdb_sqlite_dump_tool_SRCS})

target_link_libraries(${KDB_SQLITE_DUMP_TOOL}
    PRIVATE
        KDb
        ${SQLITE_LIBRARIES}
)

install(TARGETS ${KDB_SQLITE_DUMP_TOOL} ${INSTALL_TARGETS_DEFAULT_ARGS})

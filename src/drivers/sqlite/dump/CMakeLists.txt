set(predicate_sqlite_dump_tool_SRCS main.cpp shell.c)

add_executable(${PREDICATE_SQLITE_DUMP_TOOL} ${predicate_sqlite_dump_tool_SRCS})

target_link_libraries(${PREDICATE_SQLITE_DUMP_TOOL} ${SQLITE_LIBRARIES})

install(TARGETS ${PREDICATE_SQLITE_DUMP_TOOL} ${INSTALL_TARGETS_DEFAULT_ARGS})

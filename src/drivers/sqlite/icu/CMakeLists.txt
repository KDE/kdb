add_library(predicate_sqlite_icu SHARED icu.c)

include_directories(${ICU_INCLUDE_DIRS})

target_link_libraries(kdb_sqlite_icu
                      ${SQLITE_LIBRARIES}
                      ${ICU_LIBRARIES}
                      ${ICU_I18N_LIBRARIES}
)
set_target_properties(kdb_sqlite_icu PROPERTIES PREFIX ""
                      DEFINE_SYMBOL libsqlite3_EXPORTS)

install(TARGETS kdb_sqlite_icu DESTINATION ${PLUGIN_INSTALL_DIR})

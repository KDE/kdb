add_library(kdb_sqlite_icu MODULE icu.c)

include_directories(${ICU_INCLUDE_DIRS})

target_link_libraries(kdb_sqlite_icu
                      ${SQLITE_LIBRARIES}
                      ${ICU_LIBRARIES}
                      ${ICU_I18N_LIBRARIES}
)
set_target_properties(kdb_sqlite_icu PROPERTIES PREFIX ""
                      DEFINE_SYMBOL libsqlite3_EXPORTS)

install(TARGETS kdb_sqlite_icu
        DESTINATION ${KDB_PLUGIN_INSTALL_DIR}/sqlite3) # Use a subdir for clarity

add_library(kdb_sqlite_icu MODULE icu.cpp sqliteicu.h README.txt)

generate_export_header(kdb_sqlite_icu)

include_directories(${ICU_INCLUDE_DIRS})

target_link_libraries(kdb_sqlite_icu
                      ${SQLITE_LIBRARIES}
                      ${ICU_LIBRARIES}
                      ${ICU_I18N_LIBRARIES}
)
set_target_properties(kdb_sqlite_icu PROPERTIES
                      PREFIX ""
                      # nonstandard path for sqlite3 extensions, they would work too if we placed them
                      # in ${PROJECT_BINARY_DIR}/plugins/kdb/sqlite3 but we want to test the "extraSqliteExtensionPaths"
                      # connection option in ConnectionTest::testCreateDb():
                      LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/plugins/${KDB_BASE_NAME_LOWER}/sqlite3"
                      DEFINE_SYMBOL kdb_sqlite_icu_EXPORTS)

install(TARGETS kdb_sqlite_icu
        DESTINATION ${KDB_PLUGIN_INSTALL_DIR}/sqlite3) # Use a subdir for clarity

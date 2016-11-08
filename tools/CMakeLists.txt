add_custom_target(update_recommended_sqlite_ver echo "Updating recommended version of SQLite")

add_custom_command(
    TARGET update_recommended_sqlite_ver
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/update_recommended_sqlite_ver.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

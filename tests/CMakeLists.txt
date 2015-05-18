remove_definitions(
    -DQT_NO_KEYWORDS
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
    -DQT_NO_CAST_FROM_ASCII
)

add_definitions(-DKDBEXAMPLE_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(features)

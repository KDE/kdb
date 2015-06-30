remove_definitions(
    -DQT_NO_KEYWORDS
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
    -DQT_NO_CAST_FROM_ASCII
    -DQT_USE_QSTRINGBUILDER
)

set(FILES_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
add_definitions(
    -DFILES_OUTPUT_DIR=\"${FILES_OUTPUT_DIR}\"
    -DSQLITE_ICU_EXTENSION_PATH=\"${PROJECT_BINARY_DIR}/src/drivers/sqlite/icu\"
    -DYYTOKENTYPE # this removes access to yytokentype enum as we should access KDb::Token instead
)

include(ECMAddTests)

find_package(Qt5Test REQUIRED)

# A helper library for db-related tests
add_library(kdbtestutils SHARED
    KDbTestUtils.cpp
)
target_link_libraries(kdbtestutils
    PUBLIC
        KDb
    PRIVATE
        Qt5::Test
)
generate_export_header(kdbtestutils)

# Tests
ecm_add_tests(
    ConnectionTest.cpp
    DriverTest.cpp
    ExpressionsTest.cpp
    KDbTest.cpp

    LINK_LIBRARIES
        Qt5::Test
        KDb
        kdbtestutils
)

add_subdirectory(headers)
add_subdirectory(tools)
add_subdirectory(parser)

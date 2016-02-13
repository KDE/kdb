remove_definitions(
    -DQT_NO_KEYWORDS
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
    -DQT_NO_CAST_FROM_ASCII
    -DQT_USE_QSTRINGBUILDER
)

set(FILES_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(KDB_LOCAL_PLUGINS_DIR ${PROJECT_BINARY_DIR}/plugins)
add_definitions(
    -DFILES_OUTPUT_DIR=\"${FILES_OUTPUT_DIR}\"
     # make plugins work without installing them:
    -DKDB_LOCAL_PLUGINS_DIR=\"${KDB_LOCAL_PLUGINS_DIR}\"
     # nonstandard path for sqlite3 extensions, they would work too if we placed them
     # in ${KDB_LOCAL_PLUGINS_DIR}/sqlite3 but we want to test the "extraSqliteExtensionPaths"
     # connection option in ConnectionTest::testCreateDb():
    -DSQLITE_LOCAL_ICU_EXTENSION_PATH=\"${KDB_LOCAL_PLUGINS_DIR}/sqlite3\"
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

if(NOT WIN32) #TODO enable for Windows when headers_test.sh is ported e.g. to python
    add_subdirectory(headers)
endif()
add_subdirectory(tools)
add_subdirectory(parser)

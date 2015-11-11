add_definitions(-DFILES_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
add_definitions(-DOUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}")

ecm_add_test(
    SqlParserTest.cpp
    data/statements.txt
    data/sqlParserTest.kexi

    LINK_LIBRARIES
        kdbtestutils

    TEST_NAME SqlParserTest
)

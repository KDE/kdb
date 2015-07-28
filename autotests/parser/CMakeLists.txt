add_definitions(-DFILES_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

ecm_add_tests(
    SqlParserTest.cpp

    LINK_LIBRARIES
        Qt5::Test
        KDb
        kdbtestutils
)

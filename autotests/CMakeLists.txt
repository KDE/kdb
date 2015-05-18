remove_definitions(
    -DQT_NO_KEYWORDS
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
    -DQT_NO_CAST_FROM_ASCII
    -DQT_USE_QSTRINGBUILDER
)

set(FILES_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
add_definitions(-DFILES_OUTPUT_DIR=\"${FILES_OUTPUT_DIR}\")

include(ECMAddTests)

find_package(Qt5Test REQUIRED)

ecm_add_tests(
    ConnectionTest.cpp
    ExpressionsTest.cpp

    LINK_LIBRARIES KDb Qt5::Test
)

add_subdirectory(tools)
add_subdirectory(parser)

ecm_add_tests(
    IdentifierTest.cpp
    StaticSetOfStringsTest.cpp
    UtilsTest.cpp

    LINK_LIBRARIES KDb Qt${QT_MAJOR_VERSION}::Test
)

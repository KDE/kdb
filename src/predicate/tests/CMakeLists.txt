set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR} ${QT_INCLUDES} ${CMAKE_SOURCE_DIR}/Predicate )
enable_testing()

########### StaticSetOfStrings ###############

set(TestStaticSetOfStrings_SRCS TestStaticSetOfStrings.cpp)
set(TestStaticSetOfStrings_MOC_SRCS TestStaticSetOfStrings.h)
qt_wrap_cpp(TestStaticSetOfStrings TestStaticSetOfStrings_SRCS ${TestStaticSetOfStrings_MOC_SRCS})
add_executable(TestStaticSetOfStrings ${TestStaticSetOfStrings_SRCS})
target_link_libraries(TestStaticSetOfStrings predicate ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})
add_test(predicate-TestStaticSetOfStrings TestStaticSetOfStrings)
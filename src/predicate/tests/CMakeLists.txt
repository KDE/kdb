
macro(build_test __test_name)
    set(_test_name Test${__test_name})
    set(${_test_name}_SRCS ${_test_name}.cpp)
    set(${_test_name}_MOC_SRCS ${_test_name}.h)
    qt_wrap_cpp(${_test_name} ${_test_name}_SRCS ${${_test_name}_MOC_SRCS})
    add_executable(${_test_name} ${${_test_name}_SRCS})
    target_link_libraries(${_test_name} predicate ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})
    add_test(${_test_name} ${_test_name})
endmacro()

build_test(StaticSetOfStrings)
build_test(Expressions)

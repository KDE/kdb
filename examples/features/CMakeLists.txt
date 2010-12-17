include_directories(
  ${Predicate_SOURCE_DIR}
  ${Predicate_BINARY_DIR}
  ${QT_INCLUDES}
)

# temp
add_definitions(
  -DNO_GUI=1
)

########### next target ###############

set(predicatefeaturestest_SRCS main.cpp )

add_executable(predicatefeaturestest ${predicatefeaturestest_SRCS})

target_link_libraries(
  predicatefeaturestest
  ${QT_QTCORE_LIBRARY}
  predicate
)

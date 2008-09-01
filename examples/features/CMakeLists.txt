include_directories(
  ${CMAKE_SOURCE_DIR}/Predicate
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
  ${QT_INCLUDES}
  predicate
)

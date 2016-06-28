# temp
add_definitions(
  -DNO_GUI=1
)

########### next target ###############

set(kdbfeaturestest_SRCS
    main.cpp
    sqltest
    sqltest_int
    README
)

add_executable(kdbfeaturestest ${kdbfeaturestest_SRCS})

target_link_libraries(
  kdbfeaturestest
  Qt5::Core
  KDb
)

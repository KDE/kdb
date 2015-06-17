include_directories(
  ${XBase_INCLUDE_DIR}
)

add_definitions(
  -D__KDB__=
)

########### next target ###############

set(predicate_xbase_SRCS
   XbaseDriver.cpp
   XbaseConnection.cpp
   XbaseConnection_p.cpp
   XbaseCursor.cpp
   XbaseKeywords.cpp
   XbaseExport.cpp
   )


add_library(predicate_xbase SHARED ${predicate_xbase_SRCS})

target_link_libraries(predicate_xbase
  ${XBase_LIBRARIES}
  predicate
#TODO  keximigrate
)
set_target_properties(predicate_xbase PROPERTIES PREFIX "")

install(TARGETS predicate_xbase DESTINATION ${PREDICATE_PLUGINS_DIR})
install(FILES predicate_xbase.desktop DESTINATION ${PREDICATE_PLUGINS_DIR})

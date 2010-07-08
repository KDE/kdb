# define the generic version of the Predicate libraries here
# this makes it easy to advance it when the next Predicate release comes
set(PREDICATE_VERSION_MAJOR "1")
set(PREDICATE_VERSION_MINOR "00")
set(PREDICATE_VERSION_RELEASE "00")
set(PREDICATE_VERSION "1.0.0")
#todo add build date as in kdepim
set (PREDICATE_VERSION_STRING ${PREDICATE_VERSION})
set(GENERIC_PREDICATE_LIB_VERSION ${PREDICATE_VERSION})
set(GENERIC_PREDICATE_LIB_SOVERSION "0")

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/Predicate
  ${Predicate_BINARY_DIR}
  ${Predicate_BINARY_DIR}/Predicate
  ${QT_INCLUDES}
)

add_subdirectory( drivers )

ADD_DEFINITIONS(
#TODO  -DKDE_DEFAULT_DEBUG_AREA=44000
  -D__PREDICATE__= 
  -DMAKE_PREDICATE_LIB=1
  -DYYERROR_VERBOSE=1
)

if (WIN32)
  set(CMAKE_DEBUG_POSTFIX "d")
  ADD_DEFINITIONS(
    -DYY_NO_UNISTD_H=1
  )
endif (WIN32)

########### generate parser/lexer files ###############
# as described at http://public.kitware.com/pipermail/cmake/2002-September/003028.html

# Create target for the parser
ADD_CUSTOM_TARGET(parser echo "Creating parser/lexer files")

# Create custom command for flex/lex (note the outputs)
ADD_CUSTOM_COMMAND(
COMMAND ${CMAKE_SOURCE_DIR}/Predicate/parser/generate_parser_code.sh
TARGET parser
OUTPUTS
  ${CMAKE_SOURCE_DIR}/Predicate/parser/SqlParser.h
  ${CMAKE_SOURCE_DIR}/Predicate/parser/SqlParser.cpp
  ${CMAKE_SOURCE_DIR}/Predicate/parser/SqlScanner.h
  ${CMAKE_SOURCE_DIR}/Predicate/parser/SqlScanner.cpp)

# mark files as generated
SET_SOURCE_FILES_PROPERTIES(${CMAKE_SOURCE_DIR}/Predicate/parser/SqlParser.h GENERATED)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_SOURCE_DIR}/Predicate/parser/SqlParser.cpp GENERATED)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_SOURCE_DIR}/Predicate/parser/SqlScanner.h GENERATED)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_SOURCE_DIR}/Predicate/parser/SqlScanner.cpp GENERATED)

########### next target ###############

# Generate Global.h
configure_file(Global.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/Global.h )

set(predicate_parser_SRCS 
   parser/SqlScanner.cpp 
   parser/SqlParser.cpp 
   parser/Parser.cpp 
   parser/Parser_p.cpp )

set(predicate_LIB_SRCS 
   ${predicate_parser_SRCS}
   DriverManager.cpp 
   Driver.cpp 
   Driver_p.cpp 
   Connection.cpp 
   keywords.cpp 
   Object.cpp 
   Field.cpp 
   Utils.cpp 
   Expression.cpp 
   ConnectionData.cpp 
   RecordData.cpp
   FieldList.cpp 
   TableSchema.cpp 
   Cursor.cpp 
   Transaction.cpp 
   IndexSchema.cpp 
   QuerySchema.cpp 
   QuerySchemaParameter.cpp
   SchemaData.cpp 
   Global.cpp 
   Relationship.cpp 
   RowEditBuffer.cpp 
   MessageHandler.cpp 
   PreparedStatement.cpp 
   DbProperties.cpp 
   Admin.cpp 
   LookupFieldSchema.cpp 
   Alter.cpp

   tools/Validator.cpp
   tools/FieldValidator.cpp
   tools/LongLongValidator.cpp
   tools/DbObjectNameValidator.cpp 
   tools/Identifier.cpp
   tools/Utils.cpp
   tools/Static.cpp
   tools/transliteration/transliteration_table.cpp
#TODO tools/debuggui.cpp
#TODO tools/SimpleCommandLineApp.cpp
)

set(predicate_MOC_SRCS
  DriverManager_p.h
  Cursor.h
  Utils_p.h
  Driver.h
  Connection.h
)

qt_wrap_cpp(predicate predicate_LIB_SRCS ${predicate_MOC_SRCS})

add_library(predicate SHARED ${predicate_LIB_SRCS})
target_link_libraries(predicate
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTXML_LIBRARY}
#  kexiutils 
)

set_target_properties(predicate PROPERTIES 
  VERSION ${GENERIC_PREDICATE_LIB_VERSION}
  SOVERSION ${GENERIC_PREDICATE_LIB_SOVERSION}
)

install(TARGETS predicate ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES
  Admin.h
  Alter.h
  Connection.h
  Connection_p.h
  ConnectionData.h
  Cursor.h
  Cursor_p.h
  DbProperties.h
  Driver.h
  Driver_p.h
  DriverManager.h
  DriverManager_p.h
  Error.h
  Expression.h
  Field.h
  FieldList.h
  IndexSchema.h
  LookupFieldSchema.h
  MessageHandler.h
  Object.h
  predicate_export.h
  predicate_global.h
  PreparedStatement.h
  QuerySchema.h
  QuerySchemaParameter.h
  RecordData.h
  Relationship.h
  RowEditBuffer.h
  SchemaData.h
  TableSchema.h
  Transaction.h
  Utils.h
  Utils_p.h
  ${CMAKE_CURRENT_BINARY_DIR}/Global.h
  parser/Parser.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/Predicate COMPONENT Devel
)

install( FILES
  Interfaces/PreparedStatementInterface.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/Predicate/Interfaces COMPONENT Devel
)

install( FILES
  tools/Validator.h
  tools/Utils.h
  tools/Tristate.h
  tools/Static.h
#todo  tools/SimpleCommandLineApp.h
  tools/SharedPtr.h
  tools/LongLongValidator.h
  tools/Identifier.h
  tools/FieldValidator.h
  tools/DbObjectNameValidator.h

  DESTINATION ${INCLUDE_INSTALL_DIR}/Predicate/tools COMPONENT Devel
)

#TODO install( FILES kexidb_driver.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})

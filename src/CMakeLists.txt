
include_directories(
  ${Predicate_SOURCE_DIR}
  ${Predicate_SOURCE_DIR}/Predicate
  ${Predicate_BINARY_DIR}
  ${Predicate_BINARY_DIR}/Predicate
  ${QT_INCLUDES}
)

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
COMMAND ${Predicate_SOURCE_DIR}/Predicate/parser/generate_parser_code.sh
TARGET parser
OUTPUTS
  ${Predicate_SOURCE_DIR}/Predicate/parser/SqlParser.h
  ${Predicate_SOURCE_DIR}/Predicate/parser/SqlParser.cpp
  ${Predicate_SOURCE_DIR}/Predicate/parser/SqlScanner.h
  ${Predicate_SOURCE_DIR}/Predicate/parser/SqlScanner.cpp)

# mark files as generated
SET_SOURCE_FILES_PROPERTIES(${Predicate_SOURCE_DIR}/Predicate/parser/SqlParser.h GENERATED)
SET_SOURCE_FILES_PROPERTIES(${Predicate_SOURCE_DIR}/Predicate/parser/SqlParser.cpp GENERATED)
SET_SOURCE_FILES_PROPERTIES(${Predicate_SOURCE_DIR}/Predicate/parser/SqlScanner.h GENERATED)
SET_SOURCE_FILES_PROPERTIES(${Predicate_SOURCE_DIR}/Predicate/parser/SqlScanner.cpp GENERATED)

SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/ConnectionData_sdc.cpp GENERATED)

PREDICATE_CREATE_SHARED_DATA_CLASSES(
    Result.shared.h
    VersionInfo.shared.h
    ConnectionData.shared.h
    Object.shared.h
)

########### next target ###############

# Generate Global.h
configure_file(Global.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/Global.h )

set(predicate_LIB_SRCS
   EscapedString.cpp
   Result.cpp
   ConnectionData.cpp
   VersionInfo.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/ConnectionData_sdc.cpp
   Field.cpp
   QuerySchemaParameter.cpp
   Expression.cpp
   FieldList.cpp
   TableSchema.cpp
   IndexSchema.cpp
   QuerySchema.cpp
   DriverManager.cpp
   Driver.cpp
   Driver_p.cpp
   Connection.cpp
   keywords.cpp
   Object.cpp
   Utils.cpp
   RecordData.cpp
   Cursor.cpp
   Transaction.cpp
   Global.cpp
   Relationship.cpp
   RecordEditBuffer.cpp
   MessageHandler.cpp
   PreparedStatement.cpp
   DbProperties.cpp
   Admin.cpp
   LookupFieldSchema.cpp
   Alter.cpp

   parser/SqlScanner.cpp
   parser/SqlParser.cpp
   parser/Parser.cpp
   parser/Parser_p.cpp

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
#  Cursor.h
  Utils_p.h
  Driver.h
#  Connection.h
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
  ${CMAKE_CURRENT_BINARY_DIR}/ConnectionData.h
  Cursor.h
  Cursor_p.h
  DbProperties.h
  Driver.h
  Driver_p.h
  DriverManager.h
  DriverManager_p.h
  ${CMAKE_CURRENT_BINARY_DIR}/Result.h
  Error.h
  EscapedString.h
  Expression.h
  Field.h
  FieldList.h
  IndexSchema.h
  LookupFieldSchema.h
  MessageHandler.h
  ${CMAKE_CURRENT_BINARY_DIR}/Object.h
  predicate_export.h
  predicate_global.h
  PreparedStatement.h
  QuerySchema.h
  QuerySchemaParameter.h
  RecordData.h
  Relationship.h
  RecordEditBuffer.h
  TableSchema.h
  Transaction.h
  Utils.h
  ${CMAKE_CURRENT_BINARY_DIR}/Global.h
  ${CMAKE_CURRENT_BINARY_DIR}/VersionInfo.h
  parser/Parser.h
  Interfaces/PreparedStatementInterface.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/Predicate COMPONENT Devel
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

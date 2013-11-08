ADD_DEFINITIONS(
#TODO  -DKDE_DEFAULT_DEBUG_AREA=44000
  -D__PREDICATE__=
  -DMAKE_PREDICATE_LIB=1
  -DYYERROR_VERBOSE=1
)
if(CMAKE_BUILD_TYPE EQUAL Debug OR CMAKE_BUILD_TYPE EQUAL RelWithDebInfo)
  ADD_DEFINITIONS(
    -DYYDEBUG=1
  )
endif(CMAKE_BUILD_TYPE EQUAL Debug OR CMAKE_BUILD_TYPE EQUAL RelWithDebInfo)

if (WIN32)
  set(CMAKE_DEBUG_POSTFIX "d")
  ADD_DEFINITIONS(
    -DYY_NO_UNISTD_H=1
  )
endif (WIN32)

########### options ###############

option(PREDICATE_EXPRESSION_DEBUG "Enable debugging of Expression classes" OFF)
add_feature_info(PREDICATE_EXPRESSION_DEBUG PREDICATE_EXPRESSION_DEBUG "Enable debugging of Expression classes")

option(PREDICATE_DEBUG_GUI "Enable GUI for debugging" OFF)
add_feature_info(PREDICATE_DEBUG_GUI PREDICATE_DEBUG_GUI "Enable GUI for debugging")

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
   parser/SqlScanner.cpp
   parser/SqlParser.cpp
   parser/Parser.cpp
   parser/Parser_p.cpp

   Tools/Validator.cpp
   Tools/FieldValidator.cpp
   Tools/LongLongValidator.cpp
   Tools/DbObjectNameValidator.cpp
   Tools/Identifier.cpp
   Tools/Utils.cpp
   Tools/Static.cpp
   Tools/transliteration/transliteration_table.cpp
#TODO Tools/debuggui.cpp
#TODO Tools/SimpleCommandLineApp.cpp

   EscapedString.cpp
   Result.cpp
   ConnectionData.cpp
   VersionInfo.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/ConnectionData_sdc.cpp
   Field.cpp
   QuerySchemaParameter.cpp
   expression/Expression.cpp
   expression/NArgExpression.cpp
   expression/UnaryExpression.cpp
   expression/BinaryExpression.cpp
   expression/ConstExpression.cpp
   expression/QueryParameterExpression.cpp
   expression/VariableExpression.cpp
   expression/FunctionExpression.cpp
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
#TODO Alter.cpp
)

set(predicate_MOC_SRCS
  DriverManager_p.h
#  Cursor.h
  Utils_p.h
  Driver.h
  Field.h
#  Connection.h
)

qt_wrap_cpp(predicate predicate_LIB_SRCS ${predicate_MOC_SRCS})

add_library(predicate SHARED ${predicate_LIB_SRCS})
target_link_libraries(predicate
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTXML_LIBRARY}
)

if (BUILD_TEST_COVERAGE)
    target_link_libraries(predicate gcov)
endif (BUILD_TEST_COVERAGE)

set_target_properties(predicate PROPERTIES
  VERSION ${GENERIC_PREDICATE_LIB_VERSION}
  SOVERSION ${GENERIC_PREDICATE_LIB_SOVERSION}
)

install(TARGETS predicate ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES
  Admin.h includes/Predicate/Admin
#TODO  Alter.h includes/Predicate/Alter
  Connection.h includes/Predicate/Connection
  ${CMAKE_CURRENT_BINARY_DIR}/ConnectionData.h includes/Predicate/ConnectionData
  Cursor.h includes/Predicate/Cursor
  DbProperties.h includes/Predicate/DbProperties
  Driver.h includes/Predicate/Driver
  DriverManager.h includes/Predicate/DriverManager
  ${CMAKE_CURRENT_BINARY_DIR}/Result.h includes/Predicate/Result
  Error.h includes/Predicate/Error
  EscapedString.h includes/Predicate/EscapedString
  expression/Expression.h includes/Predicate/Expression
  expression/ExpressionData.h
  Field.h includes/Predicate/Field
  FieldList.h includes/Predicate/FieldList
  IndexSchema.h includes/Predicate/IndexSchema
  LookupFieldSchema.h includes/Predicate/LookupFieldSchema
  MessageHandler.h includes/Predicate/MessageHandler
  ${CMAKE_CURRENT_BINARY_DIR}/Object.h includes/Predicate/Object
  predicate_export.h
  predicate_global.h
  PreparedStatement.h includes/Predicate/PreparedStatement
  QuerySchema.h includes/Predicate/QuerySchema
  QuerySchemaParameter.h includes/Predicate/QuerySchemaParameter
  RecordData.h includes/Predicate/RecordData
  Relationship.h includes/Predicate/Relationship
  RecordEditBuffer.h includes/Predicate/RecordEditBuffer
  TableSchema.h includes/Predicate/TableSchema
  Transaction.h includes/Predicate/Transaction
  Utils.h includes/Predicate/Utils
  ${CMAKE_CURRENT_BINARY_DIR}/Global.h  includes/Predicate/Global
  ${CMAKE_CURRENT_BINARY_DIR}/VersionInfo.h includes/Predicate/VersionInfo
  parser/Parser.h includes/Predicate/Parser
  DESTINATION ${INCLUDE_INSTALL_DIR}/Predicate COMPONENT Devel
)

# Predicate/Private includes
install( FILES
  Connection_p.h includes/Predicate/Private/Connection
  Driver_p.h includes/Predicate/Private/Driver
  DESTINATION ${INCLUDE_INSTALL_DIR}/Predicate/Private COMPONENT Devel
)

# Predicate/Interfaces includes
install( FILES
  Interfaces/PreparedStatementInterface.h includes/Predicate/Interfaces/PreparedStatementInterface
  DESTINATION ${INCLUDE_INSTALL_DIR}/Predicate/Interfaces COMPONENT Devel
)

install( FILES
  Tools/Validator.h includes/Predicate/Tools/Validator
  Tools/Utils.h includes/Predicate/Tools/Utils
  Tools/Tristate.h includes/Predicate/Tools/Tristate
  Tools/Static.h includes/Predicate/Tools/Static
#todo  Tools/SimpleCommandLineApp.h includes/Predicate/Tools/SimpleCommandLineApp
#  Tools/SharedPtr.h
  Tools/LongLongValidator.h
  Tools/Identifier.h
  Tools/FieldValidator.h
  Tools/DbObjectNameValidator.h

  DESTINATION ${INCLUDE_INSTALL_DIR}/Predicate/Tools COMPONENT Devel
)

enable_testing()
add_subdirectory( expression )
add_subdirectory( parser )
add_subdirectory( Tools )

#TODO install( FILES kexidb_driver.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})

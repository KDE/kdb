# define the generic version of the Predicate libraries here
# this makes it easy to advance it when the next Predicate release comes
set(GENERIC_PREDICATE_LIB_VERSION "0.0.0")
set(GENERIC_PREDICATE_LIB_SOVERSION "0")

find_package(Qt4 REQUIRED)

add_subdirectory( tools drivers )

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/Predicate
  ${QT_INCLUDES}
)

ADD_DEFINITIONS(
#TODO  -DKDE_DEFAULT_DEBUG_AREA=44000
  -D__PREDICATE__= 
  -DYYERROR_VERBOSE=1
)

if (WIN32)
 set(CMAKE_DEBUG_POSTFIX "d")
endif (WIN32)

########### generate parser/lexer files ###############
# as described at http://public.kitware.com/pipermail/cmake/2002-September/003028.html

# Create target for the parser
ADD_CUSTOM_TARGET(parser echo "Creating parser/lexer files")

# Create custom command for flex/lex (note the outputs)
ADD_CUSTOM_COMMAND(
COMMAND ${CMAKE_SOURCE_DIR}/src/parser/generate_parser_code.sh
TARGET parser
OUTPUTS
  ${CMAKE_SOURCE_DIR}/src/parser/SqlParser.h
  ${CMAKE_SOURCE_DIR}/src/parser/SqlParser.cpp
  ${CMAKE_SOURCE_DIR}/src/parser/SqlScanner.h
  ${CMAKE_SOURCE_DIR}/src/parser/SqlScanner.cpp)

# mark files as generated
SET_SOURCE_FILES_PROPERTIES(${CMAKE_SOURCE_DIR}/src/parser/SqlParser.h GENERATED)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_SOURCE_DIR}/src/parser/SqlParser.cpp GENERATED)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_SOURCE_DIR}/src/parser/SqlScanner.h GENERATED)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_SOURCE_DIR}/src/parser/SqlScanner.cpp GENERATED)

########### next target ###############

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
   ${CMAKE_CURRENT_BINARY_DIR}/tools/transliteration/transliteration_table.cpp
#TODO tools/debuggui.cpp
#TODO tools/SimpleCommandLineApp.cpp
)

add_library(predicate SHARED ${predicate_LIB_SRCS})
target_link_libraries(predicate
  ${QT_QTGUI_LIBRARY}
#  kexiutils 
)

set_target_properties(predicate PROPERTIES 
  VERSION ${GENERIC_PREDICATE_LIB_VERSION}
  SOVERSION ${GENERIC_PREDICATE_LIB_SOVERSION}
)

if(MSVC)
   # avoid LNK1169 errors
   set_target_properties(predicate PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif(MSVC)

install(TARGETS predicate DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES  Connection.h ConnectionData.h Cursor.h 
 Driver.h DriveManager.h Error.h Expression.h Field.h FieldList.h Global.h Object.h 
 SchemaData.h TableSchema.h QuerySchema.h QuerySchemaParameter.h IndexSchema.h 
 Relationship.h parser/Parser.h parser/SqlParser.h Transaction.h PreparedStatement.h  
 RecordData.h Utils.h
 predicate_export.h predicate_global.h DESTINATION ${INCLUDE_INSTALL_DIR}/predicate)

#TODO install( FILES kexidb_driver.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})

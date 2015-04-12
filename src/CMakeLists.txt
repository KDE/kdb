find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Widgets Xml)

ADD_DEFINITIONS(
#TODO  -DKDE_DEFAULT_DEBUG_AREA=44000
  -D__KDB__=
  -DMAKE_KDB_LIB=1
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

simple_option(KDB_EXPRESSION_DEBUG "Debugging of Expression classes" OFF)
simple_option(KDB_DEBUG_GUI "GUI for debugging" OFF)

########### generate parser/lexer files ###############
# as described at http://public.kitware.com/pipermail/cmake/2002-September/003028.html

# Create target for the parser
ADD_CUSTOM_TARGET(parser echo "Creating parser/lexer files")

# Create custom command for flex/lex (note the outputs)
ADD_CUSTOM_COMMAND(
COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/parser/generate_parser_code.sh
TARGET parser
OUTPUTS
  ${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/KDbSqlParser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/KDbSqlParser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/KDbSqlScanner.h
  ${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/KDbSqlScanner.cpp)

# mark files as generated
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/KDbSqlParser.h GENERATED)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/KDbSqlParser.cpp GENERATED)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/KDbSqlScanner.h GENERATED)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/KDbSqlScanner.cpp GENERATED)

SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/kdb/KDbConnectionData_sdc.cpp GENERATED)

########### next target ###############

# Generate KDbGlobal.h
configure_file(KDbGlobal.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/KDbGlobal.h )

set(kdb_LIB_SRCS
   parser/generated/sqlscanner.cpp
   parser/generated/sqlparser.cpp
   parser/KDbParser.cpp
   parser/KDbParser_p.cpp

   tools/KDbValidator.cpp
   tools/KDbFieldValidator.cpp
   tools/KDbLongLongValidator.cpp
   tools/KDbObjectNameValidator.cpp
   tools/KDbIdentifierValidator.cpp
   tools/KDbUtils.cpp
   tools/transliteration/transliteration_table.cpp
#TODO tools/debuggui.cpp
#TODO tools/SimpleCommandLineApp.cpp

   KDbEscapedString.cpp
   KDbResult.cpp
   KDbConnectionData.cpp
   KDbVersionInfo.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/kdb/KDbConnectionData_sdc.cpp
   KDbField.cpp
   KDbQuerySchemaParameter.cpp
   expression/KDbExpression.cpp
   expression/KDbNArgExpression.cpp
   expression/KDbUnaryExpression.cpp
   expression/KDbBinaryExpression.cpp
   expression/KDbConstExpression.cpp
   expression/KDbQueryParameterExpression.cpp
   expression/KDbVariableExpression.cpp
   expression/KDbFunctionExpression.cpp
   KDbFieldList.cpp
   KDbTableSchema.cpp
   KDbIndexSchema.cpp
   KDbQuerySchema.cpp
   KDbQueryColumnInfo.cpp
   KDbDriverManager.cpp
   KDbDriver.cpp
   KDbDriver_p.cpp
   KDbConnection.cpp
   generated/sqlkeywords.cpp
   KDbObject.cpp
   KDb.cpp
   KDbRecordData.cpp
   KDbCursor.cpp
   KDbTransaction.cpp
   KDbGlobal.cpp
   KDbRelationship.cpp
   KDbRecordEditBuffer.cpp
   KDbMessageHandler.cpp
   KDbPreparedStatement.cpp
   KDbProperties.cpp
   KDbAdmin.cpp
   KDbLookupFieldSchema.cpp
   KDbAlter.cpp

   views/KDbTableViewData.cpp
   views/KDbTableViewColumn.cpp
)

add_library(KDb SHARED ${kdb_LIB_SRCS})

KDB_CREATE_SHARED_DATA_CLASSES(
    kdb_GENERATED_SHARED_DATA_CLASS_HEADERS # output variable with list of headers
    kdb # subdirectory in which the headers should be generated
    KDbResult.shared.h
    KDbVersionInfo.shared.h
    KDbConnectionData.shared.h
    KDbObject.shared.h
)

KDB_REMOVE_EXTENSIONS(
    kdb_GENERATED_SHARED_DATA_CLASS_BASENAMES
    ${kdb_GENERATED_SHARED_DATA_CLASS_HEADERS}
)

#message(STATUS "kdb_GENERATED_SHARED_DATA_CLASS_HEADERS: ${kdb_GENERATED_SHARED_DATA_CLASS_HEADERS}")

#add_dependencies(KDb _shared_classes) # generate shared classes before they can be used in KDb

generate_export_header(KDb)

target_include_directories(KDb
    INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/KDb>"
    PRIVATE expression generated interfaces parser parser/generated tools views
            ${CMAKE_CURRENT_BINARY_DIR}/kdb
)

target_link_libraries(KDb
    PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
    PRIVATE
        Qt5::Xml
)

if (BUILD_TEST_COVERAGE)
    target_link_libraries(KDb
        PRIVATE
            gcov
    )
endif (BUILD_TEST_COVERAGE)

set_target_properties(KDb
    PROPERTIES VERSION ${GENERIC_KDB_LIB_VERSION}
               SOVERSION ${GENERIC_KDB_LIB_SOVERSION}
               EXPORT_NAME KDb
)

# Create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KDb")

ecm_setup_version(${KDB_VERSION}
    VARIABLE_PREFIX KDB
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdb_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KDbConfigVersion.cmake"
)

ecm_configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KDbConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KDbConfig.cmake"
    INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

########### install files ###############

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/KDbConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/KDbConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)

install(TARGETS KDb
        EXPORT KDbTargets
        ${INSTALL_TARGETS_DEFAULT_ARGS})

install(EXPORT KDbTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KDbTargets.cmake)

ecm_generate_pri_file(
    BASE_NAME KDb
    LIB_NAME KDb
    DEPS "widgets xml"
    FILENAME_VAR PRI_FILENAME
    INCLUDE_INSTALL_DIR ${INCLUDE_INSTALL_DIR}/KDb
)

install(FILES ${PRI_FILENAME}
        DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/kdb_version.h"
        DESTINATION "${INCLUDE_INSTALL_DIR}"
        COMPONENT Devel)

ecm_generate_headers(kdb_FORWARDING_HEADERS
    ORIGINAL CAMELCASE
    PREFIX kdb
    HEADER_NAMES
        KDbAdmin
        KDbAlter
        KDbConnection
        KDbCursor
        KDbProperties
        KDbDriver
        KDbDriverInfo
        KDbDriverManager
        KDbError
        KDbEscapedString
        KDbField
        KDbFieldList
        KDbIndexSchema
        KDbLookupFieldSchema
        KDbMessageHandler
        KDbPreparedStatement
        KDbQuerySchema
        KDbQueryColumnInfo
        KDbQuerySchemaParameter
        KDbRecordData
        KDbRecordEditBuffer
        KDbRelationship
        KDbTableSchema
        KDbTransaction
        KDb

        expression/KDbExpression
        expression/KDbExpressionData

        interfaces/KDbPreparedStatementInterface

        parser/KDbParser

        views/KDbTableViewData
        views/KDbTableViewColumn

        tools/KDbValidator
        #tools/Utils
        tools/KDbTristate
        #Qt5 tools/Static
        #todo  tools/SimpleCommandLineApp
        #  tools/SharedPtr.h
        tools/KDbLongLongValidator
        tools/KDbIdentifierValidator
        tools/KDbFieldValidator
        tools/KDbObjectNameValidator

    REQUIRED_HEADERS kdb_HEADERS
)

#message(STATUS "%% ${kdb_GENERATED_SHARED_DATA_CLASS_BASENAMES}")

ecm_generate_headers(kdb_FORWARDING_HEADERS_FROM_BUILDDIR
    ORIGINAL CAMELCASE
    PREFIX kdb
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}
    HEADER_NAMES
        ${kdb_GENERATED_SHARED_DATA_CLASS_BASENAMES}
        KDbGlobal

    REQUIRED_HEADERS kdb_HEADERS_FROM_BUILDDIR
)

install(
    FILES
        ${kdb_HEADERS}
        ${kdb_HEADERS_FROM_BUILDDIR}
        ${PROJECT_BINARY_DIR}/src/kdb_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/KDb/kproperty
    COMPONENT Devel
)

install(
    FILES
        ${kdb_FORWARDING_HEADERS}
        ${kdb_FORWARDING_HEADERS_FROM_BUILDDIR}
    DESTINATION ${INCLUDE_INSTALL_DIR}/KDb/KDb
    COMPONENT Devel
)

# KDb/Private includes
# install( FILES
#   Connection_p.h
#   Driver_p.h
#   DESTINATION ${INCLUDE_INSTALL_DIR}/KDb/Private COMPONENT Devel
# )

# KDb/Interfaces includes
# install( FILES
#   Interfaces/KDbPreparedStatementInterface.h includes/KDb/Interfaces/KDbPreparedStatementInterface
#   DESTINATION ${INCLUDE_INSTALL_DIR}/KDb/Interfaces COMPONENT Devel
# )

enable_testing()

#TODO install( FILES kexidb_driver.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})

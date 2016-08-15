# Private options (visible only within KDb)
simple_option(KDB_EXPRESSION_DEBUG "Debugging of Expression classes" OFF)
simple_option(KDB_DRIVERMANAGER_DEBUG "Debugging of the Driver Manager class" OFF)
simple_option(KDB_TRANSACTIONS_DEBUG "Debugging of the Transaction class" OFF)

# Public options (affecting public behavior or contents of KDb)
simple_option(KDB_DEBUG_GUI "GUI for debugging" OFF)
# NOTE: always add public options to KDbConfig.cmake.in as well

include(CheckIncludeFile)
check_include_file(unistd.h HAVE_UNISTD_H)
configure_file(config-kdb.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdb.h)

add_subdirectory(drivers)

#add_definitions(
#    TODO  -DKDE_DEFAULT_DEBUG_AREA=44000
#)

########### generate parser/lexer files ###############
# as described at http://public.kitware.com/pipermail/cmake/2002-September/003028.html

# Create target for the parser
add_custom_target(parser echo "Creating parser/lexer files")

set(PARSER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/parser)

# Create custom command for flex/lex (note the outputs)
# TODO(GEN) uncomment GENERATED if we ever use this patch: https://phabricator.kde.org/D357 "No more generated parser/scanner files in the source dir"
add_custom_command(
    TARGET parser
    COMMAND ${PARSER_SOURCE_DIR}/generate_parser_code.sh
    DEPENDS ${PARSER_SOURCE_DIR}/KDbSqlParser.y
            ${PARSER_SOURCE_DIR}/KDbSqlScanner.l
            ${PARSER_SOURCE_DIR}/generate_parser_code.sh
    OUTPUT
    #TODO(GEN)     ${PARSER_SOURCE_DIR}/generated/sqlparser.h
    #TODO(GEN)     ${PARSER_SOURCE_DIR}/generated/sqlparser.cpp
    #TODO(GEN)     ${PARSER_SOURCE_DIR}/generated/sqlscanner.cpp
    #TODO(GEN)     ${PARSER_SOURCE_DIR}/generated/KDbToken.h
    #TODO(GEN)     ${PARSER_SOURCE_DIR}/generated/KDbToken.cpp
)

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
if("${CMAKE_BUILD_TYPE_LOWER}" MATCHES "debug")
   add_definitions(-DYYDEBUG=1) # needed where sqlparser.h is used
endif()

if(NOT HAVE_UNISTD_H)
    set(EXTRA_SCANNER_COMPILE_FLAGS "-DYY_NO_UNISTD_H=1")
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANG)
    set(EXTRA_SCANNER_COMPILE_FLAGS "${EXTRA_SCANNER_COMPILE_FLAGS} -Wno-sign-compare -Wno-unused-function -Wno-deprecated-register")
elseif(MSVC)
    set(EXTRA_SCANNER_COMPILE_FLAGS "${EXTRA_SCANNER_COMPILE_FLAGS} /wd4018") # disable warning C4018: '<' : signed/unsigned mismatch
endif()

# Mark files as generated, set compile flags
set_source_files_properties(${PARSER_SOURCE_DIR}/generated/sqlparser.cpp
    PROPERTIES
        GENERATED TRUE
        # YYERROR_VERBOSE=1 needed to get a token table for tokenName() even for release builds
        COMPILE_FLAGS "-DYYERROR_VERBOSE=1 ${EXTRA_PARSER_COMPILE_FLAGS} "
)
set_source_files_properties(${PARSER_SOURCE_DIR}/generated/sqlparser.h PROPERTIES GENERATED TRUE)
set_source_files_properties(${PARSER_SOURCE_DIR}/generated/KDbToken.h PROPERTIES GENERATED TRUE)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/KDbConnectionData_sdc.cpp PROPERTIES GENERATED TRUE)

set_source_files_properties(
    ${PARSER_SOURCE_DIR}/generated/sqlscanner.cpp
    PROPERTIES
        GENERATED TRUE
        COMPILE_FLAGS "${EXTRA_SCANNER_COMPILE_FLAGS} "
)

set(kdb_LIB_SRCS
   parser/generated/sqlscanner.cpp
   parser/generated/sqlparser.cpp
   parser/generated/KDbToken.cpp
   parser/KDbParser.cpp
   parser/KDbParser_p.cpp
   parser/KDbSqlParser.y
   parser/KDbSqlScanner.l
   parser/generate_parser_code.sh
   parser/extract_tokens.sh
   parser/TODO

   tools/KDbJsonTrader_p.cpp # mostly copied from KReport's KReportJsonTrader_p.cpp
   tools/KDbValidator.cpp
   tools/KDbFieldValidator.cpp
   tools/KDbLongLongValidator.cpp
   tools/KDbObjectNameValidator.cpp
   tools/KDbIdentifierValidator.cpp
   tools/KDbUtils.cpp
   #TODO tools/debuggui.cpp
   #TODO tools/SimpleCommandLineApp.cpp

   tools/transliteration/transliteration_table.cpp
   tools/transliteration/generate_transliteration_table.sh
   tools/transliteration/transliteration_table.readme

   KDbEscapedString.cpp
   KDbResult.cpp
   KDbConnectionData.cpp
   KDbVersionInfo.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/KDbConnectionData_sdc.cpp
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
   KDbQuerySchema_p.cpp
   KDbQueryColumnInfo.cpp
   KDbTableOrQuerySchema.cpp
   KDbDriverManager.cpp
   KDbDriver.cpp
   KDbDriver_p.cpp
   KDbDriverMetaData.cpp
   KDbConnection.cpp
   KDbConnectionProxy.cpp
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
   KDbNativeStatementBuilder.cpp
   kdb_debug.cpp

   views/KDbTableViewData.cpp
   views/KDbTableViewColumn.cpp
   views/chartable.txt

   sql/KDbSqlField.cpp
   sql/KDbSqlRecord.cpp
   sql/KDbSqlResult.cpp

   # non-source:
   Mainpage.dox
   Messages.sh
)

ecm_create_qm_loader(kdb_LIB_SRCS kdb_qt)

add_library(KDb SHARED ${kdb_LIB_SRCS})

kdb_create_shared_data_classes(
    kdb_GENERATED_SHARED_DATA_CLASS_HEADERS # output variable with list of headers
    NO_PREFIX # subdirectory in which the headers should be generated
    KDbResult.shared.h
    KDbVersionInfo.shared.h
    KDbConnectionData.shared.h
    KDbObject.shared.h
)

kdb_remove_extensions(
    kdb_GENERATED_SHARED_DATA_CLASS_BASENAMES
    ${kdb_GENERATED_SHARED_DATA_CLASS_HEADERS}
)

#message(STATUS "kdb_GENERATED_SHARED_DATA_CLASS_HEADERS: ${kdb_GENERATED_SHARED_DATA_CLASS_HEADERS}")

#add_dependencies(KDb _shared_classes) # generate shared classes before they can be used in KDb

generate_export_header(KDb)

set(kdb_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/expression
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/parser
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/generated
    ${CMAKE_CURRENT_SOURCE_DIR}/sql
    ${CMAKE_CURRENT_SOURCE_DIR}/tools
    ${CMAKE_CURRENT_SOURCE_DIR}/views
)

target_include_directories(KDb
    PUBLIC "$<BUILD_INTERFACE:${kdb_INCLUDE_DIRS}>"
    INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/KDb>"
    PRIVATE expression generated interfaces parser tools views ${ICU_INCLUDE_DIRS}
)

target_link_libraries(KDb
    PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        KF5::CoreAddons
    PRIVATE
        Qt5::Xml
        ${ICU_I18N_LIBRARY}
)

if(BUILD_TEST_COVERAGE)
    target_link_libraries(KDb
        PRIVATE
            gcov
    )
endif()

set_target_properties(KDb
    PROPERTIES VERSION 1.0
               SOVERSION 1.0.0
               EXPORT_NAME KDb
)

# Create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KDb")

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX KDB
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdb_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KDbConfigVersion.cmake"
)

install(TARGETS KDb
        EXPORT KDbTargets
        ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# Create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KDb")

ecm_configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KDbConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KDbConfig.cmake"
    INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/KDbConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/KDbConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)

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
    REQUIRED_HEADERS kdb_HEADERS
    ORIGINAL CAMELCASE
    HEADER_NAMES
        KDb
        KDbGlobal
        KDbAdmin
        KDbAlter
        KDbConnection
        KDbConnectionProxy
        KDbCursor
        KDbProperties
        KDbDriver
        KDbDriverMetaData
        KDbDriverManager
        KDbError
        KDbEscapedString
        KDbField
        KDbFieldList
        KDbIndexSchema
        KDbLookupFieldSchema
        KDbMessageHandler
        KDbNativeStatementBuilder
        KDbPreparedStatement
        KDbQuerySchema
        KDbQueryColumnInfo
        KDbQuerySchemaParameter
        KDbRecordData
        KDbRecordEditBuffer
        KDbRelationship
        KDbTableSchema
        KDbTableOrQuerySchema
        KDbTransaction
)

ecm_generate_headers(kdb_FORWARDING_HEADERS
    REQUIRED_HEADERS kdb_HEADERS
    ORIGINAL CAMELCASE
    RELATIVE expression
    HEADER_NAMES
        KDbExpression
        KDbExpressionData
)

ecm_generate_headers(kdb_FORWARDING_HEADERS
    REQUIRED_HEADERS kdb_HEADERS
    ORIGINAL CAMELCASE
    RELATIVE interfaces
    HEADER_NAMES
        KDbPreparedStatementInterface
)

ecm_generate_headers(kdb_FORWARDING_HEADERS
    REQUIRED_HEADERS kdb_HEADERS
    ORIGINAL CAMELCASE
    RELATIVE parser
    HEADER_NAMES
        KDbParser
)

ecm_generate_headers(kdb_FORWARDING_HEADERS
    REQUIRED_HEADERS kdb_HEADERS
    ORIGINAL CAMELCASE
    RELATIVE parser/generated
    HEADER_NAMES
        KDbToken
)

ecm_generate_headers(kdb_FORWARDING_HEADERS
    REQUIRED_HEADERS kdb_HEADERS
    ORIGINAL CAMELCASE
    RELATIVE sql
    HEADER_NAMES
        KDbSqlField
        KDbSqlRecord
        KDbSqlResult
        KDbSqlString
)

ecm_generate_headers(kdb_FORWARDING_HEADERS
    REQUIRED_HEADERS kdb_HEADERS
    ORIGINAL CAMELCASE
    RELATIVE views
    HEADER_NAMES
        KDbTableViewData
        KDbTableViewColumn
)

ecm_generate_headers(kdb_FORWARDING_HEADERS
    REQUIRED_HEADERS kdb_HEADERS
    ORIGINAL CAMELCASE
    RELATIVE tools
    HEADER_NAMES
        KDbValidator
        KDbUtils
        KDbTristate
        #todo SimpleCommandLineApp
        KDbLongLongValidator
        KDbIdentifierValidator
        KDbFieldValidator
        KDbObjectNameValidator
)

#message(STATUS "%% ${kdb_GENERATED_SHARED_DATA_CLASS_BASENAMES}")

ecm_generate_headers(kdb_FORWARDING_HEADERS_FROM_BUILDDIR
    REQUIRED_HEADERS kdb_HEADERS_FROM_BUILDDIR
    ORIGINAL CAMELCASE
    SOURCE_DIR ${PROJECT_BINARY_DIR}/src
    HEADER_NAMES
        ${kdb_GENERATED_SHARED_DATA_CLASS_BASENAMES}
)

#message(STATUS "%%kdb_HEADERS_FROM_BUILDDIR ${kdb_HEADERS_FROM_BUILDDIR}")

install(
    FILES
        ${kdb_HEADERS}
        ${kdb_HEADERS_FROM_BUILDDIR}
    DESTINATION ${INCLUDE_INSTALL_DIR}/KDb
    COMPONENT Devel
)

install(
    FILES
        ${kdb_FORWARDING_HEADERS}
        ${kdb_FORWARDING_HEADERS_FROM_BUILDDIR}
        ${PROJECT_BINARY_DIR}/src/kdb_export.h
        ${PROJECT_BINARY_DIR}/src/config-kdb.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/KDb
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

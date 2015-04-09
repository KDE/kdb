# This file defines the KDB_CREATE_SHARED_DATA_CLASSES macro.
#
# KDB_CREATE_SHARED_DATA_CLASSES(LIST)
#   Creates header file out of the LIST files in .shared.h format.
#   FILES should be a list of relative paths to ${CMAKE_SOURCE_DIR}.
#
# Following example creates ${CMAKE_CURRENT_BINARY_DIR}/Foo.h
# and ${CMAKE_CURRENT_BINARY_DIR}/Baz.h files:
#
# INCLUDE(KDbCreateSharedDataClass)
# KDB_CREATE_SHARED_DATA_CLASS(OUTPUT_VAR Bar/Foo.shared.h Baz.shared.h)
#
# Copyright (C) 2010 Jarosław Staniek <staniek@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

MACRO(KDB_CREATE_SHARED_DATA_CLASSES)
    set(_args "")
    list(APPEND _args ${ARGV})
    list(GET _args 0 OUTPUT_VAR)
    list(GET _args 1 PREFIX)
    list(REMOVE_AT _args 0 1)
    # message(STATUS "OUTPUT_VAR: ${OUTPUT_VAR} ${_args}")
    FOREACH(_input ${_args})
        GET_FILENAME_COMPONENT(INPUT ${_input} ABSOLUTE)
        STRING(REGEX REPLACE "\\.shared\\.h" ".h" OUTPUT ${_input})
        STRING(REGEX REPLACE ".*/([^/]+)\\.h" "\\1.h" OUTPUT ${OUTPUT})
        #MESSAGE(DEBUG "--------- ${_input} ${OUTPUT} ${INPUT}")
        #MESSAGE(DEBUG "COMMAND python ${CMAKE_SOURCE_DIR}/tools/sdc.py ${INPUT} ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT}")
        MESSAGE(STATUS "Creating shared data class in ${OUTPUT} from ${_input}")
        EXECUTE_PROCESS(
            COMMAND python ${CMAKE_SOURCE_DIR}/tools/sdc.py
                           ${INPUT}
                           ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}/${OUTPUT}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            RESULT_VARIABLE KDB_CREATE_SHARED_DATA_CLASSES_RESULT
        )
        # MESSAGE(STATUS "...result: ${KDB_CREATE_SHARED_DATA_CLASSES_RESULT}")
        if (PREFIX)
            set(OUTPUT "${PREFIX}/${OUTPUT}")
        endif(PREFIX)
        list(APPEND ${OUTPUT_VAR} "${OUTPUT}")
    ENDFOREACH(_input)
ENDMACRO(KDB_CREATE_SHARED_DATA_CLASSES)

MACRO(KDB_REMOVE_EXTENSIONS)
    # message(STATUS "ARGV: ${ARGV}")
    set(_args "")
    list(APPEND _args ${ARGV})
    list(GET _args 0 OUTPUT_VAR)
    list(REMOVE_AT _args 0)
    # message(STATUS "OUTPUT_VAR: ${OUTPUT_VAR} ${_args}")
    FOREACH(_input ${_args})
        STRING(REGEX REPLACE "\\.h" "" OUTPUT ${_input})
        MESSAGE(STATUS "...result: ${OUTPUT}")
        list(APPEND ${OUTPUT_VAR} "${OUTPUT}")
    ENDFOREACH(_input)
ENDMACRO(KDB_REMOVE_EXTENSIONS)

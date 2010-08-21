# This file defines the PREDICATE_CREATE_SHARED_DATA_CLASSES macro.
#
# PREDICATE_CREATE_SHARED_DATA_CLASSES(LIST)
#   Creates header file out of the LIST files in .shared.h format.
#   FILES should be a list of relative paths to ${CMAKE_SOURCE_DIR}.
#
# Following example creates ${CMAKE_CURRENT_BINARY_DIR}/Foo.h
# and ${CMAKE_CURRENT_BINARY_DIR}/Baz.h files:
#
# INCLUDE(CreateSharedDataClass)
# PREDICATE_CREATE_SHARED_DATA_CLASS(Bar/Foo.shared.h Baz.shared.h)
#
# Copyright (C) 2010 Jarosław Staniek <staniek@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

MACRO(PREDICATE_CREATE_SHARED_DATA_CLASSES)
    SET(_outputs "")
    FOREACH(_input ${ARGV})
        GET_FILENAME_COMPONENT(INPUT ${_input} ABSOLUTE)
        STRING(REGEX REPLACE "\\.shared\\.h" ".h" OUTPUT ${_input})
        STRING(REGEX REPLACE ".*/([^/]+)\\.h" "\\1.h" OUTPUT ${OUTPUT})
        #MESSAGE(DEBUG "--------- ${_input} ${OUTPUT} ${INPUT}")
        #MESSAGE(DEBUG "COMMAND python ${CMAKE_SOURCE_DIR}/tools/sdc.py ${INPUT} ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT}")
        ADD_CUSTOM_COMMAND(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT}
            COMMAND python ${CMAKE_SOURCE_DIR}/tools/sdc.py
            ARGS ${INPUT} ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Creating shared data class in ${OUTPUT} from ${_input}"
        )
        list(APPEND _outputs "${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT}")
    ENDFOREACH(_input)
    ADD_CUSTOM_TARGET(_shared_classes ALL DEPENDS ${_outputs})
ENDMACRO(PREDICATE_CREATE_SHARED_DATA_CLASSES)

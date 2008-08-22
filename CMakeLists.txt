project( Predicate )

cmake_minimum_required(VERSION 2.6)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

SET(BIN_INSTALL_DIR
    "${CMAKE_INSTALL_PREFIX}/bin"
    CACHE PATH "The subdirectory relative to the install prefix for executables (default is bin)"
)

SET(LIB_INSTALL_DIR
    "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}"
    CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is lib${LIB_SUFFIX})."
)

if(WIN32)
  SET(PLUGIN_INSTALL_BASE_DIR ${BIN_INSTALL_DIR})
else(WIN32)
  SET(PLUGIN_INSTALL_BASE_DIR lib${LIB_SUFFIX})
endif(WIN32)

SET(PLUGIN_INSTALL_DIR
    "${PLUGIN_INSTALL_BASE_DIR}/Predicate/plugins"
    CACHE PATH "The subdirectory relative to the install prefix where plugins will be installed (default is ${PLUGIN_INSTALL_BASE_DIR}/Predicate/plugins)."
)

add_subdirectory( cmake )
add_subdirectory( Predicate )

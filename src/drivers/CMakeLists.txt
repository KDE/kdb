if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

#TODO  add_definitions(-DKDE_DEFAULT_DEBUG_AREA=44001)

add_subdirectory( sqlite )

INCLUDE (FindMySQL)
IF (MYSQL_FOUND)
   add_subdirectory( mysql )
ENDIF (MYSQL_FOUND)

INCLUDE (FindPostgreSQL)
INCLUDE (FindPqxx)
IF (POSTGRESQL_FOUND AND PQXX_FOUND)
   add_subdirectory( pqxx )
ENDIF (POSTGRESQL_FOUND AND PQXX_FOUND)

INCLUDE(FindFreeTDS)
IF (FreeTDS_FOUND)
   add_subdirectory(sybase)
ENDIF (FreeTDS_FOUND)

INCLUDE(FindXBase)
IF(XBASE_FOUND)
   add_subdirectory(xbase)
ENDIF(XBASE_FOUND)

#message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(pgsql_dir)")
#message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(mysql_dir)")

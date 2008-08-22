INCLUDE (MacroAdditionalCleanFiles)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/transliteration_table.cpp
  COMMAND ${BZIP2_EXECUTABLE} ARGS -dc -k ${CMAKE_CURRENT_SOURCE_DIR}/transliteration_table.cpp.bz2 > ${CMAKE_CURRENT_BINARY_DIR}/transliteration_table.cpp
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/transliteration_table.cpp.bz2
)

macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/transliteration_table.cpp )

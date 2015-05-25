# - MACRO_ADDITIONAL_CLEAN_FILES(files...)
# MACRO_OPTIONAL_FIND_PACKAGE( <name> [QUIT] )

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


macro (MACRO_ADDITIONAL_CLEAN_FILES)
   get_directory_property(_tmp_DIR_PROPS ADDITIONAL_MAKE_CLEAN_FILES )
   
   if (_tmp_DIR_PROPS)
      set(_tmp_DIR_PROPS ${_tmp_DIR_PROPS} ${ARGN})
   else ()
      set(_tmp_DIR_PROPS ${ARGN})
   endif ()

   set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${_tmp_DIR_PROPS}")
endmacro (MACRO_ADDITIONAL_CLEAN_FILES)


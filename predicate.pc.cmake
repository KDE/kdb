prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

Name: Predicate
Description: A database connectivity and creation library
Version: @PREDICATE_VERSION@
URL: http://projects.kde.org/projects/predicate
Requires: QtCore QtGui QtXml
Libs: -L${libdir}
Cflags: -I${includedir}

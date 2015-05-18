prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

Name: KDb
Description: A database connectivity and creation library
Version: @KDB_VERSION@
URL: https://projects.kde.org/kdb
Requires: QtCore QtGui QtXml ICU
Libs: -L${libdir}
Cflags: -I${includedir}

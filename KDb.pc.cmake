prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

Name: KDb
Description: A database connectivity and creation library
Version: @PROJECT_VERSION@
URL: http://community.kde.org/KDb
Requires: Qt5Core Qt5Gui Qt5Xml icu-lo KF5CoreAddons
Libs: -L${libdir}
Cflags: -I${includedir}

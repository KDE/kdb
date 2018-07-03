prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

Name: @PROJECT_NAME@@PROJECT_STABLE_VERSION_MAJOR@
Description: A database connectivity and creation framework
Version: @PROJECT_VERSION@
URL: https://community.kde.org/KDb
Requires: Qt5Core Qt5Gui Qt5Widgets Qt5Xml icu-lo KF5CoreAddons
Libs: -L${libdir}
Cflags: -I${includedir}

set(REQUIRED_QT_VERSION "5.2.0")
find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Core Widgets Xml)
find_package(KF5 5.7.0 REQUIRED CoreAddons)
find_package(KDb REQUIRED)

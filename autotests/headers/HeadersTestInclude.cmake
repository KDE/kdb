set(REQUIRED_QT_VERSION "5.15.0")
find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Core Widgets Xml)
find_package(KF${QT_MAJOR_VERSION} 5.100.0 REQUIRED CoreAddons)
find_package(KDb REQUIRED)
set(LIB_INSTALL_DIR "lib")
include(KDECMakeSettings NO_POLICY_SCOPE)
include(KDECompilerSettings NO_POLICY_SCOPE)
if(POLICY CMP0063) # Honor visibility properties for all target types (since cmake 3.3)
    cmake_policy(SET CMP0063 NEW)
endif()
if(POLICY CMP0071) # Don't warn when combining AUTOMOC with qt5_wrap_ui() or qt5_add_resources() (since cmake 3.10)
    cmake_policy(SET CMP0071 NEW)
endif()

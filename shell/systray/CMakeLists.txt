cmake_minimum_required(VERSION 3.0)

project(
    libwintc-xpsystray-plugin
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion notification area xfce-panel plugin."
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(
    PROJECT_DEPENDENCIES
    libglib2.0-0
    libgtk-3-0
    libwintc-comgtk
    libxfce4panel-2.0-4
    libxfce4util7
)

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/libraries/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GDK          REQUIRED gdk-3.0)
pkg_check_modules(GIO          REQUIRED gio-2.0)
pkg_check_modules(GLIB         REQUIRED glib-2.0)
pkg_check_modules(GTK3         REQUIRED gtk+-3.0)
pkg_check_modules(WINTC_COMGTK REQUIRED wintc-comgtk)
pkg_check_modules(XFCE4PANEL   REQUIRED libxfce4panel-2.0)
pkg_check_modules(XFCE4UTIL    REQUIRED libxfce4util-1.0)

add_library(
    libwintc-xpsystray-plugin MODULE
    src/clock.h
    src/clock.c
    src/plugin.h
    src/plugin.c
)

target_compile_options(
    libwintc-xpsystray-plugin
    PRIVATE ${WINTC_COMPILE_OTPIONS}
)

target_include_directories(
    libwintc-xpsystray-plugin
    SYSTEM
    PRIVATE ${GDK_INCLUDE_DIRS}
    PRIVATE ${GIO_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${XFCE4PANEL_INCLUDE_DIRS}
    PRIVATE ${XFCE4UTIL_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-xpsystray-plugin
    PRIVATE ${GDK_LIBRARY_DIRS}
    PRIVATE ${GIO_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${XFCE4PANEL_LIBRARY_DIRS}
    PRIVATE ${XFCE4UTIL_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-xpsystray-plugin
    PRIVATE ${GDK_LIBRARIES}
    PRIVATE ${GIO_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${XFCE4PANEL_LIBRARIES}
    PRIVATE ${XFCE4UTIL_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()

install(
    FILES xpsystray.desktop
    DESTINATION share/xfce4/panel/plugins
)
install(
    TARGETS libwintc-xpsystray-plugin
    DESTINATION ${LIB_DIR}/xfce4/panel/plugins
)

cmake_minimum_required(VERSION 3.0)

project(
    libwintc-taskbuttons-plugin
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion taskbar buttons plugin."
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(
    PROJECT_DEPENDENCIES
    libglib2.0-0
    libgtk-3-0
    libpango-1.0-0
    libwintc-comgtk
    libwnck-3-0
    libxfce4panel-2.0-4
    libxfce4util7
)

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/libraries/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB         REQUIRED glib-2.0)
pkg_check_modules(GTK3         REQUIRED gtk+-3.0)
pkg_check_modules(PANGO        REQUIRED pango)
pkg_check_modules(WINTC_COMGTK REQUIRED wintc-comgtk)
pkg_check_modules(WNCK         REQUIRED libwnck-3.0)
pkg_check_modules(XFCE4PANEL   REQUIRED libxfce4panel-2.0)
pkg_check_modules(XFCE4UTIL    REQUIRED libxfce4util-1.0)

add_library(
    libwintc-taskbuttons-plugin MODULE
    src/plugin.h
    src/plugin.c
    src/taskbuttonbar.h
    src/taskbuttonbar.c
    src/windowmonitor.h
    src/windowmonitor.c
)

target_compile_definitions(
    libwintc-taskbuttons-plugin
    PRIVATE WNCK_I_KNOW_THIS_IS_UNSTABLE
)

target_compile_options(
    libwintc-taskbuttons-plugin
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    libwintc-taskbuttons-plugin
    SYSTEM
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${PANGO_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WNCK_INCLUDE_DIRS}
    PRIVATE ${XFCE4PANEL_INCLUDE_DIRS}
    PRIVATE ${XFCE4UTIL_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-taskbuttons-plugin
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${PANGO_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WNCK_LIBRARY_DIRS}
    PRIVATE ${XFCE4PANEL_LIBRARY_DIRS}
    PRIVATE ${XFCE4UTIL_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-taskbuttons-plugin
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${PANGO_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WNCK_LIBRARIES}
    PRIVATE ${XFCE4PANEL_LIBRARIES}
    PRIVATE ${XFCE4UTIL_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()

install(
    FILES taskbuttons.desktop
    DESTINATION share/xfce4/panel/plugins
)
install(
    TARGETS libwintc-taskbuttons-plugin
    DESTINATION ${LIB_DIR}/xfce4/panel/plugins
)

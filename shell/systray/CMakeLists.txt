cmake_minimum_required(VERSION 3.0)

project(
    xpsystray-plugin
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion notification area xfce-panel plugin."
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GARCON       REQUIRED garcon-1)
pkg_check_modules(GARCON_GTK3  REQUIRED garcon-gtk3-1)
pkg_check_modules(GDK          REQUIRED gdk-3.0)
pkg_check_modules(GIO          REQUIRED gio-2.0)
pkg_check_modules(GLIB         REQUIRED glib-2.0)
pkg_check_modules(GTK3         REQUIRED gtk+-3.0)
pkg_check_modules(WINTC_COMGTK REQUIRED wintc-comgtk)
pkg_check_modules(XFCE4PANEL   REQUIRED libxfce4panel-2.0)
pkg_check_modules(XFCE4UTIL    REQUIRED libxfce4util-1.0)

add_library(
    xpsystray-plugin MODULE
    src/clock.h
    src/clock.c
    src/plugin.h
    src/plugin.c
)

target_compile_options(
    xpsystray-plugin
    PRIVATE -Wall -Wextra -Wpedantic -Werror
)

target_include_directories(
    xpsystray-plugin
    SYSTEM
    PRIVATE ${GARCON_INCLUDE_DIRS}
    PRIVATE ${GARCON_GTK3_INCLUDE_DIRS}
    PRIVATE ${GDK_INCLUDE_DIRS}
    PRIVATE ${GIO_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${XFCE4PANEL_INCLUDE_DIRS}
    PRIVATE ${XFCE4UTIL_INCLUDE_DIRS}
)

target_link_directories(
    xpsystray-plugin
    PRIVATE ${GARCON_LIBRARY_DIRS}
    PRIVATE ${GARCON_GTK3_LIBRARY_DIRS}
    PRIVATE ${GDK_LIBRARY_DIRS}
    PRIVATE ${GIO_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${XFCE4PANEL_LIBRARY_DIRS}
    PRIVATE ${XFCE4UTIL_LIBRARY_DIRS}
)

target_link_libraries(
    xpsystray-plugin
    PRIVATE ${GARCON_LIBRARIES}
    PRIVATE ${GARCON_GTK3_LIBRARIES}
    PRIVATE ${GDK_LIBRARIES}
    PRIVATE ${GIO_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${XFCE4PANEL_LIBRARIES}
    PRIVATE ${XFCE4UTIL_LIBRARIES}
)

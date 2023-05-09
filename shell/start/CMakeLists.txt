cmake_minimum_required(VERSION 3.0)

project(
    libwintc-startmenu-plugin
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion Start menu xfce-panel plugin."
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(
    PROJECT_DEPENDENCIES
    libgarcon-1-0
    libgarcon-gtk3-1-0
    libglib2.0-0
    libgtk-3-0
    libwintc-comgtk
    libwintc-exec
    libwintc-shllang
    libxfce4panel-2.0-4
    libxfce4util7
    xdg-utils
)

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/libraries/CMakeLists.txt)
include(../../packaging/cmake-inc/locale/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../../packaging/cmake-inc/resources/CMakeLists.txt)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GARCON        REQUIRED garcon-1)
pkg_check_modules(GARCON_GTK3   REQUIRED garcon-gtk3-1)
pkg_check_modules(GDK           REQUIRED gdk-3.0)
pkg_check_modules(GIO           REQUIRED gio-2.0)
pkg_check_modules(GLIB          REQUIRED glib-2.0)
pkg_check_modules(GTK3          REQUIRED gtk+-3.0)
pkg_check_modules(WINTC_COMGTK  REQUIRED wintc-comgtk)
pkg_check_modules(WINTC_EXEC    REQUIRED wintc-exec)
pkg_check_modules(WINTC_SHLLANG REQUIRED wintc-shllang)
pkg_check_modules(XFCE4PANEL    REQUIRED libxfce4panel-2.0)
pkg_check_modules(XFCE4UTIL     REQUIRED libxfce4util-1.0)

wintc_compile_resources()
wintc_create_config_h()

# Main target
#
add_library(
    libwintc-startmenu-plugin MODULE
    src/action.h
    src/action.c
    src/config.h
    src/placeslist.h
    src/placeslist.c
    src/plugin.h
    src/plugin.c
    src/programslist.h
    src/programslist.c
    src/resources.c
    src/startbutton.h
    src/startbutton.c
    src/startmenu.h
    src/startmenu.c
    src/startmenuitem.h
    src/startmenuitem.c
    src/util.h
    src/util.c
)

target_compile_options(
    libwintc-startmenu-plugin
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    libwintc-startmenu-plugin
    SYSTEM
    PRIVATE ${GARCON_INCLUDE_DIRS}
    PRIVATE ${GARCON_GTK3_INCLUDE_DIRS}
    PRIVATE ${GDK_INCLUDE_DIRS}
    PRIVATE ${GIO_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_EXEC_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHLLANG_INCLUDE_DIRS}
    PRIVATE ${XFCE4PANEL_INCLUDE_DIRS}
    PRIVATE ${XFCE4UTIL_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-startmenu-plugin
    PRIVATE ${GARCON_LIBRARY_DIRS}
    PRIVATE ${GARCON_GTK3_LIBRARY_DIRS}
    PRIVATE ${GDK_LIBRARY_DIRS}
    PRIVATE ${GIO_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_EXEC_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHLLANG_LIBRARY_DIRS}
    PRIVATE ${XFCE4PANEL_LIBRARY_DIRS}
    PRIVATE ${XFCE4UTIL_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-startmenu-plugin
    PRIVATE ${GARCON_LIBRARIES}
    PRIVATE ${GARCON_GTK3_LIBRARIES}
    PRIVATE ${GDK_LIBRARIES}
    PRIVATE ${GIO_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_EXEC_LIBRARIES}
    PRIVATE ${WINTC_SHLLANG_LIBRARIES}
    PRIVATE ${XFCE4PANEL_LIBRARIES}
    PRIVATE ${XFCE4UTIL_LIBRARIES})

add_dependencies(
    libwintc-startmenu-plugin
    build-gresources
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_compile_and_install_pofiles()

install(
    DIRECTORY res/
    DESTINATION ${WINTC_ASSETS_INSTALL_DIR}/shell-res
)
install(
    FILES startmenu.desktop
    DESTINATION share/xfce4/panel/plugins
)
install(
    TARGETS libwintc-startmenu-plugin
    DESTINATION ${LIB_DIR}/xfce4/panel/plugins
)

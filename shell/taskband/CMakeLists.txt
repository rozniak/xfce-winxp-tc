cmake_minimum_required(VERSION 3.0)

project(
    wintc-taskband
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion taskband application"
    LANGUAGES C
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/locale/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../../packaging/cmake-inc/resources/CMakeLists.txt)

wintc_resolve_library(garcon-1       GARCON)
wintc_resolve_library(garcon-gtk3-1  GARCON_GTK3)
wintc_resolve_library(gdk-pixbuf-2.0 GDK_PIXBUF)
wintc_resolve_library(glib-2.0       GLIB)
wintc_resolve_library(gtk+-3.0       GTK3)
wintc_resolve_library(wintc-comgtk   WINTC_COMGTK)
wintc_resolve_library(wintc-exec     WINTC_EXEC)
wintc_resolve_library(wintc-shelldpa WINTC_SHELLDPA)
wintc_resolve_library(wintc-shllang  WINTC_SHLLANG)

wintc_compile_resources()
wintc_create_meta_h()

add_executable(
    wintc-taskband
    src/application.c
    src/application.h
    src/main.c
    src/meta.h
    src/resources.c
    src/window.c
    src/window.h
    src/start/action.c
    src/start/action.h
    src/start/placeslist.c
    src/start/placeslist.h
    src/start/programslist.c
    src/start/programslist.h
    src/start/startbutton.c
    src/start/startbutton.h
    src/start/startmenu.c
    src/start/startmenu.h
    src/start/startmenuitem.c
    src/start/startmenuitem.h
    src/start/util.c
    src/start/util.h
    src/systray/clock.c
    src/systray/clock.h
    src/systray/notifarea.c
    src/systray/notifarea.h
    src/taskbuttons/taskbuttonbar.c
    src/taskbuttons/taskbuttonbar.h
    src/taskbuttons/windowmonitor.c
    src/taskbuttons/windowmonitor.h
)

target_compile_options(
    wintc-taskband
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-taskband
    SYSTEM
    PRIVATE ${GARCON_INCLUDE_DIRS}
    PRIVATE ${GARCON_GTK3_INCLUDE_DIRS}
    PRIVATE ${GDK_PIXBUF_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_EXEC_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELLDPA_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHLLANG_INCLUDE_DIRS}
)

target_link_directories(
    wintc-taskband
    PRIVATE ${GARCON_LIBRARY_DIRS}
    PRIVATE ${GARCON_GTK3_LIBRARY_DIRS}
    PRIVATE ${GDK_PIXBUF_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_EXEC_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELLDPA_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHLLANG_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-taskband
    PRIVATE ${GARCON_LIBRARIES}
    PRIVATE ${GARCON_GTK3_LIBRARIES}
    PRIVATE ${GDK_PIXBUF_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_EXEC_LIBRARIES}
    PRIVATE ${WINTC_SHELLDPA_LIBRARIES}
    PRIVATE ${WINTC_SHLLANG_LIBRARIES}
)

add_dependencies(
    wintc-taskband
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
    TARGETS wintc-taskband
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

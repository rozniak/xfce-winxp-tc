cmake_minimum_required(VERSION 3.12)

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

set(WINTC_NO_PEDANTIC_COMPILE true) # Necessary due to NetworkManager headers

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/locale/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../../packaging/cmake-inc/resources/CMakeLists.txt)

wintc_resolve_library(libcanberra      CANBERRA)
wintc_resolve_library(libcanberra-gtk3 CANBERRA_GTK3)
wintc_resolve_library(garcon-1         GARCON)
wintc_resolve_library(garcon-gtk3-1    GARCON_GTK3)
wintc_resolve_library(gdk-pixbuf-2.0   GDK_PIXBUF)
wintc_resolve_library(glib-2.0         GLIB)
wintc_resolve_library(gtk+-3.0         GTK3)
wintc_resolve_library(upower-glib      UPOWER_GLIB)
wintc_resolve_library(wintc-comctl     WINTC_COMCTL)
wintc_resolve_library(wintc-comgtk     WINTC_COMGTK)
wintc_resolve_library(wintc-exec       WINTC_EXEC)
wintc_resolve_library(wintc-shcommon   WINTC_SHCOMMON)
wintc_resolve_library(wintc-shelldpa   WINTC_SHELLDPA)
wintc_resolve_library(wintc-shellext   WINTC_SHELLEXT)
wintc_resolve_library(wintc-shlang     WINTC_SHLANG)
wintc_resolve_library(wintc-sndapi     WINTC_SNDAPI)

wintc_compile_resources()
wintc_create_meta_h()

# FIXME: For now, exclude the network icon stuff from build on FreeBSD
set(
    TASKBAND_SOURCES
    src/application.c
    src/application.h
    src/intapi.h
    src/main.c
    src/meta.h
    src/resources.c
    src/window.c
    src/window.h
    src/start/menumod.c
    src/start/menumod.h
    src/start/mfu.c
    src/start/mfu.h
    src/start/personal.c
    src/start/personal.h
    src/start/progmenu.c
    src/start/progmenu.h
    src/start/shared.h
    src/start/toolbar.c
    src/start/toolbar.h
    src/start/util.c
    src/start/util.h
    src/systray/clock.c
    src/systray/clock.h
    src/systray/icon.c
    src/systray/icon.h
    src/systray/notifarea.c
    src/systray/notifarea.h
    src/systray/power.c
    src/systray/power.h
    src/systray/toolbar.c
    src/systray/toolbar.h
    src/systray/volume.c
    src/systray/volume.h
    src/taskbuttons/taskbuttonbar.c
    src/taskbuttons/taskbuttonbar.h
    src/taskbuttons/toolbar.c
    src/taskbuttons/toolbar.h
    src/taskbuttons/windowmonitor.c
    src/taskbuttons/windowmonitor.h
)

if (NOT ${WINTC_PKGMGR} STREQUAL "bsdpkg")
    wintc_resolve_library(libnm NM)

    set(
        TASKBAND_SOURCES
        ${TASKBAND_SOURCES}
        src/systray/network.c
        src/systray/network.h
    )
endif()

add_executable(
    wintc-taskband
    ${TASKBAND_SOURCES}
)

target_compile_options(
    wintc-taskband
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-taskband
    SYSTEM
    BEFORE
    PRIVATE ${CANBERRA_INCLUDE_DIRS}
    PRIVATE ${CANBERRA_GTK3_INCLUDE_DIRS}
    PRIVATE ${GARCON_INCLUDE_DIRS}
    PRIVATE ${GARCON_GTK3_INCLUDE_DIRS}
    PRIVATE ${GDK_PIXBUF_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${UPOWER_GLIB_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMCTL_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_EXEC_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHCOMMON_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELLDPA_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELLEXT_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHLANG_INCLUDE_DIRS}
    PRIVATE ${WINTC_SNDAPI_INCLUDE_DIRS}
)

target_link_directories(
    wintc-taskband
    PRIVATE ${CANBERRA_LIBRARY_DIRS}
    PRIVATE ${CANBERRA_GTK3_LIBRARY_DIRS}
    PRIVATE ${GARCON_LIBRARY_DIRS}
    PRIVATE ${GARCON_GTK3_LIBRARY_DIRS}
    PRIVATE ${GDK_PIXBUF_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${UPOWER_GLIB_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMCTL_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_EXEC_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHCOMMON_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELLDPA_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELLEXT_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHLANG_LIBRARY_DIRS}
    PRIVATE ${WINTC_SNDAPI_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-taskband
    PRIVATE ${CANBERRA_LIBRARIES}
    PRIVATE ${CANBERRA_GTK3_LIBRARIES}
    PRIVATE ${GARCON_LIBRARIES}
    PRIVATE ${GARCON_GTK3_LIBRARIES}
    PRIVATE ${GDK_PIXBUF_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${UPOWER_GLIB_LIBRARIES}
    PRIVATE ${WINTC_COMCTL_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_EXEC_LIBRARIES}
    PRIVATE ${WINTC_SHCOMMON_LIBRARIES}
    PRIVATE ${WINTC_SHELLDPA_LIBRARIES}
    PRIVATE ${WINTC_SHELLEXT_LIBRARIES}
    PRIVATE ${WINTC_SHLANG_LIBRARIES}
    PRIVATE ${WINTC_SNDAPI_LIBRARIES}
)

if (NOT ${WINTC_PKGMGR} STREQUAL "bsdpkg")
    target_include_directories(
        wintc-taskband
        PRIVATE ${NM_INCLUDE_DIRS}
    )
    target_link_directories(
        wintc-taskband
        PRIVATE ${NM_LIBRARY_DIRS}
    )
    target_link_libraries(
        wintc-taskband
        PRIVATE ${NM_LIBRARIES}
    )
endif()

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

cmake_minimum_required(VERSION 3.5)

project(
    wintc-shell-winver
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion 'About Windows' utility."
    LANGUAGES C
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

wintc_resolve_library(gdk-pixbuf-2.0 GDK_PIXBUF)
wintc_resolve_library(glib-2.0       GLIB)
wintc_resolve_library(gtk+-3.0       GTK3)
wintc_resolve_library(wintc-comctl   WINTC_COMCTL)
wintc_resolve_library(wintc-comgtk   WINTC_COMGTK)
wintc_resolve_library(wintc-winbrand WINTC_WINBRAND)

add_executable(
    wintc-shell-winver
    src/winver.c
)

set_target_properties(
    wintc-shell-winver
    PROPERTIES
    OUTPUT_NAME winver
)

target_compile_options(
    wintc-shell-winver
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-shell-winver
    SYSTEM
    BEFORE
    PRIVATE ${GDK_PIXBUF_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMCTL_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_WINBRAND_INCLUDE_DIRS}
)

target_link_directories(
    wintc-shell-winver
    PRIVATE ${GDK_PIXBUF_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMCTL_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_WINBRAND_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-shell-winver
    PRIVATE ${GDK_PIXBUF_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMCTL_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_WINBRAND_LIBRARIES}
)

# FreeBSD requires linking to libsysinfo port
#
if (${WINTC_PKGMGR} STREQUAL "bsdpkg")
    wintc_resolve_library(libsysinfo SYSINFO)

    target_include_directories(
        wintc-shell-winver
        PRIVATE ${SYSINFO_INCLUDE_DIRS}
    )
    target_link_directories(
        wintc-shell-winver
        PRIVATE ${SYSINFO_LIBRARY_DIRS}
    )
    target_link_libraries(
        wintc-shell-winver
        PRIVATE ${SYSINFO_LIBRARIES}
    )
endif()

# Installation
#
wintc_configure_and_install_packaging()

install(
    DIRECTORY res/
    DESTINATION ${WINTC_ASSETS_INSTALL_DIR}/shell-res
)
install(
    TARGETS wintc-shell-winver
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

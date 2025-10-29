cmake_minimum_required(VERSION 3.12)

project(
    libwintc-shell
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion shell library."
    LANGUAGES C
)

set(PROJECT_ANYARCH false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/libraries/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../../packaging/cmake-inc/resources/CMakeLists.txt)

wintc_resolve_library(libcanberra      CANBERRA)
wintc_resolve_library(libcanberra-gtk3 CANBERRA_GTK3)
wintc_resolve_library(gdk-pixbuf-2.0   GDK_PIXBUF)
wintc_resolve_library(glib-2.0         GLIB)
wintc_resolve_library(gtk+-3.0         GTK3)
wintc_resolve_library(wintc-comctl     WINTC_COMCTL)
wintc_resolve_library(wintc-comgtk     WINTC_COMGTK)
wintc_resolve_library(wintc-exec       WINTC_EXEC)
wintc_resolve_library(wintc-shcommon   WINTC_SHCOMMON)
wintc_resolve_library(wintc-shellext   WINTC_SHELLEXT)
wintc_resolve_library(wintc-shlang     WINTC_SHLANG)
wintc_resolve_library(wintc-winbrand   WINTC_WINBRAND)
wintc_resolve_library(libxml-2.0       XML2)

wintc_compile_resources()

add_library(
    libwintc-shell
    src/browser.c
    public/browser.h
    src/cpl.c
    public/cpl.h
    src/dialog.c
    public/dialog.h
    src/error.c
    public/error.h
    src/fldropts.c
    public/fldropts.h
    src/fsclipbd.c
    public/fsclipbd.h
    src/fsop.c
    public/fsop.h
    src/icnvwbeh.c
    public/icnvwbeh.h
    src/resources.c
    public/sound.h
    src/sound.c
    src/trevwbeh.c
    public/trevwbeh.h
    src/nmspace.c
    public/nmspace.h
    src/vwcpl.c
    public/vwcpl.h
    src/vwdesk.c
    public/vwdesk.h
    src/vwdrives.c
    public/vwdrives.h
    src/vwfs.c
    public/vwfs.h
)

set_target_properties(
    libwintc-shell
    PROPERTIES
    SOVERSION 1
    VERSION ${PROJECT_VERSION}
)

target_compile_options(
    libwintc-shell
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    libwintc-shell
    SYSTEM
    BEFORE
    PRIVATE ${CANBERRA_INCLUDE_DIRS}
    PRIVATE ${CANBERRA_GTK3_INCLUDE_DIRS}
    PRIVATE ${GDK_PIXBUF_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMCTL_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_EXEC_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHCOMMON_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELLEXT_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHLANG_INCLUDE_DIRS}
    PRIVATE ${WINTC_WINBRAND_INCLUDE_DIRS}
    PRIVATE ${XML2_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-shell
    PRIVATE ${CANBERRA_LIBRARY_DIRS}
    PRIVATE ${CANBERRA_GTK3_LIBRARY_DIRS}
    PRIVATE ${GDK_PIXBUF_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMCTL_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_EXEC_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHCOMMON_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELLEXT_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHLANG_LIBRARY_DIRS}
    PRIVATE ${WINTC_WINBRAND_LIBRARY_DIRS}
    PRIVATE ${XML2_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-shell
    PRIVATE ${CANBERRA_LIBRARIES}
    PRIVATE ${CANBERRA_GTK3_LIBRARIES}
    PRIVATE ${GDK_PIXBUF_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMCTL_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_EXEC_LIBRARIES}
    PRIVATE ${WINTC_SHCOMMON_LIBRARIES}
    PRIVATE ${WINTC_SHELLEXT_LIBRARIES}
    PRIVATE ${WINTC_SHLANG_LIBRARIES}
    PRIVATE ${WINTC_WINBRAND_LIBRARIES}
    PRIVATE ${XML2_LIBRARIES}
)

# FreeBSD requires linking to libsysinfo port
#
if (${WINTC_PKGMGR} STREQUAL "bsdpkg")
    wintc_resolve_library(libsysinfo SYSINFO)

    target_include_directories(
        libwintc-shell
        PRIVATE ${SYSINFO_INCLUDE_DIRS}
    )
    target_link_directories(
        libwintc-shell
        PRIVATE ${SYSINFO_LIBRARY_DIRS}
    )
    target_link_libraries(
        libwintc-shell
        PRIVATE ${SYSINFO_LIBRARIES}
    )
endif()

# Installation
#
wintc_configure_and_install_packaging()
wintc_add_pkgconfig_install()
wintc_install_public_headers()

install(
    TARGETS libwintc-shell
    LIBRARY DESTINATION ${LIB_DIR}
)

cmake_minimum_required(VERSION 3.12)

project(
    libwintc-msgina
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion GINA library."
    LANGUAGES C
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS false)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/libraries/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../../packaging/cmake-inc/resources/CMakeLists.txt)

wintc_resolve_library(gdk-pixbuf-2.0       GDK_PIXBUF)
wintc_resolve_library(glib-2.0             GLIB)
wintc_resolve_library(gtk+-3.0             GTK3)
wintc_resolve_library(liblightdm-gobject-1 LIGHTDM)
wintc_resolve_library(wintc-comctl         WINTC_COMCTL)
wintc_resolve_library(wintc-comgtk         WINTC_COMGTK)
wintc_resolve_library(wintc-winbrand       WINTC_WINBRAND)

wintc_compile_resources()

add_library(
    libwintc-msgina
    src/resources.c
    src/authwnd.c
    public/authwnd.h
    public/challenge.h
    src/error.c
    public/error.h
    src/exitwnd.c
    public/exitwnd.h
    src/if_authui.c
    public/if_authui.h
    src/if_sm.c
    public/if_sm.h
    src/logon.c
    public/logon.h
    public/state.h
    src/stripctl.c
    src/stripctl.h
    src/xdgsm.c
    public/xdgsm.h
    src/xfsm.c
    public/xfsm.h
)

set_target_properties(
    libwintc-msgina
    PROPERTIES
    SOVERSION 1
    VERSION ${PROJECT_VERSION}
)

target_compile_options(
    libwintc-msgina
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    libwintc-msgina
    SYSTEM
    BEFORE
    PRIVATE ${GDK_PIXBUF_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${LIGHTDM_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMCTL_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_WINBRAND_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-msgina
    PRIVATE ${GDK_PIXBUF_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${LIGHTDM_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMCTL_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_WINBRAND_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-msgina
    PRIVATE ${GDK_PIXBUF_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${LIGHTDM_LIBRARIES}
    PRIVATE ${WINTC_COMCTL_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_WINBRAND_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_add_pkgconfig_install()
wintc_install_public_headers()

install(
    TARGETS libwintc-msgina
    LIBRARY DESTINATION ${LIB_DIR}
)

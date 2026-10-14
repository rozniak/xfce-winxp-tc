cmake_minimum_required(VERSION 3.14)

project(
    libwintc-shelldpa
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion shell display protocol abstraction library."
    LANGUAGES C
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

set(WINTC_NO_PEDANTIC_COMPILE true) # Necessary because we use dlsym()

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/libraries/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

wintc_resolve_library(gdk-pixbuf-2.0 GDK_PIXBUF   PUBLIC)
wintc_resolve_library(glib-2.0       GLIB         PUBLIC)
wintc_resolve_library(gtk+-3.0       GTK3         PUBLIC)
wintc_resolve_library(wintc-comgtk   WINTC_COMGTK PRIVATE)

add_library(
    libwintc-shelldpa
    src/api.c
    public/api.h
    src/deskwnd.c
    public/deskwnd.h
    src/panelwnd.c
    public/panelwnd.h
    src/impl-wndmgmt-wnck.c
    src/impl-wndmgmt-wnck.h
    src/impl-wndmgmt-xfw.c
    src/impl-wndmgmt-xfw.h
    src/dll/layersh.c
    src/dll/layersh.h
    src/dll/wnck.c
    src/dll/wnck.h
    src/dll/xfw.c
    src/dll/xfw.h
)

set_target_properties(
    libwintc-shelldpa
    PROPERTIES
    SOVERSION 1
    VERSION ${PROJECT_VERSION}
)

target_compile_options(
    libwintc-shelldpa
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    libwintc-shelldpa
    SYSTEM
    BEFORE
    PUBLIC  ${GDK_PIXBUF_INCLUDE_DIRS}
    PUBLIC  ${GLIB_INCLUDE_DIRS}
    PUBLIC  ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-shelldpa
    PUBLIC  ${GDK_PIXBUF_LIBRARY_DIRS}
    PUBLIC  ${GLIB_LIBRARY_DIRS}
    PUBLIC  ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-shelldpa
    PRIVATE ${CMAKE_DL_LIBS}
    PUBLIC  ${GDK_PIXBUF_LIBRARIES}
    PUBLIC  ${GLIB_LIBRARIES}
    PUBLIC  ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_add_pkgconfig_install()
wintc_install_public_headers()

install(
    TARGETS libwintc-shelldpa
    LIBRARY DESTINATION ${LIB_DIR}
)

cmake_minimum_required(VERSION 3.14)

project(
    libwintc-sndapi
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion system audio abstracton library."
    LANGUAGES C
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/libraries/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

wintc_resolve_library(glib-2.0               GLIB            PUBLIC)
wintc_resolve_library(gtk+-3.0               GTK3            PUBLIC)
wintc_resolve_library(libpulse               PULSEAUDIO      PRIVATE)
wintc_resolve_library(libpulse-mainloop-glib PULSEAUDIO_GLIB PRIVATE)
wintc_resolve_library(wintc-comgtk           WINTC_COMGTK    PRIVATE)

add_library(
    libwintc-sndapi
    src/context.c
    public/context.h
    src/output.c
    src/output-priv.h
    public/output.h
    src/volcvt.c
    src/volcvt.h
)

set_target_properties(
    libwintc-sndapi
    PROPERTIES
    SOVERSION 1
    VERSION ${PROJECT_VERSION}
)

target_compile_options(
    libwintc-sndapi
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    libwintc-sndapi
    SYSTEM
    BEFORE
    PUBLIC  ${GLIB_INCLUDE_DIRS}
    PUBLIC  ${GTK3_INCLUDE_DIRS}
    PRIVATE ${PULSEAUDIO_INCLUDE_DIRS}
    PRIVATE ${PULSEAUDIO_GLIB_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-sndapi
    PUBLIC  ${GLIB_LIBRARY_DIRS}
    PUBLIC  ${GTK3_LIBRARY_DIRS}
    PRIVATE ${PULSEAUDIO_LIBRARY_DIRS}
    PRIVATE ${PULSEAUDIO_GLIB_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-sndapi
    PUBLIC  ${GLIB_LIBRARIES}
    PUBLIC  ${GTK3_LIBRARIES}
    PRIVATE ${PULSEAUDIO_LIBRARIES}
    PRIVATE ${PULSEAUDIO_GLIB_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_add_pkgconfig_install()
wintc_install_public_headers()

install(
    TARGETS libwintc-sndapi
    LIBRARY DESTINATION ${LIB_DIR}
)

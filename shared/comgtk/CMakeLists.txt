cmake_minimum_required(VERSION 3.5)

project(
    libwintc-comgtk
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion common GLib/GTK utilities."
    LANGUAGES C
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

# Necessary due to fn ptr casts in marshal.c
#
set(WINTC_NO_PEDANTIC_COMPILE true)

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/libraries/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

wintc_resolve_library(glib-2.0 GLIB)
wintc_resolve_library(gtk+-3.0 GTK3)

configure_file(src/version.c.in ${PROJECT_ROOT}/src/version.c @ONLY)

add_library(
    libwintc-comgtk
    src/debug.h
    src/defprocs.c
    src/defprocs.h
    src/errors.c
    src/errors.h
    src/gchar.h
    src/list.c
    src/list.h
    src/marshal.c
    src/marshal.h
    src/msgbox.c
    src/msgbox.h
    src/profile.c
    src/profile.h
    src/shorthand.h
    src/signals.c
    src/signals.h
    src/strings.c
    src/strings.h
    src/styles.c
    src/styles.h
    src/version.c
    src/version.h
    src/window.c
    src/window.h
)

set_target_properties(
    libwintc-comgtk
    PROPERTIES
    PUBLIC_HEADER public/wintc-comgtk.h
    SOVERSION 1
    VERSION ${PROJECT_VERSION}
)

target_compile_options(
    libwintc-comgtk
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    libwintc-comgtk
    SYSTEM
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-comgtk
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-comgtk
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_add_pkgconfig_install()

install(
    TARGETS libwintc-comgtk
    LIBRARY       DESTINATION ${LIB_DIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

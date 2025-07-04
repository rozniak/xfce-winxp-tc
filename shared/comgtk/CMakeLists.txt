cmake_minimum_required(VERSION 3.12)

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

wintc_resolve_library(gio-2.0  GIO)
wintc_resolve_library(glib-2.0 GLIB)
wintc_resolve_library(gtk+-3.0 GTK3)

configure_file(public/location.h.in ${PROJECT_ROOT}/public/location.h @ONLY)
configure_file(src/version.c.in   ${PROJECT_ROOT}/src/version.c   @ONLY)

add_library(
    libwintc-comgtk
    src/accelerator.c
    public/accelerator.h
    src/application.c
    public/application.h
    src/builder.c
    public/builder.h
    public/debug.h
    src/clipbd.c
    public/clipbd.h
    src/container.c
    public/container.h
    src/defprocs.c
    public/defprocs.h
    src/delegate.c
    public/delegate.h
    src/errors.c
    public/errors.h
    src/hashtable.c
    public/hashtable.h
    src/icons.c
    public/icons.h
    src/list.c
    public/list.h
    src/listbox.c
    public/listbox.h
    public/location.h
    src/marshal.c
    public/marshal.h
    src/memory.c
    public/memory.h
    src/menu.c
    public/menu.h
    src/msgbox.c
    public/msgbox.h
    src/profile.c
    public/profile.h
    src/regex.c
    public/regex.h
    public/shorthand.h
    src/signals.c
    public/signals.h
    src/strings.c
    public/strings.h
    src/styles.c
    public/styles.h
    src/version.c
    public/version.h
    src/window.c
    public/window.h
)

set_target_properties(
    libwintc-comgtk
    PROPERTIES
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
    BEFORE
    PRIVATE ${GIO_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-comgtk
    PRIVATE ${GIO_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-comgtk
    PRIVATE ${GIO_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_add_pkgconfig_install()
wintc_install_public_headers()

install(
    TARGETS libwintc-comgtk
    LIBRARY DESTINATION ${LIB_DIR}
)

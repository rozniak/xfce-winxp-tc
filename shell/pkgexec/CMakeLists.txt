cmake_minimum_required(VERSION 3.14)

project(
    wintc-pkg-exec
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion package management program."
    LANGUAGES C
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/locale/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../../packaging/cmake-inc/resources/CMakeLists.txt)

wintc_resolve_library(gio-2.0        GIO)
wintc_resolve_library(glib-2.0       GLIB)
wintc_resolve_library(gtk+-3.0       GTK3)
wintc_resolve_library(wintc-comctl   WINTC_COMCTL)
wintc_resolve_library(wintc-comgtk   WINTC_COMGTK)
wintc_resolve_library(wintc-setupapi WINTC_SETUPAPI)
wintc_resolve_library(wintc-shcommon WINTC_SHCOMMON)
wintc_resolve_library(wintc-shlang   WINTC_SHLANG)
wintc_resolve_library(wintc-wizard97 WINTC_WIZARD97)

wintc_compile_resources()
wintc_create_meta_h()

# Main target
#
add_executable(
    wintc-pkg-exec
    src/application.c
    src/application.h
    src/inswiz.c
    src/inswiz.h
    src/main.c
    src/meta.h
    src/resources.c
)

target_compile_options(
    wintc-pkg-exec
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-pkg-exec
    SYSTEM
    PRIVATE ${GIO_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMCTL_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_SETUPAPI_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHCOMMON_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHLANG_INCLUDE_DIRS}
    PRIVATE ${WINTC_WIZARD97_INCLUDE_DIRS}
)

target_link_directories(
    wintc-pkg-exec
    PRIVATE ${GIO_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMCTL_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_SETUPAPI_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHCOMMON_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHLANG_LIBRARY_DIRS}
    PRIVATE ${WINTC_WIZARD97_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-pkg-exec
    PRIVATE ${GIO_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMCTL_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_SETUPAPI_LIBRARIES}
    PRIVATE ${WINTC_SHCOMMON_LIBRARIES}
    PRIVATE ${WINTC_SHLANG_LIBRARIES}
    PRIVATE ${WINTC_WIZARD97_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()

install(
    FILES wintc-pkg-exec.desktop
    DESTINATION share/applications
)
install(
    TARGETS wintc-pkg-exec
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

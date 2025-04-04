cmake_minimum_required(VERSION 3.12)

project(
    wintc-regsvc
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion registry service."
    LANGUAGES C
)

set(PROJECT_ANYARCH false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

wintc_resolve_library(glib-2.0       GLIB)
wintc_resolve_library(gtk+-3.0       GTK3)
wintc_resolve_library(sqlite3        SQLITE3)
wintc_resolve_library(wintc-comgtk   WINTC_COMGTK)
wintc_resolve_library(wintc-registry WINTC_REGISTRY)

add_executable(
    wintc-regsvc
    src/application.c
    src/application.h
    src/backend.c
    src/backend.h
    src/main.c
)

set_target_properties(
    wintc-regsvc
    PROPERTIES
    OUTPUT_NAME regsvc
)

target_compile_options(
    wintc-regsvc
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-regsvc
    SYSTEM
    BEFORE
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${SQLITE3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_REGISTRY_INCLUDE_DIRS}
)

target_link_directories(
    wintc-regsvc
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${SQLITE3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_REGISTRY_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-regsvc
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${SQLITE3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_REGISTRY_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()

configure_file(
    res/uk.oddmatics.wintc.registry.service.in
    uk.oddmatics.wintc.registry.service
    @ONLY
)

install(
    FILES ${CMAKE_BINARY_DIR}/uk.oddmatics.wintc.registry.service
    DESTINATION share/dbus-1/services
)
install(
    TARGETS wintc-regsvc
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

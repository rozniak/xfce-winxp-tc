cmake_minimum_required(VERSION 3.12)

project(
    wintc-desktop
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion desktop application"
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
wintc_resolve_library(wintc-comgtk   WINTC_COMGTK)
wintc_resolve_library(wintc-registry WINTC_REGISTRY)
wintc_resolve_library(wintc-shelldpa WINTC_SHELLDPA)
wintc_resolve_library(wintc-syscfg   WINTC_SYSCFG)

add_executable(
    wintc-desktop
    src/application.c
    src/application.h
    src/main.c
    src/settings.c
    src/settings.h
    src/window.c
    src/window.h
)

target_compile_options(
    wintc-desktop
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-desktop
    SYSTEM
    BEFORE
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_REGISTRY_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELLDPA_INCLUDE_DIRS}
    PRIVATE ${WINTC_SYSCFG_INCLUDE_DIRS}
)

target_link_directories(
    wintc-desktop
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_REGISTRY_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELLDPA_LIBRARY_DIRS}
    PRIVATE ${WINTC_SYSCFG_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-desktop
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_REGISTRY_LIBRARIES}
    PRIVATE ${WINTC_SHELLDPA_LIBRARIES}
    PRIVATE ${WINTC_SYSCFG_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()

install(
    TARGETS wintc-desktop
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

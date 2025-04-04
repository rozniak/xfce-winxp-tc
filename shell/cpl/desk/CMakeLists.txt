cmake_minimum_required(VERSION 3.12)

project(
    wintc-cpl-desk
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion Display Control Panel applet."
    LANGUAGES C
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../../packaging/cmake-inc/locale/CMakeLists.txt)
include(../../../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../../../packaging/cmake-inc/resources/CMakeLists.txt)

wintc_resolve_library(glib-2.0       GLIB)
wintc_resolve_library(gtk+-3.0       GTK3)
wintc_resolve_library(wintc-comctl   WINTC_COMCTL)
wintc_resolve_library(wintc-comgtk   WINTC_COMGTK)
wintc_resolve_library(wintc-registry WINTC_REGISTRY)
wintc_resolve_library(wintc-shcommon WINTC_SHCOMMON)
wintc_resolve_library(wintc-shlang   WINTC_SHLANG)
wintc_resolve_library(wintc-syscfg   WINTC_SYSCFG)

wintc_compile_resources()
wintc_create_meta_h()

add_executable(
    wintc-cpl-desk
    src/application.c
    src/application.h
    src/main.c
    src/meta.h
    src/monitor.c
    src/monitor.h
    src/pagedesk.c
    src/pagedesk.h
    src/resources.c
    src/settings.c
    src/settings.h
    src/window.c
    src/window.h
)

set_target_properties(
    wintc-cpl-desk
    PROPERTIES
    OUTPUT_NAME desk.cpl
)

target_compile_options(
    wintc-cpl-desk
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-cpl-desk
    SYSTEM
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMCTL_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_REGISTRY_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHCOMMON_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHLANG_INCLUDE_DIRS}
    PRIVATE ${WINTC_SYSCFG_INCLUDE_DIRS}
)

target_link_directories(
    wintc-cpl-desk
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMCTL_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_REGISTRY_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHCOMMON_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHLANG_LIBRARY_DIRS}
    PRIVATE ${WINTC_SYSCFG_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-cpl-desk
    PRIVATE m
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMCTL_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_REGISTRY_LIBRARIES}
    PRIVATE ${WINTC_SHCOMMON_LIBRARIES}
    PRIVATE ${WINTC_SHLANG_LIBRARIES}
    PRIVATE ${WINTC_SYSCFG_LIBRARIES}
)

add_dependencies(
    wintc-cpl-desk
    build-gresources
)

# Installation
#
wintc_configure_and_install_packaging()

install(
    FILES desk.desktop
    DESTINATION share/wintc/cpl
)
install(
    FILES uk.co.oddmatics.wintc.cpl-desk.desktop
    DESTINATION share/applications
)
install(
    TARGETS wintc-cpl-desk
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

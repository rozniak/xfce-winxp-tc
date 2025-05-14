cmake_minimum_required(VERSION 3.12)

project(
    wintc-taskmgr
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion Task Manager application."
    LANGUAGES C
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/locale/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../../packaging/cmake-inc/resources/CMakeLists.txt)

wintc_resolve_library(glib-2.0       GLIB)
wintc_resolve_library(gtk+-3.0       GTK3)
wintc_resolve_library(wintc-comctl   WINTC_COMCTL)
wintc_resolve_library(wintc-comgtk   WINTC_COMGTK)
wintc_resolve_library(wintc-shcommon WINTC_SHCOMMON)
wintc_resolve_library(wintc-shell    WINTC_SHELL)
wintc_resolve_library(wintc-shellext WINTC_SHELLEXT)
wintc_resolve_library(wintc-shlang   WINTC_SHLANG)

wintc_compile_resources()
wintc_create_meta_h()

add_executable(
    wintc-taskmgr
    src/application.c
    src/application.h
    src/intapi.h
    src/main.c
    src/meta.h
    src/pageproc.c
    src/pageproc.h
    src/procmon.c
    src/procmon.h
    src/resources.c
    src/window.c
    src/window.h
)

set_target_properties(
    wintc-taskmgr
    PROPERTIES
    OUTPUT_NAME taskmgr
)

target_compile_options(
    wintc-taskmgr
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-taskmgr
    SYSTEM
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMCTL_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHCOMMON_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELL_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELLEXT_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHLANG_INCLUDE_DIRS}
)

target_link_directories(
    wintc-taskmgr
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMCTL_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHCOMMON_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELL_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELLEXT_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHLANG_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-taskmgr
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMCTL_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_SHCOMMON_LIBRARIES}
    PRIVATE ${WINTC_SHELL_LIBRARIES}
    PRIVATE ${WINTC_SHELLEXT_LIBRARIES}
    PRIVATE ${WINTC_SHLANG_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_install_icons_into_package()

install(
    TARGETS wintc-taskmgr
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

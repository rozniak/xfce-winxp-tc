cmake_minimum_required(VERSION 3.12)

project(
    wintc-icondbg
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion icon debugging tool."
    LANGUAGES C
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../../packaging/cmake-inc/resources/CMakeLists.txt)

wintc_resolve_library(glib-2.0     GLIB)
wintc_resolve_library(gtk+-3.0     GTK3)
wintc_resolve_library(wintc-comgtk WINTC_COMGTK)

wintc_compile_resources()

add_executable(
    wintc-icondbg
    src/application.c
    src/application.h
    src/main.c
    src/resources.c
    src/window.c
    src/window.h
)

target_compile_options(
    wintc-icondbg
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-icondbg
    SYSTEM
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
)

target_link_directories(
    wintc-icondbg
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-icondbg
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_install_icons_into_package()

install(
    FILES wintc-icondbg.desktop
    DESTINATION share/applications
)
install(
    TARGETS wintc-icondbg
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

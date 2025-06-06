cmake_minimum_required(VERSION 3.12)

project(
    wintc-exitwin
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion Exit Windows program."
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

wintc_resolve_library(gio-2.0      GIO)
wintc_resolve_library(glib-2.0     GLIB)
wintc_resolve_library(gtk+-3.0     GTK3)
wintc_resolve_library(wintc-comgtk WINTC_COMGTK)
wintc_resolve_library(wintc-msgina WINTC_MSGINA)

wintc_create_meta_h()

# Main target
#
add_executable(
    wintc-exitwin
    src/application.c
    src/application.h
    src/main.c
    src/meta.h
)

target_compile_options(
    wintc-exitwin
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-exitwin
    SYSTEM
    PRIVATE ${GIO_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_MSGINA_INCLUDE_DIRS}
)

target_link_directories(
    wintc-exitwin
    PRIVATE ${GIO_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_MSGINA_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-exitwin
    PRIVATE ${GIO_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_MSGINA_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
#wintc_compile_and_install_pofiles()

install(
    TARGETS wintc-exitwin
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

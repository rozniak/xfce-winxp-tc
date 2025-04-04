cmake_minimum_required(VERSION 3.12)

project(
    wintc-dbus-test-clt
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion DBus test client."
    LANGUAGES C
)

set(PROJECT_ANYARCH false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../../../packaging/cmake-inc/codegen/CMakeLists.txt)
include(../../../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../../../packaging/cmake-inc/packaging/CMakeLists.txt)

wintc_resolve_library(glib-2.0 GLIB)
wintc_resolve_library(gtk+-3.0 GTK3)
wintc_resolve_library(wintc-comgtk WINTC_COMGTK)

wintc_gdbus_codegen(
    testreg.xml
    testreg-skel
)

add_executable(
    wintc-dbus-test-clt
    src/main.c
    src/testreg-skel.c
    src/testreg-skel.h
)

target_compile_options(
    wintc-dbus-test-clt
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-dbus-test-clt
    SYSTEM
    BEFORE
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
)

target_link_directories(
    wintc-dbus-test-clt
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-dbus-test-clt
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()

install(
    TARGETS wintc-dbus-test-clt
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

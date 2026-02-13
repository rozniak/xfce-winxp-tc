cmake_minimum_required(VERSION 3.12)

project(
    wintc-setup-oobe
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion out-of-box experience."
    LANGUAGES C
)

set(PROJECT_ANYARCH false)
set(PROJECT_FREESTATUS false)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../../../packaging/cmake-inc/resources/CMakeLists.txt)

wintc_resolve_library(gio-unix-2.0   GIO_UNIX)
wintc_resolve_library(glib-2.0       GLIB)
wintc_resolve_library(gstreamer-1.0  GSTREAMER)
wintc_resolve_library(gtk+-3.0       GTK3)
wintc_resolve_library(wintc-comctl   WINTC_COMCTL)
wintc_resolve_library(wintc-comgtk   WINTC_COMGTK)
wintc_resolve_library(wintc-shcommon WINTC_SHCOMMON)
wintc_resolve_library(wintc-shelldpa WINTC_SHELLDPA)
wintc_resolve_library(wintc-shlang   WINTC_SHLANG)
wintc_resolve_library(libxml-2.0     XML2)

wintc_compile_resources()

add_executable(
    wintc-setup-oobe
    src/main.c
    src/oobeuser.c
    src/oobeuser.h
    src/oobewnd.c
    src/oobewnd.h
    src/resources.c
    src/xfconf.c
    src/xfconf.h
)

set_target_properties(
    wintc-setup-oobe
    PROPERTIES
    OUTPUT_NAME msoobe
)

target_compile_options(
    wintc-setup-oobe
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-setup-oobe
    SYSTEM
    BEFORE
    PRIVATE ${GIO_UNIX_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GSTREAMER_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMCTL_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHCOMMON_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELLDPA_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHLANG_INCLUDE_DIRS}
    PRIVATE ${XML2_INCLUDE_DIRS}
)

target_link_directories(
    wintc-setup-oobe
    PRIVATE ${GIO_UNIX_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GSTREAMER_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMCTL_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHCOMMON_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELLDPA_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHLANG_LIBRARY_DIRS}
    PRIVATE ${XML2_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-setup-oobe
    PRIVATE ${GIO_UNIX_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GSTREAMER_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMCTL_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_SHCOMMON_LIBRARIES}
    PRIVATE ${WINTC_SHELLDPA_LIBRARIES}
    PRIVATE ${WINTC_SHLANG_LIBRARIES}
    PRIVATE ${XML2_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()

install(
    DIRECTORY res/
    DESTINATION ${WINTC_ASSETS_INSTALL_DIR}/oobe
)
install(
    TARGETS wintc-setup-oobe
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

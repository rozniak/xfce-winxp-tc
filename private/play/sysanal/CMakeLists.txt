cmake_minimum_required(VERSION 3.14)

project(
    wintc-sysanal-test
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion system analysis test application."
    LANGUAGES C
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../../packaging/cmake-inc/packaging/CMakeLists.txt)

wintc_resolve_library(glib-2.0       GLIB)
wintc_resolve_library(gtk+-3.0       GTK3)
wintc_resolve_library(wintc-comgtk   WINTC_COMGTK)
wintc_resolve_library(wintc-setupapi WINTC_SETUPAPI)

add_executable(
    wintc-sysanal-test
    src/application.c
    src/application.h
    src/main.c
    src/window.c
    src/window.h
)

target_compile_options(
    wintc-sysanal-test
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-sysanal-test
    SYSTEM
    BEFORE
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_SETUPAPI_INCLUDE_DIRS}
)

target_link_directories(
    wintc-sysanal-test
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_SETUPAPI_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-sysanal-test
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_SETUPAPI_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_install_icons_into_package()

install(
    FILES wintc-sysanal-test.desktop
    DESTINATION share/applications
)
install(
    TARGETS wintc-sysanal-test
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

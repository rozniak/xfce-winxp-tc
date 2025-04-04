cmake_minimum_required(VERSION 3.12)

project(
    wintc-hello4
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion 'Hello World' application."
    LANGUAGES C
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../../../packaging/cmake-inc/packaging/CMakeLists.txt)

wintc_resolve_library(glib-2.0 GLIB)
wintc_resolve_library(gtk4     GTK4)

add_executable(
    wintc-hello4
    src/application.c
    src/application.h
    src/main.c
    src/window.c
    src/window.h
)

target_compile_options(
    wintc-hello4
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-hello4
    SYSTEM
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK4_INCLUDE_DIRS}
)

target_link_directories(
    wintc-hello4
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK4_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-hello4
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK4_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_install_icons_into_package()

install(
    FILES wintc-hello4.desktop
    DESTINATION share/applications
)
install(
    TARGETS wintc-hello4
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

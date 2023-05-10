cmake_minimum_required(VERSION 3.0)

project(
    wintc-shell-winver
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion 'About Windows' utility."
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(
    PROJECT_DEPENDENCIES
    libgdk-pixbuf-2.0-0
    libglib2.0-0
    libgtk-3-0
    libwintc-comgtk
    libwintc-winbrand
)

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GDK_PIXBUF     REQUIRED gdk-pixbuf-2.0)
pkg_check_modules(GLIB           REQUIRED glib-2.0)
pkg_check_modules(GTK3           REQUIRED gtk+-3.0)
pkg_check_modules(WINTC_COMGTK   REQUIRED wintc-comgtk)
pkg_check_modules(WINTC_WINBRAND REQUIRED wintc-winbrand)

add_executable(
    wintc-shell-winver
    src/winver.c
)

set_target_properties(
    wintc-shell-winver
    PROPERTIES
    OUTPUT_NAME winver
)

target_compile_options(
    wintc-shell-winver
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-shell-winver
    SYSTEM
    PRIVATE ${GDK_PIXBUF_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_WINBRAND_INCLUDE_DIRS}
)

target_link_directories(
    wintc-shell-winver
    PRIVATE ${GDK_PIXBUF_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_WINBRAND_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-shell-winver
    PRIVATE ${GDK_PIXBUF_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_WINBRAND_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()

install(
    DIRECTORY res/
    DESTINATION ${WINTC_ASSETS_INSTALL_DIR}/shell-res
)
install(
    TARGETS wintc-shell-winver
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

cmake_minimum_required(VERSION 3.0)

project(
    wintc-notepad
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion Notepad application."
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(
    PROJECT_DEPENDENCIES
    libglib2.0-0
    libgtk-3-0
    libwintc-comgtk
    libwintc-shllang
)

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/locale/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../../packaging/cmake-inc/resources/CMakeLists.txt)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB          REQUIRED glib-2.0)
pkg_check_modules(GTK3          REQUIRED gtk+-3.0)
pkg_check_modules(WINTC_COMGTK  REQUIRED wintc-comgtk)
pkg_check_modules(WINTC_SHLLANG REQUIRED wintc-shllang)

wintc_compile_resources()
wintc_create_meta_h()

add_executable(
    wintc-notepad
    src/application.c
    src/application.h
    src/main.c
    src/meta.h
    src/resources.c
    src/window.c
    src/window.h
)

set_target_properties(
    wintc-notepad
    PROPERTIES
    OUTPUT_NAME notepad
)

target_compile_options(
    wintc-notepad
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-notepad
    SYSTEM
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHLLANG_INCLUDE_DIRS}
)

target_link_directories(
    wintc-notepad
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHLLANG_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-notepad
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_SHLLANG_LIBRARIES}
)

add_dependencies(
    wintc-notepad
    build-gresources
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_install_icons_into_package()
wintc_compile_and_install_pofiles()

install(
    FILES notepad.desktop
    DESTINATION share/applications
)
install(
    TARGETS wintc-notepad
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

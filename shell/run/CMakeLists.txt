cmake_minimum_required(VERSION 3.0)

project(
    wintc-shell-run
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion 'Run...' dialog box."
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(
    PROJECT_DEPENDENCIES
    libglib2.0-0
    libgtk-3-0
    libwintc-comgtk
    libwintc-exec
)

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/locale/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GDK           REQUIRED gdk-3.0)
pkg_check_modules(GLIB          REQUIRED glib-2.0)
pkg_check_modules(GTK3          REQUIRED gtk+-3.0)
pkg_check_modules(WINTC_COMGTK  REQUIRED wintc-comgtk)
pkg_check_modules(WINTC_EXEC    REQUIRED wintc-exec)
pkg_check_modules(WINTC_SHLLANG REQUIRED wintc-shllang)

wintc_create_meta_h()

add_executable(
    wintc-shell-run
    src/application.c
    src/application.h
    src/dialog.c
    src/dialog.h
    src/history.h
    src/history.c
    src/main.c
    src/meta.h
)

set_target_properties(
    wintc-shell-run
    PROPERTIES
    OUTPUT_NAME run
)

target_compile_options(
    wintc-shell-run
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-shell-run
    SYSTEM
    PRIVATE ${GDK_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_EXEC_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHLLANG_INCLUDE_DIRS}
)

target_link_directories(
    wintc-shell-run
    PRIVATE ${GDK_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_EXEC_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHLLANG_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-shell-run
    PRIVATE ${GDK_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_EXEC_LIBRARIES}
    PRIVATE ${WINTC_SHLLANG_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_compile_and_install_pofiles()

install(
    TARGETS wintc-shell-run
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

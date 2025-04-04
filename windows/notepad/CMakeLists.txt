cmake_minimum_required(VERSION 3.12)

project(
    wintc-notepad
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion Notepad application."
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
wintc_resolve_library(wintc-comgtk   WINTC_COMGTK)
wintc_resolve_library(wintc-shcommon WINTC_SHCOMMON)
wintc_resolve_library(wintc-shell    WINTC_SHELL)
wintc_resolve_library(wintc-shlang   WINTC_SHLANG)
wintc_resolve_library(wintc-shellext WINTC_SHELLEXT)

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

# We would like the output executable to be called 'notepad', however some
# distros already use this name for WINE's Notepad, so we can only use the name
# on distros without this clash
#
# The var set here is so that the name can be used in the notepad.desktop file
#
set(NOTEPAD_OUTPUT_NAME ${PROJECT_NAME})

if (
    NOT ${WINTC_PKGMGR} STREQUAL "archpkg" AND
    NOT ${WINTC_PKGMGR} STREQUAL "rpm"
)
    set(NOTEPAD_OUTPUT_NAME notepad)

    set_target_properties(
        wintc-notepad
        PROPERTIES
        OUTPUT_NAME ${NOTEPAD_OUTPUT_NAME}
    )
endif()

target_compile_options(
    wintc-notepad
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-notepad
    SYSTEM
    BEFORE
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHCOMMON_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELL_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHLANG_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELLEXT_INCLUDE_DIRS}
)

target_link_directories(
    wintc-notepad
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHCOMMON_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELL_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHLANG_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELLEXT_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-notepad
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_SHCOMMON_LIBRARIES}
    PRIVATE ${WINTC_SHELL_LIBRARIES}
    PRIVATE ${WINTC_SHLANG_LIBRARIES}
    PRIVATE ${WINTC_SHELLEXT_LIBRARIES}
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

configure_file(notepad.desktop.in notepad.desktop @ONLY)

install(
    FILES ${CMAKE_BINARY_DIR}/notepad.desktop
    DESTINATION share/applications
)
install(
    TARGETS wintc-notepad
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

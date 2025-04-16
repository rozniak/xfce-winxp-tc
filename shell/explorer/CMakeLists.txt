cmake_minimum_required(VERSION 3.12)

project(
    wintc-explorer
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion Explorer application."
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

wintc_resolve_library(libcanberra      CANBERRA)
wintc_resolve_library(libcanberra-gtk3 CANBERRA_GTK3)
wintc_resolve_library(glib-2.0         GLIB)
wintc_resolve_library(gtk+-3.0         GTK3)
wintc_resolve_library(webkit2gtk-4.1   WEBKITGTK)
wintc_resolve_library(wintc-comctl     WINTC_COMCTL)
wintc_resolve_library(wintc-comgtk     WINTC_COMGTK)
wintc_resolve_library(wintc-shcommon   WINTC_SHCOMMON)
wintc_resolve_library(wintc-shell      WINTC_SHELL)
wintc_resolve_library(wintc-shellext   WINTC_SHELLEXT)
wintc_resolve_library(wintc-shlang     WINTC_SHLANG)

wintc_compile_resources()

add_executable(
    wintc-explorer
    src/application.c
    src/application.h
    src/loader.c
    src/loader.h
    src/main.c
    src/resources.c
    src/sidebar.c
    src/sidebar.h
    src/sidebars/favside.c
    src/sidebars/favside.h
    src/sidebars/fldrside.c
    src/sidebars/fldrside.h
    src/sidebars/srchside.c
    src/sidebars/srchside.h
    src/toolbar.c
    src/toolbar.h
    src/toolbars/adrbar.c
    src/toolbars/adrbar.h
    src/toolbars/stdbar.c
    src/toolbars/stdbar.h
    src/window.c
    src/window.h
)

set_target_properties(
    wintc-explorer
    PROPERTIES
    OUTPUT_NAME explorer
)

target_compile_options(
    wintc-explorer
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-explorer
    SYSTEM
    PRIVATE ${CANBERRA_INCLUDE_DIRS}
    PRIVATE ${CANBERRA_GTK3_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WEBKITGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMCTL_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHCOMMON_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELL_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELLEXT_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHLANG_INCLUDE_DIRS}
)

target_link_directories(
    wintc-explorer
    PRIVATE ${CANBERRA_LIBRARY_DIRS}
    PRIVATE ${CANBERRA_GTK3_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WEBKITGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMCTL_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHCOMMON_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELL_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELLEXT_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHLANG_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-explorer
    PRIVATE ${CANBERRA_LIBRARIES}
    PRIVATE ${CANBERRA_GTK3_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WEBKITGTK_LIBRARIES}
    PRIVATE ${WINTC_COMCTL_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_SHCOMMON_LIBRARIES}
    PRIVATE ${WINTC_SHELL_LIBRARIES}
    PRIVATE ${WINTC_SHELLEXT_LIBRARIES}
    PRIVATE ${WINTC_SHLANG_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_install_icons_into_package()

install(
    FILES explorer.desktop
    DESTINATION share/applications
)
install(
    FILES iexplore.desktop
    DESTINATION share/applications
)
install(
    FILES iexplore.sh
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ             GROUP_EXECUTE
        WORLD_READ             WORLD_EXECUTE
    RENAME iexplore
)
install(
    TARGETS wintc-explorer
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

cmake_minimum_required(VERSION 3.12)

project(
    wintc-cpl-sysdm
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion System Control Panel applet."
    LANGUAGES C
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../../packaging/cmake-inc/locale/CMakeLists.txt)
include(../../../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../../../packaging/cmake-inc/resources/CMakeLists.txt)

wintc_resolve_library(gdk-pixbuf-2.0 GDK_PIXBUF)
wintc_resolve_library(glib-2.0       GLIB)
wintc_resolve_library(gtk+-3.0       GTK3)
wintc_resolve_library(wintc-comctl   WINTC_COMCTL)
wintc_resolve_library(wintc-comgtk   WINTC_COMGTK)
wintc_resolve_library(wintc-exec     WINTC_EXEC)
wintc_resolve_library(wintc-shcommon WINTC_SHCOMMON)
wintc_resolve_library(wintc-shellext WINTC_SHELLEXT)
wintc_resolve_library(wintc-shlang   WINTC_SHLANG)

wintc_compile_resources()
wintc_create_meta_h()

add_executable(
    wintc-cpl-sysdm
    src/application.c
    src/application.h
    src/intapi.h
    src/main.c
    src/meta.h
    src/pagegen.c
    src/pagegen.h
    src/resources.c
    src/window.c
    src/window.h
)

set_target_properties(
    wintc-cpl-sysdm
    PROPERTIES
    OUTPUT_NAME sysdm.cpl
)

target_compile_options(
    wintc-cpl-sysdm
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-cpl-sysdm
    SYSTEM
    PRIVATE ${GDK_PIXBUF_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMCTL_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_EXEC_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHCOMMON_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELLEXT_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHLANG_INCLUDE_DIRS}
)

target_link_directories(
    wintc-cpl-sysdm
    PRIVATE ${GDK_PIXBUF_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMCTL_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_EXEC_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHCOMMON_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELLEXT_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHLANG_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-cpl-sysdm
    PRIVATE ${GDK_PIXBUF_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMCTL_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_EXEC_LIBRARIES}
    PRIVATE ${WINTC_SHCOMMON_LIBRARIES}
    PRIVATE ${WINTC_SHELLEXT_LIBRARIES}
    PRIVATE ${WINTC_SHLANG_LIBRARIES}
)

# FreeBSD requires linking to libsysinfo port
#
if (${WINTC_PKGMGR} STREQUAL "bsdpkg")
    wintc_resolve_library(libsysinfo SYSINFO)

    target_include_directories(
        wintc-cpl-sysdm
        PRIVATE ${SYSINFO_INCLUDE_DIRS}
    )
    target_link_directories(
        wintc-cpl-sysdm
        PRIVATE ${SYSINFO_LIBRARY_DIRS}
    )
    target_link_libraries(
        wintc-cpl-sysdm
        PRIVATE ${SYSINFO_LIBRARIES}
    )
endif()

add_dependencies(
    wintc-cpl-sysdm
    build-gresources
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_install_icons_into_package()

install(
    DIRECTORY res/${WINTC_PKGMGR}/
    DESTINATION ${WINTC_ASSETS_INSTALL_DIR}/dist-logo
)

install(
    FILES sysdm.desktop
    DESTINATION share/wintc/cpl
)
install(
    FILES uk.co.oddmatics.wintc.cpl-sysdm.desktop
    DESTINATION share/applications
)
install(
    TARGETS wintc-cpl-sysdm
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

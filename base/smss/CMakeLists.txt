cmake_minimum_required(VERSION 3.12)

project(
    wintc-smss
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion session manager subsystem."
    LANGUAGES C
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

set(WINTC_NO_PEDANTIC_COMPILE true) # Necessary for gdbus-codegen

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/codegen/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

wintc_resolve_library(glib-2.0       GLIB)
wintc_resolve_library(gtk+-3.0       GTK3)
wintc_resolve_library(wintc-comgtk   WINTC_COMGTK)
wintc_resolve_library(wintc-exec     WINTC_EXEC)
wintc_resolve_library(wintc-msgina   WINTC_MSGINA)
wintc_resolve_library(wintc-shcommon WINTC_SHCOMMON)
wintc_resolve_library(wintc-shell    WINTC_SHELL)
wintc_resolve_library(wintc-shellext WINTC_SHELLEXT)

wintc_gdbus_codegen(
    xfsm-manager-dbus.xml
    xfsm-dbus
    org.xfce.Session.
    ZWinXfsm
    false
)

add_executable(
    wintc-smss
    src/main.c
    src/xfsm.c
    src/xfsm.h
    src/xfsm-dbus.c
    src/xfsm-dbus.h
)

set_target_properties(
    wintc-smss
    PROPERTIES
    OUTPUT_NAME smss
)

target_compile_options(
    wintc-smss
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-smss
    SYSTEM
    BEFORE
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_EXEC_INCLUDE_DIRS}
    PRIVATE ${WINTC_MSGINA_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHCOMMON_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELL_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELLEXT_INCLUDE_DIRS}
)

target_link_directories(
    wintc-smss
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_EXEC_LIBRARY_DIRS}
    PRIVATE ${WINTC_MSGINA_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHCOMMON_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELL_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELLEXT_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-smss
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_EXEC_LIBRARIES}
    PRIVATE ${WINTC_MSGINA_LIBRARIES}
    PRIVATE ${WINTC_SHCOMMON_LIBRARIES}
    PRIVATE ${WINTC_SHELL_LIBRARIES}
    PRIVATE ${WINTC_SHELLEXT_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()

configure_file(
    res/startwintc.in
    startwintc
    @ONLY
)
configure_file(
    res/xinitrc.in
    xinitrc
    @ONLY
)

install(
    FILES res/wintc.desktop
    DESTINATION share/xsessions
)
install(
    PROGRAMS ${CMAKE_BINARY_DIR}/startwintc
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(
    PROGRAMS ${CMAKE_BINARY_DIR}/xinitrc
    DESTINATION ${WINTC_ASSETS_INSTALL_DIR}/smss
)
install(
    TARGETS wintc-smss
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

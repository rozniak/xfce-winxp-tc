cmake_minimum_required(VERSION 3.12)

project(
    wintc-logonui
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion logon user interface."
    LANGUAGES C
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS false)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

# Necessary because we use g_signal_handlers_disconnect...
set(WINTC_NO_PEDANTIC_COMPILE true)

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../../packaging/cmake-inc/resources/CMakeLists.txt)

wintc_resolve_library(gdk-pixbuf-2.0       GDK_PIXBUF)
wintc_resolve_library(glib-2.0             GLIB)
wintc_resolve_library(gtk+-3.0             GTK3)
wintc_resolve_library(liblightdm-gobject-1 LIGHTDM)
wintc_resolve_library(wintc-comctl         WINTC_COMCTL)
wintc_resolve_library(wintc-comgtk         WINTC_COMGTK)
wintc_resolve_library(wintc-msgina         WINTC_MSGINA)

wintc_compile_resources()

add_executable(
    wintc-logonui
    src/main.c
    src/resources.c
    src/settings.c
    src/settings.h
    src/window.c
    src/window.h
    src/classic/ui.c
    src/classic/ui.h
    src/welcome/ui.c
    src/welcome/ui.h
    src/welcome/userlist.c
    src/welcome/userlist.h
)

set_target_properties(
    wintc-logonui
    PROPERTIES
    OUTPUT_NAME logonui
)

target_compile_options(
    wintc-logonui
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-logonui
    SYSTEM
    BEFORE
    PRIVATE ${GDK_PIXBUF_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${LIGHTDM_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMCTL_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_MSGINA_INCLUDE_DIRS}
)

target_link_directories(
    wintc-logonui
    PRIVATE ${GDK_PIXBUF_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${LIGHTDM_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMCTL_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_MSGINA_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-logonui
    PRIVATE ${GDK_PIXBUF_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${LIGHTDM_LIBRARIES}
    PRIVATE ${WINTC_COMCTL_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_MSGINA_LIBRARIES}
)

add_dependencies(
    wintc-logonui
    build-gresources
)

# Installation
#
wintc_configure_and_install_packaging()

install(
    FILES wintc-logonui.desktop
    DESTINATION share/xgreeters
)
install(
    TARGETS wintc-logonui
    RUNTIME DESTINATION ${WINTC_SAFE_SBIN_DIR}
)

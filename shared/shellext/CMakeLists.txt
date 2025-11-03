cmake_minimum_required(VERSION 3.12)

project(
    libwintc-shellext
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion shell extension library."
    LANGUAGES C
)

set(PROJECT_ANYARCH false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

set(WINTC_NO_PEDANTIC_COMPILE true) # Necessary due to storing fn ptrs in map

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/libraries/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../../packaging/cmake-inc/resources/CMakeLists.txt)

wintc_resolve_library(glib-2.0     GLIB)
wintc_resolve_library(gtk+-3.0     GTK3)
wintc_resolve_library(wintc-comgtk WINTC_COMGTK)
wintc_resolve_library(wintc-exec   WINTC_EXEC)

configure_file(src/host_priv.h.in ${PROJECT_ROOT}/src/host_priv.h @ONLY)

add_library(
    libwintc-shellext
    public/category.h
    src/error.c
    public/error.h
    src/host.c
    public/host.h
    src/host_priv.h
    src/if_uihost.c
    public/if_uihost.h
    src/if_view.c
    public/if_view.h
    src/uictl.c
    public/uictl.h
    src/viewitem.c
    public/viewitem.h
    public/viewops.h
)

set_target_properties(
    libwintc-shellext
    PROPERTIES
    SOVERSION 1
    VERSION ${PROJECT_VERSION}
)

target_compile_options(
    libwintc-shellext
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    libwintc-shellext
    SYSTEM
    BEFORE
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_EXEC_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-shellext
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_EXEC_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-shellext
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_EXEC_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_add_pkgconfig_install()
wintc_install_public_headers()

install(
    TARGETS libwintc-shellext
    LIBRARY DESTINATION ${LIB_DIR}
)

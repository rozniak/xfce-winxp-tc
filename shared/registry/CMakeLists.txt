cmake_minimum_required(VERSION 3.12)

project(
    libwintc-registry
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion registry library."
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
include(../../packaging/cmake-inc/libraries/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

wintc_resolve_library(glib-2.0     GLIB)
wintc_resolve_library(gtk+-3.0     GTK3)
wintc_resolve_library(wintc-comgtk WINTC_COMGTK)

wintc_gdbus_codegen(
    dbus-if.xml
    reg-dbus
    ${WINTC_DBUS_DEFAULT_IF_PREFIX}
    ${WINTC_DBUS_DEFAULT_C_PREFIX}
    true
)

add_library(
    libwintc-registry
    public/reg-dbus.h
    src/reg-dbus.c
    src/reg-dbus.h
    src/regwrap.c
    public/regwrap.h
    src/variant.c
    public/variant.h
)

set_target_properties(
    libwintc-registry
    PROPERTIES
    SOVERSION 1
    VERSION ${PROJECT_VERSION}
)

target_compile_options(
    libwintc-registry
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    libwintc-registry
    SYSTEM
    BEFORE
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-registry
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-registry
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_add_pkgconfig_install()
wintc_install_public_headers()

install(
    TARGETS libwintc-registry
    LIBRARY DESTINATION ${LIB_DIR}
)

cmake_minimum_required(VERSION 3.0)

project(
    wintc-exec
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion launcher library."
)

include(GNUInstallDirs)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    set(ARCH_DIR "lib/x86_64-linux-gnu")
    set(ARCH_NAME "amd64")
else()
    message(FATAL_ERROR "No install dir defined for ${CMAKE_SYSTEM_PROCESSOR}!")
endif()

set(DEB_DEPENDENCIES "libglib2.0-dev, libgtk-3-dev, libwintc-comgtk-1.0")

find_package(PkgConfig REQUIRED)

pkg_check_modules(GDK  REQUIRED gdk-3.0)
pkg_check_modules(GIO  REQUIRED gio-2.0)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

add_library(
    wintc-exec
    src/exec.h
    src/exec.c
)

set_target_properties(
    wintc-exec
    PROPERTIES
    PUBLIC_HEADER public/wintc-exec.h
    SOVERSION 1
    VERSION ${PROJECT_VERSION}
)

target_compile_options(
    wintc-exec
    PRIVATE -Wall -Wextra -Wpedantic -Werror
)

target_include_directories(
    wintc-exec
    SYSTEM
    PRIVATE ${GDK_INCLUDE_DIRS}
    PRIVATE ${GIO_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
)

target_link_directories(
    wintc-exec
    PRIVATE ${GDK_LIBRARY_DIRS}
    PRIVATE ${GIO_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-exec
    PRIVATE ${GDK_LIBRARIES}
    PRIVATE ${GIO_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
)

configure_file(../debian-control.in debian-control @ONLY)
configure_file(wintc-exec.pc.in     wintc-exec.pc  @ONLY)
configure_file(public/wintc-exec.h  wintc-exec.h   COPYONLY)

install(
    TARGETS wintc-exec
    LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    FILES ${CMAKE_BINARY_DIR}/wintc-exec.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
)

cmake_minimum_required(VERSION 3.0)

project(wintc-exec VERSION 1.0 DESCRIPTION "Windows Total Conversion launcher library.")

include(GNUInstallDirs)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    set(ARCH_DIR "lib/x86_64-linux-gnu")
    set(ARCH_NAME "amd64")
else()
    message(FATAL_ERROR "No install dir defined for ${CMAKE_SYSTEM_PROCESSOR}!")
endif()

find_package(PkgConfig REQUIRED)

pkg_check_modules(GDK  REQUIRED gdk-3.0)
pkg_check_modules(GIO  REQUIRED gio-2.0)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(
    ${GDK_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS},
    ${GTK3_INCLUDE_DIRS})

link_directories(
    ${GDK_LIBRARY_DIRS}
    ${GIO_LIBRARY_DIRS}
    ${GLIB_LIBRARY_DIRS}
    ${GTK3_LIBRARY_DIRS})

add_library(wintc-exec
    src/exec.h
    src/exec.c)

set_target_properties(wintc-exec PROPERTIES
    PUBLIC_HEADER public/wintc-exec.h
    SOVERSION 1
    VERSION ${PROJECT_VERSION})

configure_file(../debian-control.in debian-control @ONLY)
configure_file(wintc-exec.pc.in wintc-exec.pc @ONLY)
configure_file(public/wintc-exec.h wintc-exec.h COPYONLY)

target_link_libraries(wintc-exec
    PRIVATE ${GDK_LIBRARIES}
    PRIVATE ${GIO_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES})

install(TARGETS wintc-exec
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${CMAKE_BINARY_DIR}/wintc-exec.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

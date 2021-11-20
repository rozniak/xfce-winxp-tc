cmake_minimum_required(VERSION 3.0)

project(wintc-comgtk VERSION 1.0 DESCRIPTION "Windows Total Conversion common GLib/GTK utilities.")

include(GNUInstallDirs)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    set(ARCH_DIR "lib/x86_64-linux-gnu")
    set(ARCH_NAME "amd64")
else()
    message(FATAL_ERROR "No install dir defined for ${CMAKE_SYSTEM_PROCESSOR}!")
endif()

set(DEB_DEPENDENCIES "libglib2.0-dev, libgtk-3-dev")

find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(
    ${GLIB_INCLUDE_DIRS}
    ${GTK3_INCLUDE_DIRS})

link_directories(
    ${GLIB_LIBRARY_DIRS}
    ${GTK3_LIBRARY_DIRS})

add_library(wintc-comgtk
    src/debug.h
    src/errors.c
    src/errors.h
    src/gchar.h
    src/list.c
    src/list.h
    src/msgbox.c
    src/msgbox.h
    src/profile.c
    src/profile.h
    src/shorthand.h
    src/strings.c
    src/strings.h
    src/styles.c
    src/styles.h
    src/window.c
    src/window.h)

set_target_properties(wintc-comgtk PROPERTIES
    PUBLIC_HEADER public/wintc-comgtk.h
    SOVERSION 1
    VERSION ${PROJECT_VERSION})

configure_file(../debian-control.in debian-control @ONLY)
configure_file(wintc-comgtk.pc.in wintc-comgtk.pc @ONLY)
configure_file(public/wintc-comgtk.h wintc-comgtk.h COPYONLY)

target_link_libraries(wintc-comgtk
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES})

install(TARGETS wintc-comgtk
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${CMAKE_BINARY_DIR}/wintc-comgtk.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

cmake_minimum_required(VERSION 3.0)

project(
    wintc-shllang
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion shell language string utilities."
)

include(GNUInstallDirs)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    set(ARCH_DIR "lib/x86_64-linux-gnu")
    set(ARCH_NAME "amd64")
else()
    message(FATAL_ERROR "No install dir defined for ${CMAKE_SYSTEM_PROCESSOR}!")
endif()

set(DEB_DEPENDENCIES "libglib2.0-dev")

find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB REQUIRED glib-2.0)

add_library(
    wintc-shllang
    src/config.h
    src/controls.c
    src/controls.h
    src/places.c
    src/places.h
    src/punctuation.h
)

set_target_properties(
    wintc-shllang
    PROPERTIES
    PUBLIC_HEADER public/wintc-shllang.h
    SOVERSION 1
    VERSION ${PROJECT_VERSION}
)

target_compile_options(
    wintc-shllang
    PRIVATE -Wall -Wextra -Wpedantic -Werror
)

target_include_directories(
    wintc-shllang
    SYSTEM
    PRIVATE ${GLIB_INCLUDE_DIRS}
)

target_link_directories(
    wintc-shllang
    PRIVATE ${GLIB_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-shllang
    PRIVATE ${GLIB_LIBRARIES}
)

configure_file(../debian-control.in   debian-control   @ONLY)
configure_file(wintc-shllang.pc.in    wintc-shllang.pc @ONLY)
configure_file(public/wintc-shllang.h wintc-shllang.h  COPYONLY)

install(
    TARGETS wintc-shllang
    LIBRARY DESTINATION       ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    FILES ${CMAKE_BINARY_DIR}/wintc-shllang.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
)

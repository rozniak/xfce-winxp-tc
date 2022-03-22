cmake_minimum_required(VERSION 3.0)

project(
    winver
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion 'About Windows' utility."
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB         REQUIRED glib-2.0)
pkg_check_modules(GTK3         REQUIRED gtk+-3.0)
pkg_check_modules(WINTC_COMGTK REQUIRED wintc-comgtk)

add_executable(
    winver
    src/winver.c
)

target_compile_options(
    winver
    PRIVATE -Wall -Wextra -Wpedantic -Werror
)

target_include_directories(
    winver
    SYSTEM
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
)

target_link_directories(
    winver
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
)

target_link_libraries(
    winver
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
)

cmake_minimum_required(VERSION 3.0)

project(
    npwrdlg
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion (new) power options dialog."
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB         REQUIRED glib-2.0)
pkg_check_modules(GTK3         REQUIRED gtk+-3.0)
pkg_check_modules(WINTC_COMGTK REQUIRED wintc-comgtk)

add_executable(
    npwrdlg
    src/application.c
    src/application.h
    src/dialog.c
    src/dialog.h
    src/main.c
    src/meta.h
)

target_compile_options(
    npwrdlg
    PRIVATE -Wall -Wextra -Wpedantic -Werror
)

target_include_directories(
    npwrdlg
    SYSTEM
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
)

target_link_directories(
    npwrdlg
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
)

target_link_libraries(
    npwrdlg
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
)

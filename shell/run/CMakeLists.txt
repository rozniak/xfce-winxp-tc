cmake_minimum_required(VERSION 3.0)

project(
    run
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion 'Run...' dialog box."
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GDK           REQUIRED gdk-3.0)
pkg_check_modules(GLIB          REQUIRED glib-2.0)
pkg_check_modules(GTK3          REQUIRED gtk+-3.0)
pkg_check_modules(WINTC_COMGTK  REQUIRED wintc-comgtk)
pkg_check_modules(WINTC_EXEC    REQUIRED wintc-exec)
pkg_check_modules(WINTC_SHLLANG REQUIRED wintc-shllang)

add_executable(
    run
    src/application.c
    src/application.h
    src/dialog.c
    src/dialog.h
    src/history.h
    src/history.c
    src/main.c
    src/meta.h
)

target_compile_options(
    run
    PRIVATE -Wall -Wextra -Wpedantic -Werror
)

target_include_directories(
    run
    SYSTEM
    PRIVATE ${GDK_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_EXEC_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHLLANG_INCLUDE_DIRS}
)

target_link_directories(
    run
    PRIVATE ${GDK_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_EXEC_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHLLANG_LIBRARY_DIRS}
)

target_link_libraries(
    run
    PRIVATE ${GDK_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_EXEC_LIBRARIES}
    PRIVATE ${WINTC_SHLLANG_LIBRARIES}
)

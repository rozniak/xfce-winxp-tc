cmake_minimum_required(VERSION 3.0)

project(
    npwrdlg
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion (new) power options dialog."
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GIO          REQUIRED gio-2.0)
pkg_check_modules(GLIB         REQUIRED glib-2.0)
pkg_check_modules(GTK3         REQUIRED gtk+-3.0)
pkg_check_modules(WINTC_COMGTK REQUIRED wintc-comgtk)

# Set up GResources target for the build
#
set(GRESOURCE_DEPENDENCIES "")

find_program(GLIB_COMPILE_RESOURCES glib-compile-resources REQUIRED)

execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/res
    COMMAND ${GLIB_COMPILE_RESOURCES} --generate-dependencies resources.xml
    OUTPUT_VARIABLE GRESOURCE_DEPENDENCIES
)

string(REPLACE "\n" ";" GRESOURCE_DEPENDENCIES ${GRESOURCE_DEPENDENCIES})
list(POP_BACK GRESOURCE_DEPENDENCIES)
list(TRANSFORM GRESOURCE_DEPENDENCIES PREPEND "src/res/")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/resources.c
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/res
    COMMAND ${GLIB_COMPILE_RESOURCES}
    ARGS
    --target=../resources.c
    --generate
    resources.xml
    VERBATIM
    MAIN_DEPENDENCY src/res/resources.xml
    DEPENDS
    ${GRESOURCE_DEPENDENCIES}
)

add_custom_target(
    project-resources
    DEPENDS
    src/resources.c
)

# Main target
#
add_executable(
    npwrdlg
    src/application.c
    src/application.h
    src/dialog.c
    src/dialog.h
    src/main.c
    src/meta.h
    src/resources.c
)

target_compile_options(
    npwrdlg
    PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-overlength-strings
)

target_include_directories(
    npwrdlg
    SYSTEM
    PRIVATE ${GIO_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
)

target_link_directories(
    npwrdlg
    PRIVATE ${GIO_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
)

target_link_libraries(
    npwrdlg
    PRIVATE ${GIO_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
)

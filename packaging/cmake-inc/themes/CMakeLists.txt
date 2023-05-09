cmake_minimum_required(VERSION 3.0)

# Define some constants for comm build paths
#
set(THEMES_BUILD_COMMON_DIR           ${REPO_ROOT}/themes/build-common)
set(THEMES_BUILD_COMMON_CMAKE_DIR     ${THEMES_BUILD_COMMON_DIR}/cmake)
set(THEMES_BUILD_COMMON_GTK2_DIR      ${THEMES_BUILD_COMMON_DIR}/gtk-2.0-base)
set(THEMES_BUILD_COMMON_GTK3_DIR      ${THEMES_BUILD_COMMON_DIR}/gtk-3.0-base)
set(THEMES_BUILD_COMMON_RESOURCES_DIR ${THEMES_BUILD_COMMON_DIR}/native-resources)

set(THEMES_BLDTHEME_TOOL_DIR ${REPO_ROOT}/tools/bldutils/bldtheme)

# Define source dependencies for GTK2 build
#
file(GLOB THEMES_BUILD_GTK2_FILES ${THEMES_BUILD_COMMON_GTK2_DIR}/*rc)

# Function to define constants for default build scope
#
function(wintc_define_theme_dirs)
    set(THEME_PROJECT_INSTALL_PATH share/themes/${THEME_DISPLAYNAME} PARENT_SCOPE)
endfunction()

# Ensure SASS program available for GTK3 build
#
find_program(SASS scss)

# Set up venv for bldtheme tool
#
find_package(PythonInterp 3.0 REQUIRED)

add_custom_command(
    OUTPUT ${THEMES_BLDTHEME_TOOL_DIR}/.venv
    WORKING_DIRECTORY ${THEMES_BLDTHEME_TOOL_DIR}
    COMMAND ${PYTHON_EXECUTABLE}
    ARGS
        -m
        venv
        .venv
    COMMAND ${THEMES_BLDTHEME_TOOL_DIR}/.venv/bin/pip
    ARGS
        install
        -r
        requirements.txt
        --upgrade
    VERBATIM
)

add_custom_target(
    venv-setup
    DEPENDS
        ${THEMES_BLDTHEME_TOOL_DIR}/.venv
)

# Add bltheme tool target to compile GTK2 resources
#
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/composed
    WORKING_DIRECTORY ${THEMES_BLDTHEME_TOOL_DIR}
    COMMAND ${THEMES_BLDTHEME_TOOL_DIR}/.venv/bin/python
    ARGS
        bldtheme.py
        ${PROJECT_ROOT}/scheme
        ${THEMES_BUILD_COMMON_RESOURCES_DIR}
        ${CMAKE_BINARY_DIR}/composed
    VERBATIM
    DEPENDS
        ${THEMES_BLDTHEME_TOOL_DIR}/.venv
)

add_custom_target(
    resources-gtk2 ALL
    DEPENDS
        venv-setup
        ${CMAKE_BINARY_DIR}/composed
)

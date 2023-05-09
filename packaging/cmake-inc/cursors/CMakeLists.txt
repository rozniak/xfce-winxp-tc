cmake_minimum_required(VERSION 3.0)

# Check stuff we need is defined
#
if (NOT DEFINED CURSOR_THEME_SHORTNAME)
    message(FATAL_ERROR "You must define CURSOR_THEME_SHORTNAME")
endif ()

# Define constants
#
set(CURSOR_PROJECT_INSTALL_PATH share/icons/${CURSOR_THEME_SHORTNAME})

set(CURSOR_COMPCURS_TOOL_DIR   ${REPO_ROOT}/tools/bldutils/compcurs)
set(CURSOR_COMPCURS_OUTPUT_DIR ${CMAKE_BINARY_DIR}/compiled-cursors)

# Define function for generating xcursors
#
function(wintc_add_gen_xcursors_target)
    find_package(PythonInterp 3.0 REQUIRED)

    add_custom_command(
        OUTPUT ${CURSOR_COMPCURS_OUTPUT_DIR}
        WORKING_DIRECTORY ${CURSOR_COMPCURS_TOOL_DIR}
        COMMAND ${PYTHON_EXECUTABLE}
        ARGS
            compcurs.py
            ${PROJECT_ROOT}/cfg
            ${PROJECT_ROOT}/res
            ${CURSOR_COMPCURS_OUTPUT_DIR}
        VERBATIM
    )

    add_custom_target(
        compile-cursors ALL
        DEPENDS
            ${CURSOR_COMPCURS_OUTPUT_DIR}
    )
endfunction()

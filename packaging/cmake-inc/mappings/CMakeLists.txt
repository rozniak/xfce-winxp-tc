cmake_minimum_required(VERSION 3.0)

# Define constants
#
set(XDG_BLDMAPS_TOOL_DIR   ${REPO_ROOT}/tools/bldutils/bldmaps)
set(XDG_BLDMAPS_OUTPUT_DIR ${CMAKE_BINARY_DIR}/xdg-compiled-maps)

# Define function for creating symlinks for themes
#
function(wintc_add_mappings_target KIND)
    find_package(PythonInterp 3.0 REQUIRED)

    add_custom_command(
        OUTPUT ${XDG_BLDMAPS_OUTPUT_DIR}
        WORKING_DIRECTORY ${XDG_BLDMAPS_TOOL_DIR}
        COMMAND ${PYTHON_EXECUTABLE}
        ARGS
            bldmaps.py
            ${PROJECT_ROOT}
            ${XDG_BLDMAPS_OUTPUT_DIR}
            ${KIND}
        VERBATIM
        MAIN_DEPENDENCY ${PROJECT_ROOT}/mappings
    )

    add_custom_target(
        xdg-symlinks ALL
        DEPENDS
            ${XDG_BLDMAPS_OUTPUT_DIR}
    )
endfunction()

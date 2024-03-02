cmake_minimum_required(VERSION 3.5)

find_package(PkgConfig REQUIRED)

# If we're using local libs, then we better have a root dir to look in for em
#
if (${WINTC_USE_LOCAL_LIBS})
    if (NOT DEFINED WINTC_LOCAL_LIBS_ROOT)
        message(
            FATAL_ERROR
            "Specify WINTC_LOCAL_LIBS_ROOT for where libs are being built."
        )
    endif()
endif()

# Macro for setting custom variable name
#
macro(wintc_set SET_NAME SET_VALUE)
    set(${SET_NAME} ${SET_VALUE} PARENT_SCOPE)
endmacro()

# Define func for resolving a library for linking
#
function(wintc_resolve_library REQUESTED_LIB REQUESTED_LIB_NICE_NAME)
    if (
        ${WINTC_USE_LOCAL_LIBS} AND
        ${REQUESTED_LIB} MATCHES "^wintc-(.+)"
    )
        # We must look for where the include dir is, in the local libs root
        #
        # CMake's file(GLOB_RECURSE) is broken crap, so we have no choice but
        # to find the file and filter out any unwanted matches ourselves
        # (such as wintc/exec/exec.h - we only want wintc/exec.h)
        #
        # After finding the file, we traverse up two path segments, to the
        # include/ dir, because source code uses #include "wintc/myapi.h" as
        # their paths
        #
        file(
            GLOB_RECURSE
            LIB_INCLUDE_FULL_PATH
            ${WINTC_LOCAL_LIBS_ROOT}/*/${CMAKE_MATCH_1}.h
        )
        list(
            FILTER
            LIB_INCLUDE_FULL_PATH
            INCLUDE
            REGEX
                wintc/${CMAKE_MATCH_1}.h
        )
        get_filename_component(
            LIB_INCLUDE_DIR
            ${LIB_INCLUDE_FULL_PATH}
            DIRECTORY
        )
        get_filename_component(
            LIB_INCLUDE_DIR
            ${LIB_INCLUDE_DIR}
            DIRECTORY
        )

        wintc_set(
            "${REQUESTED_LIB_NICE_NAME}_INCLUDE_DIRS"
            "${LIB_INCLUDE_DIR}"
        )
        wintc_set(
            "${REQUESTED_LIB_NICE_NAME}_LIBRARY_DIRS"
            ""
        )
        wintc_set(
            "${REQUESTED_LIB_NICE_NAME}_LIBRARIES"
            "${WINTC_LOCAL_LIBS_ROOT}/shared/${CMAKE_MATCH_1}/lib${REQUESTED_LIB}.so"
        )
    else()
        pkg_check_modules(${REQUESTED_LIB_NICE_NAME} REQUIRED ${REQUESTED_LIB})
    endif()
endfunction()

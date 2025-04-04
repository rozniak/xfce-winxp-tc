cmake_minimum_required(VERSION 3.12)

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
        wintc_set(
            "${REQUESTED_LIB_NICE_NAME}_INCLUDE_DIRS"
            "${WINTC_LOCAL_LIBS_ROOT}/shared/${CMAKE_MATCH_1}"
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

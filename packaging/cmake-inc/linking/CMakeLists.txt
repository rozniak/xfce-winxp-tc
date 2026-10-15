cmake_minimum_required(VERSION 3.14)

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
function(wintc_resolve_library REQUESTED_LIB REQUESTED_LIB_NICE_NAME SCOPE)
    if (
        ${WINTC_USE_LOCAL_LIBS} AND
        ${REQUESTED_LIB} MATCHES "^wintc-(.+)"
    )
        set(CMAKE_PREFIX_PATH ${WINTC_LOCAL_LIBS_ROOT} ${CMAKE_PREFIX_PATH})
    endif()

    pkg_check_modules(${REQUESTED_LIB_NICE_NAME} REQUIRED ${REQUESTED_LIB})

    if (${SCOPE} STREQUAL "PUBLIC")
        list(APPEND WINTC_PKGCONF_REQUIRE_LIST ${REQUESTED_LIB})
        wintc_set(WINTC_PKGCONF_REQUIRE_LIST "${WINTC_PKGCONF_REQUIRE_LIST}")
    endif()
endfunction()

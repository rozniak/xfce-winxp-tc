cmake_minimum_required(VERSION 3.12)

# Define some constants
#
set(DEFAULT_LIB_PKGCONFIG ${CMAKE_CURRENT_LIST_DIR}/pkgconfig.pc.in)
set(WINTC_LIB_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}/wintc)

# Prevent CMake from prepending 'lib' on libraries
#
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_SHARED_MODULE_PREFIX  "")
set(CMAKE_STATIC_LIBRARY_PREFIX "")

# Extract library's name
#
if (${PROJECT_NAME} MATCHES "^lib(wintc-(.+))")
    set(LIB_NAME        ${CMAKE_MATCH_1})
    set(LIB_NAME_SIMPLE ${CMAKE_MATCH_2})
else()
    message(
        FATAL_ERROR
        "Library project names must start with 'lib'."
    )
endif()

# Function for adding pkgconfig to install
#
function(wintc_add_pkgconfig_install)
    configure_file(${DEFAULT_LIB_PKGCONFIG} ${LIB_NAME}.pc @ONLY)
    install(FILES ${CMAKE_BINARY_DIR}/${LIB_NAME}.pc DESTINATION ${WINTC_SAFE_PKGCONFIG_DIR}/pkgconfig)
endfunction()

# Function for installing public headers
#
function(wintc_install_public_headers)
    # Pull out public header sources
    #
    get_target_property(LIB_SOURCES ${PROJECT_NAME} SOURCES)

    foreach (SOURCE_FILEPATH IN LISTS LIB_SOURCES)
        if (${SOURCE_FILEPATH} MATCHES "^public/(.+)")
            list(APPEND LIB_PUBLIC_HEADERS ${CMAKE_MATCH_0})
        else()
            continue()
        endif()
    endforeach()

    # Configure file output - output two versions of the aggregate header file,
    # one for install, and one strictly for local lib builds
    #
    set(LIB_HEADER_DIR "${LIB_NAME_SIMPLE}")
    configure_file(public/libapi.h.in ${PROJECT_ROOT}/public/libapi.h @ONLY)

    set(LIB_HEADER_DIR "${PROJECT_ROOT}/public")
    configure_file(public/libapi.h.in wintc/${LIB_NAME_SIMPLE}.h @ONLY)

    # Add installs
    #
    set(LIB_INCLUDEDIR ${WINTC_LIB_INCLUDEDIR}/${LIB_NAME_SIMPLE})

    install(
        FILES ${LIB_PUBLIC_HEADERS}
          DESTINATION ${LIB_INCLUDEDIR}
    )
    install(
        FILES public/libapi.h
          DESTINATION ${WINTC_LIB_INCLUDEDIR}
          RENAME ${LIB_NAME_SIMPLE}.h
    )
endfunction()

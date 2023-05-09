cmake_minimum_required(VERSION 3.9)

# Define some constants
#
set(DEFAULT_LIB_PKGCONFIG ${CMAKE_CURRENT_LIST_DIR}/pkgconfig.pc.in)

# Prevent CMake from prepending 'lib' on libraries
#
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_SHARED_MODULE_PREFIX  "")
set(CMAKE_STATIC_LIBRARY_PREFIX "")

# Extract library's name
#
if (${PROJECT_NAME} MATCHES "^lib(.+)")
    set(LIB_NAME ${CMAKE_MATCH_1})
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
    install(FILES ${CMAKE_BINARY_DIR}/${LIB_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endfunction()

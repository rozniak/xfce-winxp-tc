cmake_minimum_required(VERSION 3.12)

# Emit the project name we're building, this can be pulled out by packaging
# scripts
#
message(STATUS "CMAKING:${PROJECT_NAME}")

# If no target package format was specified, default to 'raw' (aka we just
# dump the files out)
#
if (NOT DEFINED WINTC_PKGMGR)
    set(WINTC_PKGMGR raw)
endif()

# Do special stuff for the target package format, if needed
#
if (${WINTC_PKGMGR} STREQUAL "raw")
    message(STATUS "Outputting raw build (no package manager)")
elseif (${WINTC_PKGMGR} STREQUAL "deb")
    message(STATUS "Outputting build for Debian packaging")

    if (${PROJECT_FREESTATUS})
        set(DEB_SECTION main)
    else()
        set(DEB_SECTION non-free)
    endif()

    if (${PROJECT_ANYARCH})
        set(DEB_ARCHITECTURE all)
    else()
        # Map uname arch to dpkg arch
        #
        if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
            set(DEB_ARCHITECTURE amd64)
        elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
            set(DEB_ARCHITECTURE i386)
        elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ia64")
            set(DEB_ARCHITECTURE ia64)
        elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv7l")
            set(DEB_ARCHITECTURE armhf)
        elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv8")
            set(DEB_ARCHITECTURE arm64)
        else()
            message(
                FATAL_ERROR
                "No dpkg architecture mapping for: ${CMAKE_SYSTEM_PROCESSOR}"
            )
        endif()
    endif()

    # Map dependencies to Debian
    #
    if (DEFINED PROJECT_DEPENDENCIES)
        list(
            JOIN
            PROJECT_DEPENDENCIES
            ", "
            DEB_DEPENDENCIES
        )

        string(
            PREPEND
            DEB_DEPENDENCIES
            "Depends: "
        )
    endif()

    configure_file(${CMAKE_CURRENT_LIST_DIR}/debian-control.in control @ONLY)
    install(FILES ${CMAKE_BINARY_DIR}/control DESTINATION ${DESTDIR}/DEBIAN)
else()
    message(
        FATAL_ERROR
        "Unsupported package manager requested: ${WINTC_PKGMGR}"
    )
endif()

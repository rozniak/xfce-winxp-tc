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

# Set up library directory
#
if (
    ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686" OR
    ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv71"
)
    set(LIB_DIR lib)
elseif (
    ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" OR
    ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ia64"   OR
    ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv8"
)
    set(LIB_DIR lib64)
else()
    message(
        FATAL_ERROR
        "No library directory for architecture: ${CMAKE_SYSTEM_PROCESSOR}"
    )
endif()

# Do special stuff for the target package format, if needed
#
set(WINTC_DEBIAN_CONTROL_IN_PATH ${CMAKE_CURRENT_LIST_DIR}/debian-control.in)

function(wintc_configure_and_install_packaging)
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

        # Map architectures to libdirs
        #
        if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
            set(LIB_DIR lib/x86_64-linux-gnu PARENT_SCOPE)
        elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
            set(LIB_DIR lib/i386-linux-gnu PARENT_SCOPE)
        elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ia64")
            set(LIB_DIR lib/ia64-linux-gnu PARENT_SCOPE)
        elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv71")
            set(LIB_DIR lib/arm-linux-gnu PARENT_SCOPE)
        elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv8")
            set(LIB_DIR lib/aarch64-linux-gnu PARENT_SCOPE)
        else()
            message(
                FATAL_ERROR
                "No deb multiarch for architecture: ${CMAKE_SYSTEM_PROCESSOR}"
            )
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

        # Add postinstall script
        #
        if (DEFINED PROJECT_POSTINSTALL_SCRIPT)
            install(
                FILES ${PROJECT_POSTINSTALL_SCRIPT}
                DESTINATION ${DESTDIR}/DEBIAN
                PERMISSIONS
                    OWNER_READ
                    OWNER_EXECUTE
                    GROUP_READ
                    GROUP_EXECUTE
                    WORLD_READ
                    WORLD_EXECUTE
                RENAME postinst
            )
        endif()

        configure_file(${WINTC_DEBIAN_CONTROL_IN_PATH} control @ONLY)
        install(FILES ${CMAKE_BINARY_DIR}/control DESTINATION ${DESTDIR}/DEBIAN)
    else()
        message(
            FATAL_ERROR
            "Unsupported package manager requested: ${WINTC_PKGMGR}"
        )
    endif()
endfunction()

# Define build for application icon deployment
#
set(
    WINTC_ICON_SIZES
    16x16
    24x24
    32x32
    48x48
)

function(wintc_install_icons_into_package)
    foreach(ICON_SIZE ${WINTC_ICON_SIZES})
        set(CURRENT_ICON_SIZE_DIR ${PROJECT_ROOT}/icons/${ICON_SIZE})

        if (
            NOT EXISTS ${CURRENT_ICON_SIZE_DIR} OR
            NOT IS_DIRECTORY ${CURRENT_ICON_SIZE_DIR}
        )
            continue()
        endif()

        file(GLOB ICONS_TO_DEPLOY ${CURRENT_ICON_SIZE_DIR}/*.png)

        install(
            FILES ${ICONS_TO_DEPLOY}
            DESTINATION share/icons/hicolor/${ICON_SIZE}/apps
        )
    endforeach()
endfunction()

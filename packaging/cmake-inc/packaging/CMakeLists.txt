cmake_minimum_required(VERSION 3.12)

# Define constants
#
set(DEPMAP_TOOL_DIR ${REPO_ROOT}/tools/bldutils/depmap)

# Emit the project name we're building, this can be pulled out by packaging
# scripts
#
message(STATUS "CMAKING:${PROJECT_NAME}")

# If no target package format was specified, default to 'raw' (aka we just
# dump the files out)
#
if (NOT DEFINED WINTC_PKGMGR OR "${WINTC_PKGMGR}" STREQUAL "")
    set(WINTC_PKGMGR raw)
endif()

# Set up library directory
#
if (
    ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686"   OR
    ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv6l" OR
    ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv7l"
)
    set(LIB_DIR lib)
elseif (
    ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "amd64"  OR
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

# Library directory varies on some distros...
#
if (
    ${WINTC_PKGMGR} STREQUAL "apk"     OR
    ${WINTC_PKGMGR} STREQUAL "archpkg" OR
    ${WINTC_PKGMGR} STREQUAL "bsdpkg"
)
    # No lib64 on some distros
    #
    set(LIB_DIR lib)
elseif (
    ${WINTC_PKGMGR} STREQUAL "deb"
)
    # Debian needs mapping to multi-arch
    #
    if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
        set(LIB_DIR lib/x86_64-linux-gnu)
    elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
        set(LIB_DIR lib/i386-linux-gnu)
    elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ia64")
        set(LIB_DIR lib/ia64-linux-gnu)
    elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv6l")
        set(LIB_DIR lib/arm-linux-gnueabihf)
    elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv7l")
        set(LIB_DIR lib/arm-linux-gnu)
    elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv8")
        set(LIB_DIR lib/aarch64-linux-gnu)
    else()
        message(
            FATAL_ERROR
            "No deb multiarch for architecture: ${CMAKE_SYSTEM_PROCESSOR}"
        )
    endif()
endif()

# Set up sbin/bin directory target (some distros don't use sbin)
#
if (
    ${WINTC_PKGMGR} STREQUAL "apk"     OR
    ${WINTC_PKGMGR} STREQUAL "archpkg"
)
    set(WINTC_SAFE_SBIN_DIR ${CMAKE_INSTALL_BINDIR})
else()
    set(WINTC_SAFE_SBIN_DIR ${CMAKE_INSTALL_SBINDIR})
endif()

# Set up pkgconfig directory target
#
if (${WINTC_PKGMGR} STREQUAL "bsdpkg")
    set(WINTC_SAFE_PKGCONFIG_DIR libdata)
else()
    set(WINTC_SAFE_PKGCONFIG_DIR ${CMAKE_INSTALL_LIBDIR})
endif()

# Define dependency mapping function
#
function(wintc_map_dependencies)
    if (${WINTC_PKGMGR} STREQUAL "raw")
        return()
    endif()

    if (NOT EXISTS ${PROJECT_ROOT}/deps)
        return()
    endif()

    find_package(
        Python3
        COMPONENTS
            Interpreter
        REQUIRED
    )

    execute_process(
        COMMAND ${Python3_EXECUTABLE} ${DEPMAP_TOOL_DIR}/depmap.py deps ${WINTC_PKGMGR}
        WORKING_DIRECTORY ${PROJECT_ROOT}
        OUTPUT_VARIABLE DEPENDENCIES_MAP_RAW
        RESULT_VARIABLE DEPMAP_RET
    )

    if (DEPMAP_RET AND NOT DEPMAP_RET EQUAL 0)
        message(
            FATAL_ERROR
            "Failed to map dependencies for packaging."
        )
    endif()

    string(REPLACE "\n" ";" DEPENDENCIES_MAP_RAW ${DEPENDENCIES_MAP_RAW})

    foreach (line ${DEPENDENCIES_MAP_RAW})
        if (${line} MATCHES "^rt:(.+)$")
            list(APPEND DEPENDENCIES_MAP ${CMAKE_MATCH_1})
        endif()
    endforeach()

    set(DEPENDENCIES_MAP ${DEPENDENCIES_MAP} PARENT_SCOPE)
endfunction()

# Do special stuff for the target package format, if needed
#
set(WINTC_ALPINE_APKBUILD_IN_PATH ${CMAKE_CURRENT_LIST_DIR}/APKBUILD.in)
set(WINTC_ARCH_PKGBUILD_IN_PATH   ${CMAKE_CURRENT_LIST_DIR}/PKGBUILD.in)
set(WINTC_BSD_MANIFEST_IN_PATH    ${CMAKE_CURRENT_LIST_DIR}/bsd-manifest.in)
set(WINTC_DEBIAN_CONTROL_IN_PATH  ${CMAKE_CURRENT_LIST_DIR}/debian-control.in)

function(wintc_configure_and_install_packaging)
    # Provide 'ALT_PROJECT_NAME' - basically some distros skip the 'lib' in
    # library names, so this alternative name is generated here
    #
    if (${PROJECT_NAME} MATCHES "^lib(.+)")
        set(ALT_PROJECT_NAME ${CMAKE_MATCH_1})
    else()
        set(ALT_PROJECT_NAME ${PROJECT_NAME})
    endif()

    # Handle the package formats here
    #
    if (${WINTC_PKGMGR} STREQUAL "raw")
        message(STATUS "Outputting raw build (no package manager)")
    elseif (${WINTC_PKGMGR} STREQUAL "apk")
        message(STATUS "Outputting build for Alpine Linux packaging")

        # Set the architecture
        #
        set(ALPINE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

        if (${PROJECT_ANYARCH})
            set(ALPINE_ARCHITECTURE noarch)
        endif()

        # Set the licence
        #
        if (${PROJECT_FREESTATUS})
            set(ALPINE_LICENCE GPL-2.0-or-later)
        else()
            set(ALPINE_LICENCE non-free) # Unsure if correct
        endif()

        configure_file(${WINTC_ALPINE_APKBUILD_IN_PATH} APKBUILD @ONLY)
    elseif (${WINTC_PKGMGR} STREQUAL "archpkg")
        message(STATUS "Outputting build for Arch Linux packaging")

        # Set the architecture
        #
        set(ARCH_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

        if (${PROJECT_ANYARCH})
            set(ARCH_ARCHITECTURE any)
        endif()

        # Set the licence
        #
        if (${PROJECT_FREESTATUS})
            set(ARCH_LICENCE GPL)
        else()
            set(ARCH_LICENCE non-free)
        endif()

        # Map dependencies to Arch Linux
        #
        wintc_map_dependencies()

        if (DEFINED DEPENDENCIES_MAP)
            list(JOIN DEPENDENCIES_MAP " " ARCH_DEPENDENCIES)
            string(PREPEND ARCH_DEPENDENCIES "depends=(")
            string(APPEND  ARCH_DEPENDENCIES ")")
        endif()

        # Handle build options
        #
        if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
            set(ARCH_PKGBUILD_OPTIONS "strip !debug")
        else()
            set(ARCH_PKGBUILD_OPTIONS "!strip !debug")
        endif()

        configure_file(${WINTC_ARCH_PKGBUILD_IN_PATH} PKGBUILD @ONLY)
    elseif (${WINTC_PKGMGR} STREQUAL "bsdpkg")
        message(STATUS "Outputting build for FreeBSD packaging")

        # Set the licence
        #
        if (${PROJECT_FREESTATUS})
            set(BSD_LICENCE GPL)
        else()
            set(BSD_LICENCE non-free)
        endif()

        # Map dependencies to FreeBSD
        #
        wintc_map_dependencies()

        if (DEFINED DEPENDENCIES_MAP)
            list(JOIN DEPENDENCIES_MAP ": { origin: '*', version: '*' },\n" BSD_DEPENDENCIES)
            string(APPEND BSD_DEPENDENCIES ": { origin: '*', version: '*' }")
        endif()

        configure_file(${WINTC_BSD_MANIFEST_IN_PATH} manifest @ONLY)
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
            elseif (
                ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv6l" OR # Raspberry Pi
                ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv7l"
            )
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
        wintc_map_dependencies()

        if (DEFINED DEPENDENCIES_MAP)
            list(JOIN DEPENDENCIES_MAP ", " DEB_DEPENDENCIES)
            string(PREPEND DEB_DEPENDENCIES "Depends: ")
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

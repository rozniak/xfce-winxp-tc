cmake_minimum_required(VERSION 3.14)

project(
    libwintc-netshell
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion shell network connections library."
    LANGUAGES C
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/libraries/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../../packaging/cmake-inc/resources/CMakeLists.txt)

wintc_resolve_library(glib-2.0     GLIB         PUBLIC)
wintc_resolve_library(gtk+-3.0     GTK3         PUBLIC)
wintc_resolve_library(wintc-comgtk WINTC_COMGTK PRIVATE)

wintc_compile_resources()

add_library(
    libwintc-netshell
    public/client.h
    public/device.h
    src/dialogs.c
    src/dialogs.h
    public/dialogs.h
    src/netwnd.c
    public/netwnd.h
    src/resources.c
    src/statwnd.c
    public/statwnd.h
    src/util.c
    public/util.h
)

# We prefer to use NetworkManager, except on FreeBSD which does not have it and
# must rely on shell tools and wpa_supplicant through pkexec
#
if (${WINTC_PKGMGR} STREQUAL "bsdpkg")
    set(
        WINTC_NETCTL_PATH
        ${CMAKE_INSTALL_FULL_LIBEXECDIR}/wintc/wintc-netctl
    )

    target_sources(
        libwintc-netshell
        PRIVATE src/bsd/backend.h
        PRIVATE src/bsd/client.c
        PRIVATE src/bsd/device.c
        PRIVATE src/bsd/wpactrl.c
        PRIVATE src/bsd/wpactrl.h
    )

    target_compile_definitions(
        libwintc-netshell
        PRIVATE WINTC_NETCTL_PATH="${WINTC_NETCTL_PATH}"
    )

    configure_file(
        res/uk.co.oddmatics.wintc.netshell.policy.in
        uk.co.oddmatics.wintc.netshell.policy
        @ONLY
    )
    configure_file(
        res/wintc-net.conf.in
        wintc-net.conf.in
        @ONLY
    )

    install(
        PROGRAMS
            res/wintc-net-devd
            res/wintc-netctl
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/wintc
    )
    install(
        FILES ${CMAKE_BINARY_DIR}/uk.co.oddmatics.wintc.netshell.policy
        DESTINATION share/polkit-1/actions
    )
    install(
        FILES ${CMAKE_BINARY_DIR}/wintc-net.conf
        DESTINATION etc/devd
    )
else()
    wintc_resolve_library(libnm NM PRIVATE)

    target_sources(
        libwintc-netshell
        PRIVATE src/nm/backend.h
        PRIVATE src/nm/client.c
        PRIVATE src/nm/device.c
    )
endif()

set_target_properties(
    libwintc-netshell
    PROPERTIES
    SOVERSION 1
    VERSION ${PROJECT_VERSION}
)

target_compile_options(
    libwintc-netshell
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    libwintc-netshell
    SYSTEM
    BEFORE
    PUBLIC ${GLIB_INCLUDE_DIRS}
    PUBLIC ${GTK3_INCLUDE_DIRS}
    PRIVATE ${NM_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-netshell
    PUBLIC ${GLIB_LIBRARY_DIRS}
    PUBLIC ${GTK3_LIBRARY_DIRS}
    PRIVATE ${NM_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-netshell
    PUBLIC ${GLIB_LIBRARIES}
    PUBLIC ${GTK3_LIBRARIES}
    PRIVATE ${NM_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_add_pkgconfig_install()
wintc_install_public_headers()

install(
    TARGETS libwintc-netshell
    LIBRARY DESTINATION ${LIB_DIR}
)

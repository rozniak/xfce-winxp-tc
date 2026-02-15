cmake_minimum_required(VERSION 3.12)

project(
    wintc-launch
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion session launcher."
    LANGUAGES C
)

set(PROJECT_ANYARCH    true)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

# Installation
#
wintc_configure_and_install_packaging()

configure_file(
    res/startwintc.in
    startwintc
    @ONLY
)
configure_file(
    res/xinitrc.in
    xinitrc
    @ONLY
)

install(
    FILES
        ${CMAKE_BINARY_DIR}/xinitrc
        res/setup-gtk-settings.ini
        res/setup-xfwm4.xml
        res/wintc-launch.service
    DESTINATION ${WINTC_ASSETS_INSTALL_DIR}/launch
)
install(
    PROGRAMS ${CMAKE_BINARY_DIR}/startwintc
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

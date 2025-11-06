cmake_minimum_required(VERSION 3.12)

project(
    wintc-user-pictures
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion user pictures."
    LANGUAGES C
)

set(PROJECT_ANYARCH    true)
set(PROJECT_FREESTATUS false)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

# Installation
#
wintc_configure_and_install_packaging()

install(
    DIRECTORY ${PROJECT_ROOT}/default/
    DESTINATION ${WINTC_ASSETS_INSTALL_DIR}/userpics/default
)
install(
    FILES ${PROJECT_ROOT}/guest.bmp
    DESTINATION ${WINTC_ASSETS_INSTALL_DIR}/userpics
)

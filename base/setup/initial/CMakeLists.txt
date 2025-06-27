cmake_minimum_required(VERSION 3.12)

project(
    wintc-setup-init
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion initial phase setup."
    LANGUAGES C
)

set(PROJECT_ANYARCH    true)
set(PROJECT_FREESTATUS false)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../../packaging/cmake-inc/common/CMakeLists.txt)

set(CMAKE_INSTALL_PREFIX "")

# Installation
#
install(
    PROGRAMS setup.sh
    DESTINATION .
)
install(
    DIRECTORY autorun
    DESTINATION ./setup
)
install(
    DIRECTORY dal
    DESTINATION ./setup
)
install(
    DIRECTORY textmode
    DESTINATION ./setup
)

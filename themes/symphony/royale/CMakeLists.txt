cmake_minimum_required(VERSION 3.12)

project(
    wintc-theme-royale
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion Media Center style (Royale) theme."
    LANGUAGES C
)

set(PROJECT_ANYARCH    true)
set(PROJECT_FREESTATUS false)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(../../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../../../packaging/cmake-inc/themes/CMakeLists.txt)

# Use standard theme build process
#
include(${THEMES_BUILD_COMMON_CMAKE_DIR}/CMakeLists.txt)

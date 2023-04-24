cmake_minimum_required(VERSION 3.0)

project(
    wintc-theme-native
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion Windows XP style (Blue) theme."
)

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(../../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../../packaging/cmake-inc/themes/CMakeLists.txt)

# Use standard theme build process
#
include(${THEMES_BUILD_COMMON_CMAKE_DIR}/CMakeLists.txt)

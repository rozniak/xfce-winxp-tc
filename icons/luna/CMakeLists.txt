cmake_minimum_required(VERSION 3.12)

project(
    wintc-icon-theme-luna
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion Windows XP icon theme."
    LANGUAGES C
)

set(PROJECT_ANYARCH    true)
set(PROJECT_FREESTATUS false)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

set(ICON_THEME_SHORTNAME luna)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/icons/CMakeLists.txt)
include(../../packaging/cmake-inc/mappings/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

wintc_add_mappings_target(icons)

# Configure postinstall script
#
set(PROJECT_POSTINSTALL_SCRIPT ${CMAKE_BINARY_DIR}/postinstall.sh)

configure_file(postinstall.sh ${PROJECT_POSTINSTALL_SCRIPT} @ONLY)

# Installation
#
wintc_configure_and_install_packaging()

install(DIRECTORY ${PROJECT_ROOT}/res         DESTINATION ${ICON_PROJECT_INSTALL_PATH})
install(DIRECTORY ${XDG_BLDMAPS_OUTPUT_DIR}/  DESTINATION ${ICON_PROJECT_INSTALL_PATH})
install(FILES     ${PROJECT_ROOT}/index.theme DESTINATION ${ICON_PROJECT_INSTALL_PATH})

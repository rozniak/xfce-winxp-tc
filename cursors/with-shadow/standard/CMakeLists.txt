cmake_minimum_required(VERSION 3.12)

project(
    wintc-cursor-theme-standard-with-shadow
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion Windows XP standard cursor theme (with shadows)."
    LANGUAGES C
)

set(PROJECT_ANYARCH    true)
set(PROJECT_FREESTATUS false)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

set(CURSOR_THEME_SHORTNAME standard-with-shadow)

include(../../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../../packaging/cmake-inc/cursors/CMakeLists.txt)
include(../../../packaging/cmake-inc/mappings/CMakeLists.txt)
include(../../../packaging/cmake-inc/packaging/CMakeLists.txt)

wintc_add_gen_xcursors_target()
wintc_add_mappings_target(cursors)

# Installation
#
wintc_configure_and_install_packaging()

install(DIRECTORY ${CURSOR_COMPCURS_OUTPUT_DIR}/ DESTINATION ${CURSOR_PROJECT_INSTALL_PATH}/cursors/res)
install(DIRECTORY ${XDG_BLDMAPS_OUTPUT_DIR}/     DESTINATION ${CURSOR_PROJECT_INSTALL_PATH}/cursors)
install(FILES     ${PROJECT_ROOT}/index.theme    DESTINATION ${CURSOR_PROJECT_INSTALL_PATH})

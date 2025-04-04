cmake_minimum_required(VERSION 3.12)

project(
    wintc-sound-theme-xp
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion Windows XP sound theme."
    LANGUAGES C
)

set(PROJECT_ANYARCH    true)
set(PROJECT_FREESTATUS false)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

set(SOUND_THEME_NAME "Windows XP Default")

include(../packaging/cmake-inc/common/CMakeLists.txt)
include(../packaging/cmake-inc/mappings/CMakeLists.txt)
include(../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../packaging/cmake-inc/sounds/CMakeLists.txt)

wintc_add_mappings_target(sounds)

# Install stuff
#
install(DIRECTORY ${PROJECT_ROOT}/res         DESTINATION ${SOUND_PROJECT_INSTALL_PATH}/stereo)
install(DIRECTORY ${XDG_BLDMAPS_OUTPUT_DIR}/  DESTINATION ${SOUND_PROJECT_INSTALL_PATH}/stereo)
install(FILES     ${PROJECT_ROOT}/index.theme DESTINATION ${SOUND_PROJECT_INSTALL_PATH})

wintc_configure_and_install_packaging()

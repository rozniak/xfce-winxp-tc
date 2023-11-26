cmake_minimum_required(VERSION 3.5)

project(
    wintc-build-tag
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion build tag."
    LANGUAGES C
)

set(PROJECT_ANYARCH    true)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

# Add target
#
set(GENTAG_TOOL     ${REPO_ROOT}/tools/bldutils/gentag/gentag.sh)
set(TAG_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bldtag)

add_custom_command(
    OUTPUT ${TAG_OUTPUT_PATH}
    WORKING_DIRECTORY ${PROJECT_ROOT}
    COMMAND bash ${GENTAG_TOOL} > ${TAG_OUTPUT_PATH}
    VERBATIM
)

add_custom_target(
    gen-buildtag ALL
    DEPENDS
        ${TAG_OUTPUT_PATH}
)

# Installation
#
wintc_configure_and_install_packaging()

install(FILES ${TAG_OUTPUT_PATH} DESTINATION ${WINTC_ASSETS_INSTALL_DIR})

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

# Generate list of library names
#
execute_process(
    COMMAND /usr/bin/bash -c "find . -mindepth 1 -maxdepth 1 -type d | cut -d'/' -f2 | tr '\n' ','"
    WORKING_DIRECTORY ${REPO_ROOT}/shared
    OUTPUT_VARIABLE WSETUP_LIBLIST
)

configure_file(complist.ini.in ${PROJECT_ROOT}/complist.ini @ONLY)

# Installation
#
install(
    PROGRAMS setup.sh
    DESTINATION .
)
install(
    FILES complist.ini
    DESTINATION ./setup
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

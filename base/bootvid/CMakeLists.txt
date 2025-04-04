cmake_minimum_required(VERSION 3.12)

project(
    wintc-bootvid
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion boot screen splash."
    LANGUAGES C
)

set(PROJECT_ANYARCH    true)
set(PROJECT_FREESTATUS false)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

set(PLYMOUTH_THEME_ROOT share/plymouth/themes/bootvid)

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

# Plymouth isn't available on FreeBSD, so build a simple splash on there and
# Plymouth for everyone else
#
if (${WINTC_PKGMGR} STREQUAL "bsdpkg")
    include(bsd-splash.cmake)
else()
    include(plymouth.cmake)
endif()

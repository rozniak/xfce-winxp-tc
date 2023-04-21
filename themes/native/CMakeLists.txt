cmake_minimum_required(VERSION 3.0)

project(
    wintc-theme-native
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion Windows Classic theme."
)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/themes/CMakeLists.txt)

# Import colour scheme
#
source_vars(scheme)

# GTK2 Theme Generation
#
configure_file(${THEMES_BUILD_COMMON_GTK2_DIR}/scheme.rc.in scheme.rc @ONLY)

install(FILES ${THEMES_BUILD_GTK2_FILES}    DESTINATION share/themes/native/gtk-2.0)
install(FILES ${CMAKE_BINARY_DIR}/scheme.rc DESTINATION share/themes/native/gtk-2.0)

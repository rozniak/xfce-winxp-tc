cmake_minimum_required(VERSION 3.12)

# Check stuff we need is defined
#
if (NOT DEFINED ICON_THEME_SHORTNAME)
    message(FATAL_ERROR "You must define ICON_THEME_SHORTNAME")
endif ()

# Define constants for build paths
#
set(ICON_PROJECT_INSTALL_PATH share/icons/${ICON_THEME_SHORTNAME})

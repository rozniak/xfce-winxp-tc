cmake_minimum_required(VERSION 3.12)

# Check stuff we need is defined
#
if (NOT DEFINED SOUND_THEME_NAME)
    message(FATAL_ERROR "You must define SOUND_THEME_NAME")
endif ()

# Define constants
#
set(SOUND_PROJECT_INSTALL_PATH share/sounds/${SOUND_THEME_NAME})

cmake_minimum_required(VERSION 3.12)

# Installation
#
wintc_configure_and_install_packaging()

install(
    FILES res/splshbsd.bmp
    DESTINATION ${WINTC_ASSETS_INSTALL_DIR}/splash
    RENAME bootvid.bmp
)

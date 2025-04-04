cmake_minimum_required(VERSION 3.12)

set(PLYMOUTH_THEME_ROOT share/plymouth/themes/bootvid)

# Pick sources based on SKU
#
set(PLYMOUTH_THEME_NAME "")

set(PLYMOUTH_IMAGE_CHUNKS "")
set(PLYMOUTH_IMAGE_SKU    "")
set(PLYMOUTH_IMAGE_SPLASH "")

if (${WINTC_SKU} MATCHES "^xpclient-(.+)")
    set(PLYMOUTH_THEME_NAME whistler)
    set(PLYMOUTH_IMAGE_SPLASH splshclx.png)
    set(PLYMOUTH_IMAGE_CHUNKS chunkpro.png)
    set(PLYMOUTH_IMAGE_SKU skunull.png)

    if (${CMAKE_MATCH_1} STREQUAL "per")
        set(PLYMOUTH_IMAGE_CHUNKS chunkper.png)
        set(PLYMOUTH_IMAGE_SKU    skuper.png)
        set(PLYMOUTH_IMAGE_SPLASH splshclt.png)
    elseif (${CMAKE_MATCH_1} STREQUAL "pro")
        if (
            ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "amd64" OR
            ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64"
        )
            set(PLYMOUTH_IMAGE_SKU skux64.png)
        elseif (
            ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ia64"    OR
            ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64" OR
            ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv8"
        )
            set(PLYMOUTH_IMAGE_SPLASH splshcl3.png)
            set(PLYMOUTH_IMAGE_SKU    sku64bit.png)
        else()
            set(PLYMOUTH_IMAGE_SKU    skupro.png)
            set(PLYMOUTH_IMAGE_SPLASH splshclt.png)
        endif()
    elseif (${CMAKE_MATCH_1} STREQUAL "mce")
        set(PLYMOUTH_IMAGE_SKU    skumce.png)
        set(PLYMOUTH_IMAGE_SPLASH splshclt.png)
    elseif (${CMAKE_MATCH_1} STREQUAL "tabletpc")
        set(PLYMOUTH_IMAGE_SKU    skutabletpc.png)
        set(PLYMOUTH_IMAGE_SPLASH splshclt.png)
    elseif (${CMAKE_MATCH_1} STREQUAL "embedded")
        set(PLYMOUTH_IMAGE_SKU    skuemb.png)
        set(PLYMOUTH_IMAGE_SPLASH splshclt.png)
    endif()
elseif (
    ${WINTC_SKU} MATCHES "^dnsrv" OR
    ${WINTC_SKU} STREQUAL "homesrv"
)
    set(PLYMOUTH_THEME_NAME whistler)
    set(PLYMOUTH_IMAGE_CHUNKS chunksrv.png)
    set(PLYMOUTH_IMAGE_SKU    skunull.png)
    set(PLYMOUTH_IMAGE_SPLASH splshsrv.png)

    # Windows Home Server, and Server 2003 CCS have their own boot splashes!
    #
    if (${WINTC_SKU} STREQUAL "dnsrv-ccs")
        set(PLYMOUTH_IMAGE_SPLASH splshccs.png)
    elseif (${WINTC_SKU} STREQUAL "homesrv")
        set(PLYMOUTH_IMAGE_SPLASH splshqhs.png)
    endif()
else()
    message(
        FATAL_ERROR
        "No splash defined for SKU: ${WINTC_SKU}"
    )
endif()

# Installation
#
wintc_configure_and_install_packaging()

configure_file(src/${PLYMOUTH_THEME_NAME}.plymouth bootvid.plymouth @ONLY)

install(
    FILES ${CMAKE_BINARY_DIR}/bootvid.plymouth
    DESTINATION ${PLYMOUTH_THEME_ROOT}
    RENAME bootvid.plymouth
)
install(
    FILES src/${PLYMOUTH_THEME_NAME}.script
    DESTINATION ${PLYMOUTH_THEME_ROOT}
    RENAME bootvid.script
)
install(
    FILES res/${PLYMOUTH_IMAGE_CHUNKS}
    DESTINATION ${PLYMOUTH_THEME_ROOT}
    RENAME chunks.png
)
install(
    FILES res/${PLYMOUTH_IMAGE_SKU}
    DESTINATION ${PLYMOUTH_THEME_ROOT}
    RENAME sku.png
)
install(
    FILES res/${PLYMOUTH_IMAGE_SPLASH}
    DESTINATION ${PLYMOUTH_THEME_ROOT}
    RENAME splash.png
)

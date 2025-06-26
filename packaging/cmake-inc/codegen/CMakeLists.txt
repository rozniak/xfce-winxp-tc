cmake_minimum_required(VERSION 3.12)

# Define function for gdbus-codegen
#
set(WINTC_DBUS_DEFAULT_IF_PREFIX uk.oddmatics.wintc.)
set(WINTC_DBUS_DEFAULT_C_PREFIX  ZWin)

function(wintc_gdbus_codegen XML_FILE OUT_FILE_NOEXT IF_PREFIX C_PREFIX CP_PUBLIC)
    find_program(GDBUS_CODEGEN gdbus-codegen REQUIRED)

    add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_SOURCE_DIR}/src/${OUT_FILE_NOEXT}.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/${OUT_FILE_NOEXT}.h
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
        COMMAND ${GDBUS_CODEGEN}
        ARGS
            --interface-prefix ${IF_PREFIX}
            --generate-c-code ${OUT_FILE_NOEXT}
            --c-namespace ${C_PREFIX}
            ${XML_FILE}
        VERBATIM
        DEPENDS
            src/${XML_FILE}
    )

    add_custom_target(
        dbus-codegen
        DEPENDS
            src/${OUT_FILE_NOEXT}.c
            src/${OUT_FILE_NOEXT}.h
    )

    # If the header is needed in public/ for libraries then copy it
    #
    if (CP_PUBLIC)
        find_program(CP cp REQUIRED)

        add_custom_command(
            OUTPUT
                ${CMAKE_CURRENT_SOURCE_DIR}/public/${OUT_FILE_NOEXT}.h
            COMMAND ${CP}
            ARGS
                ${CMAKE_CURRENT_SOURCE_DIR}/src/${OUT_FILE_NOEXT}.h
                ${CMAKE_CURRENT_SOURCE_DIR}/public/${OUT_FILE_NOEXT}.h
            VERBATIM
            DEPENDS
                src/${OUT_FILE_NOEXT}.h
        )

        add_custom_target(
            dbus-codegen-header
            DEPENDS
                public/${OUT_FILE_NOEXT}.h
        )
    endif()
endfunction()

# Define function for glib-genmashal
#
function(wintc_glib_genmarshal)
    find_program(GLIB_GENMARSHAL glib-genmarshal REQUIRED)

    add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_SOURCE_DIR}/src/marshal.h
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
        COMMAND ${GLIB_GENMARSHAL}
        ARGS
            --header
            --prefix wintc_cclosure_marshal
            --output marshal.h
            marshals.list
        VERBATIM
        DEPENDS
            src/marshals.list
    )
    add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_SOURCE_DIR}/src/marshal.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
        COMMAND ${GLIB_GENMARSHAL}
        ARGS
            --body
            --prefix wintc_cclosure_marshal
            --output marshal.c
            marshals.list
        VERBATIM
        DEPENDS
            src/marshals.list
    )

    add_custom_target(
        marshal-codegen
        DEPENDS
            src/marshal.c
            src/marshal.h
    )
endfunction()

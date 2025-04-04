cmake_minimum_required(VERSION 3.12)

# Define function for gdbus-codegen
#
function(wintc_gdbus_codegen XML_FILE OUT_FILE_NOEXT CP_PUBLIC)
    find_program(GDBUS_CODEGEN gdbus-codegen REQUIRED)

    add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_SOURCE_DIR}/src/${OUT_FILE_NOEXT}.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/${OUT_FILE_NOEXT}.h
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
        COMMAND ${GDBUS_CODEGEN}
        ARGS
            --interface-prefix uk.oddmatics.wintc.
            --generate-c-code ${OUT_FILE_NOEXT}
            --c-namespace ZWin
            --c-generate-object-manager
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

cmake_minimum_required(VERSION 3.12)

# Define build for GResources
#
function(wintc_compile_resources)
    set(GRESOURCE_DEPENDENCIES)

    find_program(GLIB_COMPILE_RESOURCES glib-compile-resources REQUIRED)

    execute_process(
        COMMAND ${GLIB_COMPILE_RESOURCES} --generate-dependencies resources.xml
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/res
        OUTPUT_VARIABLE GRESOURCE_DEPENDENCIES
    )

    string(REPLACE "\n" ";" GRESOURCE_DEPENDENCIES ${GRESOURCE_DEPENDENCIES})
    list(POP_BACK GRESOURCE_DEPENDENCIES)
    list(TRANSFORM GRESOURCE_DEPENDENCIES PREPEND "src/res/")

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/resources.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/res
        COMMAND ${GLIB_COMPILE_RESOURCES}
        ARGS
            --target=../resources.c
            --generate
            resources.xml
        VERBATIM
        DEPENDS
            ${GRESOURCE_DEPENDENCIES}
    )

    add_custom_target(
        build-gresources
        DEPENDS
            src/resources.c
    )
endfunction()

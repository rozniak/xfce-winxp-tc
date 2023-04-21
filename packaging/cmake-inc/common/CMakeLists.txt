cmake_minimum_required(VERSION 3.0)

set(REPO_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../..)

function(source_vars SOURCE_PATH)
    file(STRINGS ${SOURCE_PATH} sourceLines)

    foreach(line ${sourceLines})
        if (${line} STREQUAL "")
            continue()
        endif()

        string(REGEX MATCH "^[^=]+" varIdentifier ${line})
        string(REGEX MATCH "[^=]+$" varValue      ${line})

        set(${varIdentifier} ${varValue} PARENT_SCOPE)
    endforeach()
endfunction()

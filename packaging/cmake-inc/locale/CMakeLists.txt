cmake_minimum_required(VERSION 3.12)

# Define supported languages - these are languages supported by Windows XP MUIs
#
set(
    WINTC_SUPPORTED_LANGUAGES
    ar_SA
    bg_BG
    cs_CZ
    da_DK
    de_DE
    el_GR
    es_ES
    et_EE
    fi_FI
    fr_FR
    he_IL
    hr_HR
    hu_HU
    it_IT
    ja_JP
    ko_KR
    lt_LT
    lv_LV
    nb_NO
    nl_NL
    pl_PL
    pt_BR
    pt_PT
    ro_RO
    ru_RU
    sk_SK
    sl_SI
    sv_SE
    th_TH
    tr_TR
    zh_CN
    zh_TW
)

# Define build for config.h
#
set(WINTC_CONFIG_H_IN_PATH ${CMAKE_CURRENT_LIST_DIR}/config.h.in)

function(wintc_create_config_h)
    configure_file(${WINTC_CONFIG_H_IN_PATH} ${PROJECT_ROOT}/src/config.h @ONLY)
endfunction()

# Define build for meta.h
#
set(WINTC_META_H_IN_PATH ${CMAKE_CURRENT_LIST_DIR}/meta.h.in)

function(wintc_create_meta_h)
    configure_file(${WINTC_META_H_IN_PATH} ${PROJECT_ROOT}/src/meta.h @ONLY)
endfunction()

# Define build for languages
#
function(wintc_compile_and_install_pofiles)
    # Call msgfmt
    #
    # Add a command to build each po->mo one by one, do NOT pass all the inputs
    # to one msgfmt command like I did at first because it treats ALL inputs as
    # making ONE output (as a result you get loads of 'duplicate message
    # definition' errors and end up in a state of utter confusion)
    #
    find_program(MSGFMT msgfmt)

    foreach (LOCALE_STR ${WINTC_SUPPORTED_LANGUAGES})
        set(NEXT_POFILE ${PROJECT_ROOT}/po/${LOCALE_STR}.po)
        set(NEXT_MOFILE ${CMAKE_BINARY_DIR}/locale/${LOCALE_STR}/${PROJECT_NAME}.mo)

        list(
            APPEND
            OUTPUT_MOFILES
            ${NEXT_MOFILE}
        )

        add_custom_command(
            OUTPUT ${NEXT_MOFILE}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND mkdir
            ARGS
                -p
                ${CMAKE_BINARY_DIR}/locale/${LOCALE_STR}
            COMMAND ${MSGFMT}
            ARGS
                ${NEXT_POFILE}
                -o
                ${NEXT_MOFILE}
            VERBATIM
            DEPENDS
                ${NEXT_POFILE}
        )

        install(DIRECTORY ${CMAKE_BINARY_DIR}/locale/${LOCALE_STR}/ DESTINATION share/locale/${LOCALE_STR}/LC_MESSAGES)
    endforeach()

    add_custom_target(
        build-mofiles ALL
        DEPENDS
            ${OUTPUT_MOFILES}
    )
endfunction()

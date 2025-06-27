cmake_minimum_required(VERSION 3.12)

# Import colour scheme and theme metadata
#
file(STRINGS ${PROJECT_ROOT}/scheme scheme_lines)
file(STRINGS ${PROJECT_ROOT}/theme  theme_lines)

wintc_source_vars("${scheme_lines}")
wintc_source_vars("${theme_lines}")

# Import dir constants
#
wintc_define_theme_dirs()

# GTK2 Theme Generation
#
configure_file(${THEMES_BUILD_COMMON_GTK2_DIR}/scheme.rc.in scheme.rc @ONLY)

install(FILES ${THEMES_BUILD_GTK2_FILES}    DESTINATION ${THEME_PROJECT_INSTALL_PATH}/gtk-2.0)
install(FILES ${CMAKE_BINARY_DIR}/scheme.rc DESTINATION ${THEME_PROJECT_INSTALL_PATH}/gtk-2.0)

# GTK3 Theme Generation
#
configure_file(${THEMES_BUILD_COMMON_GTK3_DIR}/scheme.scss.in scheme.scss @ONLY)
configure_file(gtk-3.0/main.scss.in                           main.scss   @ONLY)

if (NOT (${SASS} STREQUAL SASS-NOTFOUND))
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/gtk.css
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND ${SASS}
        ARGS
            ${CMAKE_BINARY_DIR}/main.scss
            ${CMAKE_BINARY_DIR}/gtk.css
            --sourcemap=none
        VERBATIM
        MAIN_DEPENDENCY ${CMAKE_BINARY_DIR}/main.scss
        DEPENDS
            ${CMAKE_BINARY_DIR}/scheme.scss
    )
else()
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/gtk.css
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND ${SASSC}
        ARGS
            ${CMAKE_BINARY_DIR}/main.scss
            ${CMAKE_BINARY_DIR}/gtk.css
        VERBATIM
        MAIN_DEPENDENCY ${CMAKE_BINARY_DIR}/main.scss
        DEPENDS
            ${CMAKE_BINARY_DIR}/scheme.scss
    )
endif()

add_custom_target(
    theme-gtk3 ALL
    DEPENDS
        ${CMAKE_BINARY_DIR}/gtk.css
)

install(FILES ${CMAKE_BINARY_DIR}/gtk.css DESTINATION ${THEME_PROJECT_INSTALL_PATH}/gtk-3.0)

# XFWM4 Theme Generation
#
install(DIRECTORY ${PROJECT_ROOT}/xfwm4 DESTINATION ${THEME_PROJECT_INSTALL_PATH})

# Resource generation
#
install(DIRECTORY ${PROJECT_ROOT}/Resources    DESTINATION ${THEME_PROJECT_INSTALL_PATH})
install(DIRECTORY ${CMAKE_BINARY_DIR}/composed DESTINATION ${THEME_PROJECT_INSTALL_PATH}/Resources)

wintc_configure_and_install_packaging()

cmake_minimum_required(VERSION 3.0)

project(
    wintc-taskband
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion taskband application"
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(
    PROJECT_DEPENDENCIES
    libgarcon-1-0
    libgarcon-gtk3-1-0
    libgdk-pixbuf-2.0-0
    libglib2.0-0
    libgtk-3-0
    libwintc-comgtk
    libwintc-exec
    libwintc-shllang
    xdg-utils
)

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

set(WINTC_NO_PEDANTIC_COMPILE true) # Necessary because we use dlsym()

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/locale/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../../packaging/cmake-inc/resources/CMakeLists.txt)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GARCON          REQUIRED garcon-1)
pkg_check_modules(GARCON_GTK3     REQUIRED garcon-gtk3-1)
pkg_check_modules(GDK             REQUIRED gdk-3.0)
pkg_check_modules(GDK_PIXBUF      REQUIRED gdk-pixbuf-2.0)
pkg_check_modules(GLIB            REQUIRED glib-2.0)
pkg_check_modules(GTK3            REQUIRED gtk+-3.0)
pkg_check_modules(WINTC_COMGTK    REQUIRED wintc-comgtk)
pkg_check_modules(WINTC_EXEC      REQUIRED wintc-exec)
pkg_check_modules(WINTC_SHLLANG   REQUIRED wintc-shllang)

wintc_compile_resources()
wintc_create_meta_h()

add_executable(
    wintc-taskband
    src/application.c
    src/application.h
    src/dispproto.c
    src/dispproto.h
    src/dispproto-wayland.c
    src/dispproto-wayland.h
    src/dispproto-x11.c
    src/dispproto-x11.h
    src/dispproto-wndmgmt-wnck.c
    src/dispproto-wndmgmt-wnck.h
    src/dispproto-wndmgmt-xfw.c
    src/dispproto-wndmgmt-xfw.h
    src/main.c
    src/meta.h
    src/resources.c
    src/window.c
    src/window.h
    src/start/action.c
    src/start/action.h
    src/start/placeslist.c
    src/start/placeslist.h
    src/start/programslist.c
    src/start/programslist.h
    src/start/startbutton.c
    src/start/startbutton.h
    src/start/startmenu.c
    src/start/startmenu.h
    src/start/startmenuitem.c
    src/start/startmenuitem.h
    src/start/util.c
    src/start/util.h
    src/systray/clock.c
    src/systray/clock.h
    src/systray/notifarea.c
    src/systray/notifarea.h
    src/taskbuttons/taskbuttonbar.c
    src/taskbuttons/taskbuttonbar.h
    src/taskbuttons/windowmonitor.c
    src/taskbuttons/windowmonitor.h
)

target_compile_options(
    wintc-taskband
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-taskband
    SYSTEM
    PRIVATE ${GARCON_INCLUDE_DIRS}
    PRIVATE ${GARCON_GTK3_INCLUDE_DIRS}
    PRIVATE ${GDK_INCLUDE_DIRS}
    PRIVATE ${GDK_PIXBUF_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_EXEC_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHLLANG_INCLUDE_DIRS}
)

target_link_directories(
    wintc-taskband
    PRIVATE ${GARCON_LIBRARY_DIRS}
    PRIVATE ${GARCON_GTK3_LIBRARY_DIRS}
    PRIVATE ${GDK_LIBRARY_DIRS}
    PRIVATE ${GDK_PIXBUF_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_EXEC_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHLLANG_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-taskband
    PRIVATE ${CMAKE_DL_LIBS}
    PRIVATE ${GARCON_LIBRARIES}
    PRIVATE ${GARCON_GTK3_LIBRARIES}
    PRIVATE ${GDK_LIBRARIES}
    PRIVATE ${GDK_PIXBUF_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_EXEC_LIBRARIES}
    PRIVATE ${WINTC_SHLLANG_LIBRARIES}
)

add_dependencies(
    wintc-taskband
    build-gresources
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_compile_and_install_pofiles()

install(
    DIRECTORY res/
    DESTINATION ${WINTC_ASSETS_INSTALL_DIR}/shell-res
)

install(
    TARGETS wintc-taskband
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

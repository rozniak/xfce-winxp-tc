cmake_minimum_required(VERSION 3.14)

project(
    wintc-taskband
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion taskband application"
    LANGUAGES C
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

set(WINTC_NO_PEDANTIC_COMPILE true) # Necessary due to NetworkManager headers

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/codegen/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/locale/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../../packaging/cmake-inc/resources/CMakeLists.txt)

wintc_resolve_library(libcanberra       CANBERRA       PRIVATE)
wintc_resolve_library(libcanberra-gtk3  CANBERRA_GTK3  PRIVATE)
wintc_resolve_library(dbusmenu-gtk3-0.4 DBUSMENU_GTK3  PRIVATE)
wintc_resolve_library(garcon-1          GARCON         PRIVATE)
wintc_resolve_library(garcon-gtk3-1     GARCON_GTK3    PRIVATE)
wintc_resolve_library(gdk-pixbuf-2.0    GDK_PIXBUF     PRIVATE)
wintc_resolve_library(glib-2.0          GLIB           PRIVATE)
wintc_resolve_library(gtk+-3.0          GTK3           PRIVATE)
wintc_resolve_library(upower-glib       UPOWER_GLIB    PRIVATE)
wintc_resolve_library(wintc-comctl      WINTC_COMCTL   PRIVATE)
wintc_resolve_library(wintc-comgtk      WINTC_COMGTK   PRIVATE)
wintc_resolve_library(wintc-exec        WINTC_EXEC     PRIVATE)
wintc_resolve_library(wintc-shcommon    WINTC_SHCOMMON PRIVATE)
wintc_resolve_library(wintc-shelldpa    WINTC_SHELLDPA PRIVATE)
wintc_resolve_library(wintc-shellext    WINTC_SHELLEXT PRIVATE)
wintc_resolve_library(wintc-shlang      WINTC_SHLANG   PRIVATE)
wintc_resolve_library(wintc-sndapi      WINTC_SNDAPI   PRIVATE)

wintc_compile_resources()
wintc_create_meta_h()

wintc_gdbus_codegen(
    sni-dbus.xml
    sni-dbus
    org.kde.
    ZWinKde
    false
)
wintc_gdbus_codegen(
    snw-dbus.xml
    snw-dbus
    org.kde.
    ZWinKde
    false
)

add_executable(
    wintc-taskband
    src/application.c
    src/application.h
    src/intapi.h
    src/main.c
    src/meta.h
    src/net/client.h
    src/net/device.h
    src/net/dialogs.c
    src/net/dialogs.h
    src/net/netwnd.c
    src/net/netwnd.h
    src/net/statwnd.c
    src/net/statwnd.h
    src/net/util.c
    src/net/util.h
    src/resources.c
    src/sni-dbus.c
    src/sni-dbus.h
    src/snw-dbus.c
    src/snw-dbus.h
    src/window.c
    src/window.h
    src/qaccess/toolbar.c
    src/qaccess/toolbar.h
    src/start/menumod.c
    src/start/menumod.h
    src/start/mfu.c
    src/start/mfu.h
    src/start/personal.c
    src/start/personal.h
    src/start/progmenu.c
    src/start/progmenu.h
    src/start/shared.h
    src/start/toolbar.c
    src/start/toolbar.h
    src/start/util.c
    src/start/util.h
    src/systray/clock.c
    src/systray/clock.h
    src/systray/icon.c
    src/systray/icon.h
    src/systray/network.c
    src/systray/network.h
    src/systray/notifarea.c
    src/systray/notifarea.h
    src/systray/power.c
    src/systray/power.h
    src/systray/sni.c
    src/systray/sni.h
    src/systray/toolbar.c
    src/systray/toolbar.h
    src/systray/volume.c
    src/systray/volume.h
    src/taskbuttons/taskbuttonbar.c
    src/taskbuttons/taskbuttonbar.h
    src/taskbuttons/toolbar.c
    src/taskbuttons/toolbar.h
    src/taskbuttons/windowmonitor.c
    src/taskbuttons/windowmonitor.h
)

# The network adapters are driven by NetworkManager, except on FreeBSD which
# doesn't have it - there they are driven through the kernel and wpa_supplicant
# directly (see src/net/client.h), with devd(8) bringing up new adapters and a
# pkexec(1) helper for changing them (see bsd/)
#
if (${WINTC_PKGMGR} STREQUAL "bsdpkg")
    target_sources(
        wintc-taskband
        PRIVATE src/net/bsd/backend.h
        PRIVATE src/net/bsd/client.c
        PRIVATE src/net/bsd/device.c
        PRIVATE src/net/bsd/wpactrl.c
        PRIVATE src/net/bsd/wpactrl.h
    )

    set(WINTC_NETCTL_PATH ${CMAKE_INSTALL_FULL_LIBEXECDIR}/wintc/wintc-netctl)

    target_compile_definitions(
        wintc-taskband
        PRIVATE WINTC_NETCTL_PATH="${WINTC_NETCTL_PATH}"
    )

    configure_file(
        bsd/uk.co.oddmatics.wintc.taskband.policy.in
        uk.co.oddmatics.wintc.taskband.policy
        @ONLY
    )
    configure_file(
        bsd/wintc-net.conf.in
        wintc-net.conf
        @ONLY
    )

    install(
        PROGRAMS
            bsd/wintc-net-devd
            bsd/wintc-netctl
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/wintc
    )
    install(
        FILES ${CMAKE_BINARY_DIR}/uk.co.oddmatics.wintc.taskband.policy
        DESTINATION share/polkit-1/actions
    )
    install(
        FILES ${CMAKE_BINARY_DIR}/wintc-net.conf
        DESTINATION etc/devd
    )
else()
    wintc_resolve_library(libnm NM PRIVATE)

    target_sources(
        wintc-taskband
        PRIVATE src/net/nm/backend.h
        PRIVATE src/net/nm/client.c
        PRIVATE src/net/nm/device.c
    )
endif()

target_compile_options(
    wintc-taskband
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-taskband
    SYSTEM
    BEFORE
    PRIVATE ${CANBERRA_INCLUDE_DIRS}
    PRIVATE ${CANBERRA_GTK3_INCLUDE_DIRS}
    PRIVATE ${DBUSMENU_GTK3_INCLUDE_DIRS}
    PRIVATE ${GARCON_INCLUDE_DIRS}
    PRIVATE ${GARCON_GTK3_INCLUDE_DIRS}
    PRIVATE ${GDK_PIXBUF_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${NM_INCLUDE_DIRS}
    PRIVATE ${UPOWER_GLIB_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMCTL_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_EXEC_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHCOMMON_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELLDPA_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELLEXT_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHLANG_INCLUDE_DIRS}
    PRIVATE ${WINTC_SNDAPI_INCLUDE_DIRS}
)

target_link_directories(
    wintc-taskband
    PRIVATE ${CANBERRA_LIBRARY_DIRS}
    PRIVATE ${CANBERRA_GTK3_LIBRARY_DIRS}
    PRIVATE ${DBUSMENU_GTK3_LIBRARY_DIRS}
    PRIVATE ${GARCON_LIBRARY_DIRS}
    PRIVATE ${GARCON_GTK3_LIBRARY_DIRS}
    PRIVATE ${GDK_PIXBUF_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${NM_LIBRARY_DIRS}
    PRIVATE ${UPOWER_GLIB_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMCTL_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_EXEC_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHCOMMON_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELLDPA_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELLEXT_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHLANG_LIBRARY_DIRS}
    PRIVATE ${WINTC_SNDAPI_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-taskband
    PRIVATE m
    PRIVATE ${CANBERRA_LIBRARIES}
    PRIVATE ${CANBERRA_GTK3_LIBRARIES}
    PRIVATE ${DBUSMENU_GTK3_LIBRARIES}
    PRIVATE ${GARCON_LIBRARIES}
    PRIVATE ${GARCON_GTK3_LIBRARIES}
    PRIVATE ${GDK_PIXBUF_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${NM_LIBRARIES}
    PRIVATE ${UPOWER_GLIB_LIBRARIES}
    PRIVATE ${WINTC_COMCTL_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_EXEC_LIBRARIES}
    PRIVATE ${WINTC_SHCOMMON_LIBRARIES}
    PRIVATE ${WINTC_SHELLDPA_LIBRARIES}
    PRIVATE ${WINTC_SHELLEXT_LIBRARIES}
    PRIVATE ${WINTC_SHLANG_LIBRARIES}
    PRIVATE ${WINTC_SNDAPI_LIBRARIES}
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

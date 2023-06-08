#include <dlfcn.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>

#include "dispproto.h"
#include "dispproto-wayland.h"
#include "dispproto-x11.h"
#include "dispproto-wndmgmt-wnck.h"
#include "dispproto-wndmgmt-xfw.h"

//
// STATIC DATA
//
static TaskbandDisplayProtocol s_dispproto;

//
// RESOLVED FUNCS
//
static GType (*p_gdk_x11_display_get_type) (void) = NULL;
static GType (*p_gdk_wayland_display_get_type) (void) = NULL;

void (*anchor_taskband_to_bottom)(
    GtkWindow* taskband
) = NULL;

WndMgmtWindow* (*wndmgmt_screen_get_active_window) (
    WndMgmtScreen* screen
) = NULL;
WndMgmtScreen* (*wndmgmt_screen_get_default) (void) = NULL;

GdkPixbuf* (*wndmgmt_window_get_mini_icon) (
    WndMgmtWindow* window
) = NULL;
gchar* (*wndmgmt_window_get_name) (
    WndMgmtWindow* window
) = NULL;
gboolean (*wndmgmt_window_is_skip_tasklist) (
    WndMgmtWindow* window
) = NULL;
void (*wndmgmt_window_minimize) (
    WndMgmtWindow* window
) = NULL;
void (*wndmgmt_window_unminimize) (
    WndMgmtWindow* window
) = NULL;

//
// PUBLIC FUNCTIONS
//
TaskbandDisplayProtocol get_display_protocol_in_use(void)
{
    return s_dispproto;
}

gboolean init_display_protocol_apis(void)
{
    void* dl_gdk = dlopen("libgdk-3.so", RTLD_LAZY | RTLD_LOCAL);

    if (dl_gdk == NULL)
    {
        g_critical("%s", "Failed to open libgdk for symbols.");
        return FALSE;
    }

    // Resolve GObject type codes for the displays
    //
    p_gdk_x11_display_get_type     = dlsym(dl_gdk, "gdk_x11_display_get_type");
    p_gdk_wayland_display_get_type = dlsym(dl_gdk, "gdk_wayland_display_get_type");

    if (
        p_gdk_x11_display_get_type     == NULL &&
        p_gdk_wayland_display_get_type == NULL
    )
    {
        g_critical("%s", "Unable to resolve X11 nor Wayland symbols in GDK.");
        return FALSE;
    }

    // What is our display?
    //
    GdkDisplay* display = gdk_display_get_default();

    if (
        p_gdk_x11_display_get_type != NULL &&
        (G_TYPE_CHECK_INSTANCE_TYPE((display), p_gdk_x11_display_get_type()))
    )
    {
        s_dispproto = DISPPROTO_X11;

        if (!init_x11_protocol_impl())
        {
            g_critical("%s", "Failed to initialize X11 implementation.");
            return FALSE;
        }
    }
    else if (
        p_gdk_wayland_display_get_type != NULL &&
        (G_TYPE_CHECK_INSTANCE_TYPE((display), p_gdk_wayland_display_get_type()))
    )
    {
        s_dispproto = DISPPROTO_WAYLAND;

        if (!init_wayland_protocol_impl())
        {
            g_critical("%s", "Failed to initialize Wayland implementation.");
            return FALSE;
        }
    }
    else
    {
        g_critical("%s", "Can't determine display type, not X11 or Wayland?");
        return FALSE;
    }

    // Window management stuff, we prioritise loading xfce4windowing because it
    // is the future - failing that we try WNCK, but only for X cos It Don't
    // Work On Wayland! (TM)
    //
    if (!init_wndmgmt_xfw_impl())
    {
        if (get_display_protocol_in_use() == DISPPROTO_WAYLAND)
        {
            // It's over for Wayland!
            //
            g_critical(
                "%s",
                "libxfce4windowing not available, cannot continue on Wayland."
            );
            return FALSE;
        }

        if (!init_wndmgmt_wnck_impl())
        {
            // No WNCK! Computer over! Disaster = very yes! 
            //
            g_critical(
                "%s",
                "libwnck not available, cannot continue."
            );
            return FALSE;
        }
    }

    return TRUE;
}

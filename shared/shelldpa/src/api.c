#include <dlfcn.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/api.h"
#include "impl-wayland.h"
#include "impl-x11.h"
#include "impl-wndmgmt-wnck.h"
#include "impl-wndmgmt-xfw.h"

//
// STATIC DATA
//
static WinTCDisplayProtocol s_dispproto;

//
// RESOLVED FUNCS
//
static GType (*p_gdk_x11_display_get_type) (void)     = NULL;
static GType (*p_gdk_wayland_display_get_type) (void) = NULL;

void (*wintc_anchor_taskband_to_bottom) (
    GtkWindow* taskband
) = NULL;

WinTCWndMgmtWindow* (*wintc_wndmgmt_screen_get_active_window) (
    WinTCWndMgmtScreen* screen
) = NULL;
WinTCWndMgmtScreen* (*wintc_wndmgmt_screen_get_default) (void) = NULL;

void (*wintc_wndmgmt_shutdown) (void) = NULL;

void (*wintc_wndmgmt_window_close) (
    WinTCWndMgmtWindow* window,
    guint64             timestamp
) = NULL;
GdkPixbuf* (*wintc_wndmgmt_window_get_mini_icon) (
    WinTCWndMgmtWindow* window
) = NULL;
gchar* (*wintc_wndmgmt_window_get_name) (
    WinTCWndMgmtWindow* window
) = NULL;
gboolean (*wintc_wndmgmt_window_is_maximized) (
    WinTCWndMgmtWindow* window
) = NULL;
gboolean (*wintc_wndmgmt_window_is_minimized) (
    WinTCWndMgmtWindow* window
) = NULL;
gboolean (*wintc_wndmgmt_window_is_skip_tasklist) (
    WinTCWndMgmtWindow* window
) = NULL;
void (*wintc_wndmgmt_window_maximize) (
    WinTCWndMgmtWindow* window
) = NULL;
void (*wintc_wndmgmt_window_minimize) (
    WinTCWndMgmtWindow* window
) = NULL;
void (*wintc_wndmgmt_window_unmaximize) (
    WinTCWndMgmtWindow* window
) = NULL;
void (*wintc_wndmgmt_window_unminimize) (
    WinTCWndMgmtWindow* window,
    guint64             timestamp
) = NULL;

//
// FORWARD DECLARATIONS
//
static gboolean on_popup_window_focus_out(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_dpa_create_popup(
    GtkWidget* owner,
    gboolean   enable_composition
)
{
    GtkWidget* popup;

    // On GTK3, GtkPopovers are limited to the bounds of the parent when
    // running under X11 -- this sucks!! So here we do the job of creating
    // an appropriate pop-up widget based on display protocol
    //
    if (wintc_get_display_protocol_in_use() == WINTC_DISPPROTO_X11)
    {
        popup = gtk_window_new(GTK_WINDOW_TOPLEVEL);

        gtk_window_set_type_hint(
            GTK_WINDOW(popup),
            GDK_WINDOW_TYPE_HINT_POPUP_MENU
        );
        gtk_window_set_resizable(
            GTK_WINDOW(popup),
            FALSE
        );
        gtk_window_set_keep_above(
            GTK_WINDOW(popup),
            TRUE
        );
        gtk_window_set_skip_taskbar_hint(
            GTK_WINDOW(popup),
            TRUE
        );
        gtk_window_set_title(
            GTK_WINDOW(popup),
            "Popup"
        );
        gtk_widget_set_events(
            popup,
            GDK_FOCUS_CHANGE_MASK
        );

        // If the application wants composition (eg. for shadows) we fake out
        // CSD here
        //
        if (enable_composition)
        {
            GtkWidget* fake_titlebar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

            // HACK: Apply CSS to force the fake titlebar to not show up as
            //       Adwaita has a min-height on titlebar boxes! Blah!
            //
            wintc_widget_add_css(
                fake_titlebar,
                "* { min-height: 0px; }"
            );

            gtk_window_set_titlebar(
                GTK_WINDOW(popup),
                fake_titlebar
            );
        }
        else
        {
            gtk_window_set_decorated(
                GTK_WINDOW(popup),
                FALSE
            );
        }

        // Connect signals
        //
        g_signal_connect(
            popup,
            "focus-out-event",
            G_CALLBACK(on_popup_window_focus_out),
            NULL
        );
    }
    else
    {
        popup = gtk_popover_new(owner);
    }

    return popup;
}

void wintc_dpa_show_popup(
    GtkWidget* popup,
    GtkWidget* owner
)
{
    gint height;
    gint x;
    gint y;

    if (wintc_get_display_protocol_in_use() == WINTC_DISPPROTO_X11)
    {
        // FIXME: Position is UBER BROKEN here:
        //          - Calculating height too early, should be done in map-event
        //          - Not taking into account screen edges
        //
        gtk_window_present_with_time(
            GTK_WINDOW(popup),
            GDK_CURRENT_TIME
        );
        gtk_widget_show_all(popup);

        gtk_window_get_size(
            GTK_WINDOW(popup),
            NULL,
            &height
        );

        gdk_window_get_origin(
            gtk_widget_get_window(owner),
            &x,
            &y
        );

        // Adjust pos if the window doesn't have its own GDK window
        //
        if (!gtk_widget_get_has_window(owner))
        {
            gint off_x = 0;
            gint off_y = 0;

            gtk_widget_translate_coordinates(
                owner,
                GTK_WIDGET(wintc_widget_get_toplevel_window(owner)),
                0,
                0,
                &off_x,
                &off_y
            );

            x += off_x;
            y += off_y;
        }

        gtk_window_move(
            GTK_WINDOW(popup),
            x,
            y - height // FIXME: We're assuming the bottom of the screen here
        );
    }
    else
    {
        gtk_widget_show_all(popup);
    }
}

WinTCDisplayProtocol wintc_get_display_protocol_in_use(void)
{
    return s_dispproto;
}

gboolean wintc_init_display_protocol_apis(void)
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
        s_dispproto = WINTC_DISPPROTO_X11;

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
        s_dispproto = WINTC_DISPPROTO_WAYLAND;

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
    // Work On Wayland
    //
    if (!init_wndmgmt_xfw_impl())
    {
        if (wintc_get_display_protocol_in_use() == WINTC_DISPPROTO_WAYLAND)
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

//
// CALLBACKS
//
static gboolean on_popup_window_focus_out(
    GtkWidget* widget,
    WINTC_UNUSED(GdkEvent* event),
    WINTC_UNUSED(gpointer  user_data)
)
{
    gtk_widget_hide(widget);
    return TRUE;
}

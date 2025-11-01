#include <dlfcn.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/api.h"
#include "impl-wndmgmt-wnck.h"
#include "dll/wnck.h"

//
// FORWARD DECLARATIONS
//
static GdkPixbuf* wnck_window_get_mini_icon_real(
    WinTCWndMgmtWindow* window
);
static void wnck_window_unminimize_real(
    WinTCWndMgmtWindow* window,
    guint64             timestamp
);
static void wnck_window_close_real(
    WinTCWndMgmtWindow* window,
    guint64             timestamp
);

//
// PUBLIC FUNCTIONS
//
gboolean init_wndmgmt_wnck_impl(void)
{
    if (!init_dll_wnck())
    {
        WINTC_LOG_USER_DEBUG("%s", "WNDMGMT: Can't use libwnck");
        return FALSE;
    }

    // We're good, implement the API
    //
    wintc_wndmgmt_screen_get_active_window = p_wnck_screen_get_active_window;
    wintc_wndmgmt_screen_get_default       = p_wnck_screen_get_default;
    wintc_wndmgmt_shutdown                 = p_wnck_shutdown;
    wintc_wndmgmt_window_close             = &wnck_window_close_real;
    wintc_wndmgmt_window_get_mini_icon     = &wnck_window_get_mini_icon_real;
    wintc_wndmgmt_window_get_name          = p_wnck_window_get_name;
    wintc_wndmgmt_window_is_maximized      = p_wnck_window_is_maximized;
    wintc_wndmgmt_window_is_minimized      = p_wnck_window_is_minimized;
    wintc_wndmgmt_window_is_skip_tasklist  = p_wnck_window_is_skip_tasklist;
    wintc_wndmgmt_window_maximize          = p_wnck_window_maximize;
    wintc_wndmgmt_window_minimize          = p_wnck_window_minimize;
    wintc_wndmgmt_window_unmaximize        = p_wnck_window_unmaximize;
    wintc_wndmgmt_window_unminimize        = &wnck_window_unminimize_real;

    p_wnck_set_client_type(WNCK_CLIENT_TYPE_PAGER);

    return TRUE;
}

//
// PRIVATE FUNCTIONS
//
static GdkPixbuf* wnck_window_get_mini_icon_real(
    WinTCWndMgmtWindow* window
)
{
    GdkPixbuf*    icon_from_wnd = p_wnck_window_get_mini_icon(window);
    GdkPixbuf*    icon_resolv;
    GtkIconTheme* icon_theme;
    GdkPixbuf*    ret_icon;
    const gchar*  wm_class;

    ret_icon = icon_from_wnd;

    if (p_wnck_window_get_icon_is_fallback(window))
    {
        // Try resolving an icon in the theme using WM_CLASS
        //
        wm_class = p_wnck_window_get_class_instance_name(window);

        if (wm_class)
        {
            icon_theme = gtk_icon_theme_get_default();

            WINTC_LOG_DEBUG(
                "dpa: look up icon for wnd %p using WM_CLASS %s",
                window,
                wm_class
            );

            icon_resolv =
                gtk_icon_theme_load_icon(
                    icon_theme,
                    wm_class,
                    16, // GTK_ICON_SIZE_MENU
                    GTK_ICON_LOOKUP_FORCE_SIZE,
                    NULL
                );

            if (icon_resolv)
            {
                ret_icon = icon_resolv; // Pass on
            }
        }
    }

    // If we're returning the window's icon, we need to add our own ref
    //
    if (ret_icon == icon_from_wnd)
    {
        g_object_ref(ret_icon);
    }

    return ret_icon;
}

static void wnck_window_unminimize_real(
    WinTCWndMgmtWindow* window,
    guint64             timestamp
)
{
    p_wnck_window_unminimize(
        window,
        (guint32) timestamp
    );
}
static void wnck_window_close_real(
    WinTCWndMgmtWindow* window,
    guint64             timestamp
)
{
    p_wnck_window_close(
        window,
        (guint32) timestamp
    );
}

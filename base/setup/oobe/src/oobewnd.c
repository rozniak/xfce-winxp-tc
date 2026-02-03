#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shelldpa.h>

#include "oobewnd.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCOobeWindowClass
{
    WinTCDpaDesktopWindowClass __parent__;
};

struct _WinTCOobeWindow
{
    WinTCDpaDesktopWindow __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCOobeWindow,
    wintc_oobe_window,
    WINTC_TYPE_DPA_DESKTOP_WINDOW
)

static void wintc_oobe_window_class_init(
    WINTC_UNUSED(WinTCOobeWindowClass* klass)
)
{
    GtkCssProvider* css = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(
        css,
        "/uk/oddmatics/wintc/oobe/oobewnd.css"
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}

static void wintc_oobe_window_init(
    WinTCOobeWindow* self
)
{
    wintc_widget_add_style_class(
        GTK_WIDGET(self),
        "wintc-oobe"
    );
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_oobe_window_new(void)
{
    GdkMonitor* monitor =
        gdk_display_get_primary_monitor(
            gdk_screen_get_display(
                gdk_screen_get_default()
            )
        );

    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_OOBE_WINDOW,
            "type",    GTK_WINDOW_TOPLEVEL,
            "monitor", monitor,
            NULL
        )
    );
}

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shelldpa.h>
#include <wintc/shlang.h>

#include "setupwnd.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCSetupWindowClass
{
    WinTCDpaDesktopWindowClass __parent__;
};

struct _WinTCSetupWindow
{
    WinTCDpaDesktopWindow __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCSetupWindow,
    wintc_setup_window,
    WINTC_TYPE_DPA_DESKTOP_WINDOW
)

static void wintc_setup_window_class_init(
    WINTC_UNUSED(WinTCSetupWindowClass* klass)
) {}

static void wintc_setup_window_init(
    WinTCSetupWindow* self
)
{
    GtkBuilder* builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/wsetupx/setupwnd.ui"
        );

    wintc_lc_builder_preprocess_widget_text(builder);

    gtk_container_add(
        GTK_CONTAINER(self),
        GTK_WIDGET(
            gtk_builder_get_object(builder, "main-box")
        )
    );

    // Add the style context
    //
    GtkCssProvider*  css = gtk_css_provider_new();
    GtkStyleContext* ctx = gtk_widget_get_style_context(
                               GTK_WIDGET(self)
                           );

    gtk_css_provider_load_from_resource(
        css,
        "/uk/oddmatics/wintc/wsetupx/setupwnd.css"
    );
    gtk_style_context_add_provider(
        ctx,
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_setup_window_new(void)
{
    GdkMonitor* monitor =
        gdk_display_get_primary_monitor(
            gdk_screen_get_display(
                gdk_screen_get_default()
            )
        );

    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_SETUP_WINDOW,
            "type",    GTK_WINDOW_TOPLEVEL,
            "monitor", monitor,
            NULL
        )
    );
}

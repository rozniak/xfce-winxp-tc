#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>

#include "../public/device.h"
#include "../public/icons.h"
#include "../public/strings.h"
#include "util.h"

//
// GTK TYPE DEFINITIONS
//
G_DEFINE_QUARK(wintc-net-error-quark, wintc_net_error)

//
// PUBLIC FUNCTIONS
//
void wintc_net_ensure_styles(void)
{
    static gboolean s_styles_added = FALSE;

    // Add CSS once to screen - can't do this in class because GTK may not
    // have initialized first
    //
    if (s_styles_added)
    {
        return;
    }

    GtkCssProvider* css_net   = gtk_css_provider_new();
    GtkCssProvider* css_net_p = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(
        css_net,
        "/uk/oddmatics/wintc/netshell/net.css"
    );
    gtk_css_provider_load_from_resource(
        css_net_p,
        "/uk/oddmatics/wintc/netshell/net_p.css"
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_net),
        GTK_STYLE_PROVIDER_PRIORITY_FALLBACK
    );
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_net_p),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(css_net);
    g_object_unref(css_net_p);

    s_styles_added = TRUE;
}

GtkWidget* wintc_wlan_create_signal_image(void)
{
    GtkWidget* image = gtk_image_new();

    gtk_widget_set_halign(image, GTK_ALIGN_END);
    gtk_widget_set_valign(image, GTK_ALIGN_END);

    wintc_widget_add_style_class(image, "wintc-wlan-signal");

    return image;
}

void wintc_wlan_signal_image_set_signal(
    GtkWidget* image,
    gint       signal
)
{
    wintc_net_image_set_icon_names(
        GTK_IMAGE(image),
        wintc_wlan_get_signal_icon_names(signal),
        32
    );

    gtk_widget_set_tooltip_text(
        image,
        wintc_wlan_signal_to_text(signal)
    );
}

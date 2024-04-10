#include <dlfcn.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "../public/api.h"
#include "impl-wayland.h"
#include "dll/layersh.h"

//
// FORWARD DECLARATIONS
//
static void wayland_anchor_taskband_to_bottom(
    GtkWindow* taskband
);

//
// PUBLIC FUNCTIONS
//
gboolean init_wayland_protocol_impl(void)
{
    if (!init_dll_layersh())
    {
        return FALSE;
    }

    // All good, assign the API now
    //
    wintc_anchor_taskband_to_bottom = &wayland_anchor_taskband_to_bottom;

    return TRUE;
}

//
// PRIVATE FUNCTIONS
//
static void wayland_anchor_taskband_to_bottom(
    GtkWindow* taskband
)
{
    static gboolean anchors[] = { TRUE, TRUE, FALSE, TRUE };

    p_gtk_layer_init_for_window(taskband);
    p_gtk_layer_set_layer(taskband, GTK_LAYER_SHELL_LAYER_BOTTOM);
    p_gtk_layer_auto_exclusive_zone_enable(taskband);

    for (int i = 0; i < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; i++)
    {
        p_gtk_layer_set_anchor(taskband, i, anchors[i]);
    }
}

#include <dlfcn.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "dispproto.h"
#include "dispproto-wayland.h"

//
// PRIVATE ENUMS
//
typedef enum
{
    GTK_LAYER_SHELL_EDGE_LEFT = 0,
    GTK_LAYER_SHELL_EDGE_RIGHT,
    GTK_LAYER_SHELL_EDGE_TOP,
    GTK_LAYER_SHELL_EDGE_BOTTOM,
    GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER
} GtkLayerShellEdge;

typedef enum
{
    GTK_LAYER_SHELL_LAYER_BACKGROUND = 0,
    GTK_LAYER_SHELL_LAYER_BOTTOM,
    GTK_LAYER_SHELL_LAYER_TOP,
    GTK_LAYER_SHELL_LAYER_OVERLAY,
    GTK_LAYER_SHELL_LAYER_ENTRY_NUMBER
} GtkLayerShellLayer;

//
// RESOLVED FUNCS
//
void (*p_gtk_layer_auto_exclusive_zone_enable) (
    GtkWindow* window
);
void (*p_gtk_layer_init_for_window) (
    GtkWindow* window
);
void (*p_gtk_layer_set_anchor) (
    GtkWindow*        window,
    GtkLayerShellEdge edge,
    gboolean          anchor_to_edge
);
void (*p_gtk_layer_set_layer) (
    GtkWindow*         window,
    GtkLayerShellLayer layer
);

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
    void* dl_gtk_layer_shell =
        dlopen("libgtk-layer-shell.so", RTLD_LAZY | RTLD_LOCAL);

    if (dl_gtk_layer_shell == NULL)
    {
        g_critical("%s", "Failed to open libgtk-layer-shell for symbols.");
        return FALSE;
    }

    // Resolve funcs we're using
    //
    p_gtk_layer_auto_exclusive_zone_enable =
        dlsym(dl_gtk_layer_shell, "gtk_layer_auto_exclusive_zone_enable");
    p_gtk_layer_init_for_window            =
        dlsym(dl_gtk_layer_shell, "gtk_layer_init_for_window");
    p_gtk_layer_set_anchor                 =
        dlsym(dl_gtk_layer_shell, "gtk_layer_set_anchor");
    p_gtk_layer_set_layer                  =
        dlsym(dl_gtk_layer_shell, "gtk_layer_set_layer");

    if (
        p_gtk_layer_auto_exclusive_zone_enable == NULL ||
        p_gtk_layer_init_for_window            == NULL ||
        p_gtk_layer_set_anchor                 == NULL ||
        p_gtk_layer_set_layer                  == NULL
    )
    {
        g_critical("%s", "Failed to resolve symbols for GTK layer shell.");
        return FALSE;
    }

    // All good, assign the API now
    //
    anchor_taskband_to_bottom = &wayland_anchor_taskband_to_bottom;

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

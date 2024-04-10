#include <dlfcn.h>
#include <glib.h>
#include <wintc/comgtk.h>

#include "layersh.h"

//
// STATIC DATA
//
static gboolean s_initialized = FALSE;

//
// RESOLVED FUNCS
//
void (*p_gtk_layer_auto_exclusive_zone_enable) (
    GtkWindow* window
) = NULL;
void (*p_gtk_layer_init_for_window) (
    GtkWindow* window
) = NULL;
void (*p_gtk_layer_set_anchor) (
    GtkWindow*        window,
    GtkLayerShellEdge edge,
    gboolean          anchor_to_edge
) = NULL;
void (*p_gtk_layer_set_layer) (
    GtkWindow*         window,
    GtkLayerShellLayer layer
) = NULL;
void (*p_gtk_layer_set_margin) (
    GtkWindow*        window,
    GtkLayerShellEdge edge,
    int               margin_size
) = NULL;
void (*p_gtk_layer_set_monitor) (
    GtkWindow*  window,
    GdkMonitor* monitor
) = NULL;
void (*p_gtk_layer_set_namespace) (
    GtkWindow*   window,
    const gchar* name_space
) = NULL;

//
// PUBLIC FUNCTIONS
//
gboolean init_dll_layersh()
{
    void* dl_layersh = NULL;

    if (s_initialized)
    {
        return TRUE;
    }

    dl_layersh = dlopen("libgtk-layer-shell.so", RTLD_LAZY | RTLD_LOCAL);

    if (dl_layersh == NULL)
    {
        g_critical("%s", "Failed to open libgtk-layer-shell for symbols.");
        return FALSE;
    }

    // Resolve the funcs we're using
    //
    p_gtk_layer_auto_exclusive_zone_enable =
        dlsym(dl_layersh, "gtk_layer_auto_exclusive_zone_enable");

    p_gtk_layer_init_for_window =
        dlsym(dl_layersh, "gtk_layer_init_for_window");

    p_gtk_layer_set_anchor =
        dlsym(dl_layersh, "gtk_layer_set_anchor");

    p_gtk_layer_set_layer =
        dlsym(dl_layersh, "gtk_layer_set_layer");

    p_gtk_layer_set_margin =
        dlsym(dl_layersh, "gtk_layer_set_margin");

    p_gtk_layer_set_monitor =
        dlsym(dl_layersh, "gtk_layer_set_monitor");

    p_gtk_layer_set_namespace =
        dlsym(dl_layersh, "gtk_layer_set_namespace");

    if (
        p_gtk_layer_auto_exclusive_zone_enable == NULL ||
        p_gtk_layer_init_for_window            == NULL ||
        p_gtk_layer_set_anchor                 == NULL ||
        p_gtk_layer_set_layer                  == NULL ||
        p_gtk_layer_set_margin                 == NULL ||
        p_gtk_layer_set_monitor                == NULL ||
        p_gtk_layer_set_namespace              == NULL
    )
    {
        g_critical("%s", "Failed to resolve symbols for GTK layer shell.");
        return FALSE;
    }

    s_initialized = TRUE;

    return TRUE;
}

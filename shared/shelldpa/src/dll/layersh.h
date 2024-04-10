#ifndef __DLL_LAYERSH_H__
#define __DLL_LAYERSH_H__

#include <glib.h>

//
// PUBLIC ENUMS
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
extern void (*p_gtk_layer_auto_exclusive_zone_enable) (
    GtkWindow* window
);
extern void (*p_gtk_layer_init_for_window) (
    GtkWindow* window
);
extern void (*p_gtk_layer_set_anchor) (
    GtkWindow*        window,
    GtkLayerShellEdge edge,
    gboolean          anchor_to_edge
);
extern void (*p_gtk_layer_set_layer) (
    GtkWindow*         window,
    GtkLayerShellLayer layer
);
extern void (*p_gtk_layer_set_margin) (
    GtkWindow*        window,
    GtkLayerShellEdge edge,
    int               margin_size
);
extern void (*p_gtk_layer_set_monitor) (
    GtkWindow*  window,
    GdkMonitor* monitor
);
extern void (*p_gtk_layer_set_namespace) (
    GtkWindow*  window,
    const char* name_space
);

//
// PUBLIC FUNCTIONS
//
gboolean init_dll_layersh(void);

#endif

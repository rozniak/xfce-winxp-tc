#ifndef __COMGTK_TREEMODEL_H__
#define __COMGTK_TREEMODEL_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// PUBLIC FUNCTIONS
//
gint wintc_tree_model_get_insertion_sort_pos(
    GtkTreeModel* tree_model,
    GtkTreeIter*  node,
    gint          column,
    gint          data_type,
    GCompareFunc  compare_func,
    gconstpointer item
);

#endif

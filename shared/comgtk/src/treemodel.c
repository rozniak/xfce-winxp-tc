#include <glib.h>
#include <gtk/gtk.h>

#include "../public/treemodel.h"

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
)
{
    gint n_children = gtk_tree_model_iter_n_children(tree_model, node);

    if (!n_children)
    {
        return 0;
    }

    // Set ourselves up, if we're sorting top level children or children of a
    // node
    //
    GtkTreeIter child;

    gint start  = 0;
    gint middle = n_children / 2;
    gint end    = n_children;

    gint diff = end - start;

    while (TRUE)
    {
        gtk_tree_model_iter_nth_child(
            tree_model,
            &child,
            node,
            middle
        );

        // Compare item at this node
        //
        gpointer item_ptr;
        gint     item_int;
        guint    item_uint;

        gint result;

        switch (data_type)
        {
            case G_TYPE_INT:
                gtk_tree_model_get(
                    tree_model,
                    &child,
                    column, &item_int,
                    -1
                );

                result =
                    compare_func(
                        item,
                        GINT_TO_POINTER(item_int)
                    );

                break;

            case G_TYPE_UINT:
                gtk_tree_model_get(
                    tree_model,
                    &child,
                    column, &item_uint,
                    -1
                );

                result =
                    compare_func(
                        item,
                        GUINT_TO_POINTER(item_uint)
                    );

                break;

            default:
                gtk_tree_model_get(
                    tree_model,
                    &child,
                    column, &item_ptr,
                    -1
                );

                result =
                    compare_func(
                        item,
                        item_ptr
                    );

                break;
        }

        // If this is the last iteration, deal with it
        //
        if (diff == 1)
        {
            if (result > 0)
            {
                return middle + 1;
            }
            else
            {
                return middle;
            }
        }

        // Otherwise, iterate
        //
        if (result < 0)
        {
            end = middle;
        }
        else if (result > 0)
        {
            start = middle;
        }
        else
        {
            start = middle;
            end   = middle;
        }

        diff = end - start;
        middle = start + (diff / 2);
    }
}

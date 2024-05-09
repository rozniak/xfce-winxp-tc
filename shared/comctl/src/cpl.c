#include <glib.h>
#include <gtk/gtk.h>
#include <stdarg.h>
#include <wintc/comgtk.h>
#include <wintc/shlang.h>

#include "../public/cpl.h"

//
// PUBLIC FUNCTIONS
//
void wintc_ctl_cpl_notebook_append_page_from_resource(
    GtkNotebook* notebook,
    const gchar* resource_path,
    ...
)
{
    GtkBuilder* builder;
    GtkWidget*  box_page;
    GtkWidget*  label_title;

    builder = gtk_builder_new_from_resource(resource_path);

    // Handle construction into UI
    //
    wintc_lc_builder_preprocess_widget_text(builder);

    box_page    = GTK_WIDGET(gtk_builder_get_object(builder, "page-box"));
    label_title = GTK_WIDGET(gtk_builder_get_object(builder, "label-title"));

    gtk_notebook_append_page(
        notebook,
        box_page,
        label_title
    );

    // Pull out objects as requested, args are provided as pairs (name, dstptr)
    //
    va_list     ap;
    GtkWidget** next_dst;
    gchar*      next_name;

    va_start(ap, resource_path);

    next_name = va_arg(ap, gchar*);

    while (next_name)
    {
        next_dst  = va_arg(ap, GtkWidget**);
        *next_dst = GTK_WIDGET(gtk_builder_get_object(builder, next_name));

        // Iter
        //
        next_name = va_arg(ap, gchar*);
    }

    va_end(ap);

    g_object_unref(G_OBJECT(builder));
}

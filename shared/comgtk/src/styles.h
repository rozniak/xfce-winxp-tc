#ifndef __STYLES_H__
#define __STYLES_H__

#include <gtk/gtk.h>

//
// PUBLIC FUNCTIONS
//
void wintc_widget_add_css(
    GtkWidget*   widget,
    const gchar* css
);

void wintc_widget_add_style_class(
    GtkWidget*   widget,
    const gchar* class_name
);

#endif

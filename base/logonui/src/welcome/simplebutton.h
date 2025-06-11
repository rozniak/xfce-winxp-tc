
#ifndef SIMPLE_BUTTON_H
#define SIMPLE_BUTTON_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define SIMPLE_TYPE_BUTTON (simple_button_get_type())
G_DECLARE_FINAL_TYPE(SimpleButton, simple_button, SIMPLE, BUTTON, GtkButton)

GtkWidget* simple_button_new_with_pixbufs(GdkPixbuf *idle_pixbuf, 
                                           GdkPixbuf *activated_pixbuf);

G_END_DECLS

#endif

#ifndef WINTC_WELCOME_BUTTON_H
#define WINTC_WELCOME_BUTTON_H

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define WINTC_TYPE_WELCOME_BUTTON (wintc_welcome_button_get_type())

G_DECLARE_FINAL_TYPE(WinTCWelcomeButton, wintc_welcome_button, WINTC, WELCOME_BUTTON, GtkButton)

GtkWidget* wintc_welcome_button_new_with_pixbufs(GdkPixbuf *idle_pixbuf, 
                                           GdkPixbuf *activated_pixbuf);

G_END_DECLS

#endif
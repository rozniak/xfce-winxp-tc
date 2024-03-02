#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "application.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCNotepadWindowClass   WinTCNotepadWindowClass;
typedef struct _WinTCNotepadWindow        WinTCNotepadWindow;

#define WINTC_TYPE_NOTEPAD_WINDOW            (wintc_notepad_window_get_type())
#define WINTC_NOTEPAD_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_NOTEPAD_WINDOW, WinTCNotepadWindow))
#define WINTC_NOTEPAD_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_NOTEPAD_WINDOW, WinTCNotepadWindow))
#define IS_WINTC_NOTEPAD_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_NOTEPAD_WINDOW))
#define IS_WINTC_NOTEPAD_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_NOTEPAD_WINDOW))
#define WINTC_NOTEPAD_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_NOTEPAD_WINDOW))

GType wintc_notepad_window_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_notepad_window_new(
    WinTCNotepadApplication* app
);
GtkWidget* wintc_notepad_window_new_with_uri(
    WinTCNotepadApplication* app,
    const gchar*             uri
);

#endif

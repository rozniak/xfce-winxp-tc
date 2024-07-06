#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCOobeWindowClass
{
    GtkWindowClass __parent__;
};

struct _WinTCOobeWindow
{
    GtkWindow __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCOobeWindow,
    wintc_oobe_window,
    GTK_TYPE_WINDOW
)

static void wintc_oobe_window_class_init(
    WINTC_UNUSED(WinTCOobeWindowClass* klass)
) {}

static void wintc_oobe_window_init(
    WinTCOobeWindow* self
)
{
    gtk_window_set_default_size(
        GTK_WINDOW(self),
        320,
        200
    );

    gtk_container_add(
        GTK_CONTAINER(self),
        gtk_label_new("Hello Windows!")
    );
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_oobe_window_new(void)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_OOBE_WINDOW,
            "title", "Hello Windows!",
            NULL
        )
    );
}

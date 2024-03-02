#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCHelloWindowClass
{
    GtkApplicationWindowClass __parent__;
};

struct _WinTCHelloWindow
{
    GtkApplicationWindow __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCHelloWindow,
    wintc_hello_window,
    GTK_TYPE_APPLICATION_WINDOW
)

static void wintc_hello_window_class_init(
    WINTC_UNUSED(WinTCHelloWindowClass* klass)
) {}

static void wintc_hello_window_init(
    WinTCHelloWindow* self
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
GtkWidget* wintc_hello_window_new(
    WinTCHelloApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_HELLO_WINDOW,
            "application", GTK_APPLICATION(app),
            "title",       "Hello Windows!",
            NULL
        )
    );
}

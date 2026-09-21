#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/setupapi.h>

#include "application.h"
#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCSysAnalWindowClass
{
    GtkApplicationWindowClass __parent__;
};

struct _WinTCSysAnalWindow
{
    GtkApplicationWindow __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCSysAnalWindow,
    wintc_sys_anal_window,
    GTK_TYPE_APPLICATION_WINDOW
)

static void wintc_sys_anal_window_class_init(
    WINTC_UNUSED(WinTCSysAnalWindowClass* klass)
) {}

static void wintc_sys_anal_window_init(
    WinTCSysAnalWindow* self
)
{
    gtk_window_set_default_size(
        GTK_WINDOW(self),
        320,
        200
    );

    gtk_container_add(
        GTK_CONTAINER(self),
        gtk_label_new(
            wintc_init_system_get_name(
                wintc_init_system_get()
            )
        )
    );
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_sys_anal_window_new(
    WinTCSysAnalApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_SYS_ANAL_WINDOW,
            "application", GTK_APPLICATION(app),
            "title",       "My System",
            NULL
        )
    );
}

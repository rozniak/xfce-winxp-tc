#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <wintc-comgtk.h>
#include <wintc-shllang.h>

#include "application.h"
#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCNotepadWindowPrivate
{
    int blah;
};

struct _WinTCNotepadWindowClass
{
    GtkApplicationWindowClass __parent__;
};

struct _WinTCNotepadWindow
{
    GtkApplicationWindow __parent__;

    WinTCNotepadWindowPrivate* priv;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCNotepadWindow,
    wintc_notepad_window,
    GTK_TYPE_APPLICATION_WINDOW,
    G_ADD_PRIVATE(WinTCNotepadWindow)
)

static void wintc_notepad_window_class_init(
    WINTC_UNUSED(WinTCNotepadWindowClass* klass)
)
{
}

static void wintc_notepad_window_init(
    WinTCNotepadWindow* self
)
{
    GtkBuilder* builder;
    GtkWidget*  main_box;

    gtk_window_set_title(GTK_WINDOW(self), _("Notepad"));

    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/notepad/notepad.ui"
        );

    wintc_preprocess_builder_widget_text(builder);

    main_box = GTK_WIDGET(gtk_builder_get_object(builder, "main-box"));

    gtk_container_add(GTK_CONTAINER(self), main_box);

    g_object_unref(G_OBJECT(builder));
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_notepad_window_new(
    WinTCNotepadApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            TYPE_WINTC_NOTEPAD_WINDOW,
            "application", GTK_APPLICATION(app),
            NULL
        )
    );
}

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCDndTestWindowClass
{
    GtkApplicationWindowClass __parent__;
};

struct _WinTCDndTestWindow
{
    GtkApplicationWindow __parent__;
};

//
// FORWARD DECLARATIONS
//
static void on_self_drag_data_received(
    GtkWidget*        self,
    GdkDragContext*   context,
    gint              x,
    gint              y,
    GtkSelectionData* data,
    guint             info,
    guint             time,
    gpointer          user_data
);

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCDndTestWindow,
    wintc_dnd_test_window,
    GTK_TYPE_APPLICATION_WINDOW
)

static void wintc_dnd_test_window_class_init(
    WINTC_UNUSED(WinTCDndTestWindowClass* klass)
) {}

static void wintc_dnd_test_window_init(
    WinTCDndTestWindow* self
)
{
    gtk_window_set_default_size(
        GTK_WINDOW(self),
        320,
        200
    );

    gtk_container_add(
        GTK_CONTAINER(self),
        gtk_label_new("Drag something here!")
    );

    g_signal_connect(
        self,
        "drag-data-received",
        G_CALLBACK(on_self_drag_data_received),
        NULL
    );

    // Drag 'n' drop setup
    //
    GtkTargetEntry target_entry = {
        "text/uri-list",
        0,
        0
    };
    gtk_drag_dest_set(
        GTK_WIDGET(self),
        GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_DROP,
        &target_entry,
        1,
        GDK_ACTION_COPY
    );
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_dnd_test_window_new(
    WinTCDndTestApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_DND_TEST_WINDOW,
            "application", GTK_APPLICATION(app),
            "title",       "Drag 'n' Drop File(s)!",
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void on_self_drag_data_received(
    GtkWidget*        self,
    WINTC_UNUSED(GdkDragContext* context),
    WINTC_UNUSED(gint x),
    WINTC_UNUSED(gint y),
    GtkSelectionData* data,
    WINTC_UNUSED(guint info),
    WINTC_UNUSED(guint time),
    WINTC_UNUSED(gpointer user_data)
)
{
    GtkWidget* label = gtk_bin_get_child(GTK_BIN(self));

    // Update label if necessary
    //
    gchar** uris = gtk_selection_data_get_uris(data);

    if (uris == NULL)
    {
        return;
    }

    gchar* uris_joined = g_strjoinv("\n", uris);

    gtk_label_set_text(GTK_LABEL(label), uris_joined);

    g_free(uris_joined);
    g_strfreev(uris);
}

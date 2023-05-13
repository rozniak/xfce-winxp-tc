#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <wintc-comgtk.h>

#include "application.h"
#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCNotepadApplicationClass
{
    GtkApplicationClass __parent__;
};

struct _WinTCNotepadApplication
{
    GtkApplication __parent__;
};

//
// FORWARD DECLARATIONS
//
static void wintc_notepad_application_activate(
    GApplication* application
);

static void wintc_notepad_application_open(
    GApplication* application,
    GFile**       files,
    int           n_files,
    const gchar*  hint
);

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(WinTCNotepadApplication, wintc_notepad_application, GTK_TYPE_APPLICATION)

static void wintc_notepad_application_class_init(
    WinTCNotepadApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);

    application_class->activate = wintc_notepad_application_activate;
    application_class->open     = wintc_notepad_application_open;
}

static void wintc_notepad_application_init(
    WINTC_UNUSED(WinTCNotepadApplication* self)
) {}

//
// PUBLIC FUNCTIONS
//
WinTCNotepadApplication* wintc_notepad_application_new(void)
{
    WinTCNotepadApplication* app;

    g_set_application_name("Notepad");

    app =
        g_object_new(
            wintc_notepad_application_get_type(),
            "application-id", "uk.co.oddmatics.wintc.notepad",
            NULL
        );

    return app;
}

//
// CALLBACKS
//
static void wintc_notepad_application_activate(
    WINTC_UNUSED(GApplication* application)
)
{
    WinTCNotepadApplication* notepad_app = WINTC_NOTEPAD_APPLICATION(application);
    gtk_widget_show_all(wintc_notepad_window_new(notepad_app));
}

static void wintc_notepad_application_open(
    WINTC_UNUSED(GApplication* application),
    WINTC_UNUSED(GFile**       files),
    WINTC_UNUSED(int           n_files),
    WINTC_UNUSED(const gchar*  hint)
)
{
    // TODO: Implement this
}

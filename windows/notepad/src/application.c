#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

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

static void first_run(
    GApplication* application
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
// CLASS VIRTUAL METHODS
//
static void wintc_notepad_application_activate(
    GApplication* application
)
{
    WinTCNotepadApplication* notepad_app =
        WINTC_NOTEPAD_APPLICATION(application);

    first_run(application);

    gtk_widget_show_all(
        wintc_notepad_window_new(notepad_app)
    );
}

static void wintc_notepad_application_open(
    GApplication* application,
    GFile**       files,
    int           n_files,
    WINTC_UNUSED(const gchar* hint)
)
{
    WinTCNotepadApplication* notepad_app =
        WINTC_NOTEPAD_APPLICATION(application);

    first_run(application);

    for (int i = 0; i < n_files; i++)
    {
        GFile*     file = files[i];
        gchar*     uri  = g_file_get_uri(file);
        GtkWidget* wnd  = wintc_notepad_window_new_with_uri(
                              notepad_app,
                              uri
                          );

        gtk_widget_show_all(wnd);

        g_free(uri);
    }
}

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
            "flags",          G_APPLICATION_HANDLES_OPEN,
            NULL
        );

    return app;
}

//
// PRIVATE FUNCTIONS
//
static void first_run(
    WINTC_UNUSED(GApplication* application)
)
{
    static gboolean first_run = TRUE;

    if (first_run)
    {
        GtkCssProvider* css_provider = gtk_css_provider_new();

        gtk_css_provider_load_from_resource(
            css_provider,
            "/uk/oddmatics/wintc/notepad/appstyles.css"
        );

        gtk_style_context_add_provider_for_screen(
            gdk_screen_get_default(),
            GTK_STYLE_PROVIDER(css_provider),
            GTK_STYLE_PROVIDER_PRIORITY_FALLBACK
        );

        first_run = FALSE;
    }
}

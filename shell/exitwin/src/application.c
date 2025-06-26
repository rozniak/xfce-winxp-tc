#include <gio/gio.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/msgina.h>

#include "application.h"

//
// FORWARD DECLARATIONS
//
static void wintc_exitwin_application_activate(
    GApplication* application
);

//
// STATIC DATA
//
static gboolean s_cmd_pwropts = FALSE;
static gboolean s_cmd_usropts = FALSE;

static const GOptionEntry s_option_entries[] = {
    {
        "power-options",
        'p',
        G_OPTION_FLAG_NONE,
        G_OPTION_ARG_NONE,
        &s_cmd_pwropts,
        N_("Display power options dialog"),
        NULL
    },
    {
        "user-options",
        'u',
        G_OPTION_FLAG_NONE,
        G_OPTION_ARG_NONE,
        &s_cmd_usropts,
        N_("Display user options dialog"),
        NULL
    },
    { 0 }
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCExitwinApplication
{
    GtkApplication __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCExitwinApplication,
    wintc_exitwin_application,
    GTK_TYPE_APPLICATION
)

static void wintc_exitwin_application_class_init(
    WinTCExitwinApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);

    application_class->activate = wintc_exitwin_application_activate;
}

static void wintc_exitwin_application_init(
    WinTCExitwinApplication* self
)
{
    g_application_add_main_option_entries(
        G_APPLICATION(self),
        s_option_entries
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_exitwin_application_activate(
    WINTC_UNUSED(GApplication* application)
)
{
    static GtkWidget* wnd = NULL;

    if (wnd)
    {
        return;
    }

    // Spawn the session manager interface
    //
    WinTCGinaSmXfce* sm_xfce = wintc_gina_sm_xfce_new();

    if (!wintc_igina_sm_is_valid(WINTC_IGINA_SM(sm_xfce)))
    {
        // FIXME: Localise
        //
        wintc_messagebox_show(
            NULL,
            "Failed to connect to the session manager.",
            "Error",
            GTK_BUTTONS_OK,
            GTK_MESSAGE_ERROR
        );

        return;
    }

    // All good for spawning the dialog
    //
    if (s_cmd_usropts)
    {
        wnd =
            wintc_gina_exit_window_new_for_user_options(
                WINTC_IGINA_SM(sm_xfce)
            );
    }
    else
    {
        wnd =
            wintc_gina_exit_window_new_for_power_options(
                WINTC_IGINA_SM(sm_xfce)
            );
    }

    gtk_widget_show_all(wnd);

    g_object_unref(sm_xfce); // Dialog takes its own reference
}

//
// PUBLIC FUNCTIONS
//
WinTCExitwinApplication* wintc_exitwin_application_new(void)
{
    WinTCExitwinApplication* app;

    app =
        g_object_new(
            wintc_exitwin_application_get_type(),
            "application-id", "uk.oddmatics.wintc.exitwin",
            NULL
        );

    return app;
}

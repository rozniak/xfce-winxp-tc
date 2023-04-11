#include <glib.h>
#include <gtk/gtk.h>
#include <wintc-comgtk.h>

#include "application.h"
#include "dialog.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCNewPwrDlgApplicationClass
{
    GtkApplicationClass __parent__;
};

struct _WinTCNewPwrDlgApplication
{
    GtkApplication __parent__;
};

//
// FORWARD DECLARATIONS
//
static void wintc_npwrdlg_application_activate(
    GApplication* application
);

static void wintc_npwrdlg_application_finalize(
    GObject* object
);

static void wintc_npwrdlg_application_open(
    GApplication* application,
    GFile**       files,
    int           n_files,
    const gchar*  hint
);

static void wintc_npwrdlg_application_startup(
    GApplication* application
);

static void wintc_npwrdlg_application_shutdown(
    GApplication* application
);

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(WinTCNewPwrDlgApplication, wintc_npwrdlg_application, GTK_TYPE_APPLICATION)

static void wintc_npwrdlg_application_class_init(
    WinTCNewPwrDlgApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);
    GObjectClass*      object_class      = G_OBJECT_CLASS(klass);

    application_class->activate = wintc_npwrdlg_application_activate;
    application_class->open     = wintc_npwrdlg_application_open;
    application_class->startup  = wintc_npwrdlg_application_startup;
    application_class->shutdown = wintc_npwrdlg_application_shutdown;

    object_class->finalize = wintc_npwrdlg_application_finalize;
}

static void wintc_npwrdlg_application_init(
    WINTC_UNUSED(WinTCNewPwrDlgApplication* self)
) {}

//
// FINALIZE
//
static void wintc_npwrdlg_application_finalize(
    GObject* object
)
{
    (*G_OBJECT_CLASS(wintc_npwrdlg_application_parent_class)->finalize) (object);
}

//
// PUBLIC FUNCTIONS
//
WinTCNewPwrDlgApplication* wintc_npwrdlg_application_new(void)
{
    WinTCNewPwrDlgApplication* app;

    g_set_application_name("NewPwrDlg");

    app =
        g_object_new(
            wintc_npwrdlg_application_get_type(),
            "application-id", "uk.co.oddmatics.wintc.npwrdlg",
            NULL
        );

    return app;
}

//
// CALLBACKS
//
static void wintc_npwrdlg_application_activate(
    WINTC_UNUSED(GApplication* application)
) {}

static void wintc_npwrdlg_application_open(
    WINTC_UNUSED(GApplication* application),
    WINTC_UNUSED(GFile**       files),
    WINTC_UNUSED(int           n_files),
    WINTC_UNUSED(const gchar*  hint)
) {}

static void wintc_npwrdlg_application_startup(
    GApplication* application
)
{
    WinTCNewPwrDlgApplication* pwr_app = WINTC_NPWRDLG_APPLICATION(application);

    (G_APPLICATION_CLASS(wintc_npwrdlg_application_parent_class))->startup(application);

    GtkWidget* wnd = wintc_npwrdlg_dialog_new(pwr_app);

    gtk_widget_show_all(wnd);
}

static void wintc_npwrdlg_application_shutdown(
    GApplication* application
)
{
    (G_APPLICATION_CLASS(wintc_npwrdlg_application_parent_class))->shutdown(application);
}

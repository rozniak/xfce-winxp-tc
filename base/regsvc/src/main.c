#include <glib.h>
#include <glib-unix.h>
#include <signal.h>
#include <wintc/comgtk.h>
#include <wintc/registry.h>

#include "application.h"
#include "backend.h"

//
// FORWARD DECLARATIONS
//
static gboolean on_sigterm(
    gpointer user_data
);

//
// STATIC DATA
//
static GApplication* s_app = NULL;

//
// ENTRY POINT
//
int main(
    int   argc,
    char* argv[]
)
{
    s_app = G_APPLICATION(wintc_regsvc_application_new());

    g_unix_signal_add(
        SIGTERM,
        on_sigterm,
        NULL
    );

    g_application_run(s_app, argc, argv);

    return 0;
}

//
// CALLBACKS
//
static gboolean on_sigterm(
    WINTC_UNUSED(gpointer user_data)
)
{
    WINTC_LOG_DEBUG("%s", "regsvc: sigterm, exiting...");

    g_application_quit(s_app);

    return FALSE;
}

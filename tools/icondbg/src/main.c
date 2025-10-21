#include <glib.h>

#include "application.h"

int main(
    int   argc,
    char* argv[]
)
{
    WinTCIconDbgApplication* app = wintc_icon_dbg_application_new();
    int                      status;

    g_set_application_name("IconDbg");

    status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);

    return status;
}

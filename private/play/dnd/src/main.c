#include <glib.h>

#include "application.h"

int main(
    int   argc,
    char* argv[]
)
{
    WinTCDndTestApplication* app = wintc_dnd_test_application_new();
    int                      status;

    g_set_application_name("Drag 'n' Drop");

    status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);

    return status;
}

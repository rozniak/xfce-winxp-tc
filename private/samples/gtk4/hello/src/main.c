#include <glib.h>

#include "application.h"

int main(
    int   argc,
    char* argv[]
)
{
    WinTCHelloApplication* app = wintc_hello_application_new();
    int                    status;

    g_set_application_name("Hello4");

    status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);

    return status;
}

#include <gio/gio.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <locale.h>

#include "application.h"
#include "meta.h"

int main(
    int   argc,
    char* argv[]
)
{
    // Set up locales
    //
    setlocale(LC_ALL, "");
    bindtextdomain(PKG_NAME, "/usr/share/locale");
    textdomain(PKG_NAME);

    // Launch application
    //
    WinTCExitwinApplication* app = wintc_exitwin_application_new();
    int                      status;

    status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);

    return status;
}

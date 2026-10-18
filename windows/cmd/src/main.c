#include <fontconfig/fontconfig.h>
#include <glib.h>
#include <wintc/comgtk.h>

#include "application.h"

int main(
    int   argc,
    char* argv[]
)
{
    // HACK: A bit of a bodge I guess? Insert the VGA font directly into
    //       fontconfig so we can use it in VTE
    //
    FcConfig* fc = FcConfigGetCurrent();

    if (
        !fc ||
        !FcConfigAppFontAddFile(
            fc,
            (FcChar8*) (WINTC_ASSETS_DIR "/conhost/oemfon.ttf")
        )
    )
    {
        g_warning("%s", "cmd: unable to set oem font");
    }

    // Start app
    //
    WinTCCmdApplication* app = wintc_cmd_application_new();
    int                  status;

    status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);

    return status;
}

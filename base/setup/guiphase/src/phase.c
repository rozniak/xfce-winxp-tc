#include <errno.h>
#include <glib.h>
#include <sys/stat.h>
#include <wintc/comgtk.h>

#include "phase.h"

#define WINTC_SETUP_ROOT_DIR "/var/tmp/.wintc-setup"

//
// PUBLIC FUNCTIONS
//
gboolean wintc_setup_phase_set(
    WinTCSetupPhase phase
)
{
    GError* error = NULL;

    // Ensure the dir exists to write to
    //
    if (g_mkdir_with_parents(WINTC_SETUP_ROOT_DIR, S_IRWXU) < 0)
    {
        g_critical(
            "wsetupx: failed to create %s (err: %d)",
            WINTC_SETUP_ROOT_DIR,
            errno
        );
        return FALSE;
    }

    // Deploy the phase file
    //
    gchar*   phase_str = g_strdup_printf("%d", phase);
    gboolean success;

    success =
        g_file_set_contents(
            WINTC_SETUP_ROOT_DIR "/phase",
            phase_str,
            -1,
            &error
        );

    g_free(phase_str);

    if (!success)
    {
        wintc_log_error_and_clear(&error);
        return FALSE;
    }

    return TRUE;
}

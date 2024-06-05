#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>

#include "../public/cpl.h"

//
// PRIVATE CONSTANTS
//
static const gchar* S_CPL_ENTRIES_DIR = WINTC_ASSETS_DIR "/cpl";

static const gchar* S_CPL_GROUP_NAME       = "CplApplet";
static const gchar* S_CPL_KEY_COMMENT      = "Comment";
static const gchar* S_CPL_KEY_DISPLAY_NAME = "Name";
static const gchar* S_CPL_KEY_EXEC         = "Exec";
static const gchar* S_CPL_KEY_ICON_NAME    = "Icon";

//
// PUBLIC FUNCTIONS
//
GSList* wintc_sh_cpl_applet_get_all(void)
{
    GSList* entries =
        wintc_sh_fs_get_names_as_list(
            S_CPL_ENTRIES_DIR,
            TRUE,
            G_FILE_TEST_IS_REGULAR,
            FALSE,
            NULL
        );

    if (!entries)
    {
        return NULL;
    }

    WinTCShCplApplet* applet;
    GSList*           cpls     = NULL;
    GError*           error    = NULL;
    GKeyFile*         key_file = g_key_file_new();
    gboolean          success  = TRUE;

    for (GSList* iter = entries; iter; iter = iter->next)
    {
        WINTC_LOG_DEBUG("shell: cpl parse %s", (gchar*) iter->data);

        if (!g_str_has_suffix(iter->data, ".desktop"))
        {
            WINTC_LOG_DEBUG("shell: skipping %s", (gchar*) iter->data);
            continue;
        }

        if (
            !g_key_file_load_from_file(
                key_file,
                iter->data,
                G_KEY_FILE_NONE,
                &error
            )
        )
        {
            wintc_log_error_and_clear(&error);
            continue;
        }

        // Init next applet
        //
        applet = g_new0(WinTCShCplApplet, 1);

        applet->display_name = WINTC_SUCCESS(
                                   g_key_file_get_locale_string(
                                       key_file,
                                       S_CPL_GROUP_NAME,
                                       S_CPL_KEY_DISPLAY_NAME,
                                       NULL,
                                       &error
                                   ),
                                   error,
                                   success
                               );
        applet->exec         = WINTC_SUCCESS(
                                   g_key_file_get_string(
                                       key_file,
                                       S_CPL_GROUP_NAME,
                                       S_CPL_KEY_EXEC,
                                       &error
                                   ),
                                   error,
                                   success
                               );

        applet->comment   = g_key_file_get_locale_string(
                                key_file,
                                S_CPL_GROUP_NAME,
                                S_CPL_KEY_COMMENT,
                                NULL,
                                NULL
                            );
        applet->icon_name = g_key_file_get_string(
                                key_file,
                                S_CPL_GROUP_NAME,
                                S_CPL_KEY_ICON_NAME,
                                NULL
                            );

        WINTC_LOG_DEBUG("shell: cpl parse status %d", success);

        if (success)
        {
            cpls = g_slist_append(cpls, applet);
        }
        else
        {
            g_free(applet);
        }

        // Reset for next iter
        //
        success = TRUE;
    }

    g_slist_free_full(entries, g_free);
    g_key_file_free(key_file);

    return cpls;
}

gboolean wintc_sh_cpl_applet_is_executable(
    WinTCShCplApplet* applet
)
{
    // Check exec string - shell path vs. anything else (this probably isn't
    // the best way, but it'll do for now)
    //
    return !g_str_has_prefix(applet->exec, "::{");
}

void wintc_sh_cpl_applet_free(
    WinTCShCplApplet* applet
)
{
    if (!applet)
    {
        return;
    }

    g_free(applet->display_name);
    g_free(applet->comment);
    g_free(applet->exec);
    g_free(applet->icon_name);
    g_free(applet);
}

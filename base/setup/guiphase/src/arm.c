#include <errno.h>
#include <glib.h>
#include <unistd.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>

#include "arm.h"

#define WINTC_SETUP_ASSETS_DIR WINTC_ASSETS_DIR "/setup"

//
// PUBLIC FUNCTIONS
//
gboolean wintc_setup_arm_system(void)
{
    GError* error = NULL;

    //
    // To arm the system for setup, we do the following things in order:
    //     - Deploy the init service/script for launching setup during startup
    //     - Specify the 'splash' parameter to Linux in Grub
    //     - Set the plymouth boot screen, generate initrd
    //     - Update grub
    //

    //
    // FIXME: This is only tested on Debian and systemd right now!!!!!!!
    //

    // Deploy the systemd service
    //
    WINTC_LOG_DEBUG("wsetupx: attempting to link setup service");

    int res_sdlink =
        symlink(
            WINTC_SETUP_ASSETS_DIR "/wsetupx.service",
            "/etc/systemd/system/wsetupx.service"
        );

    if (!(res_sdlink == 0 || errno == EEXIST))
    {
        g_critical("wsetupx: unable to link setup service (err: %d)", errno);
        return FALSE;
    }

    // Enable it in systemd for startup
    //
    WINTC_LOG_DEBUG("wsetupx: attempting to enable setup service");

    if (
        !wintc_launch_command_sync(
            "systemctl enable wsetupx",
            NULL,
            NULL,
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
        return FALSE;
    }

    // Read in Grub config, we crudely prepend a group onto the text so that
    // it can be read as a keyfile
    //
    gchar* grub_conf = NULL;
    gchar* grub_conf_bodged;

    if (
        !g_file_get_contents(
            "/etc/default/grub", // FIXME: Debian specific I believe
            &grub_conf,
            NULL,
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
        return FALSE;
    }

    grub_conf_bodged =
        g_strdup_printf(
            "[Bodged]\n%s",
            grub_conf
        );

    g_free(grub_conf);

    // Read the Grub keyfile and append the 'splash' parameter
    //
    WINTC_LOG_DEBUG("wsetupx: install 'splash' parameter into grub");

    GKeyFile* keyfile_grub = g_key_file_new();

    if (
        !g_key_file_load_from_data(
            keyfile_grub,
            grub_conf_bodged,
            -1,
            G_KEY_FILE_NONE,
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
        return FALSE;
    }

    gchar* linux_cmdline =
        g_key_file_get_string(
            keyfile_grub,
            "Bodged",
            "GRUB_CMDLINE_LINUX_DEFAULT",
            NULL // We actually don't care about missing keys here
        );

    if (linux_cmdline)
    {
        if (!strstr(linux_cmdline, "splash"))
        {
            gchar* unquoted =
                g_shell_unquote(linux_cmdline, &error);

            if (!unquoted)
            {
                wintc_log_error_and_clear(&error);
                return FALSE;
            }

            gchar* new_linux_cmdline =
                g_strdup_printf("%s splash", unquoted);

            gchar* quoted =
                g_shell_quote(new_linux_cmdline);

            g_free(unquoted);
            g_free(new_linux_cmdline);

            g_free(linux_cmdline);
            linux_cmdline = quoted;
        }
    }
    else
    {
        linux_cmdline = g_shell_quote("splash");
    }

    g_key_file_set_string(
        keyfile_grub,
        "Bodged",
        "GRUB_CMDLINE_LINUX_DEFAULT",
        linux_cmdline
    );

    g_free(linux_cmdline);

    // Write back out
    //
    // FIXME: Should make a backup first really!
    //
    gchar* keyfile_raw = g_key_file_to_data(keyfile_grub, NULL, NULL);

    if (
        !g_file_set_contents(
            "/etc/default/grub", // FIXME: Ditto
            keyfile_raw + strlen("[Bodged]\n"),
            -1,
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
        return FALSE;
    }

    // Set plymouth theme
    //
    WINTC_LOG_DEBUG("wsetupx: attempting to set plymouth theme");

    if (
        !wintc_launch_command_sync(
            "plymouth-set-default-theme bootvid --rebuild-initrd",
            NULL,
            NULL,
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
        return FALSE;
    }

    // Update Grub
    //
    WINTC_LOG_DEBUG("wsetupx: attempting to update grub");

    if (
        !wintc_launch_command_sync(
            "update-grub",
            NULL,
            NULL,
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
        return FALSE;
    }

    // Think we're all good to go!
    //
    return TRUE;
}

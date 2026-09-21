#include <errno.h>
#include <glib.h>
#include <unistd.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>
#include <wintc/setupapi.h>

#include "arm.h"
#include "phase.h"

#define WINTC_LAUNCH_ASSETS_DIR WINTC_ASSETS_DIR "/launch"

//
// PUBLIC FUNCTIONS
//
gboolean wintc_setup_arm_system(void)
{
    GError*         error    = NULL;
    WinTCInitSystem init_sys = wintc_init_system_get();

    //
    // To arm the system for setup, we do the following things in order:
    //     - Deploy the 'phase' file set to 1 (graphical mode)
    //     - Deploy the init service/script for launching setup during startup
    //     - Specify the 'splash' parameter to Linux in Grub
    //     - Set the plymouth boot screen, generate initrd
    //     - Update grub
    //

    // Deploy phase file for booting into graphical mode
    //
    wintc_setup_phase_set(WINTC_SETUP_PHASE_GUIMODE);

    // Deploy the service for starting setup at boot
    //
    WINTC_LOG_DEBUG("wsetupx: attempting to link setup service");

    int res_svlink = 0;

    switch (init_sys)
    {
        case WINTC_INITSYS_SYSTEMD:
            res_svlink =
                symlink(
                    WINTC_LAUNCH_ASSETS_DIR "/wintc-launch.service",
                    "/etc/systemd/system/wintc-launch.service"
                );
            break;

        case WINTC_INITSYS_SYSVINIT:
            res_svlink =
                symlink(
                    WINTC_LAUNCH_ASSETS_DIR "/wintc-launch.sysv.sh",
                    "/etc/init.d/wintc-launch"
                );
            break;

        //
        // FIXME: Implement other inits here!
        //

        default:
            g_critical(
                "wsetupx: no implementation for init system: %s",
                wintc_init_system_get_name(init_sys)
            );
            return FALSE;
    }

    if (!(res_svlink == 0 || errno == EEXIST))
    {
        g_critical("wsetupx: unable to link setup service (err: %d)", errno);
        return FALSE;
    }

    // Enable it in systemd for startup
    //
    WINTC_LOG_DEBUG("wsetupx: attempting to enable setup service");

    if (
        !wintc_init_system_enable_service(
            "wintc-launch",
            WINTC_INITSYS_PRIORITY_BEFORE_DM,
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
        return FALSE;
    }

    // FIXME: Disabling LightDM here because no matter what I do I CANNOT GET
    //        WSETUPX TO START FIRST!!!
    //          (systemd problem)
    //
    if (
        init_sys == WINTC_INITSYS_SYSTEMD &&
        !wintc_init_system_disable_service(
            "lightdm",
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

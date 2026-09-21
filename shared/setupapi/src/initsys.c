#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>

#include "../public/initsys.h"

//
// FORWARD DECLARATIONS
//
WinTCInitSystem wintc_init_system_probe(void);

//
// PUBLIC FUNCTIONS
//
gboolean wintc_init_system_disable_service(
    const gchar* service_name,
    GError**     error
)
{
    WinTCInitSystem init_sys = wintc_init_system_get();

    gchar*   cmd;
    gboolean ret;

    switch (init_sys)
    {
        case WINTC_INITSYS_SYSTEMD:
        {
            cmd =
                g_strdup_printf(
                    "systemctl disable %s",
                    service_name
                );

            break;
        }

        case WINTC_INITSYS_SYSVINIT:
        {
            cmd =
                g_strdup_printf(
                    "update-rc.d %s disable",
                    service_name
                );

            break;
        }

        default:
            g_set_error(
                error,
                WINTC_GENERAL_ERROR,
                WINTC_GENERAL_ERROR_NOTIMPL,
                "Unknown init system: %s",
                wintc_init_system_get_name(init_sys)
            );

            return FALSE;
    }

    ret =
        wintc_launch_command_sync(
            cmd,
            NULL,
            NULL,
            error
        );

    g_free(cmd);

    return ret;
}

gboolean wintc_init_system_enable_service(
    const gchar*            service_name,
    WinTCInitSystemPriority priority,
    GError**                error
)
{
    WinTCInitSystem init_sys = wintc_init_system_get();

    gchar*   cmd;
    gboolean ret;

    switch (init_sys)
    {
        case WINTC_INITSYS_SYSTEMD:
        {
            cmd =
                g_strdup_printf(
                    "systemctl enable %s",
                    service_name
                );

            break;
        }

        case WINTC_INITSYS_SYSVINIT:
        {
            const gchar* priority_str = "";

            switch (priority)
            {
                case WINTC_INITSYS_PRIORITY_BEFORE_DM:
                    priority_str = "15";
                    break;
            }

            cmd =
                g_strdup_printf(
                    "update-rc.d %s defaults %s",
                    service_name,
                    priority_str
                );

            break;
        }

        default:
            g_set_error(
                error,
                WINTC_GENERAL_ERROR,
                WINTC_GENERAL_ERROR_NOTIMPL,
                "Unknown init system: %s",
                wintc_init_system_get_name(init_sys)
            );

            return FALSE;
    }

    ret =
        wintc_launch_command_sync(
            cmd,
            NULL,
            NULL,
            error
        );

    g_free(cmd);

    return ret;
}

WinTCInitSystem wintc_init_system_get(void)
{
    static WinTCInitSystem s_init_sys = WINTC_INITSYS_UNKNOWN;

    if (s_init_sys != WINTC_INITSYS_UNKNOWN)
    {
        return s_init_sys;
    }

    s_init_sys = wintc_init_system_probe();

    return s_init_sys;
}

const gchar* wintc_init_system_get_name(
    WinTCInitSystem init_sys
)
{
    switch (init_sys)
    {
        case WINTC_INITSYS_SYSTEMD:
            return "systemd";

        case WINTC_INITSYS_RUNIT:
            return "runit";

        case WINTC_INITSYS_UPSTART:
            return "Upstart";

        case WINTC_INITSYS_SYSVINIT:
            return "Sys-V";

        case WINTC_INITSYS_OPENRC:
            return "OpenRC";

        case WINTC_INITSYS_UNKNOWN:
        default:
            return "Unknown";
    }
}

//
// PRIVATE FUNCTIONS
//
WinTCInitSystem wintc_init_system_probe(void)
{
    // Simple path checks
    //
    if (g_file_test("/run/systemd/system", G_FILE_TEST_IS_DIR))
    {
        return WINTC_INITSYS_SYSTEMD;
    }

    if (g_file_test("/run/openrc", G_FILE_TEST_IS_DIR))
    {
        return WINTC_INITSYS_OPENRC;
    }

    if (g_file_test("/run/upstart", G_FILE_TEST_IS_DIR))
    {
        return WINTC_INITSYS_UPSTART;
    }

    // Check PID 1
    //
    gchar* pid1_cmd;

    if (g_file_get_contents("/proc/1/comm", &pid1_cmd, NULL, NULL))
    {
        WinTCInitSystem ret = WINTC_INITSYS_UNKNOWN;

        g_strstrip(pid1_cmd);

        if (g_strcmp0(pid1_cmd, "runit-init") == 0)
        {
            ret = WINTC_INITSYS_RUNIT;
        }
        else if (g_strcmp0(pid1_cmd, "init") == 0)
        {
            ret = WINTC_INITSYS_SYSVINIT;
        }

        g_free(pid1_cmd);

        if (ret != WINTC_INITSYS_UNKNOWN)
        {
            return ret;
        }
    }

    // Don't know what it is! Could potentially be more probing possible, or
    // it's something we genuinely don't know about
    //
    return WINTC_INITSYS_UNKNOWN;
}

#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/initsys.h"

//
// FORWARD DECLARATIONS
//
WinTCInitSystem wintc_probe_init_system(void);

//
// PUBLIC FUNCTIONS
//
WinTCInitSystem wintc_get_init_system(void)
{
    static WinTCInitSystem s_init_sys = WINTC_INITSYS_UNKNOWN;

    if (s_init_sys != WINTC_INITSYS_UNKNOWN)
    {
        return s_init_sys;
    }

    s_init_sys = wintc_probe_init_system();

    return s_init_sys;
}

const gchar* wintc_get_init_system_name(
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
WinTCInitSystem wintc_probe_init_system(void)
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

        if (g_strcmp0(pid1_cmd, "runit-init"))
        {
            ret = WINTC_INITSYS_RUNIT;
        }
        else if (g_strcmp0(pid1_cmd, "init"))
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

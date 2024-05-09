#include <glib.h>
#include <wintc/comgtk.h>

#include "testreg-skel.h"

//
// FORWARD DECLARATIONS
//
static void on_name_acquired(
    GDBusConnection* connection,
    const gchar*     name,
    gpointer         user_data
);

static gboolean on_handle_testing(
    WinTCTestregGDBUS*     iface,
    GDBusMethodInvocation* invocation,
    const gchar*           szinput,
    gpointer               user_data
);

//
// ENTRY POINT
//
int main()
{
    GMainLoop* loop;

    loop = g_main_loop_new(NULL, FALSE);

    g_bus_own_name(
        G_BUS_TYPE_SESSION,
        "uk.oddmatics.wintc.testreg",
        G_BUS_NAME_OWNER_FLAGS_NONE,
        NULL,
        on_name_acquired,
        NULL,
        NULL,
        NULL
    );

    g_main_loop_run(loop);

    return 0;
}

//
// PRIVATE FUNCTIONS
//
static void on_name_acquired(
    GDBusConnection* connection,
    WINTC_UNUSED(const gchar* name),
    WINTC_UNUSED(gpointer     user_data)
)
{
    GError*            error = NULL;
    WinTCTestregGDBUS* iface = win_tc_testreg_gdbus_skeleton_new();

    g_signal_connect(
        iface,
        "handle-testing",
        G_CALLBACK(on_handle_testing),
        NULL
    );

    if (
        !g_dbus_interface_skeleton_export(
            G_DBUS_INTERFACE_SKELETON(iface),
            connection,
            "/uk/oddmatics/wintc/testreg/GDBUS",
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
    }
}

static gboolean on_handle_testing(
    WinTCTestregGDBUS*     iface,
    GDBusMethodInvocation* invocation,
    const gchar*           szinput,
    WINTC_UNUSED(gpointer user_data)
)
{
    gchar* szoutput = g_strdup_printf("GDBUS IN: %s", szinput);

    win_tc_testreg_gdbus_complete_testing(
        iface,
        invocation,
        szoutput
    );

    g_message("%s", szoutput);
    g_free(szoutput);

    return TRUE;
}

#include <glib.h>
#include <wintc/comgtk.h>

#include "testreg-skel.h"

//
// ENTRY POINT
//
int main()
{
    GError*            error = NULL;
    WinTCTestregGDBUS* proxy;
    gchar*             szoutput;

    proxy =
        win_tc_testreg_gdbus_proxy_new_for_bus_sync(
            G_BUS_TYPE_SESSION,
            G_DBUS_PROXY_FLAGS_NONE,
            "uk.oddmatics.wintc.testreg",
            "/uk/oddmatics/wintc/testreg/GDBUS",
            NULL,
            &error
        );

    if (!proxy)
    {
        wintc_log_error_and_clear(&error);
        return 1;
    }

    win_tc_testreg_gdbus_call_testing_sync(
        proxy,
        "hello!",
        &szoutput,
        NULL,
        &error
    );

    if (error)
    {
        wintc_log_error_and_clear(&error);
        return 1;
    }

    g_message("Client got: %s", szoutput);

    g_object_unref(proxy);
    return 0;
}

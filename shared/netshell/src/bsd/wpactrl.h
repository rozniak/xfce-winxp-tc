#ifndef __NET_BSD_WPACTRL_H__
#define __NET_BSD_WPACTRL_H__

#include <gio/gio.h>
#include <glib.h>

//
// wpa_supplicant's control interface - the same text protocol wpa_cli uses,
// over the UNIX socket in the ctrl_interface directory
//

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_WPA_MONITOR (wintc_wpa_monitor_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCWpaMonitor,
    wintc_wpa_monitor,
    WINTC,
    WPA_MONITOR,
    GObject
)

//
// PUBLIC FUNCTIONS
//

// Requests block until wpa_supplicant replies, they each use their own socket
// so they can be made from worker threads
//
gchar* wintc_wpa_ctrl_request(
    const gchar* ctrl_path,
    const gchar* cmd,
    GError**     error
);
gboolean wintc_wpa_ctrl_request_ok(
    const gchar* ctrl_path,
    const gchar* cmd,
    GError**     error
);

// The monitor receives the unsolicited events (CTRL-EVENT-CONNECTED and so
// on) through the "event" signal, "closed" is emitted when wpa_supplicant
// goes away
//
WinTCWpaMonitor* wintc_wpa_monitor_new(
    const gchar* ctrl_path,
    GError**     error
);

#endif

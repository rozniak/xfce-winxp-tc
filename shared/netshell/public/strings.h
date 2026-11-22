#ifndef __NETSHELL_STRINGS_H__
#define __NETSHELL_STRINGS_H__

#include "device.h"

//
// PUBLIC FUNCTIONS
//
gchar* wintc_net_get_connection_name(
    WinTCNetDeviceKind kind,
    guint              index
);

gchar* wintc_net_prefix_to_netmask(
    gint prefix
);
gchar* wintc_net_speed_to_text(
    guint32 kbps
);
const gchar* wintc_net_state_to_text(
    WinTCNetDeviceKind kind,
    WinTCNetState      state
);

gint wintc_wlan_signal_to_bars(
    gint signal
);
const gchar* wintc_wlan_signal_to_text(
    gint signal
);
gchar* wintc_wlan_security_to_text(
    WinTCWlanSecurity security
);

#endif

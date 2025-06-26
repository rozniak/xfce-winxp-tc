#ifndef __XFSM_H__
#define __XFSM_H__

#include <glib.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_SMSS_XFSM_HOST (wintc_smss_xfsm_host_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCSmssXfsmHost,
    wintc_smss_xfsm_host,
    WINTC,
    SMSS_XFSM_HOST,
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCSmssXfsmHost* wintc_smss_xfsm_host_new(void);

gboolean wintc_smss_xfsm_host_is_valid(
    WinTCSmssXfsmHost* host
);

#endif

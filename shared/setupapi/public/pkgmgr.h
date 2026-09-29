#ifndef __SETUPAPI_PKGMGR_H__
#define __SETUPAPI_PKGMGR_H__

#include <glib.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_PKG_SESSION (wintc_pkg_session_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCPkgSession,
    wintc_pkg_session,
    WINTC,
    PKG_SESSION,
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCPkgSession* wintc_pkg_session_new(
    GList* packages
);

void wintc_pkg_session_begin(
    WinTCPkgSession* session
);
gboolean wintc_pkg_session_get_successful(
    WinTCPkgSession* session,
    GError**         error
);

#endif

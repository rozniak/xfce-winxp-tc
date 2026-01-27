#ifndef __SETUPAPI_H__
#define __SETUPAPI_H__

#include <glib.h>

//
// PUBLIC DEFINES
//
#define WINTC_SETUP_ACT_ROOT_DIR "/var/tmp/.wintc-setup"

//
// PUBLIC TYPEDEFS
//
typedef void (*WinTCSetupActErrorCallback) (
    GError** error,
    gpointer user_data
);
typedef void (*WinTCSetupActProgressCallback) (
    gdouble  progress,
    gpointer user_data
);

//
// PUBLIC CONSTANTS
//
extern gchar* WINTC_SETUP_ACT_PKG_PATH;

//
// PUBLIC FUNCTIONS
//
gboolean wintc_setup_act_init(void);
gboolean wintc_setup_act_install_packages(
    GList*                        list_packages,
    WinTCSetupActErrorCallback    error_callback,
    WinTCSetupActProgressCallback progress_callback,
    gpointer                      user_data,
    GError**                      error
);

#endif

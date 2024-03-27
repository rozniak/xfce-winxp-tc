#ifndef __SHELL_CPL_H__
#define __SHELL_CPL_H__

#include <glib.h>

//
// PUBLIC STRUCTURES
//
typedef struct _WinTCShCplApplet
{
    gchar* display_name;
    gchar* comment;
    gchar* exec;
    gchar* icon_name;
} WinTCShCplApplet;

//
// PUBLIC FUNCTIONS
//
GSList* wintc_sh_cpl_applet_get_all(void);

gboolean wintc_sh_cpl_applet_is_executable(
    WinTCShCplApplet* applet
);

void wintc_sh_cpl_applet_free(
    WinTCShCplApplet* applet
);

#endif

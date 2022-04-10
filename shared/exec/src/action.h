#ifndef __ACTIONS_H__
#define __ACTIONS_H__

#include <glib.h>

typedef enum
{
    WINTC_ACTION_MYDOCS,
    WINTC_ACTION_MYRECENTS,
    WINTC_ACTION_MYPICS,
    WINTC_ACTION_MYMUSIC,
    WINTC_ACTION_MYCOMP,
    WINTC_ACTION_CONTROL,
    WINTC_ACTION_MIMEMGMT,
    WINTC_ACTION_CONNECTTO,
    WINTC_ACTION_PRINTERS,
    WINTC_ACTION_HELP,
    WINTC_ACTION_SEARCH,
    WINTC_ACTION_RUN,
    WINTC_ACTION_LOGOFF,
    WINTC_ACTION_SHUTDOWN
} WinTCAction;

gboolean wintc_launch_action(
    WinTCAction action_id,
    GError**    out_error
);

#endif

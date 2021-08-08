#ifndef __ACTION_H__
#define __ACTION_H__

#include <glib.h>

#define XP_ACTION_MYDOCS    1
#define XP_ACTION_MYRECENTS 2
#define XP_ACTION_MYPICS    3
#define XP_ACTION_MYMUSIC   4
#define XP_ACTION_MYCOMP    5
#define XP_ACTION_CONTROL   6
#define XP_ACTION_MIMEMGMT  7
#define XP_ACTION_CONNECTTO 8
#define XP_ACTION_PRINTERS  9
#define XP_ACTION_HELP      10
#define XP_ACTION_SEARCH    11
#define XP_ACTION_RUN       12
#define XP_ACTION_LOGOFF    13
#define XP_ACTION_SHUTDOWN  14

void launch_action(
    gint action_id
);

void launch_command(
    gchar** command
);

#endif

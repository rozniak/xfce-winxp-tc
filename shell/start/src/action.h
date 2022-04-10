#ifndef __ACTION_H__
#define __ACTION_H__

#include <glib.h>
#include <wintc-exec.h>

void launch_action(
    WinTCAction action_id
);

void launch_command(
    const gchar* command
);

#endif

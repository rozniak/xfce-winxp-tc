/** @file */

#ifndef __EXEC_ACTIONS_H__
#define __EXEC_ACTIONS_H__

#include <glib.h>

//
// PUBLIC ENUMS
//

/**
 * Specifies actions common to WinTC that can be launched.
 */
typedef enum
{
    /** Opens the 'My Documents' folder. */
    WINTC_ACTION_MYDOCS,
    /** Opens the 'My Recent Documents' view. */
    WINTC_ACTION_MYRECENTS,
    /** Opens the 'My Pictures' folder. */
    WINTC_ACTION_MYPICS,
    /** Opens the 'My Music' folder. */
    WINTC_ACTION_MYMUSIC,
    /** Opens the 'My Computer' folder. */
    WINTC_ACTION_MYCOMP,
    /** Opens Control Panel. */
    WINTC_ACTION_CONTROL,
    /** Opens the program for managing MIME types. */
    WINTC_ACTION_MIMEMGMT,
    /** Opens the program for managing connections. */
    WINTC_ACTION_CONNECTTO,
    /** Opens the program for managing printers. */
    WINTC_ACTION_PRINTERS,
    /** Opens the program for getting help.*/
    WINTC_ACTION_HELP,
    /** Opens the program for performing searches. */
    WINTC_ACTION_SEARCH,
    /** Opens the 'Run' dialog. */
    WINTC_ACTION_RUN,
    /** Opens the user actions dialog. */
    WINTC_ACTION_LOGOFF,
    /** Opens the power options dialog. */
    WINTC_ACTION_SHUTDOWN
} WinTCAction;

//
// PUBLIC FUNCTIONS
//

/**
 * Launches the specified action.
 *
 * @param action_id The action.
 * @param out_error Storage location for any error that occurred.
 * @return True if the action was launched successfully.
 */
gboolean wintc_launch_action(
    WinTCAction action_id,
    GError**    error
);

#endif

/**
 * @file
 *
 * This is the logon session object, which is the main mechanism in GINA for
 * providing user logon.
 *
 * The underlying provider may be asynchonous so the functionality of the logon
 * session object works via polling and signals.
 *
 * Straight forward usage is as follows:
 *   - Create the logon session object
 *   - Connect to the 'attempt-complete' signal - see attempt_complete_cb below
 *   - Call wintc_gina_logon_session_establish to initiate connection to the
 *     logon provider
 *   - Poll wintc_gina_logon_session_is_available to determine when it is
 *     ready
 *   - Call wintc_gina_logon_session_try_logon with user credentials
 *   - The 'attempt-complete' signal is raised:
 *     - If successful, call wintc_gina_logon_session_finish to complete logon
 *     - If unsuccessful, poll wintc_gina_logon_session_is_available again
 *
 * The callback for the 'attempt-complete' signal is as follows:
 *
 * void attempt_complete_cb(
 *     WinTCGinaLogonSession* logon_session,
 *     WinTCGinaResponse      response,
 *     gpointer               user_data
 * )
 */

#ifndef __MSGINA_LOGON_H__
#define __MSGINA_LOGON_H__

#include <glib.h>

//
// GLIB BOILERPLATE
//
#define WINTC_TYPE_GINA_LOGON_SESSION (wintc_gina_logon_session_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCGinaLogonSession,
    wintc_gina_logon_session,
    WINTC,
    GINA_LOGON_SESSION,
    GObject
)

//
// PUBLIC FUNCTIONS
//

/**
 * Creates a new instance of WinTCGinaLogonSession.
 *
 * @return The new WinTCGinaLogonSession instance.
 */
WinTCGinaLogonSession* wintc_gina_logon_session_new(void);

/**
 * Establishes a connection to the underlying logon provider.
 *
 * @param logon_session The logon session.
 * @return True if a connection was successfully established.
 */
gboolean wintc_gina_logon_session_establish(
    WinTCGinaLogonSession* logon_session
);

/**
 * Requests that the logon session be completed, following successful user
 * authentication.
 *
 * @param logon_session The logon session.
 * @param error         Storage location for any error that occurred.
 */
gboolean wintc_gina_logon_session_finish(
    WinTCGinaLogonSession* logon_session,
    GError**               error
);

/**
 * Gets the session that should be started upon successful logon.
 *
 * @param logon_session The logon session.
 * @return The name of the session.
 */
const gchar* wintc_gina_logon_session_get_preferred_session(
    WinTCGinaLogonSession* logon_screen
);

/**
 * Determines whether the logon session is ready for the user authentication
 * stage.
 *
 * @param logon_session The logon session.
 * @return True if user authentication is ready.
 */
gboolean wintc_gina_logon_session_is_available(
    WinTCGinaLogonSession* logon_session
);

/**
 * Sets the session that should be started upon successful logon.
 *
 * @param logon_session The logon session.
 * @param session       The name of the session.
 */
void wintc_gina_logon_session_set_preferred_session(
    WinTCGinaLogonSession* logon_session,
    const gchar*           session
);

/**
 * Attempt to authenticate as a user with their logon credentials.
 *
 * @param logon_session The logon session.
 * @param username      The account username.
 * @param password      The account password.
 */
void wintc_gina_logon_session_try_logon(
    WinTCGinaLogonSession* logon_session,
    const gchar* username,
    const gchar* password
);

#endif


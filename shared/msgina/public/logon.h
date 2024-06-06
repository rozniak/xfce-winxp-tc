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
typedef struct _WinTCGinaLogonSessionClass WinTCGinaLogonSessionClass;

/**
 * Represents a logon session for providing user authentication.
 */
typedef struct _WinTCGinaLogonSession WinTCGinaLogonSession;

#define WINTC_TYPE_GINA_LOGON_SESSION            (wintc_gina_logon_session_get_type())
#define WINTC_GINA_LOGON_SESSION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_GINA_LOGON_SESSION, WinTCGinaLogonSession))
#define WINTC_GINA_LOGON_SESSION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_GINA_LOGON_SESSION, WinTCGinaLogonSession))
#define IS_WINTC_GINA_LOGON_SESSION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_GINA_LOGON_SESSION))
#define IS_WINTC_GINA_LOGON_SESSION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_GINA_LOGON_SESSION))
#define WINTC_GINA_LOGON_SESSION_GET_CLASS(obj)  (G_TYPE_CHECK_INSTANCE_GET_CLASS((obj), WINTC_TYPE_GINA_LOGON_SESSION)

GType wintc_gina_logon_session_get_type(void) G_GNUC_CONST;

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


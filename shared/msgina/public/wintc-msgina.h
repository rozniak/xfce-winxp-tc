#ifndef __WINTC_MSGINA_H__
#define __WINTC_MSGINA_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// Authentication window
//
typedef struct _WinTCGinaAuthWindowPrivate WinTCGinaAuthWindowPrivate;
typedef struct _WinTCGinaAuthWindowClass   WinTCGinaAuthWindowClass;
typedef struct _WinTCGinaAuthWindow        WinTCGinaAuthWindow;

#define TYPE_WINTC_GINA_AUTH_WINDOW            (wintc_gina_auth_window_get_type())
#define WINTC_GINA_AUTH_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_WINTC_GINA_AUTH_WINDOW, WinTCGinaAuthWindow))
#define WINTC_GINA_AUTH_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_WINTC_GINA_AUTH_WINDOW, WinTCGinaAuthWindow))
#define IS_WINTC_GINA_AUTH_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_WINTC_GINA_AUTH_WINDOW))
#define IS_WINTC_GINA_AUTH_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_WINTC_GINA_AUTH_WINDOW))
#define WINTC_GINA_WINDOW_GET_CLASS(obj)       (G_TYPE_CHECK_INSTANCE_GET_CLASS((obj), TYPE_WINTC_GINA_AUTH_WINDOW))

GType wintc_gina_auth_window_get_type(void) G_GNUC_CONST;

GtkWidget* wintc_gina_auth_window_new(void);

//
// Challenge auth
//
typedef enum
{
    WINTC_GINA_RESPONSE_OKAY    = 0,
    WINTC_GINA_RESPONSE_FAIL    = 1
} WinTCGinaResponse;

//
// Logon session
//
typedef struct _WinTCGinaLogonSessionClass WinTCGinaLogonSessionClass;
typedef struct _WinTCGinaLogonSession      WinTCGinaLogonSession;

#define TYPE_WINTC_GINA_LOGON_SESSION            (wintc_gina_logon_session_get_type())
#define WINTC_GINA_LOGON_SESSION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_WINTC_GINA_LOGON_SESSION, WinTCGinaLogonSession))
#define WINTC_GINA_LOGON_SESSION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_WINTC_GINA_LOGON_SESSION, WinTCGinaLogonSession))
#define IS_WINTC_GINA_LOGON_SESSION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_WINTC_GINA_LOGON_SESSION))
#define IS_WINTC_GINA_LOGON_SESSION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_WINTC_GINA_LOGON_SESSION))
#define WINTC_GINA_LOGON_SESSION_GET_CLASS(obj)  (G_TYPE_CHECK_INSTANCE_GET_CLASS((obj), TYPE_WINTC_GINA_LOGON_SESSION)

GType wintc_gina_logon_session_get_type(void) G_GNUC_CONST;

WinTCGinaLogonSession* wintc_gina_logon_session_new(void);

gboolean wintc_gina_logon_session_establish(
    WinTCGinaLogonSession* logon_session
);
void wintc_gina_logon_session_finish(
    WinTCGinaLogonSession* logon_session
);
gboolean wintc_gina_logon_session_is_available(
    WinTCGinaLogonSession* logon_session
);
void wintc_gina_logon_session_try_logon(
    WinTCGinaLogonSession* logon_session,
    const gchar* username,
    const gchar* password
);

//
// State
//
typedef enum
{
    WINTC_GINA_STATE_NONE = 0,
    WINTC_GINA_STATE_STARTING,
    WINTC_GINA_STATE_PROMPT,
    WINTC_GINA_STATE_LAUNCHING
} WinTCGinaState;

#endif
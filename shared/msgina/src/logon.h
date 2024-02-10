#ifndef __LOGON_H__
#define __LOGON_H__

#include <glib.h>

//
// GLIB BOILERPLATE
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

//
// PUBLIC FUNCTIONS
//
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

#endif


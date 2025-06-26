#include <glib.h>
#include <lightdm.h>
#include <wintc/comgtk.h>

#include "../public/challenge.h"
#include "../public/error.h"
#include "../public/logon.h"

//
// PRIVATE ENUMS
//
enum
{
    SIGNAL_ATTEMPT_COMPLETE = 0,
    N_SIGNALS
};

enum
{
    PROP_NULL,
    PROP_PREFERRED_SESSION,
    N_PROPERTIES
};

//
// FORWARD DECLARATIONS
//
static void wintc_gina_logon_session_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_gina_logon_session_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void wintc_gina_logon_session_reset_auth_state(
    WinTCGinaLogonSession* logon_session
);

static gboolean lightdm_session_exists(
    const gchar* session_name
);

static void on_greeter_authentication_complete(
    LightDMGreeter* greeter,
    gpointer        user_data
);
static void on_greeter_show_prompt(
    LightDMGreeter* greeter,
    gchar*          text,
    WINTC_UNUSED(LightDMPromptType type),
    gpointer        user_data
);

//
// STATIC DATA
//
static GParamSpec* wintc_gina_logon_session_properties[N_PROPERTIES] = { 0 };
static gint        wintc_gina_logon_session_signals[N_SIGNALS]       = { 0 };

//
// GLIB OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCGinaLogonSession
{
    GObject __parent__;

    gboolean        auth_complete;
    gboolean        auth_ready;
    LightDMGreeter* greeter;
    gchar*          preferred_session;
    gchar*          response_pwd;
};

//
// GLIB TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCGinaLogonSession,
    wintc_gina_logon_session,
    G_TYPE_OBJECT
)

static void wintc_gina_logon_session_class_init(
    WinTCGinaLogonSessionClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = wintc_gina_logon_session_get_property;
    object_class->set_property = wintc_gina_logon_session_set_property;

    wintc_gina_logon_session_signals[SIGNAL_ATTEMPT_COMPLETE] =
        g_signal_new(
            "attempt-complete",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__INT,
            G_TYPE_NONE,
            1,
            G_TYPE_INT
        );

    wintc_gina_logon_session_properties[PROP_PREFERRED_SESSION] =
        g_param_spec_string(
            "preferred-session",
            "PreferredSession",
            "The session that should be started upon successful logon.",
            NULL,
            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_gina_logon_session_properties
    );
}

static void wintc_gina_logon_session_init(
    WinTCGinaLogonSession* self
)
{
    self->auth_ready = FALSE;
    self->greeter    = NULL;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_gina_logon_session_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCGinaLogonSession* logon_session = WINTC_GINA_LOGON_SESSION(object);

    switch (prop_id)
    {
        case PROP_PREFERRED_SESSION:
            g_value_set_string(value, logon_session->preferred_session);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_gina_logon_session_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCGinaLogonSession* logon_session = WINTC_GINA_LOGON_SESSION(object);

    switch (prop_id)
    {
        case PROP_PREFERRED_SESSION:
            logon_session->preferred_session = g_value_dup_string(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;

    }
}

//
// PUBLIC FUNCTIONS
//
WinTCGinaLogonSession* wintc_gina_logon_session_new(void)
{
    return WINTC_GINA_LOGON_SESSION(
        g_object_new(
            WINTC_TYPE_GINA_LOGON_SESSION,
            NULL
        )
    );
}

gboolean wintc_gina_logon_session_establish(
    WinTCGinaLogonSession* logon_session
)
{
    WINTC_LOG_DEBUG("GINA - establish()");

    if (!logon_session->greeter)
    {
        WINTC_LOG_DEBUG("GINA - new lightdm greeter");

        logon_session->greeter = lightdm_greeter_new();

        g_signal_connect(
            logon_session->greeter,
            "authentication-complete",
            G_CALLBACK(on_greeter_authentication_complete),
            logon_session
        );
        g_signal_connect(
            logon_session->greeter,
            "show-prompt",
            G_CALLBACK(on_greeter_show_prompt),
            logon_session
        );
    }

    WINTC_LOG_DEBUG("GINA - lightdm connect sync");

    if (
        !lightdm_greeter_connect_to_daemon_sync(
            logon_session->greeter,
            NULL
        )
    )
    {
        g_critical("%s", "Failed to connect to LightDM daemon.");
        return FALSE;
    }

    // FIXME: Error handling
    //
    lightdm_greeter_authenticate(
        logon_session->greeter,
        NULL,
        NULL
    );

    return TRUE;
}

gboolean wintc_gina_logon_session_finish(
    WinTCGinaLogonSession* logon_session,
    GError**               error
)
{
    WINTC_SAFE_REF_CLEAR(error);

    WINTC_LOG_DEBUG("GINA - finish requested");

    if (!logon_session->auth_complete)
    {
        g_critical("%s", "GINA - Attempt to complete incomplete logon.");
        return FALSE;
    }

    // If there are no sessions, then we can't finish logging on
    //
    if (!lightdm_get_sessions())
    {
        // FIXME: This string will need localising, though there is no such
        //        string in Windows (or similar afaik) as this situation does
        //        not ever occur on Windows
        //
        g_set_error(
            error,
            WINTC_GINA_ERROR,
            WINTC_GINA_ERROR_NO_SESSIONS,
            "%s",
            "There are no available sessions on this system. "
            "Logon cannot be completed."
        );

        wintc_gina_logon_session_reset_auth_state(logon_session);

        return FALSE;
    }

    // Pull out preferred/default sessions and check they exist
    //
    const gchar* bak_session  = NULL;
    const gchar* def_session  = lightdm_greeter_get_default_session_hint(
                                    logon_session->greeter
                                );
    const gchar* pref_session = logon_session->preferred_session;

    if (!lightdm_session_exists(def_session))
    {
        def_session = NULL;
    }

    if (!lightdm_session_exists(pref_session))
    {
        pref_session = NULL;
    }

    // If there's no preferred sessions, try our own - otherwise fall back to
    // the first one in the list
    //
    if (!def_session && !pref_session)
    {
        // FIXME: Commented out for now, until WinTC session is ready for prime
        //        time
        //
        /**if (lightdm_session_exists("wintc"))
        {
            bak_session = "wintc";
        }
        else */if (lightdm_session_exists("xfce"))
        {
            bak_session = "xfce";
        }
        else
        {
            bak_session =
                lightdm_session_get_key(
                    (LightDMSession*) lightdm_get_sessions()->data
                );
        }
    }

    // Attempt to start one of the sessions
    //
    if (
        pref_session &&
        lightdm_greeter_start_session_sync(
            logon_session->greeter,
            pref_session,
            error
        )
    )
    {
        return TRUE;
    }
    else
    {
        wintc_log_error_and_clear(error);

        if (
            def_session &&
            lightdm_greeter_start_session_sync(
                logon_session->greeter,
                def_session,
                error
            )
        )
        {
            return TRUE;
        }
        else
        {
            if (
                lightdm_greeter_start_session_sync(
                    logon_session->greeter,
                    bak_session,
                    error
                )
            )
            {
                return TRUE;
            }
        }
    }

    wintc_gina_logon_session_reset_auth_state(logon_session);

    return FALSE;
}

const gchar* wintc_gina_logon_session_get_preferred_session(
    WinTCGinaLogonSession* logon_session
)
{
    return logon_session->preferred_session;
}

gboolean wintc_gina_logon_session_is_available(
    WinTCGinaLogonSession* logon_session
)
{
    WINTC_LOG_DEBUG("GINA - polled");

    return logon_session->auth_ready;
}

void wintc_gina_logon_session_set_preferred_session(
    WinTCGinaLogonSession* logon_session,
    const gchar*           session
)
{
    g_free(logon_session->preferred_session);
    logon_session->preferred_session = g_strdup(session);

    g_object_notify_by_pspec(
        G_OBJECT(logon_session),
        wintc_gina_logon_session_properties[PROP_PREFERRED_SESSION]
    );
}

void wintc_gina_logon_session_try_logon(
    WinTCGinaLogonSession* logon_session,
    const gchar* username,
    const gchar* password
)
{
    WINTC_LOG_DEBUG("GINA - logon attempt requested");

    if (logon_session->auth_complete)
    {
        g_warning("%s", "Attempt to logon a second time.");
        return;
    }

    logon_session->response_pwd = g_strdup(password);

    // FIXME: Error handling
    //
    lightdm_greeter_respond(
        logon_session->greeter,
        username,
        NULL
    );
}

//
// PRIVATE FUNCTIONS
//
static void wintc_gina_logon_session_reset_auth_state(
    WinTCGinaLogonSession* logon_session
)
{
    logon_session->auth_complete = FALSE;

    lightdm_greeter_authenticate(logon_session->greeter, NULL, NULL);
}

static gboolean lightdm_session_exists(
    const gchar* session_name
)
{
    if (!session_name)
    {
        return FALSE;
    }

    GList* avail_sessions = lightdm_get_sessions();

    for (GList* iter = avail_sessions; iter; iter = iter->next)
    {
        LightDMSession* session = (LightDMSession*) iter->data;

        if (g_strcmp0(session_name, lightdm_session_get_key(session)) == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

//
// CALLBACKS
//
static void on_greeter_authentication_complete(
    LightDMGreeter* greeter,
    gpointer        user_data
)
{
    WinTCGinaLogonSession* logon_session =
        WINTC_GINA_LOGON_SESSION(user_data);

    WINTC_LOG_DEBUG("GINA - lightdm auth complete");

    logon_session->auth_complete =
        lightdm_greeter_get_is_authenticated(greeter);

    g_signal_emit(
        logon_session,
        wintc_gina_logon_session_signals[SIGNAL_ATTEMPT_COMPLETE],
        0,
        logon_session->auth_complete ?
            WINTC_GINA_RESPONSE_OKAY :
            WINTC_GINA_RESPONSE_FAIL
    );

    // If the auth failed, have to restart
    // FIXME: Error handling
    //
    if (!logon_session->auth_complete)
    {
        wintc_gina_logon_session_reset_auth_state(logon_session);
    }
}

static void on_greeter_show_prompt(
    LightDMGreeter* greeter,
    gchar*          text,
    WINTC_UNUSED(LightDMPromptType type),
    gpointer        user_data
)
{
    WinTCGinaLogonSession* logon_session =
        WINTC_GINA_LOGON_SESSION(user_data);

    WINTC_LOG_DEBUG("GINA - lightdm prompt: %s", text);

    if (g_strcmp0(text, "login:") == 0)
    {
        logon_session->auth_ready = TRUE;
    }
    else if (g_strcmp0(text, "Password: ") == 0)
    {
        // FIXME: Error handling
        //
        lightdm_greeter_respond(
            greeter,
            logon_session->response_pwd,
            NULL
        );

        g_free(logon_session->response_pwd);
        logon_session->response_pwd = NULL;
    }
    else
    {
        g_critical("Unknown prompt: %s", text);
    }
}

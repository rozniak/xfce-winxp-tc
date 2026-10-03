#include <alpm.h>
#include <glib.h>
#include <wintc/comgtk.h>

#include "../../../public/pkgsesh.h"
#include "pkgcmn.h"

//
// PRIVATE ENUMS
//
enum
{
    SIGNAL_DONE,
    SIGNAL_PROGRESS,
    N_SIGNALS
};

enum
{
    PROP_NULL,
    PROP_PACKAGES,
    N_PROPERTIES
};

//
// FORWARD DECLARATIONS
//
static void wintc_pkg_session_dispose(
    GObject* object
);
static void wintc_pkg_session_finalize(
    GObject* object
);
static void wintc_pkg_session_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void cb_alpm_progress(
    gpointer        user_data,
    alpm_progress_t progress,
    const gchar*    pkg,
    gint            percent,
    size_t          howmany,
    size_t          current
);
static gpointer cb_alpm_thread_kickoff(
    gpointer user_data
);
static gboolean cb_idle_progress(
    gpointer user_data
);
static gboolean cb_idle_done(
    gpointer user_data
);

//
// STATIC DATA
//
static GParamSpec* wintc_pkg_session_properties[N_PROPERTIES] = { 0 };
static gint        wintc_pkg_session_signals[N_SIGNALS]       = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCPkgSession
{
    GObject __parent__;

    // State
    //
    GError*        error;
    alpm_handle_t* handle_alpm;
    GList*         list_alpm_pkgs;
    GList*         list_packages;
    gdouble        progress;
    GThread*       thread_alpm;

} WinTCPkgSession;

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCPkgSession,
    wintc_pkg_session,
    G_TYPE_OBJECT
)

static void wintc_pkg_session_class_init(
    WinTCPkgSessionClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose      = wintc_pkg_session_dispose;
    object_class->finalize     = wintc_pkg_session_finalize;
    object_class->set_property = wintc_pkg_session_set_property;

    wintc_pkg_session_signals[SIGNAL_DONE] =
        g_signal_new(
            "done",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0
        );
    wintc_pkg_session_signals[SIGNAL_PROGRESS] =
        g_signal_new(
            "progress",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__DOUBLE,
            G_TYPE_NONE,
            1,
            G_TYPE_DOUBLE
        );

    wintc_pkg_session_properties[PROP_PACKAGES] =
        g_param_spec_pointer(
            "packages",
            "Packages",
            "The packages to install.",
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_pkg_session_properties
    );
}

static void wintc_pkg_session_init(
    WinTCPkgSession* self
)
{
    self->handle_alpm =
        wintc_pkg_alpm_handle_get(&(self->error));

    if (!(self->handle_alpm))
    {
        return;
    }

    alpm_option_set_progresscb(
        self->handle_alpm,
        cb_alpm_progress,
        self
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_pkg_session_dispose(
    GObject* object
)
{
    WinTCPkgSession* session = WINTC_PKG_SESSION(object);

    g_clear_error(&(session->error));

    (G_OBJECT_CLASS(wintc_pkg_session_parent_class))
        ->dispose(object);
}

static void wintc_pkg_session_finalize(
    GObject* object
)
{
    WinTCPkgSession* session = WINTC_PKG_SESSION(object);

    if (session->handle_alpm)
    {
        if (session->list_alpm_pkgs)
        {
            g_clear_list(
                &(session->list_alpm_pkgs),
                NULL
            );

            alpm_trans_release(session->handle_alpm);
        }

        alpm_release(session->handle_alpm);
    }

    (G_OBJECT_CLASS(wintc_pkg_session_parent_class))
        ->finalize(object);
}

static void wintc_pkg_session_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCPkgSession* session = WINTC_PKG_SESSION(object);

    switch (prop_id)
    {
        case PROP_PACKAGES:
            session->list_packages = g_value_get_pointer(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
WinTCPkgSession* wintc_pkg_session_new(
    GList* packages
)
{
    return WINTC_PKG_SESSION(
        g_object_new(
            WINTC_TYPE_PKG_SESSION,
            "packages", packages,
            NULL
        )
    );
}

void wintc_pkg_session_begin(
    WinTCPkgSession* session
)
{
    // Init the transaction
    //
    if (alpm_trans_init(session->handle_alpm, ALPM_TRANS_FLAG_NEEDED) != 0)
    {
        wintc_pkg_alpm_error_set(
            &(session->error),
            alpm_errno(session->handle_alpm)
        );

        g_signal_emit(
            session,
            wintc_pkg_session_signals[SIGNAL_DONE],
            0
        );

        return;
    }

    // Try to load the packages from disk into the transaction
    //
    for (GList* iter = session->list_packages; iter; iter = iter->next)
    {
        const gchar* path = (gchar*) iter->data;

        alpm_pkg_t* package = NULL;

        if (
            alpm_pkg_load(
                session->handle_alpm,
                path,
                1,
                ALPM_SIG_USE_DEFAULT,
                &package
            ) != 0 ||
            alpm_add_pkg(
                session->handle_alpm,
                package
            ) != 0
        )
        {
            wintc_pkg_alpm_error_set(
                &(session->error),
                alpm_errno(session->handle_alpm)
            );

            if (package)
            {
                alpm_pkg_free(package);
            }

            g_signal_emit(
                session,
                wintc_pkg_session_signals[SIGNAL_DONE],
                0
            );

            return;
        }

        // All good, track the package
        //
        session->list_alpm_pkgs =
            g_list_prepend(session->list_alpm_pkgs, package);
    }

    // Kick off commit in new thread
    //
    session->thread_alpm =
        g_thread_new(
            "alpm-transaction",
            (GThreadFunc) cb_alpm_thread_kickoff,
            session
        );
}

gboolean wintc_pkg_session_get_successful(
    WinTCPkgSession* session,
    GError**         error
)
{
    if (!session->error)
    {
        return TRUE;
    }

    WINTC_SAFE_REF_SET(error, g_error_copy(session->error));

    return FALSE;
}

//
// CALLBACKS
//
static void cb_alpm_progress(
    gpointer user_data,
    WINTC_UNUSED(alpm_progress_t progress),
    WINTC_UNUSED(const gchar* pkg),
    gint     percent,
    size_t   howmany,
    size_t   current
)
{
    WinTCPkgSession* session = WINTC_PKG_SESSION(user_data);

    gdouble current0    = current - 1;
    gdouble per_trans   = 1.0f / howmany;
    gdouble real_pct    =
        (current0 * per_trans) + (per_trans * (percent / 100.0f));

    session->progress = real_pct;

    g_idle_add(
        (GSourceFunc) cb_idle_progress,
        session
    );
}

static gpointer cb_alpm_thread_kickoff(
    gpointer user_data
)
{
    WinTCPkgSession* session = WINTC_PKG_SESSION(user_data);

    if (alpm_trans_commit(session->handle_alpm, NULL) != 0)
    {
        wintc_pkg_alpm_error_set(
            &(session->error),
            alpm_errno(session->handle_alpm)
        );
    }

    g_idle_add(
        (GSourceFunc) cb_idle_done,
        session
    );

    return NULL;
}

static gboolean cb_idle_progress(
    gpointer user_data
)
{
    WinTCPkgSession* session = WINTC_PKG_SESSION(user_data);

    g_signal_emit(
        session,
        wintc_pkg_session_signals[SIGNAL_PROGRESS],
        0,
        G_TYPE_DOUBLE,
        session->progress
    );

    return G_SOURCE_REMOVE;
}

static gboolean cb_idle_done(
    gpointer user_data
)
{
    WinTCPkgSession* session = WINTC_PKG_SESSION(user_data);

    g_signal_emit(
        session,
        wintc_pkg_session_signals[SIGNAL_DONE],
        0
    );

    return G_SOURCE_REMOVE;
}

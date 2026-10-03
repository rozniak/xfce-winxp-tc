#include <gio/gunixinputstream.h>
#include <glib.h>
#include <wintc/comgtk.h>

#include "../../public/error.h"
#include "../../public/pkgsesh.h"

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
static void wintc_pkg_session_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void cb_read_line_pkgmgr(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);

//
// STATIC DATA
//
static GParamSpec* wintc_pkg_session_properties[N_PROPERTIES] = { 0 };
static gint        wintc_pkg_session_signals[N_SIGNALS]       = { 0 };

static gchar* S_CMD_INSTALL[] = {
    WINTC_RT_PREFIX "/bin/pkexec",
    WINTC_RT_PREFIX "/bin/wintc-setupapi-exec",
    "-i",
    NULL
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCPkgSession
{
    GObject __parent__;

    // State
    //
    GError* error;
    GList*  list_packages;
    GPid    pid;
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
    WINTC_UNUSED(WinTCPkgSession* self)
) {}

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
    // Set up command line
    //
    guint len_cmd      = g_strv_length(S_CMD_INSTALL);
    guint len_packages = g_list_length(session->list_packages);

    gchar** argv = g_malloc0(sizeof(gchar*) * (len_cmd + len_packages + 1));

    gint   i;
    GList* iter;

    memcpy(argv, S_CMD_INSTALL, sizeof(gchar*) * len_cmd);

    for (
        i = len_cmd, iter = session->list_packages;
        iter;
        i++, iter = iter->next
    )
    {
        argv[i] = iter->data;
    }

    // Spawn process
    //
    gint fd_out = 1;

    gboolean success =
        g_spawn_async_with_pipes(
            NULL,
            argv,
            NULL,
            G_SPAWN_DO_NOT_REAP_CHILD,
            NULL,
            NULL,
            &(session->pid),
            NULL,
            &fd_out,
            NULL,
            &(session->error)
        );

    g_free(argv);

    if (!success)
    {
        g_signal_emit(
            session,
            wintc_pkg_session_signals[SIGNAL_DONE],
            0
        );

        return;
    }

    // Kick off read
    //
    GInputStream*     fd_stream = g_unix_input_stream_new(fd_out, TRUE);
    GDataInputStream* stream    = g_data_input_stream_new(fd_stream);

    g_object_unref(fd_stream);

    g_data_input_stream_read_line_async(
        stream,
        G_PRIORITY_DEFAULT,
        NULL,
        cb_read_line_pkgmgr,
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
static void cb_read_line_pkgmgr(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
)
{
    GDataInputStream* stream  = G_DATA_INPUT_STREAM(source_object);
    WinTCPkgSession*  session = WINTC_PKG_SESSION(user_data);

    gchar* line =
        g_data_input_stream_read_line_finish(
            stream,
            res,
            NULL,
            &(session->error)
        );

    // Finished?
    //
    if (!line)
    {
        g_signal_emit(
            session,
            wintc_pkg_session_signals[SIGNAL_DONE],
            0
        );

        g_input_stream_close(G_INPUT_STREAM(stream), NULL, NULL);
        g_object_unref(stream);

        g_spawn_close_pid(session->pid);

        return;
    }

    // Deal with helper output
    //
    if (strstr(line, "STAT "))
    {
        g_signal_emit(
            session,
            wintc_pkg_session_signals[SIGNAL_PROGRESS],
            0,
            G_TYPE_DOUBLE,
            strtod(line + 5, NULL)
        );
    }
    else if (strstr(line, "ERR "))
    {
        g_set_error(
            &(session->error),
            WINTC_SETUPAPI_ERROR,
            WINTC_SETUPAPI_ERROR_FAILED,
            "%s",
            line + 4
        );
    }

    g_free(line);

    // Wait for next line
    //
    g_data_input_stream_read_line_async(
        stream,
        G_PRIORITY_DEFAULT,
        NULL,
        cb_read_line_pkgmgr,
        session
    );
}

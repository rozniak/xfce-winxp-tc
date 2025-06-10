#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/msgina.h>

#include "../window.h"
#include "ui.h"
#include "userlist.h"

#define DELAY_SECONDS_AT_LEAST 2
#define DELAY_SECONDS_POLL     1

//
// PRIVATE ENUMS
//
enum
{
    PROP_LOGON_SESSION = 1
};

//
// FORWARD DECLARATIONS
//
static void wintc_welcome_ui_igina_auth_ui_interface_init(
    WinTCIGinaAuthUIInterface* iface
);

static void wintc_welcome_ui_constructed(
    GObject* object
);
static void wintc_welcome_ui_dispose(
    GObject* object
);
static void wintc_welcome_ui_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_welcome_ui_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void wintc_welcome_ui_add(
    GtkContainer* container,
    GtkWidget*    widget
);
static void wintc_welcome_ui_change_state(
    WinTCWelcomeUI* welcome_ui,
    WinTCGinaState  next_state
);

static void on_logon_session_attempt_complete(
    WinTCGinaLogonSession* logon_session,
    WinTCGinaResponse      response,
    gpointer               user_data
);

static gboolean on_timeout_delay_done(
    gpointer user_data
);
static gboolean on_timeout_poll_ready(
    gpointer user_data
);

static void wintc_welcome_ui_internal_add(
    WinTCWelcomeUI* welcome_ui,
    GtkWidget*      widget
);
static void on_self_realized(
    GtkWidget* self,
    WINTC_UNUSED(gpointer user_data)
);
static void wintc_welcome_ui_remove(
    WINTC_UNUSED(GtkContainer* container),
    WINTC_UNUSED(GtkWidget*    widget)
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCWelcomeUIClass
{
    GtkContainerClass __parent__;
};

struct _WinTCWelcomeUI
{
    GtkContainer __parent__;

    GSList* child_widgets;

    // Graphic resources
    //

    // Logo resources
    //


    // UI
    //
    GtkWidget* box_container;


    // State
    //
    WinTCGinaState         current_state;
    WinTCGinaLogonSession* logon_session;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCWelcomeUI,
    wintc_welcome_ui,
    GTK_TYPE_CONTAINER,
    G_IMPLEMENT_INTERFACE(
        WINTC_TYPE_IGINA_AUTH_UI,
        wintc_welcome_ui_igina_auth_ui_interface_init
    )
)

static void wintc_welcome_ui_igina_auth_ui_interface_init(
    WINTC_UNUSED(WinTCIGinaAuthUIInterface* iface)
) {}

static void wintc_welcome_ui_class_init(
    WinTCWelcomeUIClass* klass
)
{
    GtkContainerClass* container_class = GTK_CONTAINER_CLASS(klass);
    GObjectClass*      object_class    = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_welcome_ui_constructed;
    object_class->dispose      = wintc_welcome_ui_dispose;
    object_class->get_property = wintc_welcome_ui_get_property;
    object_class->set_property = wintc_welcome_ui_set_property;

    container_class->add    = wintc_welcome_ui_add;
    container_class->remove = wintc_welcome_ui_remove;

    g_object_class_override_property(
        object_class,
        PROP_LOGON_SESSION,
        "logon-session"
    );

    // Load up styles
    //
    GtkCssProvider* css_welcome = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(
        css_welcome,
        "/uk/oddmatics/wintc/logonui/welcome-ui.css"
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_welcome),
        GTK_STYLE_PROVIDER_PRIORITY_FALLBACK
    );
}

static void wintc_welcome_ui_init(
    WinTCWelcomeUI* self
)
{
    gtk_widget_set_has_window(GTK_WIDGET(self), FALSE);
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_welcome_ui_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_welcome_ui_parent_class))
        ->constructed(object);

    WinTCWelcomeUI* welcome_ui = WINTC_WELCOME_UI(object);

    if (!(welcome_ui->logon_session))
    {
        g_critical("%s", "logonui: no logon session!");
        return;
    }

    welcome_ui->current_state = WINTC_GINA_STATE_NONE;

    g_signal_connect(
        welcome_ui->logon_session,
        "attempt-complete",
        G_CALLBACK(on_logon_session_attempt_complete),
        welcome_ui
    );

    //
    // USER INTERFACE STUFF
    //

    // Set up 'Please wait...' box

    // Set up instruction box
    //


    // Set up login box
    //

    // Set up 'welcome' box
    //


    // Set up container
    //
    welcome_ui->box_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_box_set_homogeneous(
        GTK_BOX(welcome_ui->box_container),
        TRUE
    );

    wintc_welcome_ui_internal_add(welcome_ui, welcome_ui->box_container);

    // Add style classes
    //

    // Hold an additional reference to the boxes, so we can add/remove
    // them ourselves without them getting binned
    //

    // Connect to realize signal to kick off everything when we're
    // actually live
    //
    g_signal_connect(
        welcome_ui,
        "realize",
        G_CALLBACK(on_self_realized),
        NULL
    );
}

static void wintc_welcome_ui_dispose(
    GObject* object
)
{
//    WinTCWelcomeUI* welcome_ui = WINTC_WELCOME_UI(object);

    // Bin graphical resources
    //

    // Bin additional references held for the boxes
    //


    (G_OBJECT_CLASS(wintc_welcome_ui_parent_class))->finalize(object);
}

static void wintc_welcome_ui_get_property(
    GObject*    object,
    guint       prop_id,
    WINTC_UNUSED(GValue* value),
    GParamSpec* pspec
)
{
    switch (prop_id)
    {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_welcome_ui_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCWelcomeUI* welcome_ui = WINTC_WELCOME_UI(object);

    switch (prop_id)
    {
        case PROP_LOGON_SESSION:
            welcome_ui->logon_session = g_value_dup_object(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_welcome_ui_add(
    WINTC_UNUSED(GtkContainer* container),
    WINTC_UNUSED(GtkWidget*    widget)
)
{
    g_critical("%s", "wintc_welcome_ui_add - not allowed!");
}

static void wintc_welcome_ui_remove(
    WINTC_UNUSED(GtkContainer* container),
    WINTC_UNUSED(GtkWidget*    widget)
)
{
    g_critical("%s", "wintc_welcome_ui_remove - not allowed!");
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_welcome_ui_new(
    WinTCGinaLogonSession* logon_session
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_WELCOME_UI,
            "hexpand",       TRUE,
            "vexpand",       TRUE,
            "logon-session", logon_session,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void wintc_welcome_ui_change_state(
    WinTCWelcomeUI* welcome_ui,
    WinTCGinaState    next_state
)
{
    // Disable current state, if any
    //
    switch (welcome_ui->current_state)
    {
        case WINTC_GINA_STATE_STARTING:
           
            break;

        case WINTC_GINA_STATE_PROMPT:

            break;

        case WINTC_GINA_STATE_LAUNCHING:

            break;

        default: break;
    }

    // Set up new state
    //
    switch (next_state)
    {
        case WINTC_GINA_STATE_STARTING:
            wintc_gina_logon_session_establish(
                welcome_ui->logon_session
            );
            g_timeout_add_seconds(
                DELAY_SECONDS_AT_LEAST,
                on_timeout_delay_done,
                welcome_ui
            );
            break;

        case WINTC_GINA_STATE_PROMPT:

            break;

        case WINTC_GINA_STATE_LAUNCHING:
            g_timeout_add_seconds(
                DELAY_SECONDS_AT_LEAST,
                on_timeout_delay_done,
                welcome_ui
            );
            break;

        default: break;
    }

    gtk_widget_show_all(
        welcome_ui->box_container
    );

    welcome_ui->current_state = next_state;
}

static void wintc_welcome_ui_internal_add(
    WinTCWelcomeUI* welcome_ui,
    GtkWidget*      widget
)
{
    gtk_widget_set_parent(widget, GTK_WIDGET(welcome_ui));

    welcome_ui->child_widgets =
        g_slist_append(welcome_ui->child_widgets, widget);
}

//
// CALLBACKS
//
static void on_self_realized(
    GtkWidget* self,
    WINTC_UNUSED(gpointer user_data)
)
{
    wintc_welcome_ui_change_state(
        WINTC_WELCOME_UI(self),
        WINTC_GINA_STATE_STARTING
    );
}

static void on_logon_session_attempt_complete(
    WINTC_UNUSED(WinTCGinaLogonSession* logon_session),
    WinTCGinaResponse response,
    gpointer          user_data
)
{
    WinTCWelcomeUI* welcome_ui = WINTC_WELCOME_UI(user_data);

    if (response == WINTC_GINA_RESPONSE_OKAY)
    {
        wintc_welcome_ui_change_state(
            welcome_ui,
            WINTC_GINA_STATE_LAUNCHING
        );
    }
}

static gboolean on_timeout_delay_done(
    gpointer user_data
)
{
    WinTCWelcomeUI* welcome_ui = WINTC_WELCOME_UI(user_data);

    GError* error = NULL;

    switch (welcome_ui->current_state)
    {
        case WINTC_GINA_STATE_STARTING:
            g_timeout_add_seconds(
                DELAY_SECONDS_POLL,
                on_timeout_poll_ready,
                welcome_ui
            );
            break;

        case WINTC_GINA_STATE_LAUNCHING:
            if (
                !wintc_gina_logon_session_finish(
                    welcome_ui->logon_session,
                    &error
                )
            )
            {
                wintc_nice_error_and_clear(
                    &error,
                    wintc_widget_get_toplevel_window(GTK_WIDGET(welcome_ui))
                );

                wintc_welcome_ui_change_state(
                    welcome_ui,
                    WINTC_GINA_STATE_PROMPT
                );
            }

            break;

        default:
            g_critical("%s", "Invalid state reached for delay.");
            break;
    }

    return G_SOURCE_REMOVE;
}

static gboolean on_timeout_poll_ready(
    gpointer user_data
)
{
    WinTCWelcomeUI* welcome_ui = WINTC_WELCOME_UI(user_data);

    if (
        wintc_gina_logon_session_is_available(
            welcome_ui->logon_session
        )
    )
    {
        wintc_welcome_ui_change_state(
            welcome_ui,
            WINTC_GINA_STATE_PROMPT
        );
        return G_SOURCE_REMOVE;
    }

    return G_SOURCE_CONTINUE;
}


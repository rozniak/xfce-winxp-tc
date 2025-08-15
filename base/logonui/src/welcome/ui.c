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
#include "button.h"

#define DELAY_SECONDS_AT_LEAST 2
#define DELAY_SECONDS_POLL     1

#define LOGOANI_FRAME_COUNT 50
#define LOGOANI_FRAME_RATE  15

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

static void wintc_welcome_ui_change_state(
    WinTCWelcomeUI* welcome_ui,
    WinTCGinaState  next_state
);

static void on_self_realized(
    GtkWidget* self,
    WINTC_UNUSED(gpointer user_data)
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

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCWelcomeUIClass
{
    GtkBoxClass __parent__;
};

struct _WinTCWelcomeUI
{
    GtkBox __parent__;

    // UI
    //
    GtkWidget* box_welcome;
    GtkWidget* box_login;
    GtkWidget* box_wait;

    //

    GtkWidget* box_header;
    GtkWidget* box_body;
    GtkWidget* box_footer;

    GtkWidget* button_shutdown;

    GtkWidget* stack_main;

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
    GTK_TYPE_BOX,
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
    GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);
    GObjectClass*   object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_welcome_ui_constructed;
    object_class->dispose      = wintc_welcome_ui_dispose;
    object_class->get_property = wintc_welcome_ui_get_property;
    object_class->set_property = wintc_welcome_ui_set_property;

    g_object_class_override_property(
        object_class,
        PROP_LOGON_SESSION,
        "logon-session"
    );

    gtk_widget_class_set_template_from_resource(
        widget_class,
        "/uk/oddmatics/wintc/logonui/welcome.ui"
    );

    gtk_widget_class_bind_template_child_full(
        widget_class,
        "box-header",
        FALSE,
        G_STRUCT_OFFSET(WinTCWelcomeUI, box_header)
    );
    gtk_widget_class_bind_template_child_full(
        widget_class,
        "box-body",
        FALSE,
        G_STRUCT_OFFSET(WinTCWelcomeUI, box_body)
    );
    gtk_widget_class_bind_template_child_full(
        widget_class,
        "box-footer",
        FALSE,
        G_STRUCT_OFFSET(WinTCWelcomeUI, box_footer)
    );
    gtk_widget_class_bind_template_child_full(
        widget_class,
        "button-shutdown",
        FALSE,
        G_STRUCT_OFFSET(WinTCWelcomeUI, button_shutdown)
    );
    gtk_widget_class_bind_template_child_full(
        widget_class,
        "stack-main",
        FALSE,
        G_STRUCT_OFFSET(WinTCWelcomeUI, stack_main)
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
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}

static void wintc_welcome_ui_init(
    WinTCWelcomeUI* self
)
{
    gtk_widget_init_template(GTK_WIDGET(self));
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_welcome_ui_constructed(
    GObject* object
) {
     (G_OBJECT_CLASS(wintc_welcome_ui_parent_class))
        ->constructed(object);
    
    WinTCWelcomeUI* welcome_ui = WINTC_WELCOME_UI(object);

    if (!(welcome_ui->logon_session))
    {
        g_critical("%s", "logonui: no logon session!");
        return;
    }

    gtk_widget_set_has_window(GTK_WIDGET(welcome_ui), FALSE);

    welcome_ui->current_state = WINTC_GINA_STATE_NONE;

    g_signal_connect(
        welcome_ui->logon_session,
        "attempt-complete",
        G_CALLBACK(on_logon_session_attempt_complete),
        welcome_ui
    );

    //
    // USER INTERFACE
    //
    GtkBuilder* builder = gtk_builder_new();
    GError*     error   = NULL;

    GtkWidget* anim_logo_wait  = NULL;
    GtkWidget* anim_logo_login = NULL;
    GtkWidget* user_list       = NULL;

    g_type_ensure(WINTC_TYPE_CTL_ANIMATION);
    g_type_ensure(WINTC_TYPE_WELCOME_USER_LIST);

    if (
        !gtk_builder_add_from_resource(
            builder,
            "/uk/oddmatics/wintc/logonui/welcphs1.ui",
            &error
        ) ||
        !gtk_builder_add_from_resource(
            builder,
            "/uk/oddmatics/wintc/logonui/welcphs2.ui",
            &error
        ) ||
        !gtk_builder_add_from_resource(
            builder,
            "/uk/oddmatics/wintc/logonui/welcphs3.ui",
            &error
        )
    )
    {
        g_critical("%s", "logonui: unable to load resources");
        wintc_log_error_and_clear(&error);
        return;
    }

    wintc_builder_get_objects(
        builder,
        "box-wait",        &(welcome_ui->box_wait),
        "box-login",       &(welcome_ui->box_login),
        "box-welcome",     &(welcome_ui->box_welcome),
        "anim-logo-wait",  &anim_logo_wait,
        "anim-logo-login", &anim_logo_login,
        "user-list",       &user_list,
        NULL
    );

    // Set up animations
    //
    guint      ani_id_wait;
    guint      ani_id_login;
    GdkPixbuf* pixbuf_logo;
    GdkPixbuf* pixbuf_logoani;

    pixbuf_logo =
        gdk_pixbuf_new_from_resource(
            "/uk/oddmatics/wintc/logonui/logo.png",
            NULL
        );
    pixbuf_logoani =
        gdk_pixbuf_new_from_resource(
            "/uk/oddmatics/wintc/logonui/logoani.png",
            NULL
        );

    ani_id_wait =
        wintc_ctl_animation_add_static(
            WINTC_CTL_ANIMATION(anim_logo_wait),
            pixbuf_logo
        );
    wintc_ctl_animation_play(
        WINTC_CTL_ANIMATION(anim_logo_wait),
        ani_id_wait,
        0,
        WINTC_CTL_ANIMATION_INFINITE
    );

    ani_id_login =
        wintc_ctl_animation_add_framesheet(
            WINTC_CTL_ANIMATION(anim_logo_login),
            pixbuf_logoani,
            LOGOANI_FRAME_COUNT
        );
    wintc_ctl_animation_play(
        WINTC_CTL_ANIMATION(anim_logo_login),
        ani_id_login,
        LOGOANI_FRAME_RATE,
        WINTC_CTL_ANIMATION_INFINITE
    );

    g_object_set(
        user_list,
        "logon-session", welcome_ui->logon_session,
        NULL
    );

    // Add boxes to main stack
    //
    gtk_container_add(
        GTK_CONTAINER(welcome_ui->stack_main),
        welcome_ui->box_wait
    );
    gtk_container_add(
        GTK_CONTAINER(welcome_ui->stack_main),
        welcome_ui->box_login
    );
    gtk_container_add(
        GTK_CONTAINER(welcome_ui->stack_main),
        welcome_ui->box_welcome
    );

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
    //WinTCWelcomeUI* welcome_ui = WINTC_WELCOME_UI(object);

    (G_OBJECT_CLASS(wintc_welcome_ui_parent_class))
        ->dispose(object);
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
            gtk_stack_set_visible_child(
                GTK_STACK(welcome_ui->stack_main),
                welcome_ui->box_wait
            );
            break;

        case WINTC_GINA_STATE_PROMPT:
            gtk_stack_set_visible_child(
                GTK_STACK(welcome_ui->stack_main),
                welcome_ui->box_login
            );
            break;

        case WINTC_GINA_STATE_LAUNCHING:
            g_timeout_add_seconds(
                DELAY_SECONDS_AT_LEAST,
                on_timeout_delay_done,
                welcome_ui
            );
            gtk_stack_set_visible_child(
                GTK_STACK(welcome_ui->stack_main),
                welcome_ui->box_welcome
            );
            break;

        default: break;
    }

    welcome_ui->current_state = next_state;
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

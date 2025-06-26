#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/winbrand.h>

#include "../public/authwnd.h"
#include "../public/challenge.h"
#include "../public/if_authui.h"
#include "../public/logon.h"
#include "../public/state.h"
#include "stripctl.h"

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
static void wintc_gina_auth_window_igina_auth_ui_interface_init(
    WinTCIGinaAuthUIInterface* iface
);

static void wintc_gina_auth_window_constructed(
    GObject* object
);
static void wintc_gina_auth_window_dispose(
    GObject* object
);
static void wintc_gina_auth_window_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_gina_auth_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void wintc_gina_auth_window_change_state(
    WinTCGinaAuthWindow* wnd,
    WinTCGinaState       next_state
);

static void on_self_realized(
    GtkWidget* self,
    gpointer   user_data
);

static void on_logon_session_attempt_complete(
    WinTCGinaLogonSession* logon_session,
    WinTCGinaResponse      response,
    gpointer               user_data
);

static void on_button_submit_clicked(
    GtkButton* self,
    gpointer   user_data
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
struct _WinTCGinaAuthWindowClass
{
    GtkWindowClass __parent__;
};

struct _WinTCGinaAuthWindow
{
    GtkWindow __parent__;

    // Image resources
    //
    GdkPixbuf* pixbuf_banner;
    GdkPixbuf* pixbuf_bannerx;

    // UI
    //
    GtkWidget* box_container;

    GtkWidget* box_brand;
    GtkWidget* box_login;
    GtkWidget* box_wait;

    GtkWidget* button_submit;
    GtkWidget* entry_password;
    GtkWidget* entry_username;

    GtkWidget* strip;

    // State
    //
    WinTCGinaState         current_state;
    WinTCGinaLogonSession* logon_session;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCGinaAuthWindow,
    wintc_gina_auth_window,
    GTK_TYPE_WINDOW,
    G_IMPLEMENT_INTERFACE(
        WINTC_TYPE_IGINA_AUTH_UI,
        wintc_gina_auth_window_igina_auth_ui_interface_init
    )
)

static void wintc_gina_auth_window_class_init(
    WinTCGinaAuthWindowClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_gina_auth_window_constructed;
    object_class->dispose      = wintc_gina_auth_window_dispose;
    object_class->get_property = wintc_gina_auth_window_get_property;
    object_class->set_property = wintc_gina_auth_window_set_property;

    g_object_class_override_property(
        object_class,
        PROP_LOGON_SESSION,
        "logon-session"
    );

    // Load up styles
    //
    GtkCssProvider* css_authwnd = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(
        css_authwnd,
        "/uk/oddmatics/wintc/msgina/authwnd.css"
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_authwnd),
        GTK_STYLE_PROVIDER_PRIORITY_FALLBACK
    );

    wintc_ctl_install_default_styles();
}

static void wintc_gina_auth_window_init(
    WinTCGinaAuthWindow* self
)
{
    // Acquire branding pixbufs
    //
    GError* err_banner  = NULL;
    GError* err_bannerx = NULL;

    self->pixbuf_banner  = wintc_brand_get_brand_pixmap(
                                     WINTC_BRAND_PART_BANNER,
                                     &err_banner
                                 );
    self->pixbuf_bannerx = wintc_brand_get_brand_pixmap(
                                     WINTC_BRAND_PART_BANNER_TALL,
                                     &err_bannerx
                                 );

    wintc_display_error_and_clear(&err_banner, NULL);
    wintc_display_error_and_clear(&err_bannerx, NULL);

    // Set up branding box
    //
    GtkWidget* image_brand =
        gtk_image_new_from_pixbuf(
            self->pixbuf_bannerx
        );

    self->box_brand = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    self->strip     = wintc_gina_strip_new();

    gtk_box_pack_start(
        GTK_BOX(self->box_brand),
        image_brand,
        FALSE,
        FALSE,
        0
    );
    gtk_box_pack_start(
        GTK_BOX(self->box_brand),
        self->strip,
        FALSE,
        FALSE,
        0
    );

    // Set up 'Please wait...' box
    //
    GtkWidget* label_wait = gtk_label_new("Please wait...");

    self->box_wait = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    wintc_widget_add_style_class(
        self->box_wait,
        "please-wait"
    );

    gtk_widget_set_halign(
        label_wait,
        GTK_ALIGN_START
    );
    gtk_widget_set_valign(
        label_wait,
        GTK_ALIGN_START
    );

    gtk_box_pack_start(
        GTK_BOX(self->box_wait),
        label_wait,
        TRUE,
        TRUE,
        0
    );

    // Set up login box
    //
    GtkWidget* box_buttons    = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget* grid_login     = gtk_grid_new();
    GtkWidget* label_password = gtk_label_new("Password:");
    GtkWidget* label_username = gtk_label_new("User name:");

    self->box_login = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    self->button_submit  = gtk_button_new_with_label("OK");
    self->entry_password = gtk_entry_new();
    self->entry_username = gtk_entry_new();

    wintc_widget_add_style_class(
        box_buttons,
        WINTC_CTL_BUTTON_BOX_CSS_CLASS
    );

    wintc_widget_add_style_class(
        self->box_login,
        "login"
    );

    gtk_widget_set_halign(
        label_password,
        GTK_ALIGN_START
    );
    gtk_widget_set_halign(
        label_username,
        GTK_ALIGN_START
    );

    gtk_entry_set_visibility(GTK_ENTRY(self->entry_password), FALSE);

    g_signal_connect(
        self->button_submit,
        "clicked",
        G_CALLBACK(on_button_submit_clicked),
        self
    );

    gtk_grid_attach(
        GTK_GRID(grid_login),
        label_username,
        0,
        0,
        1,
        1
    );
    gtk_grid_attach(
        GTK_GRID(grid_login),
        self->entry_username,
        1,
        0,
        1,
        1
    );

    gtk_grid_attach(
        GTK_GRID(grid_login),
        label_password,
        0,
        1,
        1,
        1
    );
    gtk_grid_attach(
        GTK_GRID(grid_login),
        self->entry_password,
        1,
        1,
        1,
        1
    );

    gtk_box_pack_end(
        GTK_BOX(box_buttons),
        self->button_submit,
        FALSE,
        FALSE,
        0
    );

    gtk_container_add(
        GTK_CONTAINER(self->box_login),
        grid_login
    );
    gtk_container_add(
        GTK_CONTAINER(self->box_login),
        box_buttons
    );

    // Set up remainder of UI
    //
    GtkWidget* header_bar = gtk_header_bar_new();

    self->box_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    gtk_header_bar_set_title(
        GTK_HEADER_BAR(header_bar),
        "Log On to Windows"
    );

    gtk_container_add(
        GTK_CONTAINER(self->box_container),
        self->box_brand
    );

    gtk_container_add(
        GTK_CONTAINER(self),
        self->box_container
    );

    gtk_window_set_titlebar(
        GTK_WINDOW(self),
        header_bar
    );

    // Hold additional references to the boxes, so we can add/remove
    // them ourselves without them getting binned
    //
    g_object_ref(self->box_login);
    g_object_ref(self->box_wait);

    // Connect to realize signal to kick off everything when we're
    // actually live
    //
    g_signal_connect(
        self,
        "realize",
        G_CALLBACK(on_self_realized),
        NULL
    );
}

static void wintc_gina_auth_window_igina_auth_ui_interface_init(
    WINTC_UNUSED(WinTCIGinaAuthUIInterface* iface)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_gina_auth_window_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_gina_auth_window_parent_class))
        ->constructed(object);

    WinTCGinaAuthWindow* wnd = WINTC_GINA_AUTH_WINDOW(object);

    if (!(wnd->logon_session))
    {
        g_critical("%s", "gina: authwnd: no logon session!");
        return;
    }

    wnd->current_state = WINTC_GINA_STATE_NONE;

    g_signal_connect(
        wnd->logon_session,
        "attempt-complete",
        G_CALLBACK(on_logon_session_attempt_complete),
        wnd
    );
}

static void wintc_gina_auth_window_dispose(
    GObject* object
)
{
    WinTCGinaAuthWindow* wnd = WINTC_GINA_AUTH_WINDOW(object);

    g_clear_object(&(wnd->logon_session));

    // Bin pixbufs
    //
    g_clear_object(&(wnd->pixbuf_banner));
    g_clear_object(&(wnd->pixbuf_bannerx));

    // Bin additional references held for the boxes
    //
    g_clear_object(&(wnd->box_login));
    g_clear_object(&(wnd->box_wait));

    (G_OBJECT_CLASS(wintc_gina_auth_window_parent_class))
        ->dispose(object);
}

static void wintc_gina_auth_window_get_property(
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

static void wintc_gina_auth_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCGinaAuthWindow* wnd = WINTC_GINA_AUTH_WINDOW(object);

    switch (prop_id)
    {
        case PROP_LOGON_SESSION:
            wnd->logon_session = g_value_dup_object(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_gina_auth_window_new(
    WinTCGinaLogonSession* logon_session
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_GINA_AUTH_WINDOW,
            "type",          GTK_WINDOW_TOPLEVEL,
            "resizable",     FALSE,
            "logon-session", logon_session,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void wintc_gina_auth_window_change_state(
    WinTCGinaAuthWindow* wnd,
    WinTCGinaState       next_state
)
{
    // Disable current state, if any
    //
    switch (wnd->current_state)
    {
        case WINTC_GINA_STATE_STARTING:
        case WINTC_GINA_STATE_LAUNCHING:
            wintc_gina_strip_stop_animating(
                WINTC_GINA_STRIP(wnd->strip)
            );
            gtk_container_remove(
                GTK_CONTAINER(wnd->box_container),
                wnd->box_wait
            );
            break;

        case WINTC_GINA_STATE_PROMPT:
            gtk_container_remove(
                GTK_CONTAINER(wnd->box_container),
                wnd->box_login
            );
            break;

        default: break;
    }

    // Set up new state
    //
    switch (next_state)
    {
        case WINTC_GINA_STATE_STARTING:
            gtk_box_pack_start(
                GTK_BOX(wnd->box_container),
                wnd->box_wait,
                TRUE,
                TRUE,
                0
            );
            wintc_gina_strip_animate(
                WINTC_GINA_STRIP(wnd->strip)
            );
            wintc_gina_logon_session_establish(
                wnd->logon_session
            );
            g_timeout_add_seconds(
                DELAY_SECONDS_AT_LEAST,
                on_timeout_delay_done,
                wnd
            );
            break;

        case WINTC_GINA_STATE_PROMPT:
            gtk_box_pack_start(
                GTK_BOX(wnd->box_container),
                wnd->box_login,
                TRUE,
                TRUE,
                0
            );
            break;

        case WINTC_GINA_STATE_LAUNCHING:
            gtk_box_pack_start(
                GTK_BOX(wnd->box_container),
                wnd->box_wait,
                TRUE,
                TRUE,
                0
            );
            wintc_gina_strip_animate(
                WINTC_GINA_STRIP(wnd->strip)
            );
            g_timeout_add_seconds(
                DELAY_SECONDS_AT_LEAST,
                on_timeout_delay_done,
                wnd
            );
            break;

        default: break;
    }

    gtk_widget_show_all(
        wnd->box_container
    );

    wnd->current_state = next_state;
}

//
// CALLBACKS
//
static void on_self_realized(
    GtkWidget* self,
    WINTC_UNUSED(gpointer user_data)
)
{
    wintc_gina_auth_window_change_state(
        WINTC_GINA_AUTH_WINDOW(self),
        WINTC_GINA_STATE_STARTING
    );
}

static void on_logon_session_attempt_complete(
    WINTC_UNUSED(WinTCGinaLogonSession* logon_session),
    WinTCGinaResponse response,
    gpointer          user_data
)
{
    WinTCGinaAuthWindow* wnd = WINTC_GINA_AUTH_WINDOW(user_data);

    // Reset the UI state
    //
    gtk_entry_set_text(
        GTK_ENTRY(wnd->entry_password),
        ""
    );
    gtk_entry_set_text(
        GTK_ENTRY(wnd->entry_username),
        ""
    );

    gtk_widget_set_sensitive(
        wnd->button_submit,
        TRUE
    );
    gtk_widget_set_sensitive(
        wnd->entry_password,
        TRUE
    );
    gtk_widget_set_sensitive(
        wnd->entry_username,
        TRUE
    );

    wintc_gina_strip_stop_animating(
        WINTC_GINA_STRIP(wnd->strip)
    );

    // Now handle specifics for whether the attempt succeeded or not
    //
    if (response == WINTC_GINA_RESPONSE_OKAY)
    {
        wintc_gina_auth_window_change_state(
            wnd,
            WINTC_GINA_STATE_LAUNCHING
        );
    }
    // else (WINTC_GINA_RESPONSE_FAIL)
    // FIXME: Error should be handled here, when we have one
}

static void on_button_submit_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer     user_data
)
{
    WinTCGinaAuthWindow* wnd = WINTC_GINA_AUTH_WINDOW(user_data);

    gtk_widget_set_sensitive(
        wnd->button_submit,
        FALSE
    );
    gtk_widget_set_sensitive(
        wnd->entry_password,
        FALSE
    );
    gtk_widget_set_sensitive(
        wnd->entry_username,
        FALSE
    );

    wintc_gina_strip_animate(
        WINTC_GINA_STRIP(wnd->strip)
    );

    wintc_gina_logon_session_try_logon(
        wnd->logon_session,
        gtk_entry_get_text(GTK_ENTRY(wnd->entry_username)),
        gtk_entry_get_text(GTK_ENTRY(wnd->entry_password))
    );
}

static gboolean on_timeout_delay_done(
    gpointer user_data
)
{
    WinTCGinaAuthWindow* wnd = WINTC_GINA_AUTH_WINDOW(user_data);

    GError* error = NULL;

    switch (wnd->current_state)
    {
        case WINTC_GINA_STATE_STARTING:
            g_timeout_add_seconds(
                DELAY_SECONDS_POLL,
                on_timeout_poll_ready,
                wnd
            );
            break;

        case WINTC_GINA_STATE_LAUNCHING:
            if (
                !wintc_gina_logon_session_finish(
                    wnd->logon_session,
                    &error
                )
            )
            {
                wintc_nice_error_and_clear(&error, GTK_WINDOW(wnd));

                wintc_gina_auth_window_change_state(
                    wnd,
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
    WinTCGinaAuthWindow* wnd = WINTC_GINA_AUTH_WINDOW(user_data);

    if (
        wintc_gina_logon_session_is_available(
            wnd->logon_session
        )
    )
    {
        wintc_gina_auth_window_change_state(
            wnd,
            WINTC_GINA_STATE_PROMPT
        );
        return G_SOURCE_REMOVE;
    }

    return G_SOURCE_CONTINUE;
}


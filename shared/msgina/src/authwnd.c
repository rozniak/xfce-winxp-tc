#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc-comctl.h>
#include <wintc-comgtk.h>
#include <wintc-winbrand.h>

#include "authwnd.h"
#include "challenge.h"
#include "logon.h"
#include "state.h"
#include "stripctl.h"

#define DELAY_SECONDS_AT_LEAST 2
#define DELAY_SECONDS_POLL     1

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCGinaAuthWindowPrivate
{
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

struct _WinTCGinaAuthWindowClass
{
    GtkWindowClass __parent__;
};

struct _WinTCGinaAuthWindow
{
    GtkWindow __parent__;

    WinTCGinaAuthWindowPrivate* priv;
};

//
// FORWARD DECLARATIONS
//
static void wintc_gina_auth_window_finalize(
    GObject* object
);

static void wintc_gina_auth_window_change_state(
    WinTCGinaAuthWindow* window,
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
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCGinaAuthWindow,
    wintc_gina_auth_window,
    GTK_TYPE_WINDOW,
    G_ADD_PRIVATE(WinTCGinaAuthWindow)
)

static void wintc_gina_auth_window_class_init(
    WinTCGinaAuthWindowClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->finalize = wintc_gina_auth_window_finalize;

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

    wintc_comctl_install_default_styles();
}

static void wintc_gina_auth_window_init(
    WinTCGinaAuthWindow* self
)
{
    self->priv = wintc_gina_auth_window_get_instance_private(self);

    // Acquire branding pixbufs
    //
    GError* err_banner  = NULL;
    GError* err_bannerx = NULL;

    self->priv->pixbuf_banner  = wintc_brand_get_brand_pixmap(
                                     WINTC_BRAND_PART_BANNER,
                                     &err_banner
                                 );
    self->priv->pixbuf_bannerx = wintc_brand_get_brand_pixmap(
                                     WINTC_BRAND_PART_BANNER_TALL,
                                     &err_bannerx
                                 );

    wintc_display_error_and_clear(&err_banner);
    wintc_display_error_and_clear(&err_bannerx);

    // Set up branding box
    //
    GtkWidget* image_brand =
        gtk_image_new_from_pixbuf(
            self->priv->pixbuf_bannerx
        );

    self->priv->box_brand = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    self->priv->strip     = wintc_gina_strip_new();

    gtk_box_pack_start(
        GTK_BOX(self->priv->box_brand),
        image_brand,
        FALSE,
        FALSE,
        0
    );
    gtk_box_pack_start(
        GTK_BOX(self->priv->box_brand),
        self->priv->strip,
        FALSE,
        FALSE,
        0
    );

    // Set up 'Please wait...' box
    //
    GtkWidget* label_wait = gtk_label_new("Please wait...");

    self->priv->box_wait = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    wintc_widget_add_style_class(
        self->priv->box_wait,
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
        GTK_BOX(self->priv->box_wait),
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

    self->priv->box_login = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    self->priv->button_submit  = gtk_button_new_with_label("OK");
    self->priv->entry_password = gtk_entry_new();
    self->priv->entry_username = gtk_entry_new();

    wintc_widget_add_style_class(
        box_buttons,
        WINTC_COMCTL_BUTTON_BOX_CSS_CLASS
    );

    wintc_widget_add_style_class(
        self->priv->box_login,
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

    gtk_entry_set_visibility(GTK_ENTRY(self->priv->entry_password), FALSE);

    g_signal_connect(
        self->priv->button_submit,
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
        self->priv->entry_username,
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
        self->priv->entry_password,
        1,
        1,
        1,
        1
    );

    gtk_box_pack_end(
        GTK_BOX(box_buttons),
        self->priv->button_submit,
        FALSE,
        FALSE,
        0
    );

    gtk_container_add(
        GTK_CONTAINER(self->priv->box_login),
        grid_login
    );
    gtk_container_add(
        GTK_CONTAINER(self->priv->box_login),
        box_buttons
    );

    // Set up remainder of UI
    //
    GtkWidget* header_bar = gtk_header_bar_new();

    self->priv->box_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    gtk_header_bar_set_title(
        GTK_HEADER_BAR(header_bar),
        "Log On to Windows"
    );

    gtk_container_add(
        GTK_CONTAINER(self->priv->box_container),
        self->priv->box_brand
    );

    gtk_container_add(
        GTK_CONTAINER(self),
        self->priv->box_container
    );

    gtk_window_set_titlebar(
        GTK_WINDOW(self),
        header_bar
    );

    // Hold additional references to the boxes, so we can add/remove
    // them ourselves without them getting binned
    //
    g_object_ref(self->priv->box_login);
    g_object_ref(self->priv->box_wait);

    // Connect to realize signal to kick off everything when we're
    // actually live
    //
    g_signal_connect(
        self,
        "realize",
        G_CALLBACK(on_self_realized),
        NULL
    );

    // Set initial state
    //
    self->priv->current_state = WINTC_GINA_STATE_NONE;
    self->priv->logon_session = wintc_gina_logon_session_new();

    g_signal_connect(
        self->priv->logon_session,
        "attempt-complete",
        G_CALLBACK(on_logon_session_attempt_complete),
        self
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_gina_auth_window_finalize(
    GObject* gobject
)
{
    WinTCGinaAuthWindow* window = WINTC_GINA_AUTH_WINDOW(gobject);

    g_clear_object(&(window->priv->pixbuf_banner));
    g_clear_object(&(window->priv->pixbuf_bannerx));

    // Bin additional references held for the boxes
    //
    g_clear_object(&(window->priv->box_login));
    g_clear_object(&(window->priv->box_wait));

    (G_OBJECT_CLASS(wintc_gina_auth_window_parent_class))->finalize(gobject);
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_gina_auth_window_new(void)
{
    return GTK_WIDGET(
        g_object_new(
            TYPE_WINTC_GINA_AUTH_WINDOW,
            "type",      GTK_WINDOW_TOPLEVEL,
            "resizable", FALSE,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void wintc_gina_auth_window_change_state(
    WinTCGinaAuthWindow* window,
    WinTCGinaState       next_state
)
{
    // Disable current state, if any
    //
    switch (window->priv->current_state)
    {
        case WINTC_GINA_STATE_STARTING:
        case WINTC_GINA_STATE_LAUNCHING:
            wintc_gina_strip_stop_animating(
                WINTC_GINA_STRIP(window->priv->strip)
            );
            gtk_container_remove(
                GTK_CONTAINER(window->priv->box_container),
                window->priv->box_wait
            );
            break;

        case WINTC_GINA_STATE_PROMPT:
            gtk_container_remove(
                GTK_CONTAINER(window->priv->box_container),
                window->priv->box_login
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
                GTK_BOX(window->priv->box_container),
                window->priv->box_wait,
                TRUE,
                TRUE,
                0
            );
            wintc_gina_strip_animate(
                WINTC_GINA_STRIP(window->priv->strip)
            );
            wintc_gina_logon_session_establish(
                window->priv->logon_session
            );
            g_timeout_add_seconds(
                DELAY_SECONDS_AT_LEAST,
                on_timeout_delay_done,
                window
            );
            break;

        case WINTC_GINA_STATE_PROMPT:
            gtk_box_pack_start(
                GTK_BOX(window->priv->box_container),
                window->priv->box_login,
                TRUE,
                TRUE,
                0
            );
            break;

        case WINTC_GINA_STATE_LAUNCHING:
            gtk_box_pack_start(
                GTK_BOX(window->priv->box_container),
                window->priv->box_wait,
                TRUE,
                TRUE,
                0
            );
            wintc_gina_strip_animate(
                WINTC_GINA_STRIP(window->priv->strip)
            );
            g_timeout_add_seconds(
                DELAY_SECONDS_AT_LEAST,
                on_timeout_delay_done,
                window
            );
            break;

        default: break;
    }

    gtk_widget_show_all(
        window->priv->box_container
    );

    window->priv->current_state = next_state;
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
    WinTCGinaAuthWindow* window = WINTC_GINA_AUTH_WINDOW(user_data);

    switch (response)
    {
        case WINTC_GINA_RESPONSE_OKAY:
            wintc_gina_auth_window_change_state(
                window,
                WINTC_GINA_STATE_LAUNCHING
            );
            break;

        case WINTC_GINA_RESPONSE_FAIL:
            // FIXME: Prompt for failure
            gtk_entry_set_text(
                GTK_ENTRY(window->priv->entry_password),
                ""
            );
            gtk_entry_set_text(
                GTK_ENTRY(window->priv->entry_username),
                ""
            );

            gtk_widget_set_sensitive(
                window->priv->button_submit,
                TRUE
            );
            gtk_widget_set_sensitive(
                window->priv->entry_password,
                TRUE
            );
            gtk_widget_set_sensitive(
                window->priv->entry_username,
                TRUE
            );

            wintc_gina_strip_stop_animating(
                WINTC_GINA_STRIP(window->priv->strip)
            );

            break;

        default: break;
    }
}

static void on_button_submit_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer     user_data
)
{
    WinTCGinaAuthWindow* window = WINTC_GINA_AUTH_WINDOW(user_data);

    gtk_widget_set_sensitive(
        window->priv->button_submit,
        FALSE
    );
    gtk_widget_set_sensitive(
        window->priv->entry_password,
        FALSE
    );
    gtk_widget_set_sensitive(
        window->priv->entry_username,
        FALSE
    );

    wintc_gina_strip_animate(
        WINTC_GINA_STRIP(window->priv->strip)
    );

    wintc_gina_logon_session_try_logon(
        window->priv->logon_session,
        gtk_entry_get_text(GTK_ENTRY(window->priv->entry_username)),
        gtk_entry_get_text(GTK_ENTRY(window->priv->entry_password))
    );
}

static gboolean on_timeout_delay_done(
    gpointer user_data
)
{
    WinTCGinaAuthWindow* window = WINTC_GINA_AUTH_WINDOW(user_data);

    switch (window->priv->current_state)
    {
        case WINTC_GINA_STATE_STARTING:
            g_timeout_add_seconds(
                DELAY_SECONDS_POLL,
                on_timeout_poll_ready,
                window
            );
            break;

        case WINTC_GINA_STATE_LAUNCHING:
            wintc_gina_logon_session_finish(
                window->priv->logon_session
            );
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
    WinTCGinaAuthWindow* window = WINTC_GINA_AUTH_WINDOW(user_data);

    if (
        wintc_gina_logon_session_is_available(
            window->priv->logon_session
        )
    )
    {
        wintc_gina_auth_window_change_state(
            window,
            WINTC_GINA_STATE_PROMPT
        );
        return G_SOURCE_REMOVE;
    }

    return G_SOURCE_CONTINUE;
}


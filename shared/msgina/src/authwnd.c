#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/winbrand.h>

#include "../public/authwnd.h"
#include "../public/challenge.h"
#include "../public/logon.h"
#include "../public/state.h"
#include "stripctl.h"

#define DELAY_SECONDS_AT_LEAST 2
#define DELAY_SECONDS_POLL     1

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
G_DEFINE_TYPE(
    WinTCGinaAuthWindow,
    wintc_gina_auth_window,
    GTK_TYPE_WINDOW
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

    // Set initial state
    //
    self->current_state = WINTC_GINA_STATE_NONE;
    self->logon_session = wintc_gina_logon_session_new();

    g_signal_connect(
        self->logon_session,
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

    g_clear_object(&(window->pixbuf_banner));
    g_clear_object(&(window->pixbuf_bannerx));

    // Bin additional references held for the boxes
    //
    g_clear_object(&(window->box_login));
    g_clear_object(&(window->box_wait));

    (G_OBJECT_CLASS(wintc_gina_auth_window_parent_class))->finalize(gobject);
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_gina_auth_window_new(void)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_GINA_AUTH_WINDOW,
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
    switch (window->current_state)
    {
        case WINTC_GINA_STATE_STARTING:
        case WINTC_GINA_STATE_LAUNCHING:
            wintc_gina_strip_stop_animating(
                WINTC_GINA_STRIP(window->strip)
            );
            gtk_container_remove(
                GTK_CONTAINER(window->box_container),
                window->box_wait
            );
            break;

        case WINTC_GINA_STATE_PROMPT:
            gtk_container_remove(
                GTK_CONTAINER(window->box_container),
                window->box_login
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
                GTK_BOX(window->box_container),
                window->box_wait,
                TRUE,
                TRUE,
                0
            );
            wintc_gina_strip_animate(
                WINTC_GINA_STRIP(window->strip)
            );
            wintc_gina_logon_session_establish(
                window->logon_session
            );
            g_timeout_add_seconds(
                DELAY_SECONDS_AT_LEAST,
                on_timeout_delay_done,
                window
            );
            break;

        case WINTC_GINA_STATE_PROMPT:
            gtk_box_pack_start(
                GTK_BOX(window->box_container),
                window->box_login,
                TRUE,
                TRUE,
                0
            );
            break;

        case WINTC_GINA_STATE_LAUNCHING:
            gtk_box_pack_start(
                GTK_BOX(window->box_container),
                window->box_wait,
                TRUE,
                TRUE,
                0
            );
            wintc_gina_strip_animate(
                WINTC_GINA_STRIP(window->strip)
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
        window->box_container
    );

    window->current_state = next_state;
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

    // Reset the UI state
    //
    gtk_entry_set_text(
        GTK_ENTRY(window->entry_password),
        ""
    );
    gtk_entry_set_text(
        GTK_ENTRY(window->entry_username),
        ""
    );

    gtk_widget_set_sensitive(
        window->button_submit,
        TRUE
    );
    gtk_widget_set_sensitive(
        window->entry_password,
        TRUE
    );
    gtk_widget_set_sensitive(
        window->entry_username,
        TRUE
    );

    wintc_gina_strip_stop_animating(
        WINTC_GINA_STRIP(window->strip)
    );

    // Now handle specifics for whether the attempt succeeded or not
    //
    if (response == WINTC_GINA_RESPONSE_OKAY)
    {
        wintc_gina_auth_window_change_state(
            window,
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
    WinTCGinaAuthWindow* window = WINTC_GINA_AUTH_WINDOW(user_data);

    gtk_widget_set_sensitive(
        window->button_submit,
        FALSE
    );
    gtk_widget_set_sensitive(
        window->entry_password,
        FALSE
    );
    gtk_widget_set_sensitive(
        window->entry_username,
        FALSE
    );

    wintc_gina_strip_animate(
        WINTC_GINA_STRIP(window->strip)
    );

    wintc_gina_logon_session_try_logon(
        window->logon_session,
        gtk_entry_get_text(GTK_ENTRY(window->entry_username)),
        gtk_entry_get_text(GTK_ENTRY(window->entry_password))
    );
}

static gboolean on_timeout_delay_done(
    gpointer user_data
)
{
    WinTCGinaAuthWindow* window = WINTC_GINA_AUTH_WINDOW(user_data);

    GError* error = NULL;

    switch (window->current_state)
    {
        case WINTC_GINA_STATE_STARTING:
            g_timeout_add_seconds(
                DELAY_SECONDS_POLL,
                on_timeout_poll_ready,
                window
            );
            break;

        case WINTC_GINA_STATE_LAUNCHING:
            if (
                !wintc_gina_logon_session_finish(
                    window->logon_session,
                    &error
                )
            )
            {
                wintc_nice_error_and_clear(&error, GTK_WINDOW(window));

                wintc_gina_auth_window_change_state(
                    window,
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
    WinTCGinaAuthWindow* window = WINTC_GINA_AUTH_WINDOW(user_data);

    if (
        wintc_gina_logon_session_is_available(
            window->logon_session
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


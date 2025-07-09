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
static void wintc_welcome_ui_size_allocate(
    GtkWidget*     widget,
    GtkAllocation* allocation
);
static void wintc_welcome_ui_forall(
    GtkContainer* container,
    WINTC_UNUSED(gboolean include_internals),
    GtkCallback   callback,
    gpointer      callback_data
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

GtkWidget* build_welcome_box(void);
GtkWidget* build_login_box(
    WinTCGinaLogonSession* logon_session
);
GtkWidget* build_wait_box(void);
GtkWidget* create_footer_widget(void);
GtkWidget* create_top_ribbon_widget(void);
GtkWidget* create_top_separator_widget(void);
GtkWidget* create_vertical_separator_widget(void);
GtkWidget* create_bottom_separator_widget(void);
GtkWidget* create_bottom_ribbon_widget(void);
GtkWidget* create_shutdown_widget(void);

static gboolean on_shutdown_button_clicked(
    WINTC_UNUSED(GtkWidget *widget), 
    WINTC_UNUSED(GdkEvent *event), 
    gpointer data
);
static gboolean on_shutdown_button_enter(
    GtkWidget *widget, 
    WINTC_UNUSED(GdkEvent *event), 
    gpointer data
);
static gboolean on_shutdown_button_leave(
    GtkWidget *widget, 
    GdkEventCrossing *event, 
    gpointer data
);
static gboolean on_cancel_pressed(
    WINTC_UNUSED(GtkWidget *widget), 
    WINTC_UNUSED(GdkEvent *event), 
    gpointer data
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

    // UI
    //
    GtkWidget* box_container; 
    
    GtkWidget* welcome_box;
    GtkWidget* login_box;
    GtkWidget* wait_box;

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

typedef struct {
    GtkWidget* shutdown_button;
    GtkWidget* shutdown_label;
} ShutdownWidgets;

static void wintc_welcome_ui_igina_auth_ui_interface_init(
    WINTC_UNUSED(WinTCIGinaAuthUIInterface* iface)
) {}


static void wintc_welcome_ui_class_init(
    WinTCWelcomeUIClass* klass
)
{
    GtkContainerClass* container_class = GTK_CONTAINER_CLASS(klass);
    GtkWidgetClass*    widget_class    = GTK_WIDGET_CLASS(klass);
    GObjectClass*      object_class    = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_welcome_ui_constructed;
    object_class->dispose      = wintc_welcome_ui_dispose;
    object_class->get_property = wintc_welcome_ui_get_property;
    object_class->set_property = wintc_welcome_ui_set_property;

    container_class->add    = wintc_welcome_ui_add;
    container_class->remove = wintc_welcome_ui_remove;
    container_class->forall = wintc_welcome_ui_forall;

    widget_class->size_allocate = wintc_welcome_ui_size_allocate;

    g_object_class_override_property(
        object_class,
        PROP_LOGON_SESSION,
        "logon-session"
    );
    
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
    WINTC_UNUSED(WinTCWelcomeUI* self)
) {}

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
    welcome_ui->wait_box = build_wait_box();
    welcome_ui->login_box = build_login_box(welcome_ui->logon_session);
    welcome_ui->welcome_box = build_welcome_box();

    welcome_ui->box_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    wintc_welcome_ui_internal_add(welcome_ui, welcome_ui->box_container);

    // Hold an additional reference to the boxes, so we can add/remove
    // them ourselves without them getting binned
    //
    g_object_ref(welcome_ui->welcome_box);
    g_object_ref(welcome_ui->login_box);
    g_object_ref(welcome_ui->wait_box);

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
) {
    WinTCWelcomeUI* welcome_ui = WINTC_WELCOME_UI(object);

    g_object_unref(welcome_ui->welcome_box);
    g_object_unref(welcome_ui->login_box);
    g_object_unref(welcome_ui->wait_box);

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
static void wintc_welcome_ui_size_allocate(
    GtkWidget*     widget,
    GtkAllocation* allocation
)
{
    WinTCWelcomeUI* welcome_ui = WINTC_WELCOME_UI(widget);
    
    gtk_widget_set_allocation(widget, allocation);
    
    if (welcome_ui->box_container && gtk_widget_get_visible(welcome_ui->box_container))
    {
        gtk_widget_size_allocate(welcome_ui->box_container, allocation);
    }
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


static void wintc_welcome_ui_forall(
    GtkContainer* container,
    WINTC_UNUSED(gboolean include_internals),
    GtkCallback   callback,
    gpointer      callback_data
)
{
    WinTCWelcomeUI* welcome_ui = WINTC_WELCOME_UI(container);

    g_slist_foreach(
        welcome_ui->child_widgets,
        (GFunc) callback,
        callback_data
    );
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
            gtk_container_remove(
                GTK_CONTAINER(welcome_ui->box_container),
                welcome_ui->wait_box
            );
            break;

        case WINTC_GINA_STATE_PROMPT:
            gtk_container_remove(
                GTK_CONTAINER(welcome_ui->box_container),
                welcome_ui->login_box
            );
            break;

        case WINTC_GINA_STATE_LAUNCHING:
            gtk_container_remove(
                GTK_CONTAINER(welcome_ui->box_container),
                welcome_ui->wait_box
            );
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
            gtk_container_add(
                GTK_CONTAINER(welcome_ui->box_container),
                welcome_ui->wait_box
            );
            break;

        case WINTC_GINA_STATE_PROMPT:
            gtk_container_add(
                GTK_CONTAINER(welcome_ui->box_container),
                welcome_ui->login_box
            );
            break;

        case WINTC_GINA_STATE_LAUNCHING:
            g_timeout_add_seconds(
                DELAY_SECONDS_AT_LEAST,
                on_timeout_delay_done,
                welcome_ui
            );
            gtk_container_add(
                GTK_CONTAINER(welcome_ui->box_container),
                welcome_ui->welcome_box
            );
            break;

        default: break;
    }

    gtk_widget_show_all(
        welcome_ui->box_container
    );

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

GtkWidget* build_welcome_box(void) {
    GtkWidget *welcome_screen = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(welcome_screen, TRUE);
    gtk_widget_set_vexpand(welcome_screen, TRUE);
    gtk_style_context_add_class(gtk_widget_get_style_context(welcome_screen), "content");

    GtkWidget *top_ribbon = create_top_ribbon_widget();
    GtkWidget *top_separator = create_top_separator_widget();

    GtkWidget* overlay = gtk_overlay_new();

    GtkWidget* bglight = gtk_image_new_from_resource("/uk/oddmatics/wintc/logonui/bglight.png");
    gtk_widget_set_halign(bglight, GTK_ALIGN_START);
    gtk_widget_set_valign(bglight, GTK_ALIGN_START);
    gtk_widget_set_vexpand(bglight, FALSE);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    GtkWidget *welcome_label = gtk_label_new("Welcome");
    gtk_style_context_add_class(gtk_widget_get_style_context(welcome_label), "welcome-label");
    gtk_label_set_xalign(GTK_LABEL(welcome_label), 0.8f);
    gtk_label_set_yalign(GTK_LABEL(welcome_label), 0.5f);

    gtk_box_pack_start(GTK_BOX(content), welcome_label, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(overlay), content);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), bglight);

    GtkWidget *bottom_separator = create_bottom_separator_widget();
    GtkWidget *bottom_ribbon = create_bottom_ribbon_widget();

    gtk_box_pack_start(GTK_BOX(welcome_screen), top_ribbon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(welcome_screen), top_separator, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(welcome_screen), overlay, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(welcome_screen), bottom_separator, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(welcome_screen), bottom_ribbon, FALSE, FALSE, 0);

    return welcome_screen;
}

GtkWidget* build_wait_box(void) {
    GtkWidget *wait_screen = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(wait_screen, TRUE);
    gtk_widget_set_vexpand(wait_screen, TRUE);
    gtk_style_context_add_class(gtk_widget_get_style_context(wait_screen), "content");

    GtkWidget *top_ribbon = create_top_ribbon_widget();
    GtkWidget *top_separator = create_top_separator_widget();

    GtkWidget* overlay = gtk_overlay_new();
    
    GtkWidget* bglight = gtk_image_new_from_resource("/uk/oddmatics/wintc/logonui/bglight.png");
    gtk_widget_set_halign(bglight, GTK_ALIGN_START);
    gtk_widget_set_valign(bglight, GTK_ALIGN_START);
    gtk_widget_set_vexpand(bglight, FALSE);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(content, TRUE);
    gtk_widget_set_vexpand(content, TRUE);

    GtkWidget *wait_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(wait_box, GTK_ALIGN_END);
    gtk_widget_set_valign(wait_box, GTK_ALIGN_CENTER);

    // FIXME: The wait box shoulld appear with a margin of about 20%
    //        not a fixed value.
    gtk_widget_set_margin_end(wait_box, 200);

    GtkWidget *label = gtk_label_new("Please wait...");
    gtk_label_set_xalign(GTK_LABEL(label), 1.0f);
    gtk_style_context_add_class(gtk_widget_get_style_context(label), "wait-label");

    GtkWidget *logo = gtk_image_new_from_resource("/uk/oddmatics/wintc/logonui/logo.png");
    gtk_widget_set_halign(logo, GTK_ALIGN_END);

    gtk_box_pack_start(GTK_BOX(wait_box), logo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(wait_box), label, FALSE, FALSE, 0);

    gtk_box_pack_end(GTK_BOX(content), wait_box, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(overlay), content);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), bglight);

    GtkWidget *bottom_separator = create_bottom_separator_widget();
    GtkWidget *bottom_ribbon = create_bottom_ribbon_widget();

    gtk_box_pack_start(GTK_BOX(wait_screen), top_ribbon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(wait_screen), top_separator, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(wait_screen), overlay, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(wait_screen), bottom_separator, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(wait_screen), bottom_ribbon, FALSE, FALSE, 0);

    gtk_widget_show_all(wait_screen);
    return wait_screen;
}

GtkWidget* build_login_box(WinTCGinaLogonSession* logon_session) {
    GtkWidget *login_screen = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *top_ribbon = create_top_ribbon_widget();
    GtkWidget *top_separator = create_top_separator_widget();
    
    GtkWidget* overlay = gtk_overlay_new();

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(content), "content");

    GtkWidget *instruction_wrapper = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    
    GtkWidget* instruction_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_end(instruction_box, 20);
    gtk_widget_set_halign(instruction_box, GTK_ALIGN_END);
    gtk_widget_set_valign(instruction_box, GTK_ALIGN_CENTER);
   
    GtkWidget* bglight = gtk_image_new_from_resource("/uk/oddmatics/wintc/logonui/bglight.png");
    gtk_widget_set_halign(bglight, GTK_ALIGN_START);
    gtk_widget_set_valign(bglight, GTK_ALIGN_START);
    gtk_widget_set_vexpand(bglight, FALSE);

    // TODO: The logo should be configurable between the static and animated version.
    GtkWidget *logo = gtk_image_new_from_resource("/uk/oddmatics/wintc/logonui/logo.png");
    gtk_widget_set_margin_bottom(GTK_WIDGET(logo), 20);
    gtk_widget_set_halign(logo, GTK_ALIGN_END);
    gtk_widget_set_valign(logo, GTK_ALIGN_CENTER);
    gtk_widget_set_vexpand(logo, FALSE);

    GtkWidget *instruction_label = gtk_label_new("To begin, click your user name");
    gtk_style_context_add_class(gtk_widget_get_style_context(instruction_label), "instruction-label");
    gtk_widget_set_halign(instruction_label, GTK_ALIGN_END);
    gtk_widget_set_margin_end(instruction_label, 22);
    gtk_widget_set_hexpand(instruction_label, TRUE);

    gtk_box_pack_start(GTK_BOX(instruction_box), logo, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(instruction_box), instruction_label, FALSE, FALSE, 0);

    gtk_box_pack_end(GTK_BOX(instruction_wrapper), instruction_box, TRUE, TRUE, 0);

    GtkWidget* vertical_separator = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(vertical_separator, 1, -1); 
    gtk_widget_set_hexpand(vertical_separator, FALSE);
    gtk_widget_set_vexpand(vertical_separator, TRUE);
    gtk_style_context_add_class(gtk_widget_get_style_context(vertical_separator), "vertical-separator");
    
    GtkWidget *userlist_wrapper = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_margin_start(userlist_wrapper, 20);
    gtk_widget_set_halign(userlist_wrapper, GTK_ALIGN_START);

    GtkWidget *userlist = wintc_welcome_user_list_new(logon_session);

    gtk_box_pack_start(GTK_BOX(userlist_wrapper), userlist, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(content), instruction_wrapper, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(content), userlist_wrapper, TRUE, TRUE, 0);
    gtk_box_set_center_widget(GTK_BOX(content), vertical_separator);

    gtk_container_add(GTK_CONTAINER(overlay), content);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), bglight);

    GtkWidget *bottom_separator = create_bottom_separator_widget();

    GtkWidget *bottom_ribbon = create_bottom_ribbon_widget();

    GtkWidget* footer_buttons = create_footer_widget();
    
    gtk_box_pack_start(GTK_BOX(bottom_ribbon), footer_buttons, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(login_screen), top_ribbon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(login_screen), top_separator, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(login_screen), overlay, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(login_screen), bottom_separator, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(login_screen), bottom_ribbon, FALSE, FALSE, 0);

    return login_screen;
}

GtkWidget* create_top_ribbon_widget(void) {
    GtkWidget *top_ribbon = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(top_ribbon, -1, 71);
    gtk_widget_set_hexpand(top_ribbon, TRUE);
    gtk_widget_set_vexpand(top_ribbon, FALSE);
    gtk_style_context_add_class(gtk_widget_get_style_context(top_ribbon), "header");

    return top_ribbon;
}

GtkWidget* create_top_separator_widget(void) {
    GtkWidget *top_separator = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_size_request(top_separator, -1, 2);
    gtk_widget_set_hexpand(top_separator, TRUE);
    gtk_widget_set_vexpand(top_separator, FALSE);
    gtk_style_context_add_class(gtk_widget_get_style_context(top_separator), "header-separator");

    return top_separator;
}

GtkWidget* create_vertical_separator_widget(void) {
    GtkWidget *vertical_separator = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(vertical_separator, 1, -1); 
    gtk_widget_set_hexpand(vertical_separator, FALSE);
    gtk_widget_set_vexpand(vertical_separator, TRUE);
    gtk_style_context_add_class(gtk_widget_get_style_context(vertical_separator), "vertical-separator");

    return vertical_separator;
}

GtkWidget* create_bottom_separator_widget(void) {
    GtkWidget *bottom_separator = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_size_request(bottom_separator, -1, 2);
    gtk_widget_set_hexpand(bottom_separator, TRUE);
    gtk_widget_set_vexpand(bottom_separator, FALSE);
    gtk_style_context_add_class(gtk_widget_get_style_context(bottom_separator), "footer-separator");

    return bottom_separator;
}   

GtkWidget* create_bottom_ribbon_widget(void) {
    GtkWidget *bottom_ribbon = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(bottom_ribbon, -1, 93);
    gtk_widget_set_hexpand(bottom_ribbon, TRUE);
    gtk_widget_set_vexpand(bottom_ribbon, FALSE);
    gtk_style_context_add_class(gtk_widget_get_style_context(bottom_ribbon), "footer");

    return bottom_ribbon;
}

GtkWidget* create_shutdown_widget(void) {
    GtkWidget *shutdown_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_modal(GTK_WINDOW(shutdown_window), FALSE);
    gtk_window_set_position(GTK_WINDOW(shutdown_window), GTK_WIN_POS_CENTER_ALWAYS);
    gtk_window_set_resizable(GTK_WINDOW(shutdown_window), FALSE);
    gtk_window_set_decorated(GTK_WINDOW(shutdown_window), FALSE);
    gtk_widget_set_size_request(shutdown_window, 250, 140);
    gtk_style_context_add_class(gtk_widget_get_style_context(shutdown_window), "content");
    gtk_style_context_add_class(gtk_widget_get_style_context(shutdown_window), "shutdown-popup");
    
    gtk_window_set_keep_above(GTK_WINDOW(shutdown_window), TRUE);
    gtk_window_set_type_hint(GTK_WINDOW(shutdown_window), GDK_WINDOW_TYPE_HINT_DIALOG);

    GtkWidget *shutdown_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(shutdown_box, TRUE);
    gtk_widget_set_vexpand(shutdown_box, TRUE);

    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(header), "header");

    
    GtkWidget* mini_logo = gtk_image_new_from_resource("/uk/oddmatics/wintc/logonui/mini-logo.png");
    GtkWidget* shutdown_label = gtk_label_new("Turn off computer");
    gtk_label_set_xalign(GTK_LABEL(shutdown_label), 0.0f);
    gtk_style_context_add_class(gtk_widget_get_style_context(shutdown_label), "shutdown-title");
    
    gtk_box_pack_start(GTK_BOX(header), shutdown_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(header), mini_logo, FALSE, FALSE, 0);

    GtkWidget *header_separator = create_top_separator_widget();

    GtkWidget* button_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(button_box, GTK_ALIGN_CENTER);

    GtkWidget *turnoff_btn = gtk_button_new();
    gtk_widget_set_valign(turnoff_btn, GTK_ALIGN_CENTER);
    gtk_button_set_image(GTK_BUTTON(turnoff_btn), gtk_image_new_from_resource("/uk/oddmatics/wintc/logonui/shtdbtn.png"));
    gtk_style_context_add_class(gtk_widget_get_style_context(turnoff_btn), "plain-button");
    gtk_box_pack_start(GTK_BOX(button_box), turnoff_btn, FALSE, FALSE, 0);
    
    GtkWidget *footer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_size_request(footer, -1, 38);
    gtk_widget_set_hexpand(footer, TRUE);
    gtk_style_context_add_class(gtk_widget_get_style_context(footer), "header");

    GtkWidget *constrainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(constrainer, -1, 30);
    gtk_widget_set_valign(constrainer, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_end(constrainer, 5);

    GtkWidget *cancel_btn = gtk_button_new_with_label("Cancel");
    gtk_widget_set_halign(cancel_btn, GTK_ALIGN_END);
    gtk_widget_set_vexpand(cancel_btn, FALSE);
    gtk_widget_set_size_request(cancel_btn, -1, 30);

    gtk_box_pack_start(GTK_BOX(constrainer), cancel_btn, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(footer), constrainer, FALSE, FALSE, 0);


    gtk_box_pack_start(GTK_BOX(shutdown_box), header, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(shutdown_box), header_separator, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(shutdown_box), button_box, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(shutdown_box), footer, FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(shutdown_window), shutdown_box);
    
    g_signal_connect(cancel_btn, "clicked", G_CALLBACK(on_cancel_pressed), shutdown_window);
    
    return shutdown_window;
}

GtkWidget* create_footer_widget(void) {
    GtkWidget *footer = GTK_WIDGET(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_widget_set_hexpand(GTK_WIDGET(footer), TRUE);
    gtk_widget_set_vexpand(GTK_WIDGET(footer), FALSE);
    gtk_widget_set_margin_top(GTK_WIDGET(footer), 20);

    GtkWidget *shutdown_event_wrapper = gtk_event_box_new();
    gtk_widget_add_events(shutdown_event_wrapper, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);

    GtkWidget *shutdown_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    GdkPixbuf *idle_pixbuf = gdk_pixbuf_new_from_resource("/uk/oddmatics/wintc/logonui/shtdbtn.png", NULL);
    GdkPixbuf *active_pixbuf = gdk_pixbuf_new_from_resource("/uk/oddmatics/wintc/logonui/shtdbtna.png", NULL);
    GtkWidget *shutdown_button = wintc_welcome_button_new_with_pixbufs(idle_pixbuf, active_pixbuf);
    gtk_widget_set_margin_start(shutdown_button, 20);
    gtk_widget_set_can_focus(shutdown_button, FALSE);

    GtkWidget *shutdown_label = gtk_label_new("Turn off computer");
    gtk_style_context_add_class(gtk_widget_get_style_context(shutdown_label), "shutdown-label");
    gtk_widget_set_margin_start(shutdown_label, 5);

    gtk_box_pack_start(GTK_BOX(shutdown_box), shutdown_button, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(shutdown_box), shutdown_label, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(shutdown_event_wrapper), shutdown_box);

    ShutdownWidgets *widgets = g_new0(ShutdownWidgets, 1);
    widgets->shutdown_button = shutdown_button;
    widgets->shutdown_label = shutdown_label;

    g_signal_connect(shutdown_event_wrapper, "button-press-event", G_CALLBACK(on_shutdown_button_clicked), widgets);
    g_signal_connect(shutdown_event_wrapper, "enter-notify-event", G_CALLBACK(on_shutdown_button_enter), widgets);
    g_signal_connect(shutdown_event_wrapper, "leave-notify-event", G_CALLBACK(on_shutdown_button_leave), widgets);

    GtkWidget *user_intstruction_label = gtk_label_new("After you log on, you can add or change accounts.\nJust go to Control Panel and click User Accounts.");
    gtk_style_context_add_class(gtk_widget_get_style_context(user_intstruction_label), "accounts-label");
    gtk_widget_set_margin_end(user_intstruction_label, 40);

    gtk_box_pack_start(GTK_BOX(footer), shutdown_event_wrapper, FALSE, FALSE, 0);
    

    gtk_box_pack_end(GTK_BOX(footer), user_intstruction_label, FALSE, FALSE, 0);
    return footer;

}

//
// CALLBACKS
//
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

static gboolean on_shutdown_button_clicked(WINTC_UNUSED(GtkWidget *widget), WINTC_UNUSED(GdkEvent *event), gpointer data) {
    ShutdownWidgets *widgets = (ShutdownWidgets *)data;
    GtkButton* button = GTK_BUTTON(widgets->shutdown_button);
    gtk_button_clicked(button);
    gtk_widget_show_all(create_shutdown_widget());
    return FALSE;
}

static gboolean on_shutdown_button_enter(GtkWidget *widget, WINTC_UNUSED(GdkEvent *event), gpointer data) {
    ShutdownWidgets *widgets = (ShutdownWidgets *)data;
    gtk_style_context_add_class(gtk_widget_get_style_context(widgets->shutdown_label), "underline");

    GdkWindow *window = gtk_widget_get_window(widget);
    GdkDisplay *display = gdk_window_get_display(window);
    GdkCursor *cursor = gdk_cursor_new_from_name(display, "pointer");
    gdk_window_set_cursor(window, cursor);
    g_object_unref(cursor);

    return FALSE;
}

static gboolean on_shutdown_button_leave(GtkWidget *widget, GdkEventCrossing *event, gpointer data) {
    if (event->detail == GDK_NOTIFY_INFERIOR)
    {
        return FALSE;
    }

    ShutdownWidgets *widgets = (ShutdownWidgets *)data;
    gtk_style_context_remove_class(gtk_widget_get_style_context(widgets->shutdown_label), "underline");

    GdkWindow *window = gtk_widget_get_window(widget);
    gdk_window_set_cursor(window, NULL); 
    return FALSE;
}

static gboolean on_cancel_pressed(WINTC_UNUSED(GtkWidget *widget), WINTC_UNUSED(GdkEvent *event), gpointer data) {
    GtkWidget* shutdown_window = GTK_WIDGET(data);
    gtk_widget_destroy(shutdown_window);
    return TRUE;
}
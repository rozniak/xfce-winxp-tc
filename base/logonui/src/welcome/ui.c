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

#define LOGOANI_FRAME_COUNT 50
#define LOGOANI_FRAME_RATE  15

#define TOP_RIBBON_THICKNESS    78
#define BOTTOM_RIBBON_THICKNESS 94
#define STRIP_THICKNESS         2

// HMARGIN is potentially 40 or thereabouts
// WIDTH is based off 360 per half + 1 for vseparator
// VMARGIN is a guesstimate
//
#define INNER_BOX_HMARGIN  32
#define INNER_BOX_WIDTH    (720 + 1)
#define INNER_BOX_VMARGIN  8
#define INNER_BOX_COLLAPSE ((INNER_BOX_HMARGIN * 2) + INNER_BOX_WIDTH)

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

static gboolean wintc_welcome_ui_draw(
    GtkWidget* widget,
    cairo_t*   cr
);
static void wintc_welcome_ui_size_allocate(
    GtkWidget*     widget,
    GtkAllocation* allocation
);

static void wintc_welcome_ui_add(
    GtkContainer* container,
    GtkWidget*    widget
);
static void wintc_welcome_ui_forall(
    GtkContainer* container,
    gboolean      include_internals,
    GtkCallback   callback,
    gpointer      callback_data
);
static void wintc_welcome_ui_remove(
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
    gpointer   user_data
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
    GtkContainerClass __parent__;
};

struct _WinTCWelcomeUI
{
    GtkContainer __parent__;

    GSList* child_widgets;

    // Graphic resources
    //
    GdkPixbuf*       pixbuf_bglight;
    GdkPixbuf*       pixbuf_hsepa;
    GdkPixbuf*       pixbuf_hsepb;
    cairo_surface_t* surface_bglight;
    cairo_surface_t* surface_hsepa;
    cairo_surface_t* surface_hsepb;

    // Logo resources
    //
    guint      ani_id_logo_ins;
    guint      ani_id_logo_wait;
    GdkPixbuf* pixbuf_logo;
    GdkPixbuf* pixbuf_logoani;

    // UI
    //
    GtkWidget* box_container;

    GtkWidget* box_instruction;
    GtkWidget* box_login;
    GtkWidget* box_wait;
    GtkWidget* box_welcome;

    GtkWidget* animation_logo_ins;
    GtkWidget* animation_logo_wait;
    GtkWidget* label_instruction;
    GtkWidget* label_wait;
    GtkWidget* label_welcome;
    GtkWidget* scrollwnd;
    GtkWidget* user_list;

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
    GtkWidgetClass*    widget_class    = GTK_WIDGET_CLASS(klass);
    GObjectClass*      object_class    = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_welcome_ui_constructed;
    object_class->dispose      = wintc_welcome_ui_dispose;
    object_class->get_property = wintc_welcome_ui_get_property;
    object_class->set_property = wintc_welcome_ui_set_property;

    widget_class->draw          = wintc_welcome_ui_draw;
    widget_class->size_allocate = wintc_welcome_ui_size_allocate;

    container_class->add    = wintc_welcome_ui_add;
    container_class->forall = wintc_welcome_ui_forall;
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

    // Set up image resources
    //
    self->pixbuf_bglight =
        gdk_pixbuf_new_from_resource(
            "/uk/oddmatics/wintc/logonui/bglight.png",
            NULL // FIXME: Error reporting
        );
    self->pixbuf_hsepa =
        gdk_pixbuf_new_from_resource(
            "/uk/oddmatics/wintc/logonui/hsepa.png",
            NULL // FIXME: Error reporting
        );
    self->pixbuf_hsepb =
        gdk_pixbuf_new_from_resource(
            "/uk/oddmatics/wintc/logonui/hsepb.png",
            NULL // FIXME: Error reporting
        );

    self->surface_bglight =
        gdk_cairo_surface_create_from_pixbuf(
            self->pixbuf_bglight,
            1,
            NULL
        );
    self->surface_hsepa =
        gdk_cairo_surface_create_from_pixbuf(
            self->pixbuf_hsepa,
            1,
            NULL
        );
    self->surface_hsepb =
        gdk_cairo_surface_create_from_pixbuf(
            self->pixbuf_hsepb,
            1,
            NULL
        );

    self->pixbuf_logo =
        gdk_pixbuf_new_from_resource(
            "/uk/oddmatics/wintc/logonui/logo.png",
            NULL // FIXME: Error reporting
        );
    self->pixbuf_logoani =
        gdk_pixbuf_new_from_resource(
            "/uk/oddmatics/wintc/logonui/logoani.png",
            NULL // FIXME: Error reporting
        );
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
    //
    welcome_ui->box_wait = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    welcome_ui->animation_logo_wait = wintc_ctl_animation_new();
    welcome_ui->label_wait          = gtk_label_new("Please wait...");

    welcome_ui->ani_id_logo_wait =
        wintc_ctl_animation_add_static(
            WINTC_CTL_ANIMATION(welcome_ui->animation_logo_wait),
            welcome_ui->pixbuf_logo
        );
    wintc_ctl_animation_set_halign(
        WINTC_CTL_ANIMATION(welcome_ui->animation_logo_wait),
        GTK_ALIGN_END
    );
    wintc_ctl_animation_set_valign(
        WINTC_CTL_ANIMATION(welcome_ui->animation_logo_wait),
        GTK_ALIGN_END
    );
    wintc_ctl_animation_play(
        WINTC_CTL_ANIMATION(welcome_ui->animation_logo_wait),
        welcome_ui->ani_id_logo_wait,
        0,
        WINTC_CTL_ANIMATION_INFINITE
    );

    gtk_label_set_xalign(GTK_LABEL(welcome_ui->label_wait), 1.0f);
    gtk_label_set_yalign(GTK_LABEL(welcome_ui->label_wait), 0.0f);

    gtk_box_pack_start(
        GTK_BOX(welcome_ui->box_wait),
        welcome_ui->animation_logo_wait,
        TRUE,
        TRUE,
        0
    );
    gtk_box_pack_start(
        GTK_BOX(welcome_ui->box_wait),
        welcome_ui->label_wait,
        TRUE,
        TRUE,
        0
    );

    // Set up instruction box
    //
    welcome_ui->box_instruction = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    welcome_ui->animation_logo_ins = wintc_ctl_animation_new();
    welcome_ui->label_instruction  =
        gtk_label_new("To begin, click your user name");

    gtk_label_set_xalign(GTK_LABEL(welcome_ui->label_instruction), 1.0f);
    gtk_label_set_yalign(GTK_LABEL(welcome_ui->label_instruction), 0.0f);

    welcome_ui->ani_id_logo_ins =
        wintc_ctl_animation_add_framesheet(
            WINTC_CTL_ANIMATION(welcome_ui->animation_logo_ins),
            welcome_ui->pixbuf_logoani,
            LOGOANI_FRAME_COUNT
        );
    wintc_ctl_animation_set_halign(
        WINTC_CTL_ANIMATION(welcome_ui->animation_logo_ins),
        GTK_ALIGN_END
    );
    wintc_ctl_animation_set_valign(
        WINTC_CTL_ANIMATION(welcome_ui->animation_logo_ins),
        GTK_ALIGN_END
    );
    wintc_ctl_animation_play(
        WINTC_CTL_ANIMATION(welcome_ui->animation_logo_ins),
        welcome_ui->ani_id_logo_ins,
        LOGOANI_FRAME_RATE,
        WINTC_CTL_ANIMATION_INFINITE
    );

    gtk_box_pack_start(
        GTK_BOX(welcome_ui->box_instruction),
        welcome_ui->animation_logo_ins,
        TRUE,
        TRUE,
        0
    );
    gtk_box_pack_start(
        GTK_BOX(welcome_ui->box_instruction),
        welcome_ui->label_instruction,
        TRUE,
        TRUE,
        0
    );

    // Set up login box
    //
    welcome_ui->box_login = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    welcome_ui->scrollwnd = gtk_scrolled_window_new(NULL, NULL);
    welcome_ui->user_list =
        wintc_welcome_user_list_new(welcome_ui->logon_session);

    gtk_container_add(
        GTK_CONTAINER(welcome_ui->scrollwnd),
        welcome_ui->user_list
    );

    gtk_box_pack_start(
        GTK_BOX(welcome_ui->box_login),
        welcome_ui->scrollwnd,
        FALSE,
        FALSE,
        0
    );
    gtk_box_set_center_widget(
        GTK_BOX(welcome_ui->box_login),
        welcome_ui->scrollwnd
    );

    // Set up 'welcome' box
    //
    welcome_ui->box_welcome   = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    welcome_ui->label_welcome = gtk_label_new("welcome");

    gtk_label_set_xalign(GTK_LABEL(welcome_ui->label_welcome), 0.8f);
    gtk_label_set_yalign(GTK_LABEL(welcome_ui->label_welcome), 0.5f);

    gtk_box_pack_start(
        GTK_BOX(welcome_ui->box_welcome),
        welcome_ui->label_welcome,
        TRUE,
        TRUE,
        0
    );

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
    wintc_widget_add_style_class(welcome_ui->box_instruction, "box-instruction");
    wintc_widget_add_style_class(welcome_ui->box_login,       "box-login");
    wintc_widget_add_style_class(welcome_ui->box_wait,        "box-wait");
    wintc_widget_add_style_class(welcome_ui->box_welcome,     "box-welcome");

    // Hold an additional reference to the boxes, so we can add/remove
    // them ourselves without them getting binned
    //
    g_object_ref(welcome_ui->box_instruction);
    g_object_ref(welcome_ui->box_login);
    g_object_ref(welcome_ui->box_wait);
    g_object_ref(welcome_ui->box_welcome);

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
    WinTCWelcomeUI* welcome_ui = WINTC_WELCOME_UI(object);

    // Bin graphical resources
    //
    cairo_surface_destroy(welcome_ui->surface_bglight);
    cairo_surface_destroy(welcome_ui->surface_hsepa);
    cairo_surface_destroy(welcome_ui->surface_hsepb);
    g_clear_object(&(welcome_ui->pixbuf_bglight));
    g_clear_object(&(welcome_ui->pixbuf_hsepa));
    g_clear_object(&(welcome_ui->pixbuf_hsepb));
    g_clear_object(&(welcome_ui->pixbuf_logoani));

    // Bin additional references held for the boxes
    //
    g_clear_object(&(welcome_ui->box_instruction));
    g_clear_object(&(welcome_ui->box_login));
    g_clear_object(&(welcome_ui->box_wait));
    g_clear_object(&(welcome_ui->box_welcome));

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

static gboolean wintc_welcome_ui_draw(
    GtkWidget* widget,
    cairo_t*   cr
)
{
    WinTCWelcomeUI* welcome_ui = WINTC_WELCOME_UI(widget);

    gint height = gtk_widget_get_allocated_height(widget);
    gint width  = gtk_widget_get_allocated_width(widget);

    // Background is #5A7EDC
    //
    cairo_set_source_rgb(cr, 0.35f, 0.49f, 0.86f);
    cairo_paint(cr);

    // Draw top banner edge
    //
    gint hsepa_width = gdk_pixbuf_get_width(welcome_ui->pixbuf_hsepa);

    cairo_save(cr);

    cairo_scale(cr, (double) width / (double) hsepa_width, 1.0f);
    cairo_set_source_surface(
        cr,
        welcome_ui->surface_hsepa,
        0.0f,
        (double) TOP_RIBBON_THICKNESS
    );
    cairo_paint(cr);

    cairo_restore(cr);

    // Draw top banner fill (#00309C)
    //
    cairo_save(cr);

    cairo_rectangle(
        cr,
        0.0f,
        0.0f,
        (double) width,
        (double) TOP_RIBBON_THICKNESS
    );
    cairo_clip(cr);
    cairo_set_source_rgb(cr, 0.0f, 0.19f, 0.61f);
    cairo_paint(cr);

    cairo_restore(cr);

    // Draw bg light
    //
    cairo_set_source_surface(
        cr,
        welcome_ui->surface_bglight,
        0.0f,
        (double) (TOP_RIBBON_THICKNESS + STRIP_THICKNESS)
    );
    cairo_paint(cr);

    // Draw bottom banner edge
    //
    gint hsepb_width = gdk_pixbuf_get_width(welcome_ui->pixbuf_hsepb);

    cairo_save(cr);

    cairo_scale(cr, (double) width / (double) hsepb_width, 1.0f);
    cairo_set_source_surface(
        cr,
        welcome_ui->surface_hsepb,
        0.0f,
        (double) (height - BOTTOM_RIBBON_THICKNESS - STRIP_THICKNESS)
    );
    cairo_paint(cr);

    cairo_restore(cr);

    // Draw bottom banner fill (#1D32A4)
    // FIXME: This should actually be a gradient!
    //
    cairo_save(cr);

    cairo_rectangle(
        cr,
        0.0f,
        (double) (height - BOTTOM_RIBBON_THICKNESS),
        (double) width,
        (double) BOTTOM_RIBBON_THICKNESS
    );
    cairo_clip(cr);
    cairo_set_source_rgb(cr, 0.11f, 0.19f, 0.64f);
    cairo_paint(cr);

    cairo_restore(cr);

    // Chain up
    //
    (GTK_WIDGET_CLASS(wintc_welcome_ui_parent_class))->draw(widget, cr);

    return FALSE;
}

static void wintc_welcome_ui_size_allocate(
    GtkWidget*     widget,
    GtkAllocation* allocation
)
{
    WinTCWelcomeUI* welcome_ui = WINTC_WELCOME_UI(widget);

    gtk_widget_set_allocation(widget, allocation);

    // FIXME: We're not handling collapse at low res (640x480)
    // FIXME: We're also not doing anything about low res on the Y axis
    //
    if (allocation->width < INNER_BOX_COLLAPSE)
    {
        g_critical(
            "Screen res is too low, 800x600 required and we got width of %d",
            allocation->width
        );
        return;
    }

    // Just deal with >= 800x600 for now
    //
    gint centre_ui  = allocation->width / 2;
    gint centre_box = (INNER_BOX_WIDTH / 2);

    GtkAllocation box_alloc;

    box_alloc.x      = centre_ui - centre_box + INNER_BOX_HMARGIN;
    box_alloc.y      = TOP_RIBBON_THICKNESS
                           + STRIP_THICKNESS
                           + INNER_BOX_VMARGIN;
    box_alloc.width  = INNER_BOX_WIDTH;
    box_alloc.height = allocation->height
                           - TOP_RIBBON_THICKNESS
                           - BOTTOM_RIBBON_THICKNESS
                           - (STRIP_THICKNESS   * 2)
                           - (INNER_BOX_VMARGIN * 2);

    gtk_widget_size_allocate(
        welcome_ui->box_container,
        &box_alloc
    );
}

static void wintc_welcome_ui_add(
    WINTC_UNUSED(GtkContainer* container),
    WINTC_UNUSED(GtkWidget*    widget)
)
{
    g_critical("%s", "wintc_welcome_ui_add - not allowed!");
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
            gtk_container_remove(
                GTK_CONTAINER(welcome_ui->box_container),
                welcome_ui->box_wait
            );
            break;

        case WINTC_GINA_STATE_PROMPT:
            gtk_container_remove(
                GTK_CONTAINER(welcome_ui->box_container),
                welcome_ui->box_instruction
            );
            gtk_container_remove(
                GTK_CONTAINER(welcome_ui->box_container),
                welcome_ui->box_login
            );
            break;

        case WINTC_GINA_STATE_LAUNCHING:
            gtk_container_remove(
                GTK_CONTAINER(welcome_ui->box_container),
                welcome_ui->box_welcome
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
                GTK_BOX(welcome_ui->box_container),
                welcome_ui->box_wait,
                TRUE,
                TRUE,
                0
            );
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
            gtk_box_pack_start(
                GTK_BOX(welcome_ui->box_container),
                welcome_ui->box_instruction,
                TRUE,
                TRUE,
                0
            );
            gtk_box_pack_start(
                GTK_BOX(welcome_ui->box_container),
                welcome_ui->box_login,
                TRUE,
                TRUE,
                0
            );
            break;

        case WINTC_GINA_STATE_LAUNCHING:
            gtk_box_pack_start(
                GTK_BOX(welcome_ui->box_container),
                welcome_ui->box_welcome,
                TRUE,
                TRUE,
                0
            );
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


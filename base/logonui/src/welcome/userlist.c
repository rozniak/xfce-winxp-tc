#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <lightdm.h>
#include <wintc/comgtk.h>
#include <wintc/msgina.h>

#include "userlist.h"

#define DELAY_SECONDS_POLL 1

// Defining offsets
//
#define USER_LISTING_WIDTH 318

#define USER_LISTING_GAP_Y_HALF 5

#define USER_TILE_OFFSET_X 7
#define USER_TILE_OFFSET_Y 6

#define USER_PIC_OFFSET    5
#define USER_PIC_SIZE      48

#define USER_NAME_OFFSET_X 75
#define USER_NAME_OFFSET_Y 25

//
// PRIVATE ENUMS
//
enum
{
    PROP_LOGON_SESSION = 1,
    PROP_HADJUSTMENT,
    PROP_VADJUSTMENT,
    PROP_HSCROLL_POLICY,
    PROP_VSCROLL_POLICY,
    N_PROPERTIES
};

//
// PRIVATE STRUCTURES
//
typedef struct _WinTCWelcomeUserpic
{
    GdkPixbuf*       pixbuf;
    cairo_surface_t* surface;
} WinTCWelcomeUserpic;

//
// FORWARD DECLARATIONS
//
static void wintc_welcome_user_list_finalize(
    GObject* gobject
);
static void wintc_welcome_user_list_get_property(
    GObject*    gobject,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_welcome_user_list_set_property(
    GObject*      gobject,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static gboolean wintc_welcome_user_list_button_press_event(
    GtkWidget*      widget,
    GdkEventButton* event
);
static gboolean wintc_welcome_user_list_draw(
    GtkWidget* widget,
    cairo_t*   cr
);
static void wintc_welcome_user_list_get_preferred_height(
    GtkWidget* widget,
    gint*      minimum_height,
    gint*      natural_height
);
static void wintc_welcome_user_list_get_preferred_height_for_width(
    GtkWidget* widget,
    gint       width,
    gint*      minimum_height,
    gint*      natural_height
);
static void wintc_welcome_user_list_get_preferred_width(
    GtkWidget* widget,
    gint*      minimum_width,
    gint*      natural_width
);
static void wintc_welcome_user_list_get_preferred_width_for_height(
    GtkWidget* widget,
    gint       height,
    gint*      minimum_width,
    gint*      natural_width
);
static void wintc_welcome_user_list_realize(
    GtkWidget* widget
);
static void wintc_welcome_user_list_size_allocate(
    GtkWidget*     widget,
    GtkAllocation* allocation
);

static void wintc_welcome_user_list_add(
    GtkContainer* container,
    GtkWidget*    widget
);
static void wintc_welcome_user_list_forall(
    GtkContainer* container,
    gboolean      include_internals,
    GtkCallback   callback,
    gpointer      callback_data
);
static void wintc_welcome_user_list_remove(
    GtkContainer* container,
    GtkWidget*    widget
);

static void wintc_welcome_user_list_internal_add(
    WinTCWelcomeUserList* user_list,
    GtkWidget*            widget
);
static void wintc_welcome_user_list_select_user(
    WinTCWelcomeUserList* user_list,
    gint                  index
);
static void wintc_welcome_user_list_set_vadjustment(
    WinTCWelcomeUserList* user_list,
    GtkAdjustment*        adjustment
);
static void wintc_welcome_user_list_set_vadjustment_values(
    WinTCWelcomeUserList* user_list
);

static void draw_user(
    cairo_t*              cr,
    LightDMUser*          user,
    gboolean              selected,
    WinTCWelcomeUserList* user_list
);
static WinTCWelcomeUserpic* wintc_welcome_user_list_get_userpic(
    WinTCWelcomeUserList* user_list,
    const gchar*          path
);

static void free_userpic(
    WinTCWelcomeUserpic* userpic
);

static void on_self_adjustment_changed(
    GtkAdjustment*        adjustment,
    WinTCWelcomeUserList* user_list
);

static void on_logon_session_attempt_complete(
    WinTCGinaLogonSession* logon_session,
    WinTCGinaResponse      response,
    gpointer               user_data
);

static void on_button_go_clicked(
    GtkButton* self,
    gpointer   user_data
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCWelcomeUserListClass
{
    GtkContainerClass __parent__;
};

struct _WinTCWelcomeUserList
{
    GtkContainer __parent__;

    // UI
    //
    GdkWindow* hwnd;

    GSList* child_widgets;

    GtkWidget* box_auth;
    GtkWidget* button_go;
    GtkWidget* entry_password;

    WinTCGinaLogonSession* logon_session;

    // Control stuff
    //
    gint    selected_index;
    GList*  users;

    // Graphic resources
    //
    GdkPixbuf*       pixbuf_tile;
    GdkPixbuf*       pixbuf_tilehot;
    GdkPixbuf*       pixbuf_usersel;
    cairo_surface_t* surface_tile;
    cairo_surface_t* surface_tilehot;
    cairo_surface_t* surface_usersel;

    GHashTable*          map_path_to_userpic;
    WinTCWelcomeUserpic* default_userpic;

    // Geometry
    //
    gint item_height;

    GtkAdjustment* hadjustment;
    GtkAdjustment* vadjustment;

    gint hscroll_policy;
    gint vscroll_policy;

    gint scroll_x;
    gint scroll_y;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCWelcomeUserList,
    wintc_welcome_user_list,
    GTK_TYPE_CONTAINER,
    G_IMPLEMENT_INTERFACE(
        GTK_TYPE_SCROLLABLE,
        NULL
    )
)

static void wintc_welcome_user_list_class_init(
    WinTCWelcomeUserListClass* klass
)
{
    GtkContainerClass* container_class = GTK_CONTAINER_CLASS(klass);
    GtkWidgetClass*    widget_class    = GTK_WIDGET_CLASS(klass);
    GObjectClass*      object_class    = G_OBJECT_CLASS(klass);

    object_class->finalize     = wintc_welcome_user_list_finalize;
    object_class->get_property = wintc_welcome_user_list_get_property;
    object_class->set_property = wintc_welcome_user_list_set_property;

    widget_class->button_press_event =
        wintc_welcome_user_list_button_press_event;
    widget_class->draw               = wintc_welcome_user_list_draw;
    widget_class->get_preferred_height =
        wintc_welcome_user_list_get_preferred_height;
    widget_class->get_preferred_height_for_width =
        wintc_welcome_user_list_get_preferred_height_for_width;
    widget_class->get_preferred_width  =
        wintc_welcome_user_list_get_preferred_width;
    widget_class->get_preferred_width_for_height =
        wintc_welcome_user_list_get_preferred_width_for_height;
    widget_class->realize            = wintc_welcome_user_list_realize;
    widget_class->size_allocate      = wintc_welcome_user_list_size_allocate;

    container_class->add    = wintc_welcome_user_list_add;
    container_class->forall = wintc_welcome_user_list_forall;
    container_class->remove = wintc_welcome_user_list_remove;

    g_object_class_install_property(
        object_class,
        PROP_LOGON_SESSION,
        g_param_spec_object(
            "logon-session",
            "LogonSession",
            "The GINA logon session instance.",
            WINTC_TYPE_GINA_LOGON_SESSION,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );

    // Scrollable interface properties
    //
    g_object_class_override_property(
        object_class,
        PROP_HADJUSTMENT,
        "hadjustment"
    );
    g_object_class_override_property(
        object_class,
        PROP_VADJUSTMENT,
        "vadjustment"
    );
    g_object_class_override_property(
        object_class,
        PROP_HSCROLL_POLICY,
        "hscroll-policy"
    );
    g_object_class_override_property(
        object_class,
        PROP_VSCROLL_POLICY,
        "vscroll-policy"
    );
}

static void wintc_welcome_user_list_init(
    WinTCWelcomeUserList* self
)
{
    gtk_widget_set_has_window(GTK_WIDGET(self), TRUE);

    // Set up image resources
    //
    self->pixbuf_tile =
        gdk_pixbuf_new_from_resource(
            "/uk/oddmatics/wintc/logonui/tile.png",
            NULL // FIXME: Error reporting
        );
    self->pixbuf_tilehot =
        gdk_pixbuf_new_from_resource(
            "/uk/oddmatics/wintc/logonui/tilehot.png",
            NULL // FIXME: Error reporting
        );
    self->pixbuf_usersel =
        gdk_pixbuf_new_from_resource(
            "/uk/oddmatics/wintc/logonui/usersel.png",
            NULL // FIXME: Error reporting
        );

    self->surface_tile =
        gdk_cairo_surface_create_from_pixbuf(
            self->pixbuf_tile,
            1,
            NULL
        );
    self->surface_tilehot =
        gdk_cairo_surface_create_from_pixbuf(
            self->pixbuf_tilehot,
            1,
            NULL
        );
    self->surface_usersel =
        gdk_cairo_surface_create_from_pixbuf(
            self->pixbuf_usersel,
            1,
            NULL
        );

    // Set up userpic map and default userpic
    //
    self->map_path_to_userpic =
        g_hash_table_new_full(
            g_str_hash,
            g_str_equal,
            (GDestroyNotify) g_free,
            (GDestroyNotify) free_userpic
        );

    self->default_userpic = g_new(WinTCWelcomeUserpic, 1);

    self->default_userpic->pixbuf =
        gdk_pixbuf_new_from_resource(
            "/uk/oddmatics/wintc/logonui/userpic.png",
            NULL // FIXME: Error reporting
        );
    self->default_userpic->surface  =
        gdk_cairo_surface_create_from_pixbuf(
            self->default_userpic->pixbuf,
            1,
            NULL
        );

    // Set up widgets
    //
    self->box_auth       = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    self->button_go      = gtk_button_new_with_label("Go");
    self->entry_password = gtk_entry_new();

    gtk_entry_set_visibility(
        GTK_ENTRY(self->entry_password),
        FALSE
    );

    gtk_box_pack_start(
        GTK_BOX(self->box_auth),
        self->entry_password,
        FALSE,
        FALSE,
        0
    );
    gtk_box_pack_start(
        GTK_BOX(self->box_auth),
        self->button_go,
        FALSE,
        FALSE,
        0
    );

    wintc_welcome_user_list_internal_add(
        self,
        self->box_auth
    );

    gtk_widget_set_sensitive(
        self->entry_password,
        FALSE
    );

    g_signal_connect(
        self->button_go,
        "clicked",
        G_CALLBACK(on_button_go_clicked),
        self
    );
    
    g_signal_connect(
        self->entry_password,
        "activate",
        G_CALLBACK(on_button_go_clicked),
        self
);

    // Add style classes
    //
    wintc_widget_add_style_class(self->button_go,      "go");
    wintc_widget_add_style_class(self->entry_password, "password");

    // Store item height
    //
    gint usersel_height = gdk_pixbuf_get_height(self->pixbuf_usersel);

    self->item_height = usersel_height + (2 * USER_LISTING_GAP_Y_HALF);

    // Set up default scroll policy
    //
    self->hscroll_policy = GTK_SCROLL_MINIMUM;
    self->vscroll_policy = GTK_SCROLL_NATURAL;

    // Retrieve users
    //
    self->users =
        lightdm_user_list_get_users(
            lightdm_user_list_get_instance()
        );

    self->selected_index = -1;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_welcome_user_list_finalize(
    GObject* gobject
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(gobject);

    cairo_surface_destroy(user_list->surface_tile);
    cairo_surface_destroy(user_list->surface_tilehot);
    cairo_surface_destroy(user_list->surface_usersel);
    g_clear_object(&user_list->pixbuf_tile);
    g_clear_object(&user_list->pixbuf_tilehot);
    g_clear_object(&user_list->pixbuf_usersel);

    (G_OBJECT_CLASS(wintc_welcome_user_list_parent_class))->finalize(gobject);
}

static void wintc_welcome_user_list_get_property(
    GObject*    gobject,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(gobject);

    switch (prop_id)
    {
        case PROP_HADJUSTMENT:
            g_value_set_object(value, user_list->hadjustment);
            break;

        case PROP_VADJUSTMENT:
            g_value_set_object(value, user_list->vadjustment);
            break;

        case PROP_HSCROLL_POLICY:
            g_value_set_enum(value, user_list->hscroll_policy);
            break;

        case PROP_VSCROLL_POLICY:
            g_value_set_enum(value, user_list->vscroll_policy);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
            break;
    }
}

static void wintc_welcome_user_list_set_property(
    GObject*      gobject,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(gobject);

    switch (prop_id)
    {
        case PROP_LOGON_SESSION:
           user_list->logon_session =
               WINTC_GINA_LOGON_SESSION(g_value_get_object(value));

           g_signal_connect(
               user_list->logon_session,
               "attempt-complete",
               G_CALLBACK(on_logon_session_attempt_complete),
               user_list
           );

           break;

        // Dummy out HADJUSTMENT - not supporting horz scrolling
        //
        case PROP_HADJUSTMENT: return;

        case PROP_VADJUSTMENT:
            wintc_welcome_user_list_set_vadjustment(
                user_list,
                g_value_get_object(value)
            );
            break;

        case PROP_HSCROLL_POLICY:
            if (user_list->hscroll_policy != g_value_get_enum(value))
            {
                user_list->hscroll_policy = g_value_get_enum(value);
                gtk_widget_queue_resize(GTK_WIDGET(user_list));
                g_object_notify_by_pspec(gobject, pspec);
            }
            break;

        case PROP_VSCROLL_POLICY:
            if (user_list->vscroll_policy != g_value_get_enum(value))
            {
                user_list->vscroll_policy = g_value_get_enum(value);
                gtk_widget_queue_resize(GTK_WIDGET(user_list));
                g_object_notify_by_pspec(gobject, pspec);
            }
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
            break;
    }
}

static gboolean wintc_welcome_user_list_button_press_event(
    GtkWidget*      widget,
    GdkEventButton* event
)
{
    gint                  hit_item;
    gdouble               scroll_y;
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(widget);

    scroll_y = gtk_adjustment_get_value(user_list->vadjustment);
    hit_item = (gint) (scroll_y + event->y) / user_list->item_height;

    if (hit_item >= (gint) g_list_length(user_list->users))
    {
        hit_item = -1;
    }

    wintc_welcome_user_list_select_user(user_list, hit_item);

    return TRUE;
}

static gboolean wintc_welcome_user_list_draw(
    GtkWidget* widget,
    cairo_t*   cr
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(widget);

    // Acquire adjustment
    //
    gdouble scroll_y = gtk_adjustment_get_value(user_list->vadjustment);

    // Acquire clip region
    //
    gdouble clip_bottom;
    gdouble clip_left;
    gdouble clip_right;
    gdouble clip_top;

    cairo_clip_extents(cr, &clip_left, &clip_top, &clip_right, &clip_bottom);

    // Check what items we need to redraw
    //
    gdouble region_bottom = clip_bottom + scroll_y;
    gdouble region_top    = clip_top    + scroll_y;

    gint   item_first = (gint) region_top    / user_list->item_height;
    gint   item_last  = (gint) region_bottom / user_list->item_height;
    gint   items      = g_list_length(user_list->users);
    gdouble width     = clip_right - clip_left;

    item_first = item_first < 0 ? 0 : item_first;

    for (int i = item_first; i < items && i <= item_last; i++)
    {
        gdouble item_y = (double) (i * user_list->item_height) - scroll_y;

        cairo_save(cr);

        cairo_rectangle(
            cr,
            0.0f,
            item_y,
            width,
            (double) user_list->item_height
        );
        cairo_clip(cr);

        // FIXME: Why on earth do we need this?
        cairo_move_to(cr, 0.0f, item_y);

        draw_user(
            cr,
            (g_list_nth(user_list->users, i))->data,
            user_list->selected_index == i,
            user_list
        );

        cairo_restore(cr);
    }

    // Chain up
    //
    (GTK_WIDGET_CLASS(wintc_welcome_user_list_parent_class))
        ->draw(widget, cr);

    return FALSE;
}

static void wintc_welcome_user_list_get_preferred_height(
    GtkWidget* widget,
    gint*      minimum_height,
    gint*      natural_height
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(widget);

    *minimum_height = user_list->item_height;
    *natural_height = user_list->item_height *
                      g_list_length(user_list->users);
}

static void wintc_welcome_user_list_get_preferred_height_for_width(
    GtkWidget* widget,
    WINTC_UNUSED(gint width),
    gint*      minimum_height,
    gint*      natural_height
)
{
    wintc_welcome_user_list_get_preferred_height(
        widget,
        minimum_height,
        natural_height
    );
}

static void wintc_welcome_user_list_get_preferred_width(
    WINTC_UNUSED(GtkWidget* widget),
    gint* minimum_width,
    gint* natural_width
)
{
    *minimum_width = USER_LISTING_WIDTH;
    *natural_width = USER_LISTING_WIDTH;
}

static void wintc_welcome_user_list_get_preferred_width_for_height(
    GtkWidget* widget,
    WINTC_UNUSED(gint height),
    gint*      minimum_width,
    gint*      natural_width
)
{
    wintc_welcome_user_list_get_preferred_width(
        widget,
        minimum_width,
        natural_width
    );
}

static void wintc_welcome_user_list_realize(
    GtkWidget* widget
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(widget);

    // Initial widget stuffs
    //
    GtkAllocation allocation;

    gtk_widget_get_allocation(widget, &allocation);

    gtk_widget_set_realized(widget, TRUE);

    // Create input window
    //
    GdkWindowAttr attribs;

    attribs.x           = allocation.x;
    attribs.y           = allocation.y;
    attribs.width       = allocation.width;
    attribs.height      = allocation.height;
    attribs.event_mask  = GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK;
    attribs.wclass      = GDK_INPUT_OUTPUT;
    attribs.visual      = gtk_widget_get_visual(widget);
    attribs.window_type = GDK_WINDOW_CHILD;

    user_list->hwnd =
        gdk_window_new(
            gtk_widget_get_parent_window(widget),
            &attribs,
            GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL
        );

    gtk_widget_set_window(
        widget,
        user_list->hwnd
    );
    gtk_widget_register_window(
        widget,
        user_list->hwnd
    );
}

static void wintc_welcome_user_list_size_allocate(
    GtkWidget*     widget,
    GtkAllocation* allocation
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(widget);

    gtk_widget_set_allocation(widget, allocation);

    // Update GDK window positioning
    //
    if (gtk_widget_get_realized(widget))
    {
        gdk_window_move_resize(
            user_list->hwnd,
            allocation->x,
            allocation->y,
            allocation->width,
            allocation->height
        );
    }

    // Position auth widget
    //
    GtkAllocation box_alloc;

    if (user_list->selected_index > -1)
    {
        gint scroll_y =
            gtk_adjustment_get_value(
                user_list->vadjustment
            );

        box_alloc.x      = 70;
        box_alloc.y      = user_list->selected_index *
                           user_list->item_height    +
                           50 - scroll_y;
        box_alloc.width  = 248;
        box_alloc.height = 30;

        gtk_widget_show_all(user_list->box_auth);
    }
    else
    {
        box_alloc.x      = 0;
        box_alloc.y      = 0;
        box_alloc.width  = 0;
        box_alloc.height = 0;

        gtk_widget_hide(user_list->box_auth);
    }

    gtk_widget_size_allocate(
        user_list->box_auth,
        &box_alloc
    );

    // Update scroll values
    //
    wintc_welcome_user_list_set_vadjustment_values(user_list);
}

static void wintc_welcome_user_list_add(
    WINTC_UNUSED(GtkContainer* container),
    WINTC_UNUSED(GtkWidget*    widget)
)
{
    g_critical("%s", "wintc_welcome_user_list_add - not allowed!");
}

static void wintc_welcome_user_list_forall(
    GtkContainer* container,
    WINTC_UNUSED(gboolean include_internals),
    GtkCallback   callback,
    gpointer      callback_data
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(container);

    g_slist_foreach(
        user_list->child_widgets,
        (GFunc) callback,
        callback_data
    );
}

static void wintc_welcome_user_list_remove(
    WINTC_UNUSED(GtkContainer* container),
    WINTC_UNUSED(GtkWidget*    widget)
)
{
    g_critical("%s", "wintc_welcome_user_list_remove - not allowed!");
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_welcome_user_list_new(
    WinTCGinaLogonSession* logon_session
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_WELCOME_USER_LIST,
            "logon-session", logon_session,
            "hexpand",       TRUE,
            "vexpand",       TRUE,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void wintc_welcome_user_list_internal_add(
    WinTCWelcomeUserList* user_list,
    GtkWidget*            widget
)
{
    gtk_widget_set_parent(widget, GTK_WIDGET(user_list));

    user_list->child_widgets =
        g_slist_append(user_list->child_widgets, widget);
}

static void wintc_welcome_user_list_select_user(
    WinTCWelcomeUserList* user_list,
    gint                  index
)
{
    user_list->selected_index = index;

    if (index >= 0)
    {
        gtk_widget_set_sensitive(
            user_list->entry_password,
            TRUE
        );
        gtk_widget_grab_focus(
            user_list->entry_password
        );
    }
    else
    {
        gtk_widget_set_sensitive(
            user_list->entry_password,
            FALSE
        );
    }

    gtk_widget_queue_allocate(GTK_WIDGET(user_list));
}

static void wintc_welcome_user_list_set_vadjustment(
    WinTCWelcomeUserList* user_list,
    GtkAdjustment*        adjustment
)
{
    if (user_list->vadjustment == adjustment)
    {
        return;
    }

    if (user_list->vadjustment)
    {
        g_signal_handlers_disconnect_by_func(
            user_list->vadjustment,
            on_self_adjustment_changed,
            user_list
        );

        g_object_unref(user_list->vadjustment);
    }

    if (adjustment == NULL)
    {
        adjustment =
            gtk_adjustment_new(
                0.0f,
                0.0f,
                0.0f,
                0.0f,
                0.0f,
                0.0f
            );
    }

    g_signal_connect(
        adjustment,
        "value-changed",
        G_CALLBACK(on_self_adjustment_changed),
        user_list
    );

    user_list->vadjustment = g_object_ref_sink(adjustment);

    wintc_welcome_user_list_set_vadjustment_values(user_list);

    g_object_notify(G_OBJECT(user_list), "vadjustment");
}

static void wintc_welcome_user_list_set_vadjustment_values(
    WinTCWelcomeUserList* user_list
)
{
    GtkAllocation alloc;
    gdouble new_upper;
    gdouble new_value;
    gdouble old_value;
    gint    user_count;

    gtk_widget_get_allocation(GTK_WIDGET(user_list), &alloc);

    old_value  =
        gtk_adjustment_get_value(user_list->vadjustment);
    user_count =
        g_list_length(user_list->users);

    new_upper =
        MAX(alloc.height, user_list->item_height * user_count);

    g_object_set(
        user_list->vadjustment,
        "lower",          0.0,
        "upper",          new_upper,
        "page-size",      (gdouble) alloc.height,
        "step-increment", alloc.height * 0.1,
        "page-increment", alloc.height * 0.9,
        NULL
    );

    new_value = CLAMP(old_value, 0, new_upper - alloc.height);

    if (new_value != old_value)
    {
        gtk_adjustment_set_value(
            user_list->vadjustment,
            new_value
        );
    }
}


static void draw_user(
    cairo_t*              cr,
    LightDMUser*          user,
    gboolean              selected,
    WinTCWelcomeUserList* user_list
)
{
    //const gchar* text = lightdm_user_get_name(user);
    double origin_y;

    cairo_get_current_point(cr, NULL, &origin_y);

    // Render usersel
    //
    if (selected)
    {
        cairo_set_source_surface(
            cr,
            user_list->surface_usersel,
            0.0f,
            origin_y
        );
        cairo_pattern_set_extend(
            cairo_get_source(cr),
            CAIRO_EXTEND_NONE
        );
        cairo_paint(cr);
    }

    // Render userpic
    //
    WinTCWelcomeUserpic* userpic =
        wintc_welcome_user_list_get_userpic(
            user_list,
            lightdm_user_get_image(user)
        );

    cairo_set_source_surface(
        cr,
        selected ?
            user_list->surface_tilehot :
            user_list->surface_tile,
        USER_TILE_OFFSET_X,
        USER_TILE_OFFSET_Y + origin_y
    );
    cairo_paint(cr);

    cairo_set_source_surface(
        cr,
        userpic->surface,
        USER_TILE_OFFSET_X + USER_PIC_OFFSET,
        USER_TILE_OFFSET_Y + USER_PIC_OFFSET + origin_y
    );
    cairo_paint(cr);

    // Text handling
    //
    cairo_set_source_rgb(cr, 1.0f, 1.0f, 1.0f);
    cairo_move_to(
        cr,
        USER_NAME_OFFSET_X,
        USER_NAME_OFFSET_Y + origin_y
    );
    cairo_select_font_face(
        cr,
        "Arial",
        CAIRO_FONT_SLANT_NORMAL,
        CAIRO_FONT_WEIGHT_NORMAL
    );
    cairo_set_font_size(cr, 14.0f);
    cairo_show_text(cr, lightdm_user_get_name(user));
}

static WinTCWelcomeUserpic* wintc_welcome_user_list_get_userpic(
    WinTCWelcomeUserList* user_list,
    const gchar*          path
)
{
    WinTCWelcomeUserpic* userpic;

    if (!path)
    {
        return user_list->default_userpic;
    }

    userpic =
        g_hash_table_lookup(
            user_list->map_path_to_userpic,
            path
        );

    if (userpic)
    {
        return userpic;
    }

    // Load the userpic
    //
    GdkPixbuf* pixbuf =
        gdk_pixbuf_new_from_file_at_scale(
            path,
            USER_PIC_SIZE,
            USER_PIC_SIZE,
            FALSE,
            NULL
        );

    if (!pixbuf)
    {
        return user_list->default_userpic;
    }

    // All good, create the userpic mapping
    //
    userpic = g_new(WinTCWelcomeUserpic, 1);

    userpic->pixbuf  = pixbuf;
    userpic->surface = gdk_cairo_surface_create_from_pixbuf(pixbuf, 1, NULL);

    g_hash_table_insert(
        user_list->map_path_to_userpic,
        g_strdup(path),
        userpic
    );

    return userpic;
}

//
// CALLBACKS
//
static void free_userpic(
    WinTCWelcomeUserpic* userpic
)
{
    cairo_surface_destroy(userpic->surface);
    g_object_unref(userpic->pixbuf);
    g_free(userpic);
}

static void on_self_adjustment_changed(
    WINTC_UNUSED(GtkAdjustment* adjustment),
    WinTCWelcomeUserList* user_list
)
{
    gtk_widget_queue_resize(GTK_WIDGET(user_list));
}

static void on_logon_session_attempt_complete(
    WINTC_UNUSED(WinTCGinaLogonSession* logon_session),
    WINTC_UNUSED(WinTCGinaResponse response),
    gpointer          user_data
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(user_data);

    // Reset the UI state after any logon attempt
    //
    gtk_entry_set_text(
        GTK_ENTRY(user_list->entry_password),
        ""
    );

    gtk_widget_set_sensitive(
        user_list->button_go,
        TRUE
    );
    gtk_widget_set_sensitive(
        user_list->entry_password,
        TRUE
    );

    gtk_widget_grab_focus(user_list->entry_password);
}

static void on_button_go_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(user_data);

    gtk_widget_set_sensitive(
        user_list->button_go,
        FALSE
    );
    gtk_widget_set_sensitive(
        user_list->entry_password,
        FALSE
    );

    wintc_gina_logon_session_try_logon(
        user_list->logon_session,
        lightdm_user_get_name(
            g_list_nth(
                user_list->users,
                user_list->selected_index
            )->data
        ),
        gtk_entry_get_text(GTK_ENTRY(user_list->entry_password))
    );
}


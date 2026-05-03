#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <lightdm.h>
#include <wintc/comgtk.h>
#include <wintc/msgina.h>

#include "userlist.h"
#include "balloon.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_LOGON_SESSION = 1,
    N_PROPERTIES
};

//
// PRIVATE STRUCTS
//
typedef struct _UserListItem
{
    WinTCWelcomeUserList* parent; 

    gchar*       name;
    LightDMUser* user;

    // UI state
    //
    gboolean faded;
    gboolean hovered;

    // UI
    //
    GtkWidget* event_wrapper;
    GtkWidget* details_box;
    GtkWidget* background_image;
    GtkWidget* userpic_box;
    GtkWidget* profile_image;
    GtkWidget* username_label;
    GtkWidget* password_entry;
    GtkWidget* instruction_label;
    GtkWidget* go_button;
} UserListItem;

//
// FORWARD DECLARATIONS
//
static void wintc_welcome_user_list_finalize(
    GObject* gobject
);
static void wintc_welcome_user_list_set_property(
    GObject*      gobject,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void wintc_welcome_user_list_realize(
    GtkWidget* widget,
    gpointer   user_data
);

static GtkWidget* build_userlist_widget(
    WinTCWelcomeUserList* user_list
);

static void show_balloon_under_widget(
    UserListItem* item,
    BalloonType   type
);
static void hide_balloon(
    WinTCWelcomeUserList* user_list
);

static void list_item_select(
    UserListItem* item
);
static void list_item_deselect(
    UserListItem* item
);
static void list_item_css_blur(
    GtkWidget* widget
);
static void list_item_css_unblur(
    GtkWidget* widget
);
static void list_item_css_unblur_fast(
    GtkWidget* widget
);

static void logon_session_attempt(
    UserListItem* item
);

static void wintc_welcome_user_list_navigate_up(
    GtkWidget* widget
);
static void wintc_welcome_user_list_navigate_down(
    GtkWidget* widget
);
static void wintc_welcome_user_list_unselect_all(
    GtkWidget*      widget,
    GdkEventButton* event,
    gpointer        data
);

static gboolean balloon_timeout_callback(
    gpointer user_data
);
static gboolean balloon_unblur_callback(
    gpointer user_data
);

static gboolean on_key_pressed(
    GtkWidget*   widget,
    GdkEventKey* event,
    gpointer     user_data
);

static gboolean on_list_hover_enter(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);
static gboolean on_list_hover_leave(
    GtkWidget*        widget,
    GdkEventCrossing* event,
    gpointer          user_data
);
static gboolean on_list_item_hover_enter(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);
static gboolean on_list_item_hover_leave(
    GtkWidget*        widget,
    GdkEventCrossing* event,
    gpointer          user_data
);
static gboolean on_list_item_clicked(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);

static gboolean on_logon_button_clicked(
    GtkButton* button,
    gpointer   user_data
);

static void on_logon_session_attempt_complete(
    WinTCGinaLogonSession* logon_session,
    WinTCGinaResponse      response,
    gpointer               user_data
);

static gboolean on_outside_click(
    GtkWidget*      widget,
    GdkEventButton* event,
    gpointer        user_data
);

static gboolean on_password_caps_pressed(
    GtkWidget*   widget,
    GdkEventKey* event,
    gpointer     user_data
);
static gboolean on_password_focus_gain(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);
static gboolean on_password_focus_out(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);

static void on_realize_enable_passthrough(
    GtkWidget* widget,  
    gpointer   user_data
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCWelcomeUserList
{
    GtkBox __parent__;

    WinTCGinaLogonSession* logon_session;

    // State flags
    //
    gboolean attempt_active;
    gboolean hovered;

    // UI
    //
    // BUG: Current logonui implementation doesn't have a compositor
    //      so an overlay and a box is used to show popup balloons
    //
    GtkWidget* overlay_wrapper;
    GtkWidget* balloon_wrapper_box;
    GtkWidget* list_box; 

    GList* list_items;
    GList* selected_li;

    GtkWidget* balloon;
    guint      balloon_timeout_id;
};


//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCWelcomeUserList,
    wintc_welcome_user_list,
    GTK_TYPE_BOX
)

static void wintc_welcome_user_list_class_init(
    WinTCWelcomeUserListClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->set_property = wintc_welcome_user_list_set_property;
    object_class->finalize     = wintc_welcome_user_list_finalize;

    g_object_class_install_property(
        object_class,
        PROP_LOGON_SESSION,
        g_param_spec_object(
            "logon-session",
            "LogonSession",
            "The GINA logon session instance.",
            WINTC_TYPE_GINA_LOGON_SESSION,
            G_PARAM_WRITABLE
        )
    );
}

static void wintc_welcome_user_list_init(
    WinTCWelcomeUserList* user_list
)
{
    user_list->balloon = NULL;
    user_list->balloon_timeout_id = 0;

    user_list->overlay_wrapper = gtk_overlay_new();

    user_list->balloon_wrapper_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(user_list->balloon_wrapper_box, TRUE);
    gtk_widget_set_vexpand(user_list->balloon_wrapper_box, TRUE);
    g_signal_connect(
        user_list->balloon_wrapper_box,
        "realize",
        G_CALLBACK(on_realize_enable_passthrough),
        NULL
    );

    user_list->list_box = GTK_WIDGET(build_userlist_widget(user_list));
    gtk_widget_set_hexpand(user_list->list_box, TRUE);
    gtk_widget_set_vexpand(user_list->list_box, TRUE);

    gtk_container_add(
        GTK_CONTAINER(user_list->overlay_wrapper),
        user_list->list_box
    );

    gtk_overlay_add_overlay(
        GTK_OVERLAY(user_list->overlay_wrapper),
        user_list->balloon_wrapper_box
    );

    gtk_box_pack_start(
        GTK_BOX(user_list),
        user_list->overlay_wrapper,
        TRUE,
        TRUE,
        0
    );

    g_signal_connect(
        GTK_WIDGET(user_list),
        "realize",
        G_CALLBACK(wintc_welcome_user_list_realize),
        user_list
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_welcome_user_list_finalize(
    GObject* gobject
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(gobject);

    if (user_list->logon_session)
    {
        g_signal_handlers_disconnect_by_data(
            user_list->logon_session,
            user_list
        );
    }

    if (user_list->balloon_timeout_id != 0)
    {
        g_source_remove(user_list->balloon_timeout_id);
        user_list->balloon_timeout_id = 0;
    }

    hide_balloon(user_list);

    for (GList* l = user_list->list_items; l != NULL; l = l->next)
    {
        UserListItem* item = (UserListItem*) l->data;

        if (item->event_wrapper)
        {
            g_signal_handlers_disconnect_by_data(item->event_wrapper, item);
        }
        
        g_free(item->name);
        g_free(item);
    }

    g_list_free(user_list->list_items);
    user_list->list_items = NULL;

    GtkWindow* toplevel =
        wintc_widget_get_toplevel_window(GTK_WIDGET(user_list));

    if (toplevel)
    {
        g_signal_handlers_disconnect_by_data(toplevel, user_list);
    }

    (G_OBJECT_CLASS(wintc_welcome_user_list_parent_class))->finalize(gobject);
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

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
            break;
    }
}

static void wintc_welcome_user_list_realize(
    GtkWidget* widget,
    WINTC_UNUSED(gpointer user_data)
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(widget);

    if (user_list->list_box)
    {
        gtk_widget_show_all(user_list->list_box);

        if (!gtk_widget_get_realized(user_list->list_box))
        {
            gtk_widget_realize(user_list->list_box);
        }
    }

    GtkWindow* toplevel =
        wintc_widget_get_toplevel_window(GTK_WIDGET(user_list));
    
    if (toplevel)
    {
        g_signal_connect(
            toplevel,
            "button-press-event",
            G_CALLBACK(on_outside_click),
            user_list
        );
        g_signal_connect(
            toplevel,
            "key-press-event",
            G_CALLBACK(on_key_pressed),
            user_list
        );
    }
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
static GtkWidget* build_userlist_widget(
    WinTCWelcomeUserList* user_list
)
{
    GtkWidget* scrollable = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(scrollable, FALSE);
    gtk_widget_set_vexpand(scrollable, TRUE);
    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scrollable),
        GTK_POLICY_NEVER,
        GTK_POLICY_AUTOMATIC
    );

    GtkWidget* list_box_event_wrapper = gtk_event_box_new();
    g_signal_connect(
        list_box_event_wrapper,
        "enter-notify-event",
        G_CALLBACK(on_list_hover_enter),
        user_list
    );
    g_signal_connect(
        list_box_event_wrapper,
        "leave-notify-event",
        G_CALLBACK(on_list_hover_leave),
        user_list
    );
    gtk_widget_add_events(
        list_box_event_wrapper,
        GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
    );
    
    GtkWidget* list_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GList* users =
        lightdm_user_list_get_users(
            lightdm_user_list_get_instance()
        );

    // Set up an event wrapper for empty space at the top of the list
    //
    GtkWidget* top_box_event_wrapper = gtk_event_box_new();
    gtk_widget_set_hexpand(top_box_event_wrapper, TRUE);
    gtk_widget_set_vexpand(top_box_event_wrapper, TRUE);
    g_signal_connect(
        top_box_event_wrapper,
        "button-press-event",
        G_CALLBACK(wintc_welcome_user_list_unselect_all),
        user_list
    );
    gtk_widget_add_events(
        top_box_event_wrapper,
        GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
    );

    gtk_box_pack_start(
        GTK_BOX(list_box),
        top_box_event_wrapper,
        TRUE,
        TRUE,
        0
    );

    // Set up usersel graphic
    //
    GdkPixbuf* pixbuf_usersel;
    GdkPixbuf* pixbuf_usersel_scaled;

    pixbuf_usersel =
        gdk_pixbuf_new_from_resource(
            "/uk/oddmatics/wintc/logonui/usersel.png",
            NULL
        );

    pixbuf_usersel_scaled =
        gdk_pixbuf_scale_simple(
            pixbuf_usersel,
            348,
            72,
            GDK_INTERP_BILINEAR
        );

    // Set up a list item for each user
    //
    for (GList* l = users; l != NULL; l = l->next)
    {
        UserListItem* item = g_new0(UserListItem, 1);
        user_list->list_items = g_list_append(user_list->list_items, item);

        item->user     = (LightDMUser*) l->data; 
        item->name     = g_strdup(lightdm_user_get_name(item->user));
        item->parent   = user_list; 
        item->faded    = FALSE;

        // Construct UI from XML
        //
        GtkBuilder* builder =
            gtk_builder_new_from_resource(
                "/uk/oddmatics/wintc/logonui/useritem.ui"
            );

        wintc_builder_get_objects(
            builder,
            "eventbox-wrapper",  &(item->event_wrapper),
            "img-background",    &(item->background_image),
            "box-details",       &(item->details_box),
            "box-userpic",       &(item->userpic_box),
            "img-profile",       &(item->profile_image),
            "label-username",    &(item->username_label),
            "label-instruction", &(item->instruction_label),
            "entry-password",    &(item->password_entry),
            "button-go",         &(item->go_button),
            NULL
        );

        gtk_widget_add_events(
            item->event_wrapper,
            GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
        );
        g_signal_connect(
            item->event_wrapper,
            "enter-notify-event",
            G_CALLBACK(on_list_item_hover_enter),
            item
        );
        g_signal_connect(
            item->event_wrapper,
            "leave-notify-event",
            G_CALLBACK(on_list_item_hover_leave),
            item
        );

        // Set up user details
        //
        GdkPixbuf* profile_image =
            gdk_pixbuf_new_from_resource(
                "/uk/oddmatics/wintc/logonui/userpic.png",
                NULL
            );

        gtk_image_set_from_pixbuf(
            GTK_IMAGE(item->profile_image),
            profile_image
        );
        gtk_label_set_text(
            GTK_LABEL(item->username_label),
            item->name
        );

        g_object_unref(profile_image);

        // Usersel background
        //
        gtk_image_set_from_pixbuf(
            GTK_IMAGE(item->background_image),
            pixbuf_usersel_scaled
        );

        // Connect signals
        //
        g_signal_connect(
            item->instruction_label,
            "realize",
            G_CALLBACK(gtk_widget_hide),
            NULL
        );

        g_signal_connect(
            item->password_entry,
            "realize",
            G_CALLBACK(gtk_widget_hide),
            item
        );
        g_signal_connect(
            item->password_entry,
            "focus-out-event",
            G_CALLBACK(on_password_focus_out),
            user_list
        );
        g_signal_connect(
            item->password_entry,
            "focus-in-event",
            G_CALLBACK(on_password_focus_gain),
            item
        );
        g_signal_connect(
            item->password_entry,
            "key-press-event",
            G_CALLBACK(on_password_caps_pressed),
            item
        );

        g_signal_connect(
            item->go_button,
            "realize",
            G_CALLBACK(gtk_widget_hide),
            NULL
        );

        g_signal_connect(
            item->go_button,
            "clicked",
            G_CALLBACK(on_logon_button_clicked),
            item
        );

        g_signal_connect(
            item->background_image,
            "realize",
            G_CALLBACK(gtk_widget_hide),
            NULL
        );

        g_signal_connect(
            G_OBJECT(item->event_wrapper),
            "button_press_event",
            G_CALLBACK(on_list_item_clicked),
            item
        );

        // Add user list item to the list box
        //
        gtk_box_pack_start(
            GTK_BOX(list_box),
            item->event_wrapper,
            FALSE,
            FALSE,
            0
        );

        g_object_unref(builder);
    }

    g_object_unref(pixbuf_usersel);
    g_object_unref(pixbuf_usersel_scaled);

    // Set up an event wrapper for empty space at the bottom of the list
    //
    GtkWidget* bottom_box_event_wrapper = gtk_event_box_new();
    gtk_widget_set_hexpand(bottom_box_event_wrapper, TRUE);
    gtk_widget_set_vexpand(bottom_box_event_wrapper, TRUE);
    g_signal_connect(
        bottom_box_event_wrapper,
        "button-press-event",
        G_CALLBACK(wintc_welcome_user_list_unselect_all),
        user_list
    );
    gtk_widget_add_events(
        bottom_box_event_wrapper,
        GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
    );

    gtk_box_pack_start(
        GTK_BOX(list_box),
        bottom_box_event_wrapper,
        TRUE,
        TRUE,
        0
    );
    
    // Set up the list box
    //
    gtk_container_add(GTK_CONTAINER(list_box_event_wrapper), list_box);
    gtk_container_add(GTK_CONTAINER(scrollable), list_box_event_wrapper);

    return scrollable;
}

static void hide_balloon(
    WinTCWelcomeUserList* user_list
)
{
    if (user_list->balloon)
    {
        gtk_container_remove(
            GTK_CONTAINER(user_list->balloon_wrapper_box),
            user_list->balloon
        );

        gtk_widget_destroy(user_list->balloon);
        user_list->balloon = NULL;
        gtk_widget_show_all(user_list->balloon_wrapper_box);
    }

    if (user_list->balloon_timeout_id != 0)
    {
        g_source_remove(user_list->balloon_timeout_id);
        user_list->balloon_timeout_id = 0;
    }
}

static void show_balloon_under_widget(
    UserListItem* item,
    BalloonType   type
) 
{
    WinTCWelcomeUserList* user_list = item->parent; 
    
    hide_balloon(user_list);
    
    user_list->balloon =
        wintc_welcome_balloon_new_with_type(type, item->password_entry);

    gtk_widget_set_halign(user_list->balloon, GTK_ALIGN_START);
    gtk_widget_set_valign(user_list->balloon, GTK_ALIGN_START);
    wintc_widget_add_style_class(user_list->balloon, "transparent");

    GtkAllocation allocation;
    gtk_widget_get_allocation(item->password_entry, &allocation);

    gint x, y;
    gtk_widget_translate_coordinates(
        item->profile_image, 
        user_list->balloon_wrapper_box,             
        0, 0,                               
        &x, &y                               
    );

    gtk_widget_set_margin_start(user_list->balloon, x + 70);
    gtk_widget_set_margin_top(user_list->balloon, y + 60);

    user_list->balloon_timeout_id =
        g_timeout_add(
            6000,
            (GSourceFunc) balloon_timeout_callback,
            user_list
        );

    g_idle_add(
        (GSourceFunc) balloon_unblur_callback,
        user_list
    );

    // BUG: Because the balloon is shown in an overlay, it gets clipped by the
    //      overlay's size. This could be fixed by moving the balloon to ui.c
    //      and using event signals.
    //
    gtk_box_pack_start(
        GTK_BOX(user_list->balloon_wrapper_box),
        user_list->balloon,
        TRUE,
        TRUE,
        0
    );

    gtk_widget_show_all(user_list->balloon_wrapper_box);
}

static void list_item_select(
    UserListItem* item
)
{
    wintc_widget_add_style_class(item->userpic_box, "hot");

    gtk_widget_set_visible(item->background_image,  TRUE);
    gtk_widget_set_visible(item->instruction_label, TRUE);
    gtk_widget_set_visible(item->password_entry,    TRUE);
    gtk_widget_set_visible(item->go_button,         TRUE);

    gtk_widget_grab_focus(item->password_entry);
}

static void list_item_deselect(
    UserListItem* item
)
{
    wintc_widget_remove_style_class(item->userpic_box, "hot");

    gtk_widget_set_visible(item->background_image,  FALSE);
    gtk_widget_set_visible(item->instruction_label, FALSE);
    gtk_widget_set_visible(item->password_entry,    FALSE);
    gtk_widget_set_visible(item->go_button,         FALSE);
}

static void list_item_css_blur(
    GtkWidget* widget
)
{
    wintc_widget_remove_style_class(widget, "unblur");
    wintc_widget_remove_style_class(widget, "unblur-fast");
    wintc_widget_add_style_class(widget, "blur");
}

static void list_item_css_unblur_fast(
    GtkWidget* widget
)
{
    wintc_widget_remove_style_class(widget, "unblur");
    wintc_widget_remove_style_class(widget, "blur");
    wintc_widget_add_style_class(widget, "unblur-fast");
}

static void list_item_css_unblur(
    GtkWidget* widget
)
{
    wintc_widget_remove_style_class(widget, "blur");
    wintc_widget_remove_style_class(widget, "unblur-fast");
    wintc_widget_add_style_class(widget, "unblur");
}

static void logon_session_attempt(
    UserListItem* item
)
{
    WinTCWelcomeUserList* user_list = item->parent;

    user_list->attempt_active = TRUE;

    wintc_gina_logon_session_try_logon(
        user_list->logon_session,
        item->name,
        gtk_entry_get_text(GTK_ENTRY(item->password_entry))
    );

    gtk_widget_set_sensitive(item->go_button, FALSE);
    gtk_widget_set_sensitive(item->password_entry, FALSE);
    gtk_entry_set_text(GTK_ENTRY(item->password_entry), "");
}

static void wintc_welcome_user_list_navigate_up(
    GtkWidget* widget
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(widget);

    UserListItem* item;

    if (user_list->selected_li)
    {
        item = (UserListItem*) user_list->selected_li->data;

        if (user_list->selected_li->prev)
        {
            GList* prev_li = user_list->selected_li->prev;

            // Deselect this item
            //
            list_item_deselect(item);

            if (item->hovered)
            {
                wintc_widget_add_style_class(item->userpic_box, "hot");
            }
            else
            {
                list_item_css_blur(item->profile_image);
                list_item_css_blur(item->username_label);
            }

            gtk_entry_set_text(GTK_ENTRY(item->password_entry), "");
            hide_balloon(user_list);

            // Select prev item
            //
            user_list->selected_li = prev_li;
            item = (UserListItem*) user_list->selected_li->data;

            list_item_select(item);
            list_item_css_unblur_fast(item->profile_image);
            list_item_css_unblur_fast(item->username_label);
        } 
    }
    else if (user_list->list_items)
    {
        user_list->selected_li = user_list->list_items;
        item = (UserListItem*) user_list->selected_li->data;

        list_item_select(item);
        list_item_css_unblur_fast(item->profile_image);
        list_item_css_unblur_fast(item->username_label);
    }

    // Blur all other items
    // 
    for (GList* iter = user_list->list_items; iter; iter = iter->next)
    {
        item = (UserListItem*) iter->data;

        if (iter == user_list->selected_li || item->hovered)
        {
            continue;
        }

        list_item_css_blur(item->profile_image);
        list_item_css_blur(item->username_label);
    } 
}

static void wintc_welcome_user_list_navigate_down(
    GtkWidget* widget
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(widget);

    UserListItem* item;

    if (user_list->selected_li)
    {
        item = (UserListItem*) user_list->selected_li->data;

        if (user_list->selected_li->next)
        {
            GList* next_li = user_list->selected_li->next;

            // Deselect this item
            //
            list_item_deselect(item);

            if (item->hovered)
            {
                wintc_widget_add_style_class(item->userpic_box, "hot");
            }
            else
            {
                list_item_css_blur(item->profile_image);
                list_item_css_blur(item->username_label);
            }

            gtk_entry_set_text(GTK_ENTRY(item->password_entry), "");

            // Select next item
            //
            user_list->selected_li = next_li;
            item = (UserListItem*) user_list->selected_li->data;

            hide_balloon(user_list);

            list_item_select(item);
            list_item_css_unblur_fast(item->profile_image);
            list_item_css_unblur_fast(item->username_label);
        }
    }
    else if (user_list->list_items)
    {
        user_list->selected_li = user_list->list_items;
        item = (UserListItem*) user_list->selected_li->data;

        list_item_select(item);
        list_item_css_unblur_fast(item->profile_image);
        list_item_css_unblur_fast(item->username_label);
    }

    // Blur all other items
    //
    for (GList* iter = user_list->list_items; iter; iter = iter->next)
    {
        item = (UserListItem*) iter->data;

        if (iter == user_list->selected_li || item->hovered)
        {
            continue;
        }
    }
}

static void wintc_welcome_user_list_unselect_all(
    WINTC_UNUSED(GtkWidget*      w),
    WINTC_UNUSED(GdkEventButton* e),
    gpointer data
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(data);

    hide_balloon(user_list);

    if (!user_list->selected_li)
    {
        return;
    }

    UserListItem* item = (UserListItem*) user_list->selected_li->data;

    user_list->selected_li = NULL;

    gtk_entry_set_text(GTK_ENTRY(item->password_entry), "");

    list_item_deselect(item);
    list_item_css_blur(item->profile_image);
    list_item_css_blur(item->username_label);
}

//
// CALLBACKS
//
static gboolean balloon_timeout_callback(
    gpointer user_data
)
{
    WinTCWelcomeUserList* user_list = (WinTCWelcomeUserList*) user_data;

    if (user_list->balloon)
    {
        hide_balloon(user_list);
    }

    return FALSE; 
}

static gboolean balloon_unblur_callback(
    gpointer user_data
)
{
    WinTCWelcomeUserList* user_list = (WinTCWelcomeUserList*) user_data;

    if (user_list->balloon)
    {
        wintc_widget_remove_style_class(user_list->balloon, "transparent");
        wintc_widget_add_style_class(user_list->balloon, "unblur");
    }

    return G_SOURCE_REMOVE; 
}

static gboolean on_key_pressed(
    WINTC_UNUSED(GtkWidget* widget),
    GdkEventKey* event,
    gpointer     user_data
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(user_data);

    if (user_list->attempt_active)
    {
        return FALSE;
    }

    switch (event->keyval)
    {
        case GDK_KEY_Down:
        {
            wintc_welcome_user_list_navigate_down(GTK_WIDGET(user_list));
            break;
        }

        case GDK_KEY_Up:
        {
            wintc_welcome_user_list_navigate_up(GTK_WIDGET(user_list));
            break;
        }

        default: break;
    }

    return FALSE;
}

static gboolean on_list_hover_enter(
    WINTC_UNUSED(GtkWidget* widget),
    WINTC_UNUSED(GdkEvent*  event),
    gpointer user_data
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(user_data);

    user_list->hovered = TRUE;

    if (user_list->attempt_active)
    {
        return FALSE;
    }

    for (GList* iter = user_list->list_items; iter; iter = iter->next)
    {
        UserListItem* item = (UserListItem*) iter->data;

        if (iter == user_list->selected_li || item->hovered)
        {
            continue;
        }

        list_item_css_blur(item->profile_image);
        list_item_css_blur(item->username_label);
    }

    return FALSE;
}

static gboolean on_list_hover_leave(
    WINTC_UNUSED(GtkWidget* widget),
    GdkEventCrossing* event,
    gpointer          user_data
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(user_data);

    user_list->hovered = FALSE;

    if (
        event->detail == GDK_NOTIFY_INFERIOR ||
        user_list->attempt_active            ||
        user_list->selected_li
    )
    {
        return FALSE;
    }

    for (GList* iter = user_list->list_items; iter; iter = iter->next)
    {
        UserListItem* item = (UserListItem*) iter->data;

        item->faded = FALSE;

        list_item_css_unblur(item->profile_image);
        list_item_css_unblur(item->username_label);
    }

    return FALSE;
}

static gboolean on_list_item_clicked(
    WINTC_UNUSED(GtkWidget* widget),
    WINTC_UNUSED(GdkEvent*  event),
    gpointer user_data
)
{
    UserListItem*         item      = (UserListItem*) user_data;
    WinTCWelcomeUserList* user_list = item->parent;

    if (user_list->attempt_active)
    {
        return FALSE;
    }

    if (user_list->selected_li)
    {
        if (user_list->selected_li->data == item)
        {
            return FALSE;
        }
        else
        {
            UserListItem* other_item =
                (UserListItem*) user_list->selected_li->data;

            gtk_entry_set_text(GTK_ENTRY(other_item->password_entry), "");

            list_item_css_blur(other_item->profile_image);
            list_item_css_blur(other_item->username_label);
            list_item_deselect(other_item);
        }
    }

    user_list->selected_li = g_list_find(user_list->list_items, item);

    hide_balloon(user_list);

    list_item_select(item);

    wintc_widget_add_style_class(item->userpic_box, "hot");
    list_item_css_unblur(item->profile_image);
    list_item_css_unblur(item->username_label);

    return FALSE;
}

static gboolean on_list_item_hover_enter(
    WINTC_UNUSED(GtkWidget* widget),
    WINTC_UNUSED(GdkEvent*  event),
    gpointer user_data
)
{
    UserListItem*         item      = (UserListItem*) user_data;
    WinTCWelcomeUserList* user_list = item->parent;

    if (user_list->attempt_active)
    {
        return FALSE;
    }

    GdkWindow*  window  = gtk_widget_get_window(widget);
    GdkDisplay* display = gdk_window_get_display(window);
    GdkCursor*  cursor  = gdk_cursor_new_from_name(display, "pointer");

    gdk_window_set_cursor(window, cursor);
    g_object_unref(cursor);

    if (user_list->selected_li && user_list->selected_li->data == item)
    {
        return FALSE;
    }

    wintc_widget_add_style_class(item->userpic_box, "hot");

    list_item_css_unblur(item->username_label);
    list_item_css_unblur(item->profile_image);

    item->hovered = TRUE;

    return FALSE;
}

static gboolean on_list_item_hover_leave(
    WINTC_UNUSED(GtkWidget*        widget),
    WINTC_UNUSED(GdkEventCrossing* event),
    gpointer user_data
)
{
    UserListItem*         item = (UserListItem*) user_data;
    WinTCWelcomeUserList* user_list = item->parent;

    if (user_list->attempt_active)
    {
        return FALSE;
    }

    if (event->detail == GDK_NOTIFY_INFERIOR)
    {
        return FALSE;
    }

    item->hovered = FALSE;

    if (!(user_list->selected_li) || user_list->selected_li->data != item)
    {
        wintc_widget_remove_style_class(item->userpic_box, "hot");

        list_item_css_blur(item->username_label);
        list_item_css_blur(item->profile_image);
    }

    GdkWindow* window = gtk_widget_get_window(widget);
    gdk_window_set_cursor(window, NULL);

    return FALSE;
}

static gboolean on_logon_button_clicked(
    WINTC_UNUSED(GtkButton* button),
    gpointer user_data
)
{
    UserListItem* item = (UserListItem*) user_data;

    logon_session_attempt(item);

    return FALSE;
}

static void on_logon_session_attempt_complete(
    WINTC_UNUSED(WinTCGinaLogonSession* logon_session),
    WinTCGinaResponse response,
    gpointer          user_data
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(user_data);

    hide_balloon(user_list);

    if (response == WINTC_GINA_RESPONSE_FAIL)
    {
        UserListItem* item = (UserListItem*) user_list->selected_li->data;

        gtk_widget_set_sensitive(item->go_button, TRUE);
        gtk_widget_set_sensitive(item->password_entry, TRUE);
        show_balloon_under_widget(item, BALLOON_TYPE_ERROR);

        user_list->attempt_active = FALSE;
    } 
}

static gboolean on_outside_click(
    WINTC_UNUSED(GtkWidget* w),
    GdkEventButton* e,
    gpointer        user_data
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(user_data);

    if (user_list->attempt_active)
    {
        return FALSE;
    }

    // Check, was the click event within the user list?
    //
    GtkWidget* search = gtk_get_event_widget((GdkEvent*) e);

    while (search && search != GTK_WIDGET(user_list))
    {
        search = gtk_widget_get_parent(search);
    }

    if (search)
    {
        return FALSE;
    }

    // Clicked outside, so deactivate everything
    //
    UserListItem* item;

    hide_balloon(user_list);

    if (user_list->selected_li)
    {
        item = (UserListItem*) user_list->selected_li->data;

        gtk_entry_set_text(GTK_ENTRY(item->password_entry), "");
        list_item_deselect(item);

        user_list->selected_li = NULL;
    }

    for (GList* iter = user_list->list_items; iter; iter = iter->next)
    {
        item = (UserListItem*) iter->data;

        list_item_css_unblur(item->profile_image);
        list_item_css_unblur(item->username_label);
    }

    return FALSE;
}

static gboolean on_password_caps_pressed(
    WINTC_UNUSED(GtkWidget* widget),
    GdkEventKey* event,
    gpointer     user_data
)
{
    UserListItem*         item      = (UserListItem*) user_data; 
    WinTCWelcomeUserList* user_list = item->parent;

    if (user_list->attempt_active)
    {
        return FALSE;
    }

    switch (event->keyval)
    {
        case GDK_KEY_Caps_Lock:
            if (!(event->state & GDK_LOCK_MASK))
            {
                show_balloon_under_widget(item, BALLOON_TYPE_WARNING);
            }
            else
            {
                hide_balloon(user_list);
            }

            break;

        case GDK_KEY_Return:
            gtk_button_clicked(GTK_BUTTON(item->go_button));
            break;

        case GDK_KEY_Down:
        case GDK_KEY_Up:
             break;

        default:
            hide_balloon(user_list);
            break;
    }

    return FALSE;
}

static gboolean on_password_focus_gain(
    GtkWidget* widget,
    WINTC_UNUSED(GdkEvent* event),
    gpointer   user_data
)
{
    UserListItem* item = (UserListItem*) user_data;
    WinTCWelcomeUserList* user_list = item->parent;

    if (user_list->attempt_active)
    {
        return FALSE;
    }

    GdkDisplay* display = gtk_widget_get_display(widget);

    gboolean caps_lock_on =
        gdk_keymap_get_caps_lock_state(
            gdk_keymap_get_for_display(display)
        );

    if (caps_lock_on)
    {
        show_balloon_under_widget(item, BALLOON_TYPE_WARNING); 
    }

    return FALSE;
}

static gboolean on_password_focus_out(
    WINTC_UNUSED(GtkWidget* widget),
    WINTC_UNUSED(GdkEvent* event),
    gpointer user_data
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(user_data);

    if (user_list->attempt_active)
    {
        return FALSE;
    }

    if (user_list->selected_li)
    {
        UserListItem* item = (UserListItem*) user_list->selected_li->data;

        list_item_deselect(item);

        list_item_css_blur(item->profile_image);
        list_item_css_blur(item->username_label);

        user_list->selected_li = NULL;
    }

    return FALSE;
}

static void on_realize_enable_passthrough(
    GtkWidget* widget,
    WINTC_UNUSED(gpointer user_data)
)
{
    GdkWindow* window = gtk_widget_get_window(widget);

    if (window)
    {
        gdk_window_set_pass_through(window, TRUE);
    }
}

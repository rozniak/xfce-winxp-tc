#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <lightdm.h>
#include <wintc/comgtk.h>
#include <wintc/msgina.h>

#include "userlist.h"
#include "balloon.h"
#include "button.h"


//
// PRIVATE ENUMS
//
enum
{
    PROP_LOGON_SESSION = 1,
    N_PROPERTIES
};

//
// FORWARD DECLARATIONS
//
typedef struct UserListItem UserListItem;

static void wintc_welcome_user_list_set_property(
    GObject*      gobject,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);
static void wintc_welcome_user_list_get_property(
    GObject*    gobject,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_welcome_user_list_realize(
    GtkWidget* widget,
    gpointer user_data
);
static void wintc_welcome_user_list_finalize(
    GObject* gobject
);

static void logon_session_attempt(
    UserListItem *item
);
static void on_logon_session_attempt_complete(
    WINTC_UNUSED(WinTCGinaLogonSession* logon_session),
    WINTC_UNUSED(WinTCGinaResponse response),
    gpointer          user_data
);

void wintc_welcome_user_list_navigate_up(
    GtkWidget* widget
);
void wintc_welcome_user_list_navigate_down(
    GtkWidget* widget
);
static gboolean on_list_hover_enter(
    GtkWidget *widget,
    GdkEvent *event,
    gpointer user_data
);
static gboolean on_list_hover_leave(
    GtkWidget *widget,
    GdkEventCrossing *event,
    gpointer user_data
);
static gboolean on_list_item_hover_enter(
    GtkWidget *widget,
    GdkEvent *event,
    gpointer user_data
);
static gboolean on_list_item_hover_leave(
    GtkWidget *widget,
    GdkEventCrossing *event,
    gpointer user_data
);
static gboolean on_list_item_clicked(
    GtkWidget *widget,
    GdkEvent *event,
    gpointer user_data
);
static void list_item_select(
    UserListItem *item
);
static void list_item_deselect(
    UserListItem *item
);
static void list_item_css_blur(
    GtkWidget *widget
);
static void list_item_css_unblur(
    GtkWidget *widget
);
static void list_item_css_unblur_fast(
    GtkWidget *widget
);
static void wintc_welcome_user_list_unselect_all(
    GtkWidget *w,
    GdkEventButton *e,
    gpointer data
);

static gboolean on_password_focus_gain(
    GtkWidget *widget,
    GdkEvent *event,
    gpointer user_data
);
static gboolean on_password_focus_out(
    GtkWidget *widget,
    GdkEvent *event,
    gpointer user_data
);
static gboolean on_password_caps_pressed(
    GtkWidget *widget,
    GdkEventKey *event,
    gpointer user_data
);
static gboolean on_logon_button_clicked(
    GtkButton *button,
    gpointer user_data
);

static gboolean on_outside_click(
    GtkWidget *w,
    GdkEventButton *e,
    gpointer data
);
static gboolean on_key_pressed(
    GtkWidget *widget,
    GdkEventKey *event,
    gpointer user_data
);

static void show_balloon_under_widget(
    UserListItem *item,
    BalloonType type
);
static void hide_balloon(
    WinTCWelcomeUserList *user_list
);
static gboolean balloon_timeout_callback(
    gpointer user_data
);

static gboolean balloon_unblur_callback(
    gpointer user_data
);

static GtkWidget *build_userlist_widget(
    WinTCWelcomeUserList *user_list
);

static void on_realize_enable_passthrough(
    GtkWidget *widget,  
    WINTC_UNUSED(gpointer user_data)
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCWelcomeUserListClass
{
    GtkBoxClass __parent__;
};

struct _WinTCWelcomeUserList
{
    GtkBox __parent__;

    // State flags
    //
    gboolean hovered;

    // UI
    //
    // BUG: Current logonui implementation doesn't have a compositor
    //      so an overlay and a box is used to show popup balloons
    GtkWidget *overlay_wrapper;

    GtkWidget *balloon_wrapper_box;
    GtkWidget *list_box; 

    GList *list_items;
    WinTCGinaLogonSession* logon_session;

    GtkWidget *balloon;
    guint balloon_timeout_id;
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
    WINTC_UNUSED(GtkContainerClass* container_class) = GTK_CONTAINER_CLASS(klass);
    WINTC_UNUSED(GtkWidgetClass*    widget_class)    = GTK_WIDGET_CLASS(klass);
    GObjectClass*      object_class    = G_OBJECT_CLASS(klass);

    object_class->get_property = wintc_welcome_user_list_get_property;
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
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
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
    g_signal_connect(user_list->balloon_wrapper_box, "realize", G_CALLBACK(on_realize_enable_passthrough), NULL);

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

    g_signal_connect(GTK_WIDGET(user_list), "realize", G_CALLBACK(wintc_welcome_user_list_realize), user_list);
}

struct UserListItem
{
    WinTCWelcomeUserList* parent; 

    gchar *name;
    LightDMUser *user;

    // State flags used for UI behavior
    // Carefully managed by each action
    gboolean faded;
    gboolean selected;
    gboolean hovered;

    GdkPixbuf *tile;
    GdkPixbuf *tile_hot;

    GtkWidget *event_wrapper;
    GtkWidget *details_box;
    GtkWidget *background_image;
    GtkWidget *profile_image;
    GtkWidget *username_label;
    GtkWidget *password_entry;
    GtkWidget *instruction_label;
    GtkWidget *go_button;

};

//
// CLASS VIRTUAL METHODS
//
static void wintc_welcome_user_list_realize(
    GtkWidget* widget,
    gpointer user_data
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(widget);
    user_data = user_data;
    if (user_list->list_box) {
        gtk_widget_show_all(user_list->list_box);
        if (!gtk_widget_get_realized(user_list->list_box)) {
            gtk_widget_realize(user_list->list_box);
        }
    }

    GtkWidget *toplevel = gtk_widget_get_toplevel(GTK_WIDGET(user_list));
    
    if (GTK_IS_WINDOW(toplevel)) {
        g_signal_connect(toplevel, "button-press-event", 
                         G_CALLBACK(on_outside_click), user_list);
        g_signal_connect(toplevel, "key-press-event", G_CALLBACK(on_key_pressed), user_list);
    }
}

static void wintc_welcome_user_list_finalize(
    GObject* gobject
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(gobject);

    if (user_list->logon_session) {
        g_signal_handlers_disconnect_by_data(user_list->logon_session, user_list);
    }

    if (user_list->balloon_timeout_id != 0) {
        g_source_remove(user_list->balloon_timeout_id);
        user_list->balloon_timeout_id = 0;
    }

    hide_balloon(user_list);

    for (GList *l = user_list->list_items; l != NULL; l = l->next)
    {
        UserListItem *item = (UserListItem *)l->data;

        if (item->event_wrapper) {
            g_signal_handlers_disconnect_by_data(item->event_wrapper, item);
        }
        
        g_object_unref(item->background_image);
        g_object_unref(item->profile_image);
        g_object_unref(item->username_label);
        g_object_unref(item->password_entry);
        g_object_unref(item->instruction_label);
        g_object_unref(item->go_button);
        g_object_unref(item->tile);
        g_object_unref(item->tile_hot); 
        g_free(item->name);
        g_free(item);
    }

    g_list_free(user_list->list_items);
    user_list->list_items = NULL;

    GtkWidget *toplevel = gtk_widget_get_toplevel(GTK_WIDGET(user_list));
    if (toplevel && GTK_IS_WINDOW(toplevel)) {
        g_signal_handlers_disconnect_by_data(toplevel, user_list);
    }

    (G_OBJECT_CLASS(wintc_welcome_user_list_parent_class))->finalize(gobject);
}

static void wintc_welcome_user_list_get_property(
    GObject*    gobject,
    guint       prop_id,
    WINTC_UNUSED(GValue*     value),
    GParamSpec* pspec
)
{
    WINTC_UNUSED(WinTCWelcomeUserList* user_list) = WINTC_WELCOME_USER_LIST(gobject);

    switch (prop_id)
    {

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

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
            break;
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
static void list_item_css_blur(GtkWidget *widget)
{
    gtk_style_context_remove_class(gtk_widget_get_style_context(widget), "unblur");
    gtk_style_context_remove_class(gtk_widget_get_style_context(widget), "unblur-fast");
    gtk_style_context_add_class(gtk_widget_get_style_context(widget), "blur");
}

static void list_item_css_unblur_fast(GtkWidget *widget)
{
    gtk_style_context_remove_class(gtk_widget_get_style_context(widget), "unblur");
    gtk_style_context_remove_class(gtk_widget_get_style_context(widget), "blur");
    gtk_style_context_add_class(gtk_widget_get_style_context(widget), "unblur-fast");
}

static void list_item_css_unblur(GtkWidget *widget)
{
    gtk_style_context_remove_class(gtk_widget_get_style_context(widget), "blur");
    gtk_style_context_remove_class(gtk_widget_get_style_context(widget), "unblur-fast");
    gtk_style_context_add_class(gtk_widget_get_style_context(widget), "unblur");
}

static void wintc_welcome_user_list_unselect_all(WINTC_UNUSED(GtkWidget *w), WINTC_UNUSED(GdkEventButton *e), gpointer data)
{
    WinTCWelcomeUserList *user_list = WINTC_WELCOME_USER_LIST(data);
    hide_balloon(user_list);

    for (GList *l = user_list->list_items; l != NULL; l = l->next)
    {
        UserListItem *item = (UserListItem *)l->data;
        if (item->selected)
        {
            item->selected = FALSE;
            gtk_entry_set_text(GTK_ENTRY(item->password_entry), "");
            list_item_deselect(item);

            list_item_css_blur(item->profile_image);
            list_item_css_blur(item->username_label);
        }
    }
}

void wintc_welcome_user_list_navigate_up(GtkWidget* widget) {
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(widget);
    gboolean found_selected = FALSE;
    for (GList *l = g_list_last(user_list->list_items); l != NULL; l = g_list_previous(l)) {
        UserListItem* item = (UserListItem*)l->data;

        if (item->selected) {
            found_selected = TRUE;
            if (g_list_previous(l)) {
                item->selected = FALSE;
                list_item_deselect(item);
                if (item->hovered) {
                    gtk_image_set_from_pixbuf(GTK_IMAGE(item->profile_image), item->tile_hot);
                } else {
                    list_item_css_blur(item->profile_image);
                    list_item_css_blur(item->username_label);
                }
                gtk_entry_set_text(GTK_ENTRY(item->password_entry), "");
                item->selected = FALSE;
                hide_balloon(user_list);

                UserListItem* next = (UserListItem*) g_list_previous(l)->data;
                next->selected = TRUE;
                hide_balloon(user_list); 
                list_item_select(next);
                list_item_css_unblur_fast(next->profile_image);
                list_item_css_unblur_fast(next->username_label);

                break;
            } 
        } 
    } 
        
    // If no item was selected, select the first one
    if (user_list->list_items->data && !found_selected) {
        UserListItem* item = (UserListItem*) user_list->list_items->data;
        item->selected = TRUE;
        list_item_select(item);
        list_item_css_unblur_fast(item->profile_image);
        list_item_css_unblur_fast(item->username_label);
    }

    // Blur all other items
    for (GList *l = g_list_last(user_list->list_items); l != NULL; l = g_list_previous(l)) {
        UserListItem* item = (UserListItem*)l->data;
        if (!item->selected) {
            if (!item->hovered) {
                list_item_css_blur(item->profile_image);
                list_item_css_blur(item->username_label);
            }
        }
    } 
}

void wintc_welcome_user_list_navigate_down(GtkWidget* widget) {
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(widget);
    gboolean found_selected = FALSE;
    for (GList *l = user_list->list_items; l != NULL; l = l->next) {
        UserListItem* item = (UserListItem*)l->data;
        if (item->selected) {
            found_selected = TRUE;

            if (l->next) {
                item->selected = FALSE;
                list_item_deselect(item);
                if (item->hovered) {
                    gtk_image_set_from_pixbuf(GTK_IMAGE(item->profile_image), item->tile_hot);
                } else {
                    list_item_css_blur(item->profile_image);
                    list_item_css_blur(item->username_label);
                }

                gtk_entry_set_text(GTK_ENTRY(item->password_entry), "");
                item->selected = FALSE;
                hide_balloon(user_list);
                
                UserListItem* next = (UserListItem*) l->next->data;
                next->selected = TRUE;
                list_item_select(next);
                list_item_css_unblur_fast(next->profile_image);
                list_item_css_unblur_fast(next->username_label);

                break;        
            } 
        }
    }

    // If no item was selected, select the first one
    if (user_list->list_items->data && !found_selected) {
        UserListItem* item = (UserListItem*) user_list->list_items->data;
        item->selected = TRUE;
        list_item_select(item);
        list_item_css_unblur_fast(item->profile_image);
        list_item_css_unblur_fast(item->username_label);
    }
    
    // Blur all other items
    for (GList *l = user_list->list_items; l != NULL; l = l->next) {
        UserListItem* item = (UserListItem*)l->data;
        if (!item->selected) {
           if (!item->hovered) {
                list_item_css_blur(item->profile_image);
                list_item_css_blur(item->username_label);
            }
        } 
    }
}


static void logon_session_attempt(UserListItem *item) 
{
    WinTCWelcomeUserList *user_list = item->parent;

    wintc_gina_logon_session_try_logon(
        user_list->logon_session,
        item->name,
        gtk_entry_get_text(GTK_ENTRY(item->password_entry))
    );
}

static void list_item_select(UserListItem *item)
{
    gtk_image_set_from_pixbuf(GTK_IMAGE(item->profile_image), item->tile_hot);

    gtk_widget_set_visible(item->background_image, TRUE);
    gtk_widget_set_visible(item->instruction_label, TRUE);
    gtk_widget_set_visible(item->password_entry, TRUE);
    gtk_widget_set_visible(item->go_button, TRUE);

    gtk_widget_grab_focus(item->password_entry);
}



static void list_item_deselect(UserListItem *item)
{
    gtk_image_set_from_pixbuf(GTK_IMAGE(item->profile_image), item->tile);

    gtk_widget_set_visible(item->background_image, FALSE);
    gtk_widget_set_visible(item->instruction_label, FALSE);
    gtk_widget_set_visible(item->password_entry, FALSE);
    gtk_widget_set_visible(item->go_button, FALSE);
}



static void hide_balloon(WinTCWelcomeUserList *user_list)
{
    if (user_list->balloon) {
        gtk_container_remove(GTK_CONTAINER(user_list->balloon_wrapper_box), user_list->balloon);
        gtk_widget_destroy(user_list->balloon);
        user_list->balloon = NULL;
        gtk_widget_show_all(user_list->balloon_wrapper_box);

    }
    if (user_list->balloon_timeout_id != 0) {
        g_source_remove(user_list->balloon_timeout_id);
        user_list->balloon_timeout_id = 0;
    }
}


static void show_balloon_under_widget(UserListItem *item, BalloonType type) 
{
    WinTCWelcomeUserList *user_list = item->parent; 
    
    hide_balloon(user_list);
    
    user_list->balloon = wintc_welcome_balloon_new_with_type(type, item->password_entry);
    gtk_widget_set_halign(user_list->balloon, GTK_ALIGN_START);
    gtk_widget_set_valign(user_list->balloon, GTK_ALIGN_START);
    gtk_style_context_add_class(gtk_widget_get_style_context(user_list->balloon), "transparent");

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

    user_list->balloon_timeout_id = g_timeout_add(6000, (GSourceFunc)balloon_timeout_callback, user_list);
    g_idle_add((GSourceFunc)balloon_unblur_callback, user_list);

    // BUG: Because the balloon is shown in an overlay, it gets clipped by the overlay's size.
    //      This could be fixed by moving the balloon to ui.c and using event signals.
    gtk_box_pack_start(GTK_BOX(user_list->balloon_wrapper_box), user_list->balloon, TRUE, TRUE, 0);
    gtk_widget_show_all(user_list->balloon_wrapper_box);
}

static GtkWidget *build_userlist_widget(WinTCWelcomeUserList *user_list)
{
    GtkWidget *scrollable = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(scrollable, FALSE);
    gtk_widget_set_vexpand(scrollable, TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollable), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget *list_box_event_wrapper = gtk_event_box_new();
    g_signal_connect(list_box_event_wrapper, "enter-notify-event",
                     G_CALLBACK(on_list_hover_enter), user_list);
    g_signal_connect(list_box_event_wrapper, "leave-notify-event",
                     G_CALLBACK(on_list_hover_leave), user_list);
    gtk_widget_add_events(list_box_event_wrapper, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
    
    GtkWidget *list_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GList *users =
    lightdm_user_list_get_users(
        lightdm_user_list_get_instance()
    );

    // Set up an event wrapper for empty space at the top of the list
    //
    GtkWidget *top_box_event_wrapper = gtk_event_box_new();
    gtk_widget_set_hexpand(top_box_event_wrapper, TRUE);
    gtk_widget_set_vexpand(top_box_event_wrapper, TRUE);
    g_signal_connect(top_box_event_wrapper, "button-press-event",
                     G_CALLBACK(wintc_welcome_user_list_unselect_all), user_list);
    gtk_widget_add_events(top_box_event_wrapper, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);

    gtk_box_pack_start(GTK_BOX(list_box), top_box_event_wrapper, TRUE, TRUE, 0);

    // Set up a list item for each user
    //
    for (GList *l = users; l != NULL; l = l->next)
    {
        UserListItem *item = g_new0(UserListItem, 1);
        user_list->list_items = g_list_append(user_list->list_items, item);
        
        item->event_wrapper = gtk_event_box_new();
        gtk_widget_add_events(item->event_wrapper, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
        g_signal_connect(item->event_wrapper, "enter-notify-event",
                         G_CALLBACK(on_list_item_hover_enter), item);
        g_signal_connect(item->event_wrapper, "leave-notify-event",
                         G_CALLBACK(on_list_item_hover_leave), item);

        GtkWidget *list_item_overlay_wrapper = gtk_overlay_new();

        item->details_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_size_request(item->details_box, 348, -1);
        gtk_widget_set_hexpand(item->details_box, FALSE);
        gtk_widget_set_vexpand(item->details_box, FALSE);

        // While the background is invisibile (user not selected) nothing will show without 
        // a visible filler (transparent)
        GtkWidget* background_wrapper_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_halign(background_wrapper_box, GTK_ALIGN_START);
        gtk_widget_set_valign(background_wrapper_box, GTK_ALIGN_START);
        gtk_widget_set_size_request(background_wrapper_box, 348, 92);

        gtk_container_add(GTK_CONTAINER(list_item_overlay_wrapper), background_wrapper_box);

        GtkWidget *picture_column = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        GtkWidget *info_column = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

        item->user = (LightDMUser *)l->data; 
        item->name = g_strdup(lightdm_user_get_name(item->user));
        item->parent = user_list; 
        item->selected = FALSE;
        item->faded = FALSE;

        item->tile = gdk_pixbuf_new_from_resource("/uk/oddmatics/wintc/logonui/tile.png", NULL);
        item->tile_hot = gdk_pixbuf_new_from_resource("/uk/oddmatics/wintc/logonui/tilehot.png", NULL);
        {
            GdkPixbuf *profile_image = gdk_pixbuf_new_from_resource("/uk/oddmatics/wintc/logonui/userpic.png", NULL);
            gdk_pixbuf_composite(profile_image, item->tile, 5, 5, 48, 48, 
                                 0, 0, 1.0, 1.0, GDK_INTERP_BILINEAR, 255);
            gdk_pixbuf_composite(profile_image, item->tile_hot, 5, 5, 48, 48, 
                                 0, 0, 1.0, 1.0, GDK_INTERP_BILINEAR, 255);
            g_object_unref(profile_image);
        }

        item->profile_image = gtk_image_new_from_pixbuf(item->tile);
        gtk_widget_set_margin_start(item->profile_image, 7);
        gtk_widget_set_margin_top(item->profile_image, 7);

        item->username_label = gtk_label_new(item->name);
        gtk_style_context_add_class(gtk_widget_get_style_context(item->username_label), "user-label");
        gtk_label_set_ellipsize(GTK_LABEL(item->username_label), PANGO_ELLIPSIZE_END);
        gtk_label_set_xalign(GTK_LABEL(item->username_label), 0.0);
        gtk_widget_set_valign(item->username_label, GTK_ALIGN_START);
        gtk_widget_set_vexpand(item->username_label, TRUE);
        gtk_widget_set_margin_start(item->username_label, 12);

        item->instruction_label = gtk_label_new("Type your password");
        gtk_style_context_add_class(gtk_widget_get_style_context(item->instruction_label), "password-label");
        gtk_label_set_xalign(GTK_LABEL(item->instruction_label), 0.0);
        gtk_widget_set_valign(item->instruction_label, GTK_ALIGN_END);
        gtk_widget_set_margin_start(item->instruction_label, 12);

        g_signal_connect(item->instruction_label, "realize",
                         G_CALLBACK(gtk_widget_hide), NULL);

        item->password_entry = gtk_entry_new();
        gtk_style_context_add_class(gtk_widget_get_style_context(item->password_entry), "password-box");
        gtk_entry_set_visibility(GTK_ENTRY(item->password_entry), FALSE);
        g_object_set(item->password_entry, "caps-lock-warning", FALSE, NULL); 
        gtk_widget_set_size_request(item->password_entry, 164, 27);
        gtk_widget_set_halign(item->password_entry, GTK_ALIGN_START);
        gtk_widget_set_margin_start(item->password_entry, 7);
        gtk_widget_set_margin_top(item->password_entry, 3);

        g_signal_connect(item->password_entry, "realize",
                         G_CALLBACK(gtk_widget_hide), item);
        g_signal_connect(item->password_entry, "focus-out-event",
                         G_CALLBACK(on_password_focus_out), user_list);
        g_signal_connect(item->password_entry, "focus-in-event",
                         G_CALLBACK(on_password_focus_gain), item);
        g_signal_connect(item->password_entry, "key-press-event", G_CALLBACK(on_password_caps_pressed), item); 

        
        {
            GdkPixbuf *go_idle = gdk_pixbuf_new_from_resource("/uk/oddmatics/wintc/logonui/gobtn.png", NULL);
            GdkPixbuf *go_activated = gdk_pixbuf_new_from_resource("/uk/oddmatics/wintc/logonui/gobtna.png", NULL);
            item->go_button = wintc_welcome_button_new_with_pixbufs(go_idle, go_activated);
            g_object_unref(go_idle);
            g_object_unref(go_activated);
        }

        gtk_style_context_add_class(gtk_widget_get_style_context(item->go_button), "plain-button");
        gtk_widget_set_can_focus(item->go_button, FALSE);
        gtk_widget_set_size_request(item->go_button, 22, 27);
        gtk_widget_set_margin_start(item->go_button, 13);
        gtk_widget_set_margin_top(item->go_button, 3);

        g_signal_connect(item->go_button, "realize",
                         G_CALLBACK(gtk_widget_hide), NULL);
        g_signal_connect(item->go_button, "clicked", G_CALLBACK(on_logon_button_clicked), item); 

        {
            GdkPixbuf *bg_pix = gdk_pixbuf_new_from_resource("/uk/oddmatics/wintc/logonui/usersel.png", NULL);
            bg_pix = gdk_pixbuf_scale_simple(bg_pix, 348, 72, GDK_INTERP_BILINEAR);
            item->background_image = gtk_image_new_from_pixbuf(bg_pix);
            g_object_unref(bg_pix);
        }
        
        gtk_widget_set_halign(item->background_image, GTK_ALIGN_START);
        gtk_widget_set_valign(item->background_image, GTK_ALIGN_START);
        g_signal_connect(item->background_image, "realize",
                         G_CALLBACK(gtk_widget_hide), NULL);
        
        g_object_ref(item->background_image);
        g_object_ref(item->profile_image);
        g_object_ref(item->username_label);
        g_object_ref(item->password_entry);
        g_object_ref(item->instruction_label);
        g_object_ref(item->go_button);
        
        // Set up background
        //
        gtk_box_pack_start(GTK_BOX(background_wrapper_box), item->background_image, FALSE, FALSE, 0);
        
        // Set up picture column
        //
        gtk_box_pack_start(GTK_BOX(picture_column), item->profile_image, FALSE, FALSE, 0);
        
        // Set up info column
        //
        GtkWidget *top_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_hexpand(top_row, FALSE);
        gtk_widget_set_vexpand(top_row, FALSE);
        gtk_widget_set_halign(top_row, GTK_ALIGN_START);
        gtk_widget_set_valign(top_row, GTK_ALIGN_START);
        gtk_widget_set_size_request(top_row, 290, 27);
        gtk_widget_set_margin_top(top_row, 5);
        
        gtk_box_pack_start(GTK_BOX(top_row), item->username_label, FALSE, FALSE, 0);


        GtkWidget *middle_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_halign(middle_row, GTK_ALIGN_START);
        // gtk_style_context_add_class(gtk_widget_get_style_context(middle_row), "red-bg");
        gtk_widget_set_size_request(middle_row, -1, 16);
        gtk_box_pack_start(GTK_BOX(middle_row), item->instruction_label, FALSE, FALSE, 0);
        
        GtkWidget *bottom_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

        gtk_box_pack_start(GTK_BOX(bottom_row), item->password_entry, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(bottom_row), item->go_button, FALSE, FALSE, 0);

        gtk_box_pack_start(GTK_BOX(info_column), top_row, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(info_column), middle_row, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(info_column), bottom_row, FALSE, FALSE, 0);

        gtk_box_pack_start(GTK_BOX(item->details_box), picture_column, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(item->details_box), info_column, FALSE, FALSE, 0);

        gtk_overlay_add_overlay(GTK_OVERLAY(list_item_overlay_wrapper), item->details_box);

        gtk_container_add(GTK_CONTAINER(item->event_wrapper), list_item_overlay_wrapper);
        
        g_signal_connect(G_OBJECT(item->event_wrapper), "button_press_event",
                         G_CALLBACK(on_list_item_clicked), item);
        
        // Add user list item to the list box
        gtk_box_pack_start(GTK_BOX(list_box), item->event_wrapper, FALSE, FALSE, 0);
    }

    // Set up an event wrapper for empty space at the bottom of the list
    //
    GtkWidget *bottom_box_event_wrapper = gtk_event_box_new();
    gtk_widget_set_hexpand(bottom_box_event_wrapper, TRUE);
    gtk_widget_set_vexpand(bottom_box_event_wrapper, TRUE);
    g_signal_connect(bottom_box_event_wrapper, "button-press-event",
                     G_CALLBACK(wintc_welcome_user_list_unselect_all), user_list);
    gtk_widget_add_events(bottom_box_event_wrapper, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
    gtk_box_pack_start(GTK_BOX(list_box), bottom_box_event_wrapper, TRUE, TRUE, 0);
    
    // Set up the list box
    //
    gtk_container_add(GTK_CONTAINER(list_box_event_wrapper), list_box);
    gtk_container_add(GTK_CONTAINER(scrollable), list_box_event_wrapper);

    return scrollable;
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
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(user_data);

    hide_balloon(user_list);
    if (response == WINTC_GINA_RESPONSE_FAIL) {
        for (GList *l = user_list->list_items; l != NULL; l = l->next)
        {
            UserListItem *item = (UserListItem *)l->data;
            if (item->selected) {
                show_balloon_under_widget(item, BALLOON_TYPE_ERROR);
                break;
            }
        }
    } 
}

static gboolean on_password_focus_gain(GtkWidget *widget, WINTC_UNUSED(GdkEvent *event), gpointer user_data)
{
    UserListItem *item = (UserListItem *) user_data; 

    GdkDisplay *display = gtk_widget_get_display(widget);

    gboolean caps_lock_on = gdk_keymap_get_caps_lock_state(gdk_keymap_get_for_display(display));
    if (caps_lock_on) {
        show_balloon_under_widget(item, BALLOON_TYPE_WARNING); 
    }
    return FALSE;
}


static gboolean on_outside_click(WINTC_UNUSED(GtkWidget *w), GdkEventButton *e, gpointer data)
{
    WinTCWelcomeUserList *user_list = WINTC_WELCOME_USER_LIST(data);
    GtkWidget *p = gtk_get_event_widget((GdkEvent *)e);

    while (p && p != GTK_WIDGET(user_list))
        p = gtk_widget_get_parent(p);

    if (!p)
    {
        hide_balloon(user_list);
        for (GList *l = user_list->list_items; l; l = l->next)
        {
            UserListItem *item = l->data;
            if (item->selected)
            {
                item->selected = FALSE;
                gtk_entry_set_text(GTK_ENTRY(item->password_entry), "");
                list_item_deselect(item);
            }

            list_item_css_unblur(item->profile_image);
            list_item_css_unblur(item->username_label);
        }
    }
    return FALSE;
}

static gboolean on_key_pressed(WINTC_UNUSED(GtkWidget *widget), GdkEventKey *event, gpointer user_data) {
    WinTCWelcomeUserList *list = WINTC_WELCOME_USER_LIST(user_data);
    switch (event->keyval) {
        case GDK_KEY_Down: {
            wintc_welcome_user_list_navigate_down(GTK_WIDGET(list));
            break;
        }
        case GDK_KEY_Up: {
            wintc_welcome_user_list_navigate_up(GTK_WIDGET(list));
            break;
        }
        default:
            break;
    }

    return FALSE;
}

static gboolean on_list_item_hover_enter(WINTC_UNUSED(GtkWidget *widget), WINTC_UNUSED(GdkEvent *event), gpointer user_data)
{
    UserListItem *item = (UserListItem *)user_data;

    GdkWindow *window = gtk_widget_get_window(widget);
    GdkDisplay *display = gdk_window_get_display(window);
    GdkCursor *cursor = gdk_cursor_new_from_name(display, "pointer");
    gdk_window_set_cursor(window, cursor);
    g_object_unref(cursor);

    if (item->selected)
    {
        return FALSE;
    }

    gtk_image_set_from_pixbuf(GTK_IMAGE(item->profile_image), item->tile_hot);

    list_item_css_unblur(item->username_label);
    list_item_css_unblur(item->profile_image);

    item->hovered = TRUE;

    
    
    return FALSE;
}

static gboolean on_list_item_hover_leave(WINTC_UNUSED(GtkWidget *widget), WINTC_UNUSED(GdkEventCrossing *event), gpointer user_data)
{
    if (event->detail == GDK_NOTIFY_INFERIOR)
    {
        return FALSE;
    }


    UserListItem *item = (UserListItem *)user_data;
    item->hovered = FALSE;

    if (!item->selected)
    {
        gtk_image_set_from_pixbuf(GTK_IMAGE(item->profile_image), item->tile);

        list_item_css_blur(item->username_label);
        list_item_css_blur(item->profile_image);
    }

    GdkWindow *window = gtk_widget_get_window(widget);
    gdk_window_set_cursor(window, NULL); 
    return FALSE;
}

static gboolean on_password_focus_out(WINTC_UNUSED(GtkWidget *widget), WINTC_UNUSED(GdkEvent *event), gpointer user_data)
{
    WinTCWelcomeUserList *user_list = WINTC_WELCOME_USER_LIST(user_data);

    for (GList *l = user_list->list_items; l != NULL; l = l->next)
    {
        UserListItem *item = (UserListItem *)l->data;
        if (item->selected)
        {
            list_item_deselect(item);

            list_item_css_blur(item->profile_image);
            list_item_css_blur(item->username_label);

            item->selected = FALSE;
        }
    }
    return FALSE;
}

static gboolean on_list_item_clicked(WINTC_UNUSED(GtkWidget *widget), WINTC_UNUSED(GdkEvent *event), gpointer user_data)
{
    UserListItem *item = (UserListItem *)user_data; 
    WinTCWelcomeUserList *user_list = item->parent; 

    if (!item->selected)
    {
        for (GList *l = user_list->list_items; l != NULL; l = l->next)
        {
            UserListItem *other_item = (UserListItem *)l->data;
            if (other_item != item && other_item->selected)
            {
                other_item->selected = FALSE;
                gtk_entry_set_text(GTK_ENTRY(other_item->password_entry), "");

                list_item_css_blur(other_item->profile_image);
                list_item_css_blur(other_item->username_label);
                list_item_deselect(other_item);
            }
        }
        item->selected = TRUE;
        hide_balloon(user_list);
        list_item_select(item);
        gtk_image_set_from_pixbuf(GTK_IMAGE(item->profile_image), item->tile_hot);
        list_item_css_unblur(item->profile_image);
        list_item_css_unblur(item->username_label);
    }

    return FALSE;
}

static gboolean balloon_timeout_callback(gpointer user_data)
{
    WinTCWelcomeUserList *user_list = (WinTCWelcomeUserList *)user_data;
    if (user_list->balloon) {
        hide_balloon(user_list);
    }
    return FALSE; 
}

static gboolean balloon_unblur_callback(gpointer user_data)
{
    WinTCWelcomeUserList *user_list = (WinTCWelcomeUserList *)user_data;
    if (user_list->balloon) {
        gtk_style_context_remove_class(gtk_widget_get_style_context(user_list->balloon), "transparent");
        gtk_style_context_add_class(gtk_widget_get_style_context(user_list->balloon), "unblur");
    }
    return G_SOURCE_REMOVE; 
}

static gboolean on_password_caps_pressed(WINTC_UNUSED(GtkWidget *widget), GdkEventKey *event, gpointer user_data)
{
    UserListItem *item = (UserListItem *) user_data; 
    WinTCWelcomeUserList *user_list = item->parent; 

    switch (event->keyval) {
        case GDK_KEY_Caps_Lock:
            if (!(event->state & GDK_LOCK_MASK)) {
                show_balloon_under_widget(item, BALLOON_TYPE_WARNING);
              } else {
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

static gboolean on_logon_button_clicked(WINTC_UNUSED(GtkButton *button), gpointer user_data)
{
    UserListItem *item = (UserListItem *) user_data; 
    logon_session_attempt(item); 
    gtk_entry_set_text(GTK_ENTRY(item->password_entry), "");
    return FALSE;
}

static void on_realize_enable_passthrough(GtkWidget *widget,  WINTC_UNUSED(gpointer user_data))
{
    GdkWindow *window = gtk_widget_get_window(widget);
    if (window) {
        gdk_window_set_pass_through(window, TRUE);
    }
}

gboolean on_list_hover_enter(WINTC_UNUSED(GtkWidget *widget), WINTC_UNUSED(GdkEvent *event), gpointer user_data)
{
    WinTCWelcomeUserList *user_list = WINTC_WELCOME_USER_LIST(user_data);
    user_list->hovered = TRUE;

    for (GList *l = user_list->list_items; l != NULL; l = l->next)
    {
        UserListItem *item = (UserListItem *)l->data;
        if (!item->selected && !item->hovered)
        {
            list_item_css_blur(item->profile_image);
            list_item_css_blur(item->username_label);
        }
    }
    return FALSE;
}

gboolean on_list_hover_leave(WINTC_UNUSED(GtkWidget *widget), GdkEventCrossing *event, gpointer user_data)
{
    WinTCWelcomeUserList *user_list = WINTC_WELCOME_USER_LIST(user_data);
    user_list->hovered = FALSE;

    if (event->detail == GDK_NOTIFY_INFERIOR)
    {
        return FALSE;
    }

    for (GList *l = user_list->list_items; l != NULL; l = l->next)
    {
        UserListItem *item = (UserListItem *)l->data;
        if (item->selected)
        {
            return FALSE;
        }
    }

    for (GList *l = user_list->list_items; l != NULL; l = l->next)
    {
        UserListItem *item = (UserListItem *)l->data;
        item->faded = FALSE;

        list_item_css_unblur(item->profile_image);
        list_item_css_unblur(item->username_label);
    }
    return FALSE;
}
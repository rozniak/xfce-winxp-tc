#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <lightdm.h>
#include <wintc/comgtk.h>
#include <wintc/msgina.h>

#include "userlist.h"
#include "balloon.h"
#include "simplebutton.h"


//
// PRIVATE ENUMS
//
enum
{
    PROP_LOGON_SESSION = 1,
    N_PROPERTIES
};

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
    GtkWidget *overlay_wrapper;

    GtkWidget *balloon_wrapper;
    GtkWidget *box; 

    GList *list;
    WinTCGinaLogonSession* logon_session;

    GtkWidget *balloon;
    guint timeout_id;
};

//
// FORWARD DECLARATIONS
//
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

typedef struct UserListItem UserListItem;

static void on_logon_session_attempt_complete(
    WINTC_UNUSED(WinTCGinaLogonSession* logon_session),
    WINTC_UNUSED(WinTCGinaResponse response),
    gpointer          user_data
);
void userlist_navigate_up(
    GtkWidget* widget
);
void userlist_navigate_down(
    GtkWidget* widget
);
gboolean on_list_hover_enter(GtkWidget *widget, GdkEvent *event, gpointer user_data);
gboolean on_list_hover_leave(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data);
gboolean on_list_item_hover_enter(GtkWidget *widget, GdkEvent *event, gpointer user_data);
gboolean on_list_item_hover_leave(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data);
gboolean on_list_item_clicked(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void hot(UserListItem *item);
void idle(UserListItem *item);
static gboolean on_password_focus_gain(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static gboolean on_password_focus_out(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void clear_selections(GtkWidget *w, GdkEventButton *e, gpointer data);
static void login_attempt(UserListItem *item);
static gboolean on_password_caps_pressed(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
gboolean on_button_clicked(GtkButton *button, gpointer user_data);
static gboolean on_outside_click(GtkWidget *w, GdkEventButton *e, gpointer data);
static gboolean on_key_pressed(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
static void item_blur(GtkWidget *widget);
static void item_unblur(GtkWidget *widget);
static void item_unblur_fast(GtkWidget *widget);
static void show_balloon_under_widget(UserListItem *item, BalloonType type);
static void hide_balloon(WinTCWelcomeUserList *self);
static gboolean balloon_timeout_callback(gpointer user_data);
static gboolean balloon_unblur_callback(gpointer user_data);

static GtkWidget *create_userlist_widget(WinTCWelcomeUserList *self);

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
    // GtkContainerClass* container_class = GTK_CONTAINER_CLASS(klass);
    // GtkWidgetClass*    widget_class    = GTK_WIDGET_CLASS(klass);
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

static void on_wrapper_realize(GtkWidget *widget,  WINTC_UNUSED(gpointer user_data))
{
    GdkWindow *window = gtk_widget_get_window(widget);
    if (window) {
        gdk_window_set_pass_through(window, TRUE);
    }
}

static void wintc_welcome_user_list_init(
    WinTCWelcomeUserList* self
)
{
    self->balloon = NULL;
    self->timeout_id = 0;

    self->overlay_wrapper = gtk_overlay_new();

    // Replace fixed layout with a box
    self->balloon_wrapper = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(self->balloon_wrapper, TRUE);
    gtk_widget_set_vexpand(self->balloon_wrapper, TRUE);
    g_signal_connect(self->balloon_wrapper, "realize", G_CALLBACK(on_wrapper_realize), NULL);

    self->box = GTK_WIDGET(create_userlist_widget(self));
    gtk_widget_set_hexpand(self->box, TRUE);
    gtk_widget_set_vexpand(self->box, TRUE);

    gtk_container_add(
        GTK_CONTAINER(self->overlay_wrapper),
        self->box
    );

    gtk_overlay_add_overlay(
        GTK_OVERLAY(self->overlay_wrapper),
        self->balloon_wrapper
    );

    gtk_box_pack_start(
        GTK_BOX(self),
        self->overlay_wrapper,
        TRUE,
        TRUE,
        0
    );

    g_signal_connect(GTK_WIDGET(self), "realize", G_CALLBACK(wintc_welcome_user_list_realize), self);
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

    GtkWidget *event;
    GtkWidget *layout;
    GtkWidget *background;
    GtkWidget *picture;
    GtkWidget *username_label;
    GtkWidget *password;
    GtkWidget *instruction;
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
    if (user_list->box) {
        gtk_widget_show_all(user_list->box);
        if (!gtk_widget_get_realized(user_list->box)) {
            gtk_widget_realize(user_list->box);
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

    if (user_list->timeout_id != 0) {
        g_source_remove(user_list->timeout_id);
        user_list->timeout_id = 0;
    }

    hide_balloon(user_list);

    for (GList *l = user_list->list; l != NULL; l = l->next)
    {
        UserListItem *item = (UserListItem *)l->data;

        if (item->event) {
            g_signal_handlers_disconnect_by_data(item->event, item);
        }
        
        g_object_unref(item->background);
        g_object_unref(item->picture);
        g_object_unref(item->username_label);
        g_object_unref(item->password);
        g_object_unref(item->instruction);
        g_object_unref(item->go_button);
        g_object_unref(item->tile);
        g_object_unref(item->tile_hot); 
        g_free(item->name);
        g_free(item);
    }

    g_list_free(user_list->list);
    user_list->list = NULL;

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
static void item_blur(GtkWidget *widget)
{
    gtk_style_context_remove_class(gtk_widget_get_style_context(widget), "unblur");
    gtk_style_context_remove_class(gtk_widget_get_style_context(widget), "unblur-fast");
    gtk_style_context_add_class(gtk_widget_get_style_context(widget), "blur");
}

static void item_unblur_fast(GtkWidget *widget)
{
    gtk_style_context_remove_class(gtk_widget_get_style_context(widget), "unblur");
    gtk_style_context_remove_class(gtk_widget_get_style_context(widget), "blur");
    gtk_style_context_add_class(gtk_widget_get_style_context(widget), "unblur-fast");
}

static void item_unblur(GtkWidget *widget)
{
    gtk_style_context_remove_class(gtk_widget_get_style_context(widget), "blur");
    gtk_style_context_remove_class(gtk_widget_get_style_context(widget), "unblur-fast");
    gtk_style_context_add_class(gtk_widget_get_style_context(widget), "unblur");
}


gboolean on_list_hover_enter(WINTC_UNUSED(GtkWidget *widget), WINTC_UNUSED(GdkEvent *event), gpointer user_data)
{
    WinTCWelcomeUserList *self = WINTC_WELCOME_USER_LIST(user_data);
    self->hovered = TRUE;

    for (GList *l = self->list; l != NULL; l = l->next)
    {
        UserListItem *item = (UserListItem *)l->data;
        if (!item->selected && !item->hovered)
        {
            item_blur(item->picture);
            item_blur(item->username_label);
        }
    }
    return FALSE;
}

gboolean on_list_hover_leave(WINTC_UNUSED(GtkWidget *widget), GdkEventCrossing *event, gpointer user_data)
{
    WinTCWelcomeUserList *self = WINTC_WELCOME_USER_LIST(user_data);
    self->hovered = FALSE;

    if (event->detail == GDK_NOTIFY_INFERIOR)
    {
        return FALSE;
    }

    for (GList *l = self->list; l != NULL; l = l->next)
    {
        UserListItem *item = (UserListItem *)l->data;
        if (item->selected)
        {
            return FALSE;
        }
    }

    for (GList *l = self->list; l != NULL; l = l->next)
    {
        UserListItem *item = (UserListItem *)l->data;
        item->faded = FALSE;

        item_unblur(item->picture);
        item_unblur(item->username_label);
    }
    return FALSE;
}

static void clear_selections(WINTC_UNUSED(GtkWidget *w), WINTC_UNUSED(GdkEventButton *e), gpointer data)
{
    WinTCWelcomeUserList *self = WINTC_WELCOME_USER_LIST(data);
    hide_balloon(self);

    for (GList *l = self->list; l != NULL; l = l->next)
    {
        UserListItem *item = (UserListItem *)l->data;
        if (item->selected)
        {
            item->selected = FALSE;
            gtk_entry_set_text(GTK_ENTRY(item->password), "");
            idle(item);

            item_blur(item->picture);
            item_blur(item->username_label);
        }
    }
}

static gboolean on_outside_click(WINTC_UNUSED(GtkWidget *w), GdkEventButton *e, gpointer data)
{
    WinTCWelcomeUserList *self = WINTC_WELCOME_USER_LIST(data);
    GtkWidget *p = gtk_get_event_widget((GdkEvent *)e);

    while (p && p != GTK_WIDGET(self))
        p = gtk_widget_get_parent(p);

    if (!p)
    {
        hide_balloon(self);
        for (GList *l = self->list; l; l = l->next)
        {
            UserListItem *item = l->data;
            if (item->selected)
            {
                item->selected = FALSE;
                gtk_entry_set_text(GTK_ENTRY(item->password), "");
                idle(item);
            }

            item_unblur(item->picture);
            item_unblur(item->username_label);
        }
    }
    return FALSE;
}

static gboolean on_key_pressed(WINTC_UNUSED(GtkWidget *widget), GdkEventKey *event, gpointer user_data) {
    WinTCWelcomeUserList *list = WINTC_WELCOME_USER_LIST(user_data);
    switch (event->keyval) {
        case GDK_KEY_Down: {
            userlist_navigate_down(GTK_WIDGET(list));
            break;
        }
        case GDK_KEY_Up: {
            userlist_navigate_up(GTK_WIDGET(list));
            break;
        }
        default:
            break;
    }

    return FALSE;
}

gboolean on_list_item_hover_enter(WINTC_UNUSED(GtkWidget *widget), WINTC_UNUSED(GdkEvent *event), gpointer user_data)
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

    gtk_image_set_from_pixbuf(GTK_IMAGE(item->picture), item->tile_hot);

    item_unblur(item->username_label);
    item_unblur(item->picture);

    item->hovered = TRUE;

    
    
    return FALSE;
}

gboolean on_list_item_hover_leave(WINTC_UNUSED(GtkWidget *widget), WINTC_UNUSED(GdkEventCrossing *event), gpointer user_data)
{
    if (event->detail == GDK_NOTIFY_INFERIOR)
    {
        return FALSE;
    }


    UserListItem *item = (UserListItem *)user_data;
    item->hovered = FALSE;

    if (!item->selected)
    {
        gtk_image_set_from_pixbuf(GTK_IMAGE(item->picture), item->tile);

        item_blur(item->username_label);
        item_blur(item->picture);

        gtk_widget_show_all(item->layout);
    }

    GdkWindow *window = gtk_widget_get_window(widget);
    gdk_window_set_cursor(window, NULL); 
    return FALSE;
}

static gboolean on_password_focus_out(WINTC_UNUSED(GtkWidget *widget), WINTC_UNUSED(GdkEvent *event), gpointer user_data)
{
    WinTCWelcomeUserList *self = WINTC_WELCOME_USER_LIST(user_data);

    for (GList *l = self->list; l != NULL; l = l->next)
    {
        UserListItem *item = (UserListItem *)l->data;
        if (item->selected)
        {
            idle(item);

            item_blur(item->picture);
            item_blur(item->username_label);

            gtk_widget_show_all(item->layout);
            item->selected = FALSE;
        }
    }
    return FALSE;
}

void userlist_navigate_up(GtkWidget* widget) {
    WinTCWelcomeUserList* self = WINTC_WELCOME_USER_LIST(widget);
    gboolean found_selected = FALSE;
    for (GList *l = g_list_last(self->list); l != NULL; l = g_list_previous(l)) {
        UserListItem* item = (UserListItem*)l->data;

        if (item->selected) {
            found_selected = TRUE;
            if (g_list_previous(l)) {
                item->selected = FALSE;
                idle(item);
                if (item->hovered) {
                    gtk_image_set_from_pixbuf(GTK_IMAGE(item->picture), item->tile_hot);
                } else {
                    item_blur(item->picture);
                    item_blur(item->username_label);
                }
                gtk_widget_show_all(item->layout);
                gtk_entry_set_text(GTK_ENTRY(item->password), "");
                item->selected = FALSE;
                hide_balloon(self);

                UserListItem* next = (UserListItem*) g_list_previous(l)->data;
                next->selected = TRUE;
                hide_balloon(self); 
                hot(next);
                item_unblur_fast(next->picture);
                item_unblur_fast(next->username_label);

                break;
            } 
        } 
    } 
        
    // If no item was selected, select the first one
    if (self->list->data && !found_selected) {
        UserListItem* item = (UserListItem*) self->list->data;
        item->selected = TRUE;
        hot(item);
        item_unblur_fast(item->picture);
        item_unblur_fast(item->username_label);
    }

    // Blur all other items
    for (GList *l = g_list_last(self->list); l != NULL; l = g_list_previous(l)) {
        UserListItem* item = (UserListItem*)l->data;
        if (!item->selected) {
            if (!item->hovered) {
                item_blur(item->picture);
                item_blur(item->username_label);
            }
        }
    } 
}

void userlist_navigate_down(GtkWidget* widget) {
    WinTCWelcomeUserList* self = WINTC_WELCOME_USER_LIST(widget);
    gboolean found_selected = FALSE;
    for (GList *l = self->list; l != NULL; l = l->next) {
        UserListItem* item = (UserListItem*)l->data;
        if (item->selected) {
            found_selected = TRUE;

            if (l->next) {
                item->selected = FALSE;
                idle(item);
                if (item->hovered) {
                    gtk_image_set_from_pixbuf(GTK_IMAGE(item->picture), item->tile_hot);
                } else {
                    item_blur(item->picture);
                    item_blur(item->username_label);
                }
                gtk_widget_show_all(item->layout);
                gtk_entry_set_text(GTK_ENTRY(item->password), "");
                item->selected = FALSE;
                hide_balloon(self);
                
                UserListItem* next = (UserListItem*) l->next->data;
                next->selected = TRUE;
                hot(next);
                item_unblur_fast(next->picture);
                item_unblur_fast(next->username_label);

                break;        
            } 
        }
    }

    // If no item was selected, select the first one
    if (self->list->data && !found_selected) {
        UserListItem* item = (UserListItem*) self->list->data;
        item->selected = TRUE;
        hot(item);
        item_unblur_fast(item->picture);
        item_unblur_fast(item->username_label);
    }
    
    // Blur all other items
    for (GList *l = self->list; l != NULL; l = l->next) {
        UserListItem* item = (UserListItem*)l->data;
        if (!item->selected) {
           if (!item->hovered) {
                item_blur(item->picture);
                item_blur(item->username_label);
            }
        } 
    }
}

static gboolean on_password_focus_gain(GtkWidget *widget, WINTC_UNUSED(GdkEvent *event), gpointer user_data)
{
    UserListItem *item = (UserListItem *) user_data; 
    // WinTCWelcomeUserList *self = item->parent; 

    GdkDisplay *display = gtk_widget_get_display(widget);

    gboolean caps_lock_on = gdk_keymap_get_caps_lock_state(gdk_keymap_get_for_display(display));
    if (caps_lock_on) {
        show_balloon_under_widget(item, BALLOON_TYPE_WARNING); 
    }
    return FALSE;
}

static void login_attempt(UserListItem *item) 
{
    WinTCWelcomeUserList *self = item->parent;

    wintc_gina_logon_session_try_logon(
        self->logon_session,
        item->name,
        gtk_entry_get_text(GTK_ENTRY(item->password))
    );
}

static gboolean on_password_caps_pressed(WINTC_UNUSED(GtkWidget *widget), GdkEventKey *event, gpointer user_data)
{
    UserListItem *item = (UserListItem *) user_data; 
    WinTCWelcomeUserList *self = item->parent; 

    switch (event->keyval) {
        case GDK_KEY_Caps_Lock:
            if (!(event->state & GDK_LOCK_MASK)) {
                show_balloon_under_widget(item, BALLOON_TYPE_WARNING);
              } else {
                hide_balloon(self);
             }
            break;

        case GDK_KEY_Return:
            gtk_button_clicked(GTK_BUTTON(item->go_button));
            break;

        case GDK_KEY_Down:
        case GDK_KEY_Up:
             break;

        default:
            hide_balloon(self);
            break;
    }
    return FALSE;
}

gboolean on_button_clicked(WINTC_UNUSED(GtkButton *button), gpointer user_data)
{
    UserListItem *item = (UserListItem *) user_data; 
    login_attempt(item); 
    gtk_entry_set_text(GTK_ENTRY(item->password), "");
    return FALSE;
}

void hot(UserListItem *item)
{
    gtk_image_set_from_pixbuf(GTK_IMAGE(item->picture), item->tile_hot);

    gtk_container_remove(GTK_CONTAINER(item->layout), item->picture);
    gtk_container_remove(GTK_CONTAINER(item->layout), item->background);
    gtk_container_remove(GTK_CONTAINER(item->layout), item->username_label);

    gtk_fixed_put(GTK_FIXED(item->layout), item->background, 0, 0);
    gtk_fixed_put(GTK_FIXED(item->layout), item->picture, 7, 7);
    gtk_fixed_put(GTK_FIXED(item->layout), item->username_label, 76, 7);

    gtk_fixed_put(GTK_FIXED(item->layout), item->instruction, 76, 30);
    gtk_fixed_put(GTK_FIXED(item->layout), item->password, 70, 50);
    gtk_fixed_put(GTK_FIXED(item->layout), item->go_button, 247, 51);

    gtk_widget_show_all(item->layout);
    gtk_widget_grab_focus(item->password);
}

void idle(UserListItem *item)
{
    gtk_image_set_from_pixbuf(GTK_IMAGE(item->picture), item->tile);

    gtk_container_remove(GTK_CONTAINER(item->layout), item->picture);
    gtk_container_remove(GTK_CONTAINER(item->layout), item->instruction);
    gtk_container_remove(GTK_CONTAINER(item->layout), item->background);
    gtk_container_remove(GTK_CONTAINER(item->layout), item->password);
    gtk_container_remove(GTK_CONTAINER(item->layout), item->go_button);
    gtk_container_remove(GTK_CONTAINER(item->layout), item->username_label);

    gtk_fixed_put(GTK_FIXED(item->layout), item->picture, 7, 7);
    gtk_fixed_put(GTK_FIXED(item->layout), item->username_label, 76, 7);

    gtk_widget_show_all(item->layout);
}

gboolean on_list_item_clicked(WINTC_UNUSED(GtkWidget *widget), WINTC_UNUSED(GdkEvent *event), gpointer user_data)
{
    UserListItem *item = (UserListItem *)user_data; 
    WinTCWelcomeUserList *self = item->parent; 

    if (!item->selected)
    {
        for (GList *l = self->list; l != NULL; l = l->next)
        {
            UserListItem *other_item = (UserListItem *)l->data;
            if (other_item != item && other_item->selected)
            {
                other_item->selected = FALSE;
                gtk_entry_set_text(GTK_ENTRY(other_item->password), "");

                item_blur(other_item->picture);
                item_blur(other_item->username_label);
                idle(other_item);
            }
        }
        item->selected = TRUE;
        hide_balloon(self);
        hot(item);
        gtk_image_set_from_pixbuf(GTK_IMAGE(item->picture), item->tile_hot);
        item_unblur(item->picture);
        item_unblur(item->username_label);
    }

    return FALSE;
}

static void hide_balloon(WinTCWelcomeUserList *self)
{
    if (self->balloon) {
        gtk_container_remove(GTK_CONTAINER(self->balloon_wrapper), self->balloon);
        gtk_widget_destroy(self->balloon);
        self->balloon = NULL;
        gtk_widget_show_all(self->balloon_wrapper);

    }
    if (self->timeout_id != 0) {
        g_source_remove(self->timeout_id);
        self->timeout_id = 0;
    }
}

static gboolean balloon_timeout_callback(gpointer user_data)
{
    WinTCWelcomeUserList *self = (WinTCWelcomeUserList *)user_data;
    if (self->balloon) {
        hide_balloon(self);
    }
    return FALSE; 
}

static gboolean balloon_unblur_callback(gpointer user_data)
{
    WinTCWelcomeUserList *self = (WinTCWelcomeUserList *)user_data;
    if (self->balloon) {
        gtk_style_context_remove_class(gtk_widget_get_style_context(self->balloon), "transparent");
        gtk_style_context_add_class(gtk_widget_get_style_context(self->balloon), "unblur");
    }
    return G_SOURCE_REMOVE; 
}

static void show_balloon_under_widget(UserListItem *item, BalloonType type) 
{
    WinTCWelcomeUserList *self = item->parent; 
    
    // Clear any existing balloon
    hide_balloon(self);
    
    self->balloon = balloon_new(type, item->password);
    gtk_widget_set_halign(self->balloon, GTK_ALIGN_START);
    gtk_widget_set_valign(self->balloon, GTK_ALIGN_START);
    gtk_style_context_add_class(gtk_widget_get_style_context(self->balloon), "transparent");

    GtkAllocation allocation;
    gtk_widget_get_allocation(item->password, &allocation);

    gint x, y;
    gtk_widget_translate_coordinates(
        item->picture, // Use picture as an anchor for positioning as other widgets may not be realized yet
        self->balloon_wrapper,             
        0,                               
        0,                               
        &x,                               
        &y                               
    );

    gtk_widget_set_margin_start(self->balloon, x + 70);
    gtk_widget_set_margin_top(self->balloon, y + 60);

    self->timeout_id = g_timeout_add(6000, (GSourceFunc)balloon_timeout_callback, self);
    g_idle_add((GSourceFunc)balloon_unblur_callback, self);

    gtk_box_pack_start(GTK_BOX(self->balloon_wrapper), self->balloon, TRUE, TRUE, 0);


    gtk_widget_show_all(self->balloon_wrapper);
    gtk_widget_queue_resize(GTK_WIDGET(self->balloon));
}

static GtkWidget *create_userlist_widget(WinTCWelcomeUserList *self)
{
    GtkWidget *scrollable = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(scrollable, FALSE);
    gtk_widget_set_vexpand(scrollable, TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollable), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget *main_box_event_wrapper = gtk_event_box_new();
    g_signal_connect(main_box_event_wrapper, "enter-notify-event",
                     G_CALLBACK(on_list_hover_enter), self);
    g_signal_connect(main_box_event_wrapper, "leave-notify-event",
                     G_CALLBACK(on_list_hover_leave), self);
    gtk_widget_add_events(main_box_event_wrapper, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
    
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GList *users =
    lightdm_user_list_get_users(
        lightdm_user_list_get_instance()
    );

    GtkWidget *top_box_event_wrapper = gtk_event_box_new();
    gtk_widget_set_hexpand(top_box_event_wrapper, TRUE);
    gtk_widget_set_vexpand(top_box_event_wrapper, TRUE);
    g_signal_connect(top_box_event_wrapper, "button-press-event",
                     G_CALLBACK(clear_selections), self);
    gtk_widget_add_events(top_box_event_wrapper, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);

    gtk_box_pack_start(GTK_BOX(main_box), top_box_event_wrapper, TRUE, TRUE, 0);

    for (GList *l = users; l != NULL; l = l->next)
    {
        UserListItem *item = g_new0(UserListItem, 1);
        item->user = (LightDMUser *)l->data; 
        item->name = g_strdup(lightdm_user_get_name(item->user));
        item->parent = self; 
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

        item->picture = gtk_image_new_from_pixbuf(item->tile);
        item->username_label = gtk_label_new(item->name);
        gtk_style_context_add_class(gtk_widget_get_style_context(item->username_label), "user-label");

        item->password = gtk_entry_new();
        gtk_entry_set_visibility(GTK_ENTRY(item->password), FALSE);
        g_object_set(item->password, "caps-lock-warning", FALSE, NULL); 
        gtk_style_context_add_class(gtk_widget_get_style_context(item->password), "password-box");
        gtk_widget_set_size_request(item->password, 164, 27);

        g_signal_connect(item->password, "focus-out-event",
                         G_CALLBACK(on_password_focus_out), self);
        g_signal_connect(item->password, "focus-in-event",
                         G_CALLBACK(on_password_focus_gain), item);
        g_signal_connect(item->password, "key-press-event", G_CALLBACK(on_password_caps_pressed), item); 

        item->instruction = gtk_label_new("Type your password");
        gtk_style_context_add_class(gtk_widget_get_style_context(item->instruction), "password-label");
        
        {
            GdkPixbuf *go_idle = gdk_pixbuf_new_from_resource("/uk/oddmatics/wintc/logonui/gobtn.png", NULL);
            GdkPixbuf *go_activated = gdk_pixbuf_new_from_resource("/uk/oddmatics/wintc/logonui/gobtna.png", NULL);
            item->go_button = simple_button_new_with_pixbufs(go_idle, go_activated);
            g_object_unref(go_idle);
            g_object_unref(go_activated);
        }

        gtk_style_context_add_class(gtk_widget_get_style_context(item->go_button), "plain-button");
        gtk_widget_set_can_focus(item->go_button, FALSE);
        gtk_widget_set_size_request(item->go_button, 22, 27);
        g_signal_connect(item->go_button, "clicked", G_CALLBACK(on_button_clicked), item); 

        GtkWidget *event = gtk_event_box_new();
        GtkWidget *layout = gtk_fixed_new();

        {
            GdkPixbuf *bg_pix = gdk_pixbuf_new_from_resource("/uk/oddmatics/wintc/logonui/usersel.png", NULL);
            bg_pix = gdk_pixbuf_scale_simple(bg_pix, 348, 72, GDK_INTERP_BILINEAR);
            item->background = gtk_image_new_from_pixbuf(bg_pix);
            g_object_unref(bg_pix);
            gtk_widget_set_visible(item->background, FALSE);
        }

        GtkWidget *filler = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_size_request(filler, 348, 80);
        gtk_style_context_add_class(gtk_widget_get_style_context(filler), "transparent");

        item->event = event;
        item->layout = layout;

        self->list = g_list_append(self->list, item);

        g_object_ref(item->background);
        g_object_ref(item->picture);
        g_object_ref(item->username_label);
        g_object_ref(item->password);
        g_object_ref(item->instruction);
        g_object_ref(item->go_button);

        gtk_fixed_put(GTK_FIXED(layout), filler, 0, 0);
        gtk_fixed_put(GTK_FIXED(layout), item->picture, 7, 7);
        gtk_fixed_put(GTK_FIXED(layout), item->username_label, 76, 7);

        g_signal_connect(event, "enter-notify-event",
                         G_CALLBACK(on_list_item_hover_enter), item); 
        g_signal_connect(event, "leave-notify-event",
                         G_CALLBACK(on_list_item_hover_leave), item);
        gtk_widget_add_events(event, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);

        gtk_container_add(GTK_CONTAINER(event), layout);
        
        g_signal_connect(G_OBJECT(event), "button_press_event",
                         G_CALLBACK(on_list_item_clicked), item);

        gtk_box_pack_start(GTK_BOX(main_box), event, FALSE, FALSE, 0);
    }

    GtkWidget *bottom_box_event_wrapper = gtk_event_box_new();
    gtk_widget_set_hexpand(bottom_box_event_wrapper, TRUE);
    gtk_widget_set_vexpand(bottom_box_event_wrapper, TRUE);
    g_signal_connect(bottom_box_event_wrapper, "button-press-event",
                     G_CALLBACK(clear_selections), self);
    gtk_widget_add_events(bottom_box_event_wrapper, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
    gtk_box_pack_start(GTK_BOX(main_box), bottom_box_event_wrapper, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(main_box_event_wrapper), main_box);
    gtk_container_add(GTK_CONTAINER(scrollable), main_box_event_wrapper);

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
        for (GList *l = user_list->list; l != NULL; l = l->next)
        {
            UserListItem *item = (UserListItem *)l->data;
            if (item->selected) {
                show_balloon_under_widget(item, BALLOON_TYPE_ERROR);
                break;
            }
        }
    } 
}


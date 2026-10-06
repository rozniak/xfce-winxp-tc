#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <math.h>
#include <wintc/comgtk.h>

#include "../public/lstvwctl.h"

#define LABEL_TEXT_SHADOW_INTENSITY 3
#define LABEL_TEXT_SHADOW_OFFSET    (LABEL_TEXT_SHADOW_RADIUS * 2)
#define LABEL_TEXT_SHADOW_RADIUS    2

#define DRAG_THRESHOLD    5
#define HITBOX_LARGE_ICON 32

#define CELL_SIZE_LARGE_ICON 75

#define ICON_IS_COMMITTED(i) (i->icon && i->text)

//
// PRIVATE ENUMS
//
enum
{
    SIGNAL_ITEM_ACTIVATED = 0,
    N_SIGNALS
};

typedef enum _WinTCCtlListViewIconStyle
{
    WINTC_CTL_LIST_VIEW_ICON_STYLE_REGULAR,
    WINTC_CTL_LIST_VIEW_ICON_STYLE_SELECTED,
    WINTC_CTL_LIST_VIEW_ICON_STYLE_GHOSTED
} WinTCCtlListViewIconStyle;

//
// PRIVATE STRUCTURES
//
typedef struct _WinTCCtlListViewIcon
{
    // Basic state
    //
    GdkPixbuf* icon;
    gchar*     text;

    gboolean     realize_idx; // If we're to delay positioning to realize
    GdkRectangle hitbox_icon;
    GdkRectangle hitbox_label;

    GdkPoint offset_label_render;

    // Drawing cache
    //
    cairo_surface_t* surface_text_shadow;
} WinTCCtlListViewIcon;

//
// FORWARD DECLARATIONS
//
static void wintc_ctl_list_view_dispose(
    GObject* object
);

static gboolean wintc_ctl_list_view_draw(
    GtkWidget* widget,
    cairo_t*   cr
);
static void wintc_ctl_list_view_get_preferred_height(
    GtkWidget* widget,
    gint*      minimum_height,
    gint*      natural_height
);
static void wintc_ctl_list_view_get_preferred_height_for_width(
    GtkWidget* widget,
    gint       width,
    gint*      minimum_height,
    gint*      natural_height
);
static void wintc_ctl_list_view_get_preferred_width(
    GtkWidget* widget,
    gint*      minimum_width,
    gint*      natural_width
);
static void wintc_ctl_list_view_get_preferred_width_for_height(
    GtkWidget* widget,
    gint       height,
    gint*      minimum_width,
    gint*      natural_width
);
static GtkSizeRequestMode wintc_ctl_list_view_get_request_mode(
    GtkWidget* widget
);
static void wintc_ctl_list_view_size_allocate(
    GtkWidget*     widget,
    GtkAllocation* allocation
);

static void wintc_ctl_list_view_auto_arrange(
    WinTCCtlListView* list_view,
    GSequenceIter*    iter_seq
);
static void wintc_ctl_list_view_commit_icon_drag(
    WinTCCtlListView* list_view
);
static WinTCCtlListViewIcon* wintc_ctl_list_view_create_large_icon(
    WinTCCtlListView* list_view
);
static PangoLayout* wintc_ctl_list_view_create_pango_layout(
    WinTCCtlListView* list_view
);
static void wintc_ctl_list_view_emit_item_activated(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon
);
static void wintc_ctl_list_view_get_max_icon_pos(
    WinTCCtlListView* list_view,
    gint*             x,
    gint*             y
);
static void wintc_ctl_list_view_get_next_icon_pos_for_cell(
    WinTCCtlListView* list_view,
    gint              cell_idx,
    gint*             x,
    gint*             y
);
static GtkTreePath* wintc_ctl_list_view_get_path_for_icon(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon
);
static void wintc_ctl_list_view_get_preferred_size(
    WinTCCtlListView* list_view,
    GtkOrientation    orientation,
    gint              provided_size,
    gint*             minimum_size,
    gint*             natural_size
);
static gboolean wintc_ctl_list_view_has_solid_bg(
    WinTCCtlListView* list_view
);
static WinTCCtlListViewIcon* wintc_ctl_list_view_hit_test(
    WinTCCtlListView* list_view,
    gint              x,
    gint              y,
    gboolean*         hit_label
);
static void wintc_ctl_list_view_insert_icon(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon,
    gint                  idx
);
static void wintc_ctl_list_view_move_icon(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon,
    gint                  x,
    gint                  y
);
static void wintc_ctl_list_view_raise_icon(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon
);
static void wintc_ctl_list_view_render_large_icon(
    WinTCCtlListView*         list_view,
    WinTCCtlListViewIcon*     large_icon,
    cairo_t*                  cr,
    WinTCCtlListViewIconStyle style,
    gint                      offset_x,
    gint                      offset_y
);
static void wintc_ctl_list_view_reset_hit_state(
    WinTCCtlListView* list_view
);
static void wintc_ctl_list_view_set_icon_pixbuf(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon,
    GdkPixbuf*            pixbuf
);
static void wintc_ctl_list_view_set_icon_text(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon,
    gchar*                text
);
static void wintc_ctl_list_view_update_dnd_state(
    WinTCCtlListView* list_view
);
static void wintc_ctl_list_view_update_icon(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon,
    GtkTreeIter*          iter
);
static void wintc_ctl_list_view_update_icon_text_shadow(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon
);

static void wintc_ctl_list_view_icon_free(
    WinTCCtlListViewIcon* icon
);

static void on_gtk_settings_font_name_notify(
    GObject*    self,
    GParamSpec* pspec,
    gpointer    user_data
);

static void on_list_view_drag_end(
    GtkWidget*      widget,
    GdkDragContext* context,
    gpointer        user_data
);
static gboolean on_list_view_drag_motion(
    GtkWidget*      widget,
    GdkDragContext* context,
    int             x,
    int             y,
    guint           time,
    gpointer        user_data
);
static gboolean on_list_view_button_press_event(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);
static gboolean on_list_view_button_release_event(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);
static gboolean on_list_view_enter_notify_event(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);
static gboolean on_list_view_key_press_event(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);
static gboolean on_list_view_leave_notify_event(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);
static gboolean on_list_view_motion_notify_event(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);
static void on_list_view_realize(
    GtkWidget* widget,
    gpointer   user_data
);

static void on_model_row_changed(
    GtkTreeModel* tree_model,
    GtkTreePath*  path,
    GtkTreeIter*  iter,
    gpointer      user_data
);
static void on_model_row_deleted(
    GtkTreeModel* tree_model,
    GtkTreePath*  path,
    gpointer      user_data
);
static void on_model_row_inserted(
    GtkTreeModel* tree_model,
    GtkTreePath*  path,
    GtkTreeIter*  iter,
    gpointer      user_data
);
static void on_model_rows_reordered(
    GtkTreeModel* tree_model,
    GtkTreePath*  path,
    GtkTreeIter*  iter,
    gpointer      new_order,
    gpointer      user_data
);

//
// STATIC DATA
//
static gint wintc_ctl_list_view_signals[N_SIGNALS] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCCtlListViewClass
{
    GtkDrawingAreaClass __parent__;
};

struct _WinTCCtlListView
{
    GtkDrawingArea __parent__;

    GList* list_icons;
    GList* list_selected;

    // Model related
    //
    GtkTreeModel* model;
    gint          col_pixbuf;
    gint          col_text;

    GSequence* seq_icons;

    gulong sigid_row_changed;
    gulong sigid_row_deleted;
    gulong sigid_row_inserted;
    gulong sigid_rows_reordered;

    // Item positioning
    //
    gint           itempos_count;
    GdkPoint       itempos_max;
    GtkOrientation orientation;

    gboolean auto_arrange;

    // UI State
    //
    GdkRectangle motion_rect;

    WinTCCtlListViewIcon* hit_icon;
    gboolean              hit_started;
    gboolean              hit_dragging;
    gboolean              hit_in_widget;

    WinTCCtlListViewIcon* hit_icon_last;

    // DND stuff
    //
    GtkTargetList*  dnd_dest_targets;
    GdkDragAction   dnd_dest_actions;
    GtkTargetList*  dnd_src_targets;
    GdkDragAction   dnd_src_actions;
    GdkDragContext* dnd_ctx;

    WinTCCtlListViewIcon* dnd_icon_target;

    // Rendering
    //
    gboolean dirty_labels;
    gboolean solid_bg;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCCtlListView,
    wintc_ctl_list_view,
    GTK_TYPE_DRAWING_AREA
)

static void wintc_ctl_list_view_class_init(
    WinTCCtlListViewClass* klass
)
{
    GObjectClass*   object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);

    object_class->dispose = wintc_ctl_list_view_dispose;

    widget_class->draw = wintc_ctl_list_view_draw;
    widget_class->get_preferred_height =
        wintc_ctl_list_view_get_preferred_height;
    widget_class->get_preferred_height_for_width =
        wintc_ctl_list_view_get_preferred_height_for_width;
    widget_class->get_preferred_width =
        wintc_ctl_list_view_get_preferred_width;
    widget_class->get_preferred_width_for_height =
        wintc_ctl_list_view_get_preferred_width_for_height;
    widget_class->get_request_mode =
        wintc_ctl_list_view_get_request_mode;
    widget_class->size_allocate =
        wintc_ctl_list_view_size_allocate;

    wintc_ctl_list_view_signals[SIGNAL_ITEM_ACTIVATED] =
        g_signal_new(
            "item-activated",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__POINTER,
            G_TYPE_NONE,
            1,
            G_TYPE_POINTER
        );

    gtk_widget_class_set_css_name(widget_class, "listview");
}

static void wintc_ctl_list_view_init(
    WinTCCtlListView* self
)
{
    self->col_pixbuf = -1;
    self->col_text   = -1;
    self->solid_bg   = TRUE;

    gtk_widget_add_events(
        GTK_WIDGET(self),
        GDK_BUTTON_PRESS_MASK   |
        GDK_BUTTON_MOTION_MASK  |
        GDK_BUTTON_RELEASE_MASK |
        GDK_ENTER_NOTIFY_MASK   |
        GDK_LEAVE_NOTIFY_MASK   |
        GDK_KEY_PRESS_MASK
    );

    gtk_widget_set_can_focus(GTK_WIDGET(self), TRUE);

    g_signal_connect(
        self,
        "button-press-event",
        G_CALLBACK(on_list_view_button_press_event),
        NULL
    );
    g_signal_connect(
        self,
        "button-release-event",
        G_CALLBACK(on_list_view_button_release_event),
        NULL
    );
    g_signal_connect(
        self,
        "drag-end",
        G_CALLBACK(on_list_view_drag_end),
        NULL
    );
    g_signal_connect(
        self,
        "drag-motion",
        G_CALLBACK(on_list_view_drag_motion),
        NULL
    );
    g_signal_connect(
        self,
        "enter-notify-event",
        G_CALLBACK(on_list_view_enter_notify_event),
        NULL
    );
    g_signal_connect(
        self,
        "key-press-event",
        G_CALLBACK(on_list_view_key_press_event),
        NULL
    );
    g_signal_connect(
        self,
        "leave-notify-event",
        G_CALLBACK(on_list_view_leave_notify_event),
        NULL
    );
    g_signal_connect(
        self,
        "motion-notify-event",
        G_CALLBACK(on_list_view_motion_notify_event),
        NULL
    );
    g_signal_connect(
        self,
        "realize",
        G_CALLBACK(on_list_view_realize),
        NULL
    );

    // Watch for font setting changes
    //
    GtkSettings* settings = gtk_settings_get_default();

    g_signal_connect_object(
        settings,
        "notify::gtk-font-name",
        G_CALLBACK(on_gtk_settings_font_name_notify),
        self,
        G_CONNECT_DEFAULT
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_ctl_list_view_dispose(
    GObject* object
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(object);

    wintc_ctl_list_view_set_model(list_view, NULL);
    wintc_ctl_list_view_unset_drag_dest(list_view);
    wintc_ctl_list_view_unset_drag_source(list_view);

    (G_OBJECT_CLASS(wintc_ctl_list_view_parent_class))
        ->dispose(object);
}

static gboolean wintc_ctl_list_view_draw(
    GtkWidget* widget,
    cairo_t*   cr
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);

    // Update icon labels if needed
    //
    if (list_view->dirty_labels)
    {
        // Iterate over icons to update text shadows
        //
        for (GList* iter = list_view->list_icons; iter; iter = iter->next)
        {
            wintc_ctl_list_view_update_icon_text_shadow(
                list_view,
                (WinTCCtlListViewIcon*) iter->data
            );
        }

        list_view->dirty_labels = FALSE;
    }

    // Paint BG
    //
    GtkAllocation alloc;

    gtk_widget_get_allocation(widget, &alloc);

    gtk_render_background(
        gtk_widget_get_style_context(widget),
        cr,
        alloc.x,
        alloc.y,
        alloc.width,
        alloc.height
    );

    // Paint icons - painting from end to start because the first item in the
    // list is the highest z-order
    //
    for (
        GList* iter = g_list_last(list_view->list_icons);
        iter;
        iter = iter->prev
    )
    {
        WinTCCtlListViewIcon* large_icon =
            (WinTCCtlListViewIcon*) iter->data;

        if (!ICON_IS_COMMITTED(large_icon))
        {
            continue;
        }

        gboolean render_selected =
            large_icon == list_view->dnd_icon_target ||
            g_list_find(list_view->list_selected, large_icon);

        wintc_ctl_list_view_render_large_icon(
            list_view,
            large_icon,
            cr,
            render_selected ?
                WINTC_CTL_LIST_VIEW_ICON_STYLE_SELECTED :
                WINTC_CTL_LIST_VIEW_ICON_STYLE_REGULAR,
            0,
            0
        );
    }

    // Handle dragging ops...
    //
    if (list_view->hit_started)
    {
        if (list_view->hit_dragging) // Dragging icons
        {
            for (
                GList* iter = list_view->list_selected;
                iter;
                iter = iter->next
            )
            {
                WinTCCtlListViewIcon* large_icon =
                    (WinTCCtlListViewIcon*) iter->data;

                wintc_ctl_list_view_render_large_icon(
                    list_view,
                    large_icon,
                    cr,
                    WINTC_CTL_LIST_VIEW_ICON_STYLE_GHOSTED,
                    list_view->motion_rect.width,
                    list_view->motion_rect.height
                );
            }
        }
        else // Selection box drag
        {
            const gdouble s_dashes[] = { 1.0f };

            cairo_rectangle(
                cr,
                (gdouble) list_view->motion_rect.x,
                (gdouble) list_view->motion_rect.y,
                (gdouble) list_view->motion_rect.width,
                (gdouble) list_view->motion_rect.height
            );

            cairo_set_dash(
                cr,
                s_dashes,
                1,
                0.0f
            );
            cairo_set_operator(cr, CAIRO_OPERATOR_XOR);
            cairo_set_source_rgb(cr, 0.0f, 0.0f, 0.0f);

            cairo_stroke(cr);
        }
    }

    return FALSE;
}

static void wintc_ctl_list_view_get_preferred_height(
    GtkWidget* widget,
    gint*      minimum_height,
    gint*      natural_height
)
{
    wintc_ctl_list_view_get_preferred_size(
        WINTC_CTL_LIST_VIEW(widget),
        GTK_ORIENTATION_VERTICAL,
        -1,
        minimum_height,
        natural_height
    );
}

static void wintc_ctl_list_view_get_preferred_height_for_width(
    GtkWidget* widget,
    gint       width,
    gint*      minimum_height,
    gint*      natural_height
)
{
    wintc_ctl_list_view_get_preferred_size(
        WINTC_CTL_LIST_VIEW(widget),
        GTK_ORIENTATION_VERTICAL,
        width,
        minimum_height,
        natural_height
    );
}

static void wintc_ctl_list_view_get_preferred_width(
    GtkWidget* widget,
    gint*      minimum_width,
    gint*      natural_width
)
{
    wintc_ctl_list_view_get_preferred_size(
        WINTC_CTL_LIST_VIEW(widget),
        GTK_ORIENTATION_HORIZONTAL,
        -1,
        minimum_width,
        natural_width
    );
}

static void wintc_ctl_list_view_get_preferred_width_for_height(
    GtkWidget* widget,
    gint       height,
    gint*      minimum_width,
    gint*      natural_width
)
{
    wintc_ctl_list_view_get_preferred_size(
        WINTC_CTL_LIST_VIEW(widget),
        GTK_ORIENTATION_HORIZONTAL,
        height,
        minimum_width,
        natural_width
    );
}

static GtkSizeRequestMode wintc_ctl_list_view_get_request_mode(
    GtkWidget* widget
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);

    if (list_view->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
        return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
    }
    else // GTK_ORIENTATION_VERTICAL
    {
        return GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT;
    }
}

static void wintc_ctl_list_view_size_allocate(
    GtkWidget*     widget,
    GtkAllocation* allocation
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);

    (GTK_WIDGET_CLASS(wintc_ctl_list_view_parent_class))
        ->size_allocate(widget, allocation);

    if (list_view->auto_arrange)
    {
        wintc_ctl_list_view_auto_arrange(
            list_view,
            g_sequence_get_begin_iter(list_view->seq_icons)
        );
    }
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_ctl_list_view_new(void)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_CTL_LIST_VIEW,
            NULL
        )
    );
}

void wintc_ctl_list_view_enable_drag_dest(
    WinTCCtlListView*     list_view,
    const GtkTargetEntry* targets,
    gint                  n_targets,
    GdkDragAction         actions
)
{
    if (list_view->dnd_dest_targets)
    {
        wintc_ctl_list_view_unset_drag_dest(list_view);
    }

    list_view->dnd_dest_targets = gtk_target_list_new(targets, n_targets);
    list_view->dnd_dest_actions = actions;

    wintc_ctl_list_view_update_dnd_state(list_view);
}

void wintc_ctl_list_view_enable_drag_source(
    WinTCCtlListView*     list_view,
    const GtkTargetEntry* targets,
    gint                  n_targets,
    GdkDragAction         actions
)
{
    if (list_view->dnd_src_targets)
    {
        wintc_ctl_list_view_unset_drag_source(list_view);
    }

    list_view->dnd_src_targets = gtk_target_list_new(targets, n_targets);
    list_view->dnd_src_actions = actions;

    // Must set ourselves up as a drag destination to be able to receive
    // drag motion events
    //
    gtk_drag_dest_set(
        GTK_WIDGET(list_view),
        0,
        NULL,
        0,
        GDK_ACTION_COPY
    );

    wintc_ctl_list_view_update_dnd_state(list_view);
}

gboolean wintc_ctl_list_view_get_auto_arrange(
    WinTCCtlListView* list_view
)
{
    return list_view->auto_arrange;
}

GtkTargetList* wintc_ctl_list_view_get_dest_target_list(
    WinTCCtlListView* list_view
)
{
    return list_view->dnd_dest_targets;
}

GtkTreePath* wintc_ctl_list_view_get_drop_target(
    WinTCCtlListView* list_view
)
{
    if (!(list_view->dnd_icon_target))
    {
        return NULL;
    }

    return wintc_ctl_list_view_get_path_for_icon(
        list_view,
        list_view->dnd_icon_target
    );
}

GtkTreeModel* wintc_ctl_list_view_get_model(
    WinTCCtlListView* list_view
)
{
    return list_view->model;
}

GtkOrientation wintc_ctl_list_view_get_orientation(
    WinTCCtlListView* list_view
)
{
    return list_view->orientation;
}

GtkTreePath* wintc_ctl_list_view_get_path_at_pos(
    WinTCCtlListView* list_view,
    gint              x,
    gint              y
)
{
    WinTCCtlListViewIcon* icon =
        wintc_ctl_list_view_hit_test(list_view, x, y, NULL);

    if (!icon)
    {
        return NULL;
    }

    return wintc_ctl_list_view_get_path_for_icon(list_view, icon);
}

gint wintc_ctl_list_view_get_pixbuf_column(
    WinTCCtlListView* list_view
)
{
    return list_view->col_pixbuf;
}

GList* wintc_ctl_list_view_get_selected_items(
    WinTCCtlListView* list_view
)
{
    GList* ret = NULL;

    for (GList* iter = list_view->list_selected; iter; iter = iter->next)
    {
        ret =
            g_list_prepend(
                ret,
                wintc_ctl_list_view_get_path_for_icon(
                    list_view,
                    (WinTCCtlListViewIcon*) iter->data
                )
            );
    }

    ret = g_list_reverse(ret);

    return ret;
}

GtkTargetList* wintc_ctl_list_view_get_source_target_list(
    WinTCCtlListView* list_view
)
{
    return list_view->dnd_src_targets;
}

gint wintc_ctl_list_view_get_text_column(
    WinTCCtlListView* list_view
)
{
    return list_view->col_text;
}

void wintc_ctl_list_view_reset_layout(
    WinTCCtlListView* list_view
)
{
    list_view->itempos_count = 0;
    list_view->itempos_max.x = 0;
    list_view->itempos_max.y = 0;

    // Iterate over the SEQUENCE to position everything
    //
    GSequenceIter* iter = g_sequence_get_begin_iter(list_view->seq_icons);

    while (!g_sequence_iter_is_end(iter))
    {
        WinTCCtlListViewIcon* icon = g_sequence_get(iter);

        gint x;
        gint y;

        wintc_ctl_list_view_get_next_icon_pos_for_cell(
            list_view,
            list_view->itempos_count++,
            &x,
            &y
        );

        wintc_ctl_list_view_move_icon(
            list_view,
            icon,
            x,
            y
        );

        iter = g_sequence_iter_next(iter);
    }
}

void wintc_ctl_list_view_select_path(
    WinTCCtlListView* list_view,
    GtkTreePath*      path
)
{
    if (gtk_tree_path_get_depth(path) > 1)
    {
        return;
    }

    // Retrieve the icon we want
    //
    gint idx = gtk_tree_path_get_indices(path)[0];

    GSequenceIter* iter_seq =
        g_sequence_get_iter_at_pos(list_view->seq_icons, idx);

    WinTCCtlListViewIcon* icon = g_sequence_get(iter_seq);

    // Handle selection
    //
    if (!g_list_find(list_view->list_selected, icon))
    {
        list_view->list_selected =
            g_list_prepend(list_view->list_selected, icon);

        gtk_widget_queue_draw(GTK_WIDGET(list_view));
    }
}

void wintc_ctl_list_view_set_auto_arrange(
    WinTCCtlListView* list_view,
    gboolean          auto_arrange
)
{
    list_view->auto_arrange = auto_arrange;

    gtk_widget_queue_resize(GTK_WIDGET(list_view));
}

void wintc_ctl_list_view_set_model(
    WinTCCtlListView* list_view,
    GtkTreeModel*     model
)
{
    // Detach from model
    //
    if (list_view->model)
    {
        g_signal_handler_disconnect(
            list_view->model,
            list_view->sigid_row_changed
        );
        g_signal_handler_disconnect(
            list_view->model,
            list_view->sigid_row_deleted
        );
        g_signal_handler_disconnect(
            list_view->model,
            list_view->sigid_row_inserted
        );
        g_signal_handler_disconnect(
            list_view->model,
            list_view->sigid_rows_reordered
        );

        list_view->sigid_row_changed    = 0;
        list_view->sigid_row_deleted    = 0;
        list_view->sigid_row_inserted   = 0;
        list_view->sigid_rows_reordered = 0;

        // Erase all rows
        //
        g_clear_list(&(list_view->list_selected), NULL);
        g_sequence_free(list_view->seq_icons);

        g_clear_list(
            &(list_view->list_icons),
            (GDestroyNotify) wintc_ctl_list_view_icon_free
        );
    }

    // Attach to new model
    //
    list_view->model = model;

    if (!model)
    {
        goto done;
    }

    list_view->seq_icons = g_sequence_new(NULL);

    list_view->sigid_row_changed =
        g_signal_connect_object(
            list_view->model,
            "row-changed",
            G_CALLBACK(on_model_row_changed),
            list_view,
            G_CONNECT_DEFAULT
        );
    list_view->sigid_row_deleted =
        g_signal_connect_object(
            list_view->model,
            "row-deleted",
            G_CALLBACK(on_model_row_deleted),
            list_view,
            G_CONNECT_DEFAULT
        );
    list_view->sigid_row_inserted =
        g_signal_connect_object(
            list_view->model,
            "row-inserted",
            G_CALLBACK(on_model_row_inserted),
            list_view,
            G_CONNECT_DEFAULT
        );
    list_view->sigid_rows_reordered =
        g_signal_connect_object(
            list_view->model,
            "rows-reordered",
            G_CALLBACK(on_model_rows_reordered),
            list_view,
            G_CONNECT_DEFAULT
        );

    // Create items for new model
    //
    gint        idx   = 0;
    GtkTreeIter iter;
    gboolean    valid =
        gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);

    while (valid)
    {
        WinTCCtlListViewIcon* icon =
            wintc_ctl_list_view_create_large_icon(list_view);

        GdkPixbuf* pixbuf;
        gchar*     text;

        gtk_tree_model_get(
            GTK_TREE_MODEL(model),
            &iter,
            list_view->col_pixbuf, &pixbuf,
            list_view->col_text,   &text,
            -1
        );

        wintc_ctl_list_view_set_icon_pixbuf(
            list_view,
            icon,
            pixbuf
        );
        wintc_ctl_list_view_set_icon_text(
            list_view,
            icon,
            text
        );

        wintc_ctl_list_view_insert_icon(
            list_view,
            icon,
            idx
        );

        idx++;
        valid = gtk_tree_model_iter_next(model, &iter);
    }

done:
    gtk_widget_queue_resize(GTK_WIDGET(list_view));
}

void wintc_ctl_list_view_set_orientation(
    WinTCCtlListView* list_view,
    GtkOrientation    orientation
)
{
    list_view->orientation = orientation;

    gtk_widget_queue_resize(GTK_WIDGET(list_view));
}

void wintc_ctl_list_view_set_pixbuf_column(
    WinTCCtlListView* list_view,
    gint              column
)
{
    list_view->col_pixbuf = column;

    if (!(list_view->model))
    {
        return;
    }

    // Update from model
    //
    GtkTreeIter    iter;
    GSequenceIter* iter_seq = g_sequence_get_begin_iter(list_view->seq_icons);
    gboolean       valid =
        gtk_tree_model_get_iter_first(list_view->model, &iter);

    while (valid)
    {
        WinTCCtlListViewIcon* icon = g_sequence_get(iter_seq);
        GdkPixbuf*            pixbuf;

        gtk_tree_model_get(
            list_view->model,
            &iter,
            list_view->col_pixbuf, &pixbuf,
            -1
        );

        wintc_ctl_list_view_set_icon_pixbuf(
            list_view,
            icon,
            pixbuf
        );

        valid = gtk_tree_model_iter_next(list_view->model, &iter);
    }
}

void wintc_ctl_list_view_set_text_column(
    WinTCCtlListView* list_view,
    gint              column
)
{
    list_view->col_text = column;

    if (!(list_view->model))
    {
        return;
    }

    // Update from model
    //
    GtkTreeIter    iter;
    GSequenceIter* iter_seq = g_sequence_get_begin_iter(list_view->seq_icons);
    gboolean       valid =
        gtk_tree_model_get_iter_first(list_view->model, &iter);

    while (valid)
    {
        WinTCCtlListViewIcon* icon = g_sequence_get(iter_seq);
        gchar*                text;

        gtk_tree_model_get(
            list_view->model,
            &iter,
            list_view->col_text, &text,
            -1
        );

        wintc_ctl_list_view_set_icon_text(
            list_view,
            icon,
            text
        );

        valid = gtk_tree_model_iter_next(list_view->model, &iter);
    }
}

gboolean wintc_ctl_list_view_should_ignore_drop(
    WinTCCtlListView* list_view
)
{
    return list_view->dnd_ctx && !(list_view->dnd_icon_target);
}

void wintc_ctl_list_view_unselect_all(
    WinTCCtlListView* list_view
)
{
    g_clear_list(&(list_view->list_selected), NULL);
    gtk_widget_queue_draw(GTK_WIDGET(list_view));
}

void wintc_ctl_list_view_unset_drag_dest(
    WinTCCtlListView* list_view
)
{
    if (list_view->dnd_dest_targets)
    {
        gtk_target_list_unref(list_view->dnd_dest_targets);
    }

    list_view->dnd_dest_targets   = NULL;
    list_view->dnd_dest_actions   = 0;

    wintc_ctl_list_view_update_dnd_state(list_view);
}

void wintc_ctl_list_view_unset_drag_source(
    WinTCCtlListView* list_view
)
{
    if (list_view->dnd_ctx)
    {
        gtk_drag_cancel(list_view->dnd_ctx);
        g_clear_object(&(list_view->dnd_ctx));
    }

    if (list_view->dnd_src_targets)
    {
        gtk_target_list_unref(list_view->dnd_src_targets);
    }

    list_view->dnd_src_targets = NULL;
    list_view->dnd_src_actions = 0;

    wintc_ctl_list_view_update_dnd_state(list_view);
}

//
// PRIVATE FUNCTIONS
//
static void wintc_ctl_list_view_auto_arrange(
    WinTCCtlListView* list_view,
    GSequenceIter*    iter_seq
)
{
    while (!g_sequence_iter_is_end(iter_seq))
    {
        WinTCCtlListViewIcon* icon = g_sequence_get(iter_seq);

        gint x;
        gint y;

        wintc_ctl_list_view_get_next_icon_pos_for_cell(
            list_view,
            g_sequence_iter_get_position(iter_seq),
            &x,
            &y
        );

        wintc_ctl_list_view_move_icon(
            list_view,
            icon,
            x,
            y
        );

        iter_seq = g_sequence_iter_next(iter_seq);
    }
}

static void wintc_ctl_list_view_commit_icon_drag(
    WinTCCtlListView* list_view
)
{
    if (list_view->auto_arrange)
    {
        return;
    }

    for (GList* iter = list_view->list_selected; iter; iter = iter->next)
    {
        WinTCCtlListViewIcon* large_icon =
            (WinTCCtlListViewIcon*) iter->data;

        large_icon->hitbox_icon.x  += list_view->motion_rect.width;
        large_icon->hitbox_icon.y  += list_view->motion_rect.height;
        large_icon->hitbox_label.x += list_view->motion_rect.width;
        large_icon->hitbox_label.y += list_view->motion_rect.height;

        // Track max icon pos
        //
        list_view->itempos_max.x =
            MAX(list_view->itempos_max.x, large_icon->hitbox_icon.x);
        list_view->itempos_max.y =
            MAX(list_view->itempos_max.y, large_icon->hitbox_icon.y);
    }

    gtk_widget_queue_resize(GTK_WIDGET(list_view));
}

static WinTCCtlListViewIcon* wintc_ctl_list_view_create_large_icon(
    WINTC_UNUSED(WinTCCtlListView* list_view)
)
{
    WinTCCtlListViewIcon* large_icon = g_new0(WinTCCtlListViewIcon, 1);

    large_icon->realize_idx        = -1;
    large_icon->hitbox_icon.width  = HITBOX_LARGE_ICON;
    large_icon->hitbox_icon.height = HITBOX_LARGE_ICON;

    return large_icon;
}

static PangoLayout* wintc_ctl_list_view_create_pango_layout(
    WinTCCtlListView* list_view
)
{
    PangoContext* ctx    = gtk_widget_get_pango_context(GTK_WIDGET(list_view));
    PangoLayout*  layout = pango_layout_new(ctx);

    pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);
    pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_END);
    pango_layout_set_height(layout, -3);
    pango_layout_set_width(layout, CELL_SIZE_LARGE_ICON * PANGO_SCALE);
    pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);

    return layout;
}

static void wintc_ctl_list_view_emit_item_activated(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon
)
{
    GtkTreePath* path =
        wintc_ctl_list_view_get_path_for_icon(
            list_view,
            icon
        );

    g_signal_emit(
        list_view,
        wintc_ctl_list_view_signals[SIGNAL_ITEM_ACTIVATED],
        0,
        path
    );

    gtk_tree_path_free(path);
}

static void wintc_ctl_list_view_get_max_icon_pos(
    WinTCCtlListView* list_view,
    gint*             x,
    gint*             y
)
{
    gint offset =
        HITBOX_LARGE_ICON +
        ((CELL_SIZE_LARGE_ICON - HITBOX_LARGE_ICON) / 2);

    WINTC_SAFE_REF_SET(x, list_view->itempos_max.x + offset);
    WINTC_SAFE_REF_SET(y, list_view->itempos_max.y + offset);
}

static void wintc_ctl_list_view_get_next_icon_pos_for_cell(
    WinTCCtlListView* list_view,
    gint              cell_idx,
    gint*             x,
    gint*             y
)
{
    // Work out which cell this should be in
    //
    gint basis_x;
    gint basis_y;
    gint cell_x;
    gint cell_y;
    gint per_col;
    gint widget_h = gtk_widget_get_allocated_height(GTK_WIDGET(list_view));
    gint widget_w = gtk_widget_get_allocated_width(GTK_WIDGET(list_view));

    if (list_view->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
        per_col = widget_w / CELL_SIZE_LARGE_ICON;

        cell_x = cell_idx % per_col;
        cell_y = cell_idx / per_col;
    }
    else // GTK_ORIENTATION_VERTICAL
    {
        per_col = widget_h / CELL_SIZE_LARGE_ICON;

        cell_x = cell_idx / per_col;
        cell_y = cell_idx % per_col;
    }

    basis_x = cell_x * CELL_SIZE_LARGE_ICON;
    basis_y = cell_y * CELL_SIZE_LARGE_ICON;

    // Add offset to icon
    //
    *x = basis_x + ((CELL_SIZE_LARGE_ICON - HITBOX_LARGE_ICON) / 2);
    *y = basis_y + 2;
}

static GtkTreePath* wintc_ctl_list_view_get_path_for_icon(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon
)
{
    GSequenceIter* iter_seq =
        wintc_sequence_find(list_view->seq_icons, icon);

    gint idx = g_sequence_iter_get_position(iter_seq);

    return gtk_tree_path_new_from_indices(idx, -1);
}

static void wintc_ctl_list_view_get_preferred_size(
    WinTCCtlListView* list_view,
    GtkOrientation    orientation,
    gint              provided_size,
    gint*             minimum_size,
    gint*             natural_size
)
{
    gint required;

    //
    // A bit of clarification for the meaning of 'orientation' as it pertains
    // to the parameter passed into this function vs. the property of the list
    // view:
    //     orientation - horizontal for width, vertical for height
    //     list_view->orientation - horizontal for populating items LTR/RTL
    //                              vertical for populating items TTB/BTT
    //
    // For the auto-arrange logic, this means that the flexible size (the one
    // we need to calculate) is in the direction of **wrapping**, which will
    // always be the opposite to the equivalent dimension
    //
    // eg. for a list view in the vertical orientation, it will fill up TTB
    // then wrap towards the right horizontally (so the height can be shrunk
    // to the size of 1 item, the width varies depending on wrapping)
    //

    if (list_view->auto_arrange)
    {
        if (list_view->orientation != orientation)
        {
            if (provided_size < 0)
            {
                required =
                    g_sequence_get_length(list_view->seq_icons) *
                    CELL_SIZE_LARGE_ICON;
            }
            else
            {
                gint cols = MAX(provided_size / CELL_SIZE_LARGE_ICON, 1);
                gint rows =
                    ceil(
                        (gdouble) 
                        g_sequence_get_length(list_view->seq_icons) / cols
                    );

                required = rows * CELL_SIZE_LARGE_ICON;
            }
        }
        else
        {
            required = CELL_SIZE_LARGE_ICON;
        }
    }
    else
    {
        wintc_ctl_list_view_get_max_icon_pos(
            list_view,
            orientation == GTK_ORIENTATION_HORIZONTAL ? &required : NULL,
            orientation == GTK_ORIENTATION_VERTICAL   ? &required : NULL
        );
    }

    *minimum_size = required;
    *natural_size = required;
}

static gboolean wintc_ctl_list_view_has_solid_bg(
    WinTCCtlListView* list_view
)
{
    cairo_surface_t* fake_surface =
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);

    cairo_t* cr = cairo_create(fake_surface);

    gtk_render_background(
        gtk_widget_get_style_context(GTK_WIDGET(list_view)),
        cr,
        0,
        0,
        1,
        1
    );

    cairo_surface_flush(fake_surface);

    // Sample the pixel
    //
    unsigned char* data =
        cairo_image_surface_get_data(fake_surface);

    gboolean ret = data[3] == 255; // Alpha value

    cairo_destroy(cr);
    cairo_surface_destroy(fake_surface);

    return ret;
}

static WinTCCtlListViewIcon* wintc_ctl_list_view_hit_test(
    WinTCCtlListView* list_view,
    gint              x,
    gint              y,
    gboolean*         hit_label
)
{
    // Crude hit box search
    //
    for (GList* iter = list_view->list_icons; iter; iter = iter->next)
    {
        WinTCCtlListViewIcon* large_icon =
            (WinTCCtlListViewIcon*) iter->data;

        if (
            wintc_point_xy_in_rect(
                x,
                y,
                &(large_icon->hitbox_icon)
            )
        )
        {
            return large_icon;
        }

        if (
            wintc_point_xy_in_rect(
                x,
                y,
                &(large_icon->hitbox_label)
            )
        )
        {
            WINTC_SAFE_REF_SET(hit_label, TRUE);
            return large_icon;
        }
    }

    return NULL;
}

static void wintc_ctl_list_view_insert_icon(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon,
    gint                  idx
)
{
    // Insert into our collections
    //
    GSequenceIter* iter_seq =
        g_sequence_get_iter_at_pos(list_view->seq_icons, idx);

    iter_seq =
        g_sequence_insert_before(iter_seq, icon);

    list_view->list_icons =
        g_list_prepend(list_view->list_icons, icon);

    // Arrange the icon layout
    //
    icon->realize_idx = list_view->itempos_count;

    if (gtk_widget_get_realized(GTK_WIDGET(list_view)))
    {
        if (list_view->auto_arrange)
        {
            wintc_ctl_list_view_auto_arrange(
                list_view,
                iter_seq
            );
        }
        else
        {
            gint target_x;
            gint target_y;

            wintc_ctl_list_view_get_next_icon_pos_for_cell(
                list_view,
                list_view->itempos_count,
                &target_x,
                &target_y
            );

            wintc_ctl_list_view_move_icon(
                list_view,
                icon,
                target_x,
                target_y
            );
        }

        icon->realize_idx = -1;
    }

    list_view->itempos_count++;
}

static void wintc_ctl_list_view_move_icon(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon,
    gint                  x,
    gint                  y
)
{
    gint dx = x - icon->hitbox_icon.x;
    gint dy = y - icon->hitbox_icon.y;

    icon->hitbox_icon.x = x;
    icon->hitbox_icon.y = y;

    icon->hitbox_label.x += dx;
    icon->hitbox_label.y += dy;

    // Track the maximum icon pos
    //
    list_view->itempos_max.x = MAX(list_view->itempos_max.x, x);
    list_view->itempos_max.y = MAX(list_view->itempos_max.y, y);

    // FIXME: Could require allocation adjustment if the icon is move out of
    //        bounds
    //
    gtk_widget_queue_resize(GTK_WIDGET(list_view));
}

static void wintc_ctl_list_view_raise_icon(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon
)
{
    GList* el = g_list_find(list_view->list_icons, icon);

    list_view->list_icons =
        g_list_remove_link(list_view->list_icons, el);
    list_view->list_icons =
        g_list_concat(el, list_view->list_icons);
}

static void wintc_ctl_list_view_render_large_icon(
    WinTCCtlListView*         list_view,
    WinTCCtlListViewIcon*     large_icon,
    cairo_t*                  cr,
    WinTCCtlListViewIconStyle style,
    gint                      offset_x,
    gint                      offset_y
)
{
    gdouble          alpha        = 1.0f;
    cairo_surface_t* surface_icon = NULL;

    if (style == WINTC_CTL_LIST_VIEW_ICON_STYLE_GHOSTED)
    {
        alpha = 0.5f;
    }

    // Icon itself
    //
    gdk_cairo_set_source_pixbuf(
        cr,
        large_icon->icon,
        (gdouble) large_icon->hitbox_icon.x + offset_x,
        (gdouble) large_icon->hitbox_icon.y + offset_y
    );

    cairo_pattern_get_surface(
        cairo_get_source(cr),
        &surface_icon
    );

    cairo_paint_with_alpha(cr, alpha);

    // If the icon is selected, it needs the highlight
    //
    GdkRGBA selected_color;

    gtk_style_context_lookup_color(
        gtk_widget_get_style_context(GTK_WIDGET(list_view)),
        "theme_selected_bg_color",
        &selected_color
    );

    if (style == WINTC_CTL_LIST_VIEW_ICON_STYLE_SELECTED)
    {
        cairo_save(cr);
        cairo_set_source_rgba(
            cr,
            selected_color.red,
            selected_color.green,
            selected_color.blue,
            0.5f
        );
        cairo_mask_surface(
            cr,
            surface_icon,
            (gdouble) large_icon->hitbox_icon.x + offset_x,
            (gdouble) large_icon->hitbox_icon.y + offset_y
        );
        cairo_restore(cr);
    }

    // Icon label
    //
    PangoLayout* layout = wintc_ctl_list_view_create_pango_layout(list_view);

    pango_layout_set_text(layout, large_icon->text, -1);

    if (style == WINTC_CTL_LIST_VIEW_ICON_STYLE_REGULAR)
    {
        // Only render the text shadow if there's no background
        //
        if (!(list_view->solid_bg))
        {
            cairo_save(cr);
            cairo_set_source_rgba(cr, 0.0f, 0.0f, 0.0f, 1.0f);

            for (gint i = 0; i < LABEL_TEXT_SHADOW_INTENSITY; i++)
            {
                // The shadow has a slight drop to it in XP, we render directly
                // behind then slightly off to reproduce an accurate shadow
                //
                gint drop = i / 2;

                cairo_mask_surface(
                    cr,
                    large_icon->surface_text_shadow,
                    (gdouble) large_icon->hitbox_label.x +
                    offset_x - LABEL_TEXT_SHADOW_OFFSET -
                    large_icon->offset_label_render.x + drop,
                    (gdouble) large_icon->hitbox_label.y +
                    offset_y - LABEL_TEXT_SHADOW_OFFSET -
                    large_icon->offset_label_render.y + drop
                );
            }

            cairo_restore(cr);
        }
    }
    else
    {
        cairo_save(cr);
        cairo_rectangle(
            cr,
            (gdouble) large_icon->hitbox_label.x + offset_x,
            (gdouble) large_icon->hitbox_label.y + offset_y,
            (gdouble) large_icon->hitbox_label.width,
            (gdouble) large_icon->hitbox_label.height
        );
        cairo_clip(cr);
        cairo_set_source_rgba(
            cr,
            selected_color.red,
            selected_color.green,
            selected_color.blue,
            alpha
        );
        cairo_paint(cr);

        cairo_set_source_rgba(cr, 1.0f, 1.0f, 1.0f, alpha);
        cairo_stroke(cr);
        cairo_restore(cr);
    }

    // Use black text if there's a solid background, otherwise white
    //
    if (
        list_view->solid_bg &&
        style != WINTC_CTL_LIST_VIEW_ICON_STYLE_SELECTED
    )
    {
        cairo_set_source_rgb(cr, 0.0f, 0.0f, 0.0f);
    }
    else
    {
        cairo_set_source_rgb(cr, 1.0f, 1.0f, 1.0f);
    }

    cairo_move_to(
        cr,
        large_icon->hitbox_label.x +
        offset_x - large_icon->offset_label_render.x,
        large_icon->hitbox_label.y +
        offset_y - large_icon->offset_label_render.y
    );

    pango_cairo_show_layout(cr, layout);

    g_object_unref(layout);
}

static void wintc_ctl_list_view_reset_hit_state(
    WinTCCtlListView* list_view
)
{
    memset(&(list_view->motion_rect), 0, sizeof(GdkRectangle));

    list_view->hit_icon        = NULL;
    list_view->hit_started     = FALSE;
    list_view->hit_dragging    = FALSE;
    list_view->dnd_icon_target = NULL;
}

static void wintc_ctl_list_view_set_icon_pixbuf(
    WINTC_UNUSED(WinTCCtlListView* list_view),
    WinTCCtlListViewIcon* icon,
    GdkPixbuf*            pixbuf
)
{
    if (icon->icon)
    {
        g_clear_object(&(icon->icon));
    }

    if (!pixbuf)
    {
        return;
    }

    icon->icon = pixbuf; // Ref should've already been taken from model_get
}

static void wintc_ctl_list_view_set_icon_text(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon,
    gchar*                text
)
{
    g_free(icon->text);
    icon->text = text; // Ref should've already been taken from model_get

    wintc_ctl_list_view_update_icon_text_shadow(list_view, icon);
}

static void wintc_ctl_list_view_update_dnd_state(
    WinTCCtlListView* list_view
)
{
    gtk_drag_dest_unset(GTK_WIDGET(list_view));

    if (
        list_view->dnd_src_targets ||
        list_view->dnd_dest_targets
    )
    {
        gtk_drag_dest_set(
            GTK_WIDGET(list_view),
            0,
            NULL,
            0,
            list_view->dnd_dest_actions ?
                list_view->dnd_dest_actions :
                GDK_ACTION_COPY
        );

        if (list_view->dnd_dest_targets)
        {
            gtk_drag_dest_set_target_list(
                GTK_WIDGET(list_view),
                list_view->dnd_dest_targets
            );
        }
    }
}

static void wintc_ctl_list_view_update_icon(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon,
    GtkTreeIter*          iter
)
{
    if (list_view->col_pixbuf > -1)
    {
        GdkPixbuf* pixbuf = NULL;

        gtk_tree_model_get(
            list_view->model,
            iter,
            list_view->col_pixbuf, &pixbuf,
            -1
        );

        wintc_ctl_list_view_set_icon_pixbuf(
            list_view,
            icon,
            pixbuf
        );
    }

    if (list_view->col_text > -1)
    {
        gchar* text = NULL;

        gtk_tree_model_get(
            list_view->model,
            iter,
            list_view->col_text, &text,
            -1
        );

        wintc_ctl_list_view_set_icon_text(
            list_view,
            icon,
            text
        );
    }
}

static void wintc_ctl_list_view_update_icon_text_shadow(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon
)
{
    if (icon->surface_text_shadow)
    {
        cairo_surface_destroy(icon->surface_text_shadow);
    }

    if (!(icon->text))
    {
        return;
    }

    // Retrieve the extents for the text used for the shadow (ink extents) and
    // hitbox (logical extents)
    //
    PangoRectangle extents;
    PangoLayout*   layout = wintc_ctl_list_view_create_pango_layout(list_view);

    pango_layout_set_text(layout, icon->text, -1);

    pango_layout_get_pixel_extents(
        layout,
        &extents,
        (PangoRectangle*) &(icon->hitbox_label)
    );

    icon->hitbox_label.x =
        icon->hitbox_icon.x +
        (icon->hitbox_icon.width / 2) - (icon->hitbox_label.width / 2);
    icon->hitbox_label.y =
        icon->hitbox_icon.y + 35;

    icon->offset_label_render.x = extents.x;
//    icon->offset_label_render.y = extents.y;

    // Render to a backing image surface
    //
    cairo_t* cr;
    gint     surface_height;
    gint     surface_stride;
    gint     surface_width;

    icon->surface_text_shadow =
        cairo_image_surface_create(
            CAIRO_FORMAT_ARGB32,
            extents.x + extents.width  + (LABEL_TEXT_SHADOW_RADIUS * 4),
            extents.y + extents.height + (LABEL_TEXT_SHADOW_RADIUS * 4)
        );

    surface_height = cairo_image_surface_get_height(icon->surface_text_shadow);
    surface_stride = cairo_image_surface_get_stride(icon->surface_text_shadow);
    surface_width  = cairo_image_surface_get_width(icon->surface_text_shadow);

    cr = cairo_create(icon->surface_text_shadow);

    cairo_move_to(cr, LABEL_TEXT_SHADOW_RADIUS * 2, LABEL_TEXT_SHADOW_RADIUS * 2);
    cairo_set_source_rgba(cr, 0.0f, 0.0f, 0.0f, 1.0f);

    pango_cairo_show_layout(cr, layout);

    cairo_surface_flush(icon->surface_text_shadow);

    // Perform the blur
    //
    unsigned char* buf = cairo_image_surface_get_data(icon->surface_text_shadow);
    unsigned char* tmp = g_malloc(surface_stride * surface_height);

    for (gint y = 0; y < surface_height; y++) // Horz pass
    {
        unsigned char* src = buf + (y * surface_stride);
        unsigned char* dst = tmp + (y * surface_stride);

        for (gint x = 0; x < surface_width; x++)
        {
            gint len = 0;
            gint sum = 0;

            for (
                gint offset  = -LABEL_TEXT_SHADOW_RADIUS;
                offset      <=  LABEL_TEXT_SHADOW_RADIUS;
                offset++
            )
            {
                gint dx = x + offset;

                if (dx >= 0 && dx < surface_width)
                {
                    sum += src[dx * 4 + 3];
                    len++;
                }
            }

            dst[x * 4 + 3] = sum / len;
        }
    }

    for (gint x = 0; x < surface_width; x++) // Vert pass
    {
        for (gint y = 0; y < surface_height; y++)
        {
            gint len = 0;
            gint sum = 0;

            for (
                gint offset = -LABEL_TEXT_SHADOW_RADIUS;
                offset      <  LABEL_TEXT_SHADOW_RADIUS;
                offset++
            )
            {
                gint dy = y + offset;

                if (dy >= 0 && dy < surface_height)
                {
                    sum += tmp[(dy * surface_stride) + (x * 4) + 3];
                    len++;
                }
            }

            buf[(y * surface_stride) + (x * 4) + 3] = sum / len;
        }
    }

    g_free(tmp);
    cairo_destroy(cr);
    g_object_unref(layout);

    cairo_surface_mark_dirty(icon->surface_text_shadow);
}

static void wintc_ctl_list_view_icon_free(
    WinTCCtlListViewIcon* icon
)
{
    g_object_unref(icon->icon);
    g_free(icon->text);

    if (icon->surface_text_shadow)
    {
        cairo_surface_destroy(icon->surface_text_shadow);
    }

    g_free(icon);
}

//
// CALLBACKS
//
static void on_gtk_settings_font_name_notify(
    WINTC_UNUSED(GObject* self),
    WINTC_UNUSED(GParamSpec* pspec),
    gpointer user_data
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(user_data);

    list_view->dirty_labels = TRUE;
}

static gboolean on_list_view_button_press_event(
    GtkWidget* widget,
    GdkEvent*  event,
    WINTC_UNUSED(gpointer user_data)
)
{
    GdkEventButton*   e         = (GdkEventButton*) event;
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);

    if (e->button != GDK_BUTTON_PRIMARY)
    {
        return FALSE;
    }

    list_view->hit_started = TRUE;

    list_view->motion_rect.x = e->x;
    list_view->motion_rect.y = e->y;

    // Hit check
    //
    list_view->hit_icon =
        wintc_ctl_list_view_hit_test(
            list_view,
            e->x,
            e->y,
            NULL
        );

    // Check for potential double-click
    //
    if (
        e->type == GDK_2BUTTON_PRESS &&
        list_view->hit_icon &&
        list_view->hit_icon == list_view->hit_icon_last
    )
    {
        wintc_ctl_list_view_emit_item_activated(
            list_view,
            list_view->hit_icon
        );

        wintc_ctl_list_view_reset_hit_state(list_view);

        return TRUE;
    }

    // Update the selection state for whether we hit the icon or not
    //
    if (list_view->hit_icon)
    {
        if (
            !list_view->list_selected ||
            !g_list_find(
                list_view->list_selected,
                list_view->hit_icon
            )
        )
        {
            g_clear_list(&(list_view->list_selected), NULL);

            list_view->list_selected =
                g_list_append(
                    list_view->list_selected,
                    list_view->hit_icon
                );
        }
    }
    else
    {
        g_clear_list(&(list_view->list_selected), NULL);
    }

    return TRUE;
}

static gboolean on_list_view_button_release_event(
    GtkWidget* widget,
    WINTC_UNUSED(GdkEvent*  event),
    WINTC_UNUSED(gpointer   user_data)
)
{
    GdkEventButton*   e         = (GdkEventButton*) event;
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);

    if (e->button != GDK_BUTTON_PRIMARY)
    {
        return FALSE;
    }

    // Raise z-order for selected items
    //
    if (list_view->hit_icon)
    {
        for (GList* iter = list_view->list_selected; iter; iter = iter->next)
        {
            wintc_ctl_list_view_raise_icon(
                list_view,
                (WinTCCtlListViewIcon*) iter->data
            );
        }
    }

    // Commit a drag if there is one
    //
    if (list_view->hit_dragging)
    {
        wintc_ctl_list_view_commit_icon_drag(list_view);
    }
    else
    {
        // Didn't drag? Store hit icon for a potential double-click
        //
        list_view->hit_icon_last = list_view->hit_icon;
    }

    wintc_ctl_list_view_reset_hit_state(list_view);

    gtk_widget_queue_draw(widget);

    return TRUE;
}

static void on_list_view_drag_end(
    GtkWidget* widget,
    WINTC_UNUSED(GdkDragContext* context),
    WINTC_UNUSED(gpointer user_data)
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);

    WINTC_LOG_DEBUG("Ended drag");

    // If we were dragging icons then commit the drag now
    //
    if (list_view->dnd_ctx)
    {
        for (GList* iter = list_view->list_selected; iter; iter = iter->next)
        {
            wintc_ctl_list_view_raise_icon(
                list_view,
                (WinTCCtlListViewIcon*) iter->data
            );
        }

        wintc_ctl_list_view_commit_icon_drag(list_view);
        wintc_ctl_list_view_reset_hit_state(list_view);
    }

    list_view->dnd_ctx = NULL;

    gtk_widget_queue_draw(widget);
}

static gboolean on_list_view_drag_motion(
    GtkWidget* widget,
    WINTC_UNUSED(GdkDragContext* context),
    int        x,
    int        y,
    WINTC_UNUSED(guint time),
    WINTC_UNUSED(gpointer user_data)
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);

    // If it's our drag, update the dragging position
    //
    if (list_view->dnd_ctx)
    {
        list_view->motion_rect.width =
            x - list_view->motion_rect.x;
        list_view->motion_rect.height =
            y - list_view->motion_rect.y;
    }

    list_view->dnd_icon_target =
        wintc_ctl_list_view_hit_test(
            list_view,
            x,
            y,
            NULL
        );

    gtk_widget_queue_draw(widget);

    return FALSE;
}

static gboolean on_list_view_enter_notify_event(
    GtkWidget* widget,
    WINTC_UNUSED(GdkEvent*  event),
    WINTC_UNUSED(gpointer   user_data)
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);

    list_view->hit_in_widget = TRUE;

    return FALSE;
}

static gboolean on_list_view_key_press_event(
    GtkWidget* widget,
    GdkEvent*  event,
    WINTC_UNUSED(gpointer user_data)
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);
    GdkEventKey*      e         = (GdkEventKey*) event;

    if (e->keyval == GDK_KEY_Return)
    {
        if (!(list_view->list_selected))
        {
            return FALSE;
        }

        // Fire item-activated for each selected item
        //
        for (GList* iter = list_view->list_selected; iter; iter = iter->next)
        {
            wintc_ctl_list_view_emit_item_activated(
                list_view,
                (WinTCCtlListViewIcon*) iter->data
            );
        }
    }

    return FALSE;
}

static gboolean on_list_view_leave_notify_event(
    GtkWidget* widget,
    WINTC_UNUSED(GdkEvent*  event),
    WINTC_UNUSED(gpointer   user_data)
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);

    list_view->hit_in_widget = FALSE;

    return FALSE;
}

static gboolean on_list_view_motion_notify_event(
    GtkWidget* widget,
    GdkEvent*  event,
    WINTC_UNUSED(gpointer user_data)
)
{
    GdkEventMotion*   e         = (GdkEventMotion*) event;
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);

    // Do not update any position information if we're not in bounds
    //
    if (!(list_view->hit_in_widget))
    {
        return FALSE;
    }

    list_view->motion_rect.width =
        e->x - list_view->motion_rect.x;
    list_view->motion_rect.height =
        e->y - list_view->motion_rect.y;

    // If no icon hit-tested, then this is a selection box dragging op
    //
    if (list_view->hit_icon)
    {
        if (
            !(list_view->hit_dragging) &&
            (
                abs(list_view->motion_rect.width)  > DRAG_THRESHOLD ||
                abs(list_view->motion_rect.height) > DRAG_THRESHOLD
            )
        )
        {
            list_view->hit_dragging = TRUE;

            if (list_view->dnd_src_targets)
            {
                WINTC_LOG_DEBUG("Initiate drag");

                list_view->dnd_ctx =
                    gtk_drag_begin_with_coordinates(
                        widget,
                        list_view->dnd_src_targets,
                        list_view->dnd_src_actions,
                        GDK_BUTTON1_MASK,
                        event,
                        -1,
                        -1
                    );
            }
        }
    }
    else
    {
        g_clear_list(&(list_view->list_selected), NULL);

        for (GList* iter = list_view->list_icons; iter; iter = iter->next)
        {
            WinTCCtlListViewIcon* large_icon =
                (WinTCCtlListViewIcon*) iter->data;

            GdkRectangle rect = list_view->motion_rect;

            wintc_rectangle_normalize(&rect);

            if (
                gdk_rectangle_intersect(
                    &rect,
                    &(large_icon->hitbox_icon),
                    NULL
                ) ||
                gdk_rectangle_intersect(
                    &rect,
                    &(large_icon->hitbox_label),
                    NULL
                )
            )
            {
                list_view->list_selected =
                    g_list_prepend(
                        list_view->list_selected,
                        large_icon
                    );
            }
        }

        list_view->list_selected =
            g_list_reverse(list_view->list_selected);
    }

    gtk_widget_queue_draw(widget);

    return TRUE;
}

static void on_list_view_realize(
    GtkWidget* widget,
    WINTC_UNUSED(gpointer user_data)
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);

    list_view->solid_bg =
        wintc_ctl_list_view_has_solid_bg(list_view);

    //
    // FIXME: Shouldn't this just reload the layout?
    //

    // Iterate over icons and check for any that need their position realized
    //
    for (GList* iter = list_view->list_icons; iter; iter = iter->next)
    {
        WinTCCtlListViewIcon* large_icon =
            (WinTCCtlListViewIcon*) iter->data;

        if (large_icon->realize_idx < 0)
        {
            continue;
        }

        // We must position this icon
        //
        gint new_x;
        gint new_y;

        wintc_ctl_list_view_get_next_icon_pos_for_cell(
            list_view,
            large_icon->realize_idx,
            &new_x,
            &new_y
        );

        wintc_ctl_list_view_move_icon(
            list_view,
            large_icon,
            new_x,
            new_y
        );

        large_icon->realize_idx = -1;
    }
}

static void on_model_row_changed(
    WINTC_UNUSED(GtkTreeModel* tree_model),
    GtkTreePath* path,
    GtkTreeIter* iter,
    gpointer     user_data
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(user_data);

    if (gtk_tree_path_get_depth(path) > 1)
    {
        return;
    }

    gint idx = gtk_tree_path_get_indices(path)[0];

    GSequenceIter* iter_seq =
        g_sequence_get_iter_at_pos(
            list_view->seq_icons,
            idx
        );

    WinTCCtlListViewIcon* large_icon = g_sequence_get(iter_seq);

    wintc_ctl_list_view_update_icon(list_view, large_icon, iter);

    gtk_widget_queue_draw(GTK_WIDGET(list_view));
}

static void on_model_row_deleted(
    WINTC_UNUSED(GtkTreeModel* tree_model),
    GtkTreePath* path,
    gpointer     user_data
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(user_data);

    if (gtk_tree_path_get_depth(path) > 1)
    {
        return;
    }

    gint idx = gtk_tree_path_get_indices(path)[0];

    GSequenceIter* iter =
        g_sequence_get_iter_at_pos(
            list_view->seq_icons,
            idx
        );

    WinTCCtlListViewIcon* large_icon = g_sequence_get(iter);

    g_sequence_remove(iter);

    list_view->list_icons =
        g_list_delete_link(
            list_view->list_icons,
            g_list_find(
                list_view->list_icons,
                large_icon
            )
        );

    GList* el_selected = g_list_find(list_view->list_selected, large_icon);

    if (el_selected)
    {
        list_view->list_selected =
            g_list_delete_link(list_view->list_selected, el_selected);
    }

    wintc_ctl_list_view_icon_free(large_icon);

    if (list_view->auto_arrange)
    {
        wintc_ctl_list_view_auto_arrange(
            list_view,
            g_sequence_get_iter_at_pos(
                list_view->seq_icons,
                idx
            )
        );
    }

    gtk_widget_queue_resize(GTK_WIDGET(list_view));
}

static void on_model_row_inserted(
    WINTC_UNUSED(GtkTreeModel* tree_model),
    GtkTreePath*  path,
    WINTC_UNUSED(GtkTreeIter* iter),
    gpointer      user_data
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(user_data);

    if (gtk_tree_path_get_depth(path) > 1)
    {
        return;
    }

    // Create the new icon
    //
    WinTCCtlListViewIcon* large_icon =
        wintc_ctl_list_view_create_large_icon(list_view);

    wintc_ctl_list_view_insert_icon(
        list_view,
        large_icon,
        gtk_tree_path_get_indices(path)[0]
    );

    gtk_widget_queue_draw(GTK_WIDGET(list_view));
}

static void on_model_rows_reordered(
    WINTC_UNUSED(GtkTreeModel* tree_model),
    GtkTreePath* path,
    GtkTreeIter* iter,
    gpointer     new_order,
    gpointer     user_data
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(user_data);

    if (gtk_tree_path_get_depth(path) > 0)
    {
        return;
    }

    // Create a new GSequence to replace the existing one
    //
    gint       len = gtk_tree_model_iter_n_children(
                         list_view->model,
                         iter
                     );
    GSequence* seq = g_sequence_new(NULL);

    for (gint i = 0; i < len; i++)
    {
        gint old_idx = ((gint*) new_order)[i];

        GSequenceIter* iter = g_sequence_get_iter_at_pos(list_view->seq_icons, old_idx);

        WinTCCtlListViewIcon* large_icon = g_sequence_get(iter);

        g_sequence_append(seq, large_icon);
    }

    g_sequence_free(list_view->seq_icons);
    list_view->seq_icons = seq;
}

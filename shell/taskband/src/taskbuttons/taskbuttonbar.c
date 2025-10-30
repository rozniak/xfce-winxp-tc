#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../taskband.h"
#include "taskbuttonbar.h"
#include "windowmonitor.h"

#define TASKBUTTON_MAX_WIDTH        160
#define TASKBUTTON_MIN_WIDTH        52
#define TASKBUTTON_BAR_UPDOWN_WIDTH 17

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _TaskButtonBarClass
{
    GtkContainerClass __parent__;
};

struct _TaskButtonBar
{
    GtkContainer __parent__;

    GSList*        buttons;
    gboolean       flip_bias;
    WindowMonitor* window_monitor;
};

//
// FORWARD DECLARATIONS
//
static void taskbutton_bar_finalize(
    GObject* object
);

static void taskbutton_bar_get_preferred_height(
    GtkWidget* widget,
    gint*      minimum_height,
    gint*      natural_height
);
static void taskbutton_bar_get_preferred_height_for_width(
    GtkWidget* widget,
    gint       width,
    gint*      minimum_height,
    gint*      natural_height
);
static void taskbutton_bar_get_preferred_width(
    GtkWidget* widget,
    gint*      minimum_width,
    gint*      natural_width
);
static void taskbutton_bar_get_preferred_width_for_height(
    GtkWidget* widget,
    gint       height,
    gint*      minimum_width,
    gint*      natural_width
);
static void taskbutton_bar_size_allocate(
    GtkWidget*     widget,
    GtkAllocation* allocation
);

static void taskbutton_bar_add(
    GtkContainer* container,
    GtkWidget*    widget
);
static GType taskbutton_bar_child_type(
    GtkContainer* container
);
static void taskbutton_bar_forall(
    GtkContainer* container,
    gboolean      include_internals,
    GtkCallback   callback,
    gpointer      callback_data
);
static void taskbutton_bar_remove(
    GtkContainer* container,
    GtkWidget*    widget
);

static gboolean taskbutton_bar_has_button(
    TaskButtonBar*   taskbutton_bar,
    GtkToggleButton* button
);

static gboolean cb_timeout_taskband_flip_bias(
    gpointer user_data
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    TaskButtonBar,
    taskbutton_bar,
    GTK_TYPE_CONTAINER
)

static void taskbutton_bar_class_init(
    TaskButtonBarClass* klass
)
{
    GtkContainerClass* container_class = GTK_CONTAINER_CLASS(klass);
    GtkWidgetClass*    widget_class    = GTK_WIDGET_CLASS(klass);
    GObjectClass*      object_class    = G_OBJECT_CLASS(klass);

    object_class->finalize = taskbutton_bar_finalize;

    widget_class->get_preferred_height = taskbutton_bar_get_preferred_height;
    widget_class->get_preferred_height_for_width =
        taskbutton_bar_get_preferred_height_for_width;
    widget_class->get_preferred_width  = taskbutton_bar_get_preferred_width;
    widget_class->get_preferred_width_for_height =
        taskbutton_bar_get_preferred_width_for_height;
    widget_class->size_allocate        = taskbutton_bar_size_allocate;

    container_class->add        = taskbutton_bar_add;
    container_class->child_type = taskbutton_bar_child_type;
    container_class->forall     = taskbutton_bar_forall;
    container_class->remove     = taskbutton_bar_remove;
}

static void taskbutton_bar_init(
    TaskButtonBar* self
)
{
    self->window_monitor =
        window_monitor_init_management(GTK_CONTAINER(self));

    gtk_widget_set_has_window(GTK_WIDGET(self), FALSE);
    gtk_widget_set_hexpand(GTK_WIDGET(self), TRUE);

    wintc_widget_add_style_class(GTK_WIDGET(self), "wintc-taskbuttons");

    // HACK: A workaround when launching - initial windows populated by WNCK
    //       are in the reverse order to how they were opened (so the oldest
    //       window will appear last instead of first)
    //
    //       Set the 'bias' initially to prepend the window buttons and then
    //       after a time out, flip the bias to normal to append newer buttons
    //
    //       This is very crude but not sure of any alternative solutions...
    //
    self->flip_bias = TRUE;

    g_timeout_add(
        500,
        (GSourceFunc) cb_timeout_taskband_flip_bias,
        self
    );
}

//
// CLASS VIRTUAL METHODS
//
static void taskbutton_bar_finalize(
    GObject* object
)
{
    TaskButtonBar* taskbutton_bar = TASKBUTTON_BAR(object);

    window_monitor_destroy(
        taskbutton_bar->window_monitor
    );

    (G_OBJECT_CLASS(taskbutton_bar_parent_class))->finalize(object);
}

static void taskbutton_bar_add(
    GtkContainer* container,
    GtkWidget*    widget
)
{
    TaskButtonBar* taskbutton_bar = TASKBUTTON_BAR(container);

    if (
        taskbutton_bar_has_button(
            taskbutton_bar,
            GTK_TOGGLE_BUTTON(widget)
        )
    )
    {
        return;
    }

    gtk_widget_set_parent(widget, GTK_WIDGET(container));

    if (taskbutton_bar->flip_bias)
    {
        taskbutton_bar->buttons =
            g_slist_prepend(taskbutton_bar->buttons, widget);
    }
    else
    {
        taskbutton_bar->buttons =
            g_slist_append(taskbutton_bar->buttons, widget);
    }

    gtk_widget_queue_resize(GTK_WIDGET(container));
}

static GType taskbutton_bar_child_type(
    WINTC_UNUSED(GtkContainer* container)
)
{
    return GTK_TYPE_TOGGLE_BUTTON;
}

static void taskbutton_bar_forall(
    GtkContainer* container,
    WINTC_UNUSED(gboolean include_internals),
    GtkCallback   callback,
    gpointer      callback_data
)
{
    TaskButtonBar* taskbutton_bar = TASKBUTTON_BAR(container);

    g_slist_foreach(
        taskbutton_bar->buttons,
        (GFunc) callback,
        callback_data
    );
}

static void taskbutton_bar_remove(
    GtkContainer* container,
    GtkWidget*    widget
)
{
    TaskButtonBar* taskbutton_bar = TASKBUTTON_BAR(container);
    GSList*        to_remove;

    to_remove = g_slist_find(taskbutton_bar->buttons, widget);

    if (to_remove == NULL)
    {
        return;
    }

    gtk_widget_unparent(GTK_WIDGET(to_remove->data));

    taskbutton_bar->buttons =
        g_slist_delete_link(taskbutton_bar->buttons, to_remove);

    gtk_widget_queue_resize(GTK_WIDGET(container));
}

static void taskbutton_bar_get_preferred_height(
    WINTC_UNUSED(GtkWidget* widget),
    gint* minimum_height,
    gint* natural_height
)
{
    *minimum_height = TASKBAND_ROW_HEIGHT;
    *natural_height = TASKBAND_ROW_HEIGHT;
}

static void taskbutton_bar_get_preferred_height_for_width(
    WINTC_UNUSED(GtkWidget* widget),
    WINTC_UNUSED(gint       width),
    gint* minimum_height,
    gint* natural_height
)
{
    *minimum_height = TASKBAND_ROW_HEIGHT;
    *natural_height = TASKBAND_ROW_HEIGHT;
}

static void taskbutton_bar_get_preferred_width(
    GtkWidget* widget,
    gint*      minimum_width,
    gint*      natural_width
)
{
    gint             accum_min_width = 0;
    gint             accum_nat_width = 0;
    GtkWidget*       button;
    GtkBorder        button_margins;
    GtkStyleContext* button_style;
    GSList*          child;
    TaskButtonBar*   taskbutton_bar = TASKBUTTON_BAR(widget);

    child = taskbutton_bar->buttons;

    while (child)
    {
        button         = GTK_WIDGET(child->data);
        button_style   = gtk_widget_get_style_context(button);
        
        gtk_style_context_get_margin(
            button_style,
            GTK_STATE_FLAG_NORMAL,
            &button_margins
        );

        accum_nat_width += TASKBUTTON_MAX_WIDTH +
                           button_margins.left  +
                           button_margins.right;

        // Set min width only for one child
        //
        if (accum_min_width == 0)
        {
            accum_min_width = TASKBUTTON_MIN_WIDTH +
                              button_margins.left  +
                              button_margins.right;
        }

        child = child->next;
    }

    accum_nat_width += TASKBUTTON_BAR_UPDOWN_WIDTH;

    *minimum_width = accum_min_width;
    *natural_width = accum_nat_width;
}

static void taskbutton_bar_get_preferred_width_for_height(
    GtkWidget* widget,
    WINTC_UNUSED(gint height),
    gint*      minimum_width,
    gint*      natural_width
)
{
    taskbutton_bar_get_preferred_width(
        widget,
        minimum_width,
        natural_width
    );
}

static void taskbutton_bar_size_allocate(
    GtkWidget*     widget,
    GtkAllocation* allocation
)
{
    gint             accum_margins     = 0;
    gint             accum_width       = 0;
    gint             alloc_per_child   = 0;
    GtkWidget*       button;
    GtkBorder        button_margins;
    GtkStyleContext* button_style;
    GSList*          child;
    GtkAllocation    child_alloc;
    guint            i;
    guint            n_buttons         = 0;
    guint            n_buttons_can_fit = 0;
    TaskButtonBar*   taskbutton_bar    = TASKBUTTON_BAR(widget);

    n_buttons = g_slist_length(taskbutton_bar->buttons);

    gtk_widget_set_allocation(widget, allocation);

    // Phase 1 - calculate how much space would be taken up if all the buttons
    //           are the minimum size
    //
    child = taskbutton_bar->buttons;

    while (child)
    {
        button = GTK_WIDGET(child->data);
        button_style = gtk_widget_get_style_context(button);

        gtk_style_context_get_margin(
            button_style,
            GTK_STATE_FLAG_NORMAL,
            &button_margins
        );

        accum_margins += button_margins.left + button_margins.right;
        accum_width   += TASKBUTTON_MIN_WIDTH +
                         button_margins.left  +
                         button_margins.right;

        if (accum_width > allocation->width)
        {
            break;
        }

        n_buttons_can_fit++;
        child = child->next;
    }

    // Phase 2 - based on the number of widgets that can fit, share the free
    //           space in the container
    //
    if (n_buttons_can_fit > 0)
    {
        alloc_per_child =
            MIN(
                TASKBUTTON_MAX_WIDTH,
                (allocation->width - accum_margins) / n_buttons_can_fit
            );
    }

    child = taskbutton_bar->buttons;

    child_alloc.x      = allocation->x;
    child_alloc.y      = 0;
    child_alloc.width  = 0;
    child_alloc.height = TASKBAND_ROW_HEIGHT;

    for (i = 0; i < n_buttons_can_fit; i++)
    {
        button         = GTK_WIDGET(child->data);
        button_style   = gtk_widget_get_style_context(button);

        gtk_style_context_get_margin(
            button_style,
            GTK_STATE_FLAG_NORMAL,
            &button_margins
        );

        child_alloc.width = alloc_per_child     +
                            button_margins.left +
                            button_margins.right;

        gtk_widget_size_allocate(button, &child_alloc);
        gtk_widget_set_visible(button, TRUE);

        child_alloc.x += child_alloc.width;

        child = child->next;
    }

    for (i = n_buttons_can_fit; i < n_buttons; i++)
    {
        button = GTK_WIDGET(child->data);

        gtk_widget_set_visible(button, FALSE);

        child = child->next;
    }
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* taskbutton_bar_new(void)
{
    return GTK_WIDGET(
        g_object_new(
            TYPE_TASKBUTTON_BAR,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static gboolean taskbutton_bar_has_button(
    TaskButtonBar*   taskbutton_bar,
    GtkToggleButton* button
)
{
    return g_slist_find(taskbutton_bar->buttons, button) != NULL;
}

//
// CALLBACKS
//
static gboolean cb_timeout_taskband_flip_bias(
    gpointer user_data
)
{
    TaskButtonBar* taskbutton_bar = TASKBUTTON_BAR(user_data);

    taskbutton_bar->flip_bias = FALSE;

    return G_SOURCE_REMOVE;
}

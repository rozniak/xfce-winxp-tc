#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "menumod.h"

//
// FORWARD DECLARATIONS
//
static void wintc_menu_modded_size_allocate(
    GtkWidget*     widget,
    GtkAllocation* allocation
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCMenuModdedClass
{
    GtkMenuBarClass __parent__;
};

struct _WinTCMenuModded
{
    GtkMenuBar __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCMenuModded,
    wintc_menu_modded,
    GTK_TYPE_MENU_BAR
)

static void wintc_menu_modded_class_init(
    WinTCMenuModdedClass* klass
)
{
    GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);

    widget_class->size_allocate = wintc_menu_modded_size_allocate;
}

static void wintc_menu_modded_init(
    WINTC_UNUSED(WinTCMenuModded* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_menu_modded_size_allocate(
    GtkWidget*     widget,
    GtkAllocation* allocation
)
{
    // Omega giga ultra hax! There's some GTK gadget fancy pants stuff in the
    // menu bar's allocate function what we don't have access to, so we chain
    // up early for it to handle all that jazz...
    //
    (GTK_WIDGET_CLASS(wintc_menu_modded_parent_class))
     ->size_allocate(widget, allocation);

    // ...and then assuming that it took care of all the CSS box model business
    // for the menu bar itself, we retrieve the allocation that was set, and
    // use that to layout the children a *second* time in the way *we* want
    //
    GtkAllocation menu_allocation;
    GtkAllocation remaining_space;

    gtk_widget_get_allocation(widget, &menu_allocation);
    remaining_space   = menu_allocation;
    remaining_space.x = 0;
    remaining_space.y = 0;

    if (
        gtk_menu_bar_get_pack_direction(GTK_MENU_BAR(widget))
            != GTK_PACK_DIRECTION_TTB
    )
    {
        // I'm only implementing TTB for now since that's all the Start menu
        // needs
        //
        g_critical("%s", "Modded menu class only supports TTB packing!");
        return;
    }

    // Calculate children's size allocations
    //
    GArray*    allocations;
    GtkWidget* child;
    GList*     children    = gtk_container_get_children(GTK_CONTAINER(widget));
    GList*     li;
    gint       n_expanders = 0;
    gint       share_space = 0;

    allocations =
        g_array_new(
            FALSE,
            TRUE,
            sizeof(GtkRequestedSize)
        );

    for (li = children; li; li = li->next)
    {
        GtkRequestedSize request;
        gint             toggle_size;

        child = li->data;

        if (!gtk_widget_get_visible(child))
        {
            continue;
        }

        if (gtk_widget_get_vexpand(child))
        {
            n_expanders++;
        }

        request.data = child;

        gtk_widget_get_preferred_height_for_width(
            child,
            menu_allocation.width,
            &(request.minimum_size),
            &(request.natural_size)
        );
        gtk_menu_item_toggle_size_request(
            GTK_MENU_ITEM(child),
            &toggle_size
        );

        request.minimum_size += toggle_size;
        request.natural_size += toggle_size;

        gtk_menu_item_toggle_size_allocate(
            GTK_MENU_ITEM(child),
            toggle_size
        );

        g_array_append_val(allocations, request);

        remaining_space.height -= request.minimum_size;
    }

    remaining_space.height =
        gtk_distribute_natural_allocation(
            remaining_space.height,
            allocations->len,
            (GtkRequestedSize*) allocations->data
        );

    if (n_expanders > 0)
    {
        share_space = remaining_space.height / n_expanders;
    }

    for (guint i = 0; i < allocations->len; i++)
    {
        GtkAllocation     child_alloc = remaining_space;
        GtkRequestedSize* request     = &g_array_index(
                                            allocations,
                                            GtkRequestedSize,
                                            i
                                        );

        child_alloc.height = request->minimum_size;

        if (gtk_widget_get_vexpand(request->data))
        {
            child_alloc.height += share_space;
        }

        remaining_space.y += child_alloc.height;

        gtk_widget_size_allocate(
            request->data,
            &child_alloc
        );
    }

    g_list_free(children);
    g_array_unref(allocations);
}

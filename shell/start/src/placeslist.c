#include <glib.h>
#include <gtk/gtk.h>
#include <wintc-comgtk.h>
#include <wintc-exec.h>

#include "startmenuitem.h"
#include "placeslist.h"
#include "util.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _PlacesListClass
{
    GtkMenuBarClass __parent__;
};

struct _PlacesList
{
    GtkMenuBar __parent__;
};

//
// FORWARD DECLARATIONS
//
static void places_list_finalize(
    GObject* object
);

static void places_list_append_item(
    PlacesList* places_list,
    WinTCAction action_id,
    gboolean    significant
);

static void places_list_append_separator(
    PlacesList* places_list
);

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(PlacesList, places_list, GTK_TYPE_MENU_BAR)

static void places_list_class_init(
    PlacesListClass* klass
)
{
    GObjectClass* gclass = G_OBJECT_CLASS(klass);

    gclass->finalize = places_list_finalize;
}

static void places_list_init(
    PlacesList* self
)
{
    // Set up structure
    //
    gtk_menu_bar_set_pack_direction(
        GTK_MENU_BAR(self),
        GTK_PACK_DIRECTION_TTB
    );

    places_list_append_item(
        self,
        WINTC_ACTION_MYDOCS,
        TRUE
    );
    places_list_append_item(
        self,
        WINTC_ACTION_MYRECENTS,
        TRUE
    );
    places_list_append_item(
        self,
        WINTC_ACTION_MYPICS,
        TRUE
    );
    places_list_append_item(
        self,
        WINTC_ACTION_MYMUSIC,
        TRUE
    );
    places_list_append_item(
        self,
        WINTC_ACTION_MYCOMP,
        TRUE
    );

    places_list_append_separator(self);

    places_list_append_item(
        self,
        WINTC_ACTION_CONTROL,
        FALSE
    );
    places_list_append_item(
        self,
        WINTC_ACTION_MIMEMGMT,
        FALSE
    );
    places_list_append_item(
        self,
        WINTC_ACTION_CONNECTTO,
        FALSE
    );
    places_list_append_item(
        self,
        WINTC_ACTION_PRINTERS,
        FALSE
    );

    places_list_append_separator(self);

    places_list_append_item(
        self,
        WINTC_ACTION_HELP,
        FALSE
    );
    places_list_append_item(
        self,
        WINTC_ACTION_SEARCH,
        FALSE
    );
    places_list_append_item(
        self,
        WINTC_ACTION_RUN,
        FALSE
    );
}

//
// FINALIZE
//
static void places_list_finalize(
    GObject* object
)
{
    (*G_OBJECT_CLASS(places_list_parent_class)->finalize) (object);
}

//
// PRIVATE FUNCTIONS
//
static void places_list_append_item(
    PlacesList* places_list,
    WinTCAction action_id,
    gboolean    significant
)
{
    GtkWidget* item = start_menu_item_new_from_action(action_id);

    // If the item is significant, add the style
    //
    if (significant)
    {
        wintc_widget_add_style_class(item, "significant");
    }

    // FIXME: Shift 24 size to constant
    //
    start_menu_item_set_icon_size(START_MENU_ITEM(item), 24);

    gtk_menu_shell_append(GTK_MENU_SHELL(places_list), item);
}

static void places_list_append_separator(
    PlacesList* places_list
)
{
    GtkWidget* separator = gtk_separator_menu_item_new();

    gtk_menu_shell_append(GTK_MENU_SHELL(places_list), separator);
}

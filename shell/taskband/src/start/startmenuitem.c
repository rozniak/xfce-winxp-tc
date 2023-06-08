#include <garcon/garcon.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <wintc-comgtk.h>
#include <wintc-exec.h>
#include <wintc-shllang.h>

#include "../meta.h"
#include "action.h"
#include "startmenuitem.h"
#include "util.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _StartMenuItemPrivate
{
    StartMenuItem*  menuitem;

    WinTCAction     action;
    gchar*          cmdline;
    gboolean        is_action;

    GtkWidget*      icon;
    GtkWidget*      label;
    GtkWidget*      label_generic;
};

struct _StartMenuItemClass
{
    GtkMenuItemClass __parent__;
};

struct _StartMenuItem
{
    GtkMenuItem __parent__;

    StartMenuItemPrivate* priv;
};

//
// FORWARD DECLARATIONS
//
static void start_menu_item_finalize(
    GObject* object
);

static GtkWidget* start_menu_item_new_manual(
    const gchar* icon_name,
    const gchar* program_name,
    const gchar* comment,
    const gchar* generic_name
);

static void on_menu_item_activate(
    GtkWidget* widget,
    gpointer   user_data
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    StartMenuItem,
    start_menu_item,
    GTK_TYPE_MENU_ITEM,
    G_ADD_PRIVATE(StartMenuItem)
)

static void start_menu_item_class_init(
    StartMenuItemClass* klass
)
{
    GObjectClass* gclass = G_OBJECT_CLASS(klass);

    gclass->finalize = start_menu_item_finalize;
}

static void start_menu_item_init(
    StartMenuItem* self
)
{
    self->priv = start_menu_item_get_instance_private(self);

    g_signal_connect(
        G_OBJECT(self),
        "activate",
        G_CALLBACK(on_menu_item_activate),
        NULL
    );
}

//
// FINALIZE
//
static void start_menu_item_finalize(
    GObject* object
)
{
    StartMenuItem* start_menu_item = START_MENU_ITEM(object);

    if (start_menu_item->priv->cmdline != NULL)
    {
        g_free(start_menu_item->priv->cmdline);
    }

    (*G_OBJECT_CLASS(start_menu_item_parent_class)->finalize) (object);
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* start_menu_item_new_from_action(
    WinTCAction action
)
{
    const gchar* comment;
    const gchar* icon_name;
    const gchar* name;
    GtkWidget*   start_menu_item;

    switch (action)
    {
        case WINTC_ACTION_MYDOCS:
            comment   = _("Opens the My Documents folder, where you can store letters, reports, notes, and other kinds of documents.");
            icon_name = "folder-documents";
            name      = wintc_get_place_name(WINTC_PLACE_DOCUMENTS);
            break;

        case WINTC_ACTION_MYRECENTS:
            comment   = _("Displays recently opened documents and folders.");
            icon_name = "document-open-recent";
            name      = wintc_get_place_name(WINTC_PLACE_RECENTS);
            break;

        case WINTC_ACTION_MYPICS:
            comment   = _("Opens the My Pictures folder, where you can store digital photos, images, and graphics files.");
            icon_name = "folder-pictures";
            name      = wintc_get_place_name(WINTC_PLACE_PICTURES);
            break;

        case WINTC_ACTION_MYMUSIC:
            comment   = _("Opens the My Music folder, where you can store music and other audio files.");
            icon_name = "folder-music";
            name      = wintc_get_place_name(WINTC_PLACE_MUSIC);
            break;

        case WINTC_ACTION_MYCOMP:
            comment   = _("Gives access to, and information about, the disk drives, cameras, scanners, and other hardware connected to your computer.");
            icon_name = "computer";
            name      = wintc_get_place_name(WINTC_PLACE_DRIVES);
            break;

        case WINTC_ACTION_CONTROL:
            comment   = _("Provides options for you to customize the appearance and functionality of your computer, add or remove programs, and set up network connections and user accounts.");
            icon_name = "preferences-other";
            name      = wintc_get_place_name(WINTC_PLACE_CONTROLPANEL);
            break;

        case WINTC_ACTION_MIMEMGMT:
            comment   = _("Chooses default programs for certain activities, such as Web browsing or sending e-mail, and specifies which programs are accessible from the Start menu, desktop, and other locations.");
            icon_name = "preferences-desktop-default-applications";
            name      = _("Set Program Access and Defaults");
            break;

        case WINTC_ACTION_CONNECTTO:
            comment   = _("Connects to other computers, networks, and the Internet.");
            icon_name = "preferences-system-network";
            name      = _("Connect To");
            break;

        case WINTC_ACTION_PRINTERS:
            comment   = _("Shows installed printers and fax printers and helps you add new ones.");
            icon_name = "printer";
            name      = wintc_get_place_name(WINTC_PLACE_PRINTERS);
            break;

        case WINTC_ACTION_HELP:
            comment   = _("Opens a central location for Help topics, tutorials, troubleshooting, and other support services.");
            icon_name = "help-browser";
            name      = _("Help and Support");
            break;

        case WINTC_ACTION_SEARCH:
            comment   = _("Opens a window where you can pick search options and work with search results.");
            icon_name = "system-search";
            name      = _("Search");
            break;

        case WINTC_ACTION_RUN:
            comment   = _("Opens a program, folder, document, or Web site.");
            icon_name = "system-run";
            name      = _("Run...");
            break;

        default:
            comment   = "Unknown action.";
            icon_name = "dialog-error";
            name      = "Unknown";
            break;
    }


    start_menu_item =
        start_menu_item_new_manual(
            icon_name,
            name,
            comment,
            NULL
        );

    // Add action state
    //
    (START_MENU_ITEM(start_menu_item))->priv->action    = action;
    (START_MENU_ITEM(start_menu_item))->priv->is_action = TRUE;

    return start_menu_item;
}

GtkWidget* start_menu_item_new_from_desktop_entry(
    GDesktopAppInfo* entry,
    const gchar*     generic_name,
    const gchar*     comment
)
{
    // FIXME: Temp bodge for handling NULL desktop entry (handle properly in
    // programslist.c)
    //
    //  (We just insert a default item to open the MIME management action)
    //
    if (entry == NULL)
    {
        GtkWidget* null_menu_item =
            start_menu_item_new_manual(
                "important",
                "Click to specify a default",
                comment != NULL ? comment : "No default program could be identified.",
                generic_name
            );

        (START_MENU_ITEM(null_menu_item))->priv->action    = WINTC_ACTION_MIMEMGMT;
        (START_MENU_ITEM(null_menu_item))->priv->is_action = TRUE;

        return null_menu_item;
    }

    // Normal code - desktop entry actually exists! Wahey!
    //
    GAppInfo* app_info = G_APP_INFO(entry);

    const gchar* app_desc = g_app_info_get_description(app_info);
    const gchar* exe_path = g_app_info_get_executable(app_info);
    const gchar* name     = g_app_info_get_name(app_info);

    gchar* exe_name = g_path_get_basename(exe_path);

    GtkWidget* start_menu_item =
        start_menu_item_new_manual(
            exe_name,
            name,
            comment != NULL ? comment : app_desc,
            generic_name
        );

    (START_MENU_ITEM(start_menu_item))->priv->cmdline =
        wintc_desktop_app_info_get_command(entry);

    g_free(exe_name);

    return start_menu_item;
}

GtkWidget* start_menu_item_new_from_garcon_item(
    GarconMenuItem* item,
    const gchar*    generic_name,
    const gchar*    comment
)
{
    GtkWidget* start_menu_item =
        start_menu_item_new_manual(
            garcon_menu_item_get_icon_name(item),
            garcon_menu_item_get_name(item),
            comment != NULL ?
                comment :
                garcon_menu_item_get_comment(item),
            generic_name
        );

    (START_MENU_ITEM(start_menu_item))->priv->cmdline =
        garcon_menu_item_get_command_expanded(item);

    return start_menu_item;
}

void start_menu_item_set_icon_size(
    StartMenuItem* item,
    gint           size
)
{
    g_assert(item->priv->icon != NULL);

    gtk_image_set_pixel_size(
        GTK_IMAGE(item->priv->icon),
        size
    );
}

//
// PRIVATE FUNCTIONS
//
static GtkWidget* start_menu_item_new_manual(
    const gchar* icon_name,
    const gchar* program_name,
    const gchar* comment,
    const gchar* generic_name
)
{
    StartMenuItem* start_menu_item =
        START_MENU_ITEM(g_object_new(TYPE_START_MENU_ITEM, NULL));

    // Application icon
    //
    start_menu_item->priv->icon =
        gtk_image_new_from_icon_name(
            icon_name,
            GTK_ICON_SIZE_MENU
        );

    // Depending on if a generic name is specified, we create either a normal item or
    // a 'default' item (the kind that go at the top of the Start menu)
    //
    if (generic_name == NULL) // Normal item
    {
        // Program name
        //
        start_menu_item->priv->label =
            gtk_label_new(program_name);
        
        // Set up structure
        //
        GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        
        gtk_box_pack_start(
            GTK_BOX(box),
            start_menu_item->priv->icon,
            FALSE,
            FALSE,
            0
        );
        gtk_box_pack_start(
            GTK_BOX(box),
            start_menu_item->priv->label,
            FALSE,
            FALSE,
            0
        );
        
        gtk_container_add(GTK_CONTAINER(start_menu_item), box);
    }
    else // Default item
    {
        // Add style class to distinguish this menu item
        //
        wintc_widget_add_style_class(
            GTK_WIDGET(start_menu_item),
            "xp-start-default-item"
        );

        // Generic program type
        //
        start_menu_item->priv->label_generic =
            gtk_label_new(generic_name);

        gtk_widget_set_halign(
            start_menu_item->priv->label_generic,
            GTK_ALIGN_START
        );

        // Program name
        //
        start_menu_item->priv->label =
            gtk_label_new(program_name);
        
        // Set up structure
        //
        GtkWidget* outer_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        GtkWidget* label_box = gtk_box_new(GTK_ORIENTATION_VERTICAL,   0);

        gtk_widget_set_valign(label_box, GTK_ALIGN_CENTER);

        gtk_box_pack_start(
            GTK_BOX(label_box),
            start_menu_item->priv->label_generic,
            FALSE,
            FALSE,
            0
        );
        gtk_box_pack_start(
            GTK_BOX(label_box),
            start_menu_item->priv->label,
            FALSE,
            FALSE,
            0
        );

        gtk_box_pack_start(
            GTK_BOX(outer_box),
            start_menu_item->priv->icon,
            FALSE,
            FALSE,
            0
        );
        gtk_box_pack_start(
            GTK_BOX(outer_box),
            label_box,
            FALSE,
            FALSE,
            0
        );

        gtk_container_add(GTK_CONTAINER(start_menu_item), outer_box);
    }

    // Set program name line wrapping
    //
    gtk_label_set_line_wrap(
        GTK_LABEL(start_menu_item->priv->label),
        TRUE
    );
    gtk_label_set_max_width_chars(
        GTK_LABEL(start_menu_item->priv->label),
        24
    );
    gtk_label_set_xalign(
        GTK_LABEL(start_menu_item->priv->label),
        0.0
    );

    // Add program comment
    //
    if (comment != NULL)
    {
        gchar* real_comment = wintc_str_set_suffix(comment, ".");

        gtk_widget_set_tooltip_text(
            GTK_WIDGET(start_menu_item),
            real_comment
        );

        g_free(real_comment);
    }

    return GTK_WIDGET(start_menu_item);
}

//
// CALLBACKS
//
static void on_menu_item_activate(
    GtkWidget* widget,
    WINTC_UNUSED(gpointer user_data)
)
{
    StartMenuItem* start_menu_item = START_MENU_ITEM(widget);

    if (start_menu_item->priv->is_action)
    {
        launch_action(start_menu_item->priv->action);
    }
    else
    {
        launch_command(start_menu_item->priv->cmdline);
    }
}

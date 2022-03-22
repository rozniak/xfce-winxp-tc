#include <garcon/garcon.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <wintc-comgtk.h>

#include "action.h"
#include "startmenuitem.h"
#include "util.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _StartMenuItemPrivate
{
    StartMenuItem*  menuitem;

    gint            action;
    gchar**         cmd_argv;
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

    if (start_menu_item->priv->cmd_argv != NULL)
    {
        g_strfreev(start_menu_item->priv->cmd_argv);
    }

    (*G_OBJECT_CLASS(start_menu_item_parent_class)->finalize) (object);
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* start_menu_item_new_from_action(
    gint action
)
{
    const gchar* comment;
    const gchar* icon_name;
    const gchar* name;
    GtkWidget*   start_menu_item;

    // FIXME: Localize the names and comments for these
    //
    switch (action)
    {
        case XP_ACTION_MYDOCS:
            comment   = "Opens the My Documents folder, where you can store letters, reports, notes, and other kinds of documents.";
            icon_name = "folder-documents";
            name      = "My Documents";
            break;

        case XP_ACTION_MYRECENTS:
            comment   = "Displays recently opened documents and folders.";
            icon_name = "document-open-recent";
            name      = "My Recent Documents";
            break;

        case XP_ACTION_MYPICS:
            comment   = "Opens the My Pictures folder, where you can store digital photos, images, and graphics files.";
            icon_name = "folder-pictures";
            name      = "My Pictures";
            break;

        case XP_ACTION_MYMUSIC:
            comment   = "Opens the My Music folder, where you can store music and other audio files.";
            icon_name = "folder-music";
            name      = "My Music";
            break;

        case XP_ACTION_MYCOMP:
            comment   = "Gives access to, and information about, the disk drives, cameras, scanners, and other hardware connected to your computer.";
            icon_name = "computer";
            name      = "My Computer";
            break;

        case XP_ACTION_CONTROL:
            comment   = "Provides options for you to customize the appearance and functionality of your computer, add or remove programs, and set up network connections and user accounts.";
            icon_name = "preferences-other";
            name      = "Control Panel";
            break;

        case XP_ACTION_MIMEMGMT:
            comment   = "Chooses default programs for certain activities, such as Web browsing or sending e-mail, and specifies which programs are accessible from the Start menu, desktop, and other locations.";
            icon_name = "preferences-desktop-default-applications";
            name      = "Set Program Access and Defaults";
            break;

        case XP_ACTION_CONNECTTO:
            comment   = "Connects to other computers, networks, and the Internet.";
            icon_name = "preferences-system-network";
            name      = "Connect To";
            break;

        case XP_ACTION_PRINTERS:
            comment   = "Shows installed printers and fax printers and helps you add new ones.";
            icon_name = "printer";
            name      = "Printers and Faxes";
            break;

        case XP_ACTION_HELP:
            comment   = "Opens a central location for Help topics, tutorials, troubleshooting, and other support services.";
            icon_name = "help-browser";
            name      = "Help and Support";
            break;

        case XP_ACTION_SEARCH:
            comment   = "Opens a window where you can pick search options and work with search results.";
            icon_name = "system-search";
            name      = "Search";
            break;

        case XP_ACTION_RUN:
            comment   = "Opens a program, folder, document or Web site.";
            icon_name = "system-run";
            name      = "Run...";
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
    GAppInfo* app_info = G_APP_INFO(entry);

    const gchar* app_desc = g_app_info_get_description(app_info);
    const gchar* exe_path = g_app_info_get_executable(app_info);
    const gchar* name     = g_app_info_get_name(app_info);

    gchar* cmd      = g_desktop_app_info_get_command_expanded(entry);
    gchar* exe_name = g_path_get_basename(exe_path);

    GtkWidget* start_menu_item =
        start_menu_item_new_manual(
            exe_name,
            name,
            comment != NULL ? comment : app_desc,
            generic_name
        );

    (START_MENU_ITEM(start_menu_item))->priv->cmd_argv =
        true_shell_parse_argv(cmd);

    g_free(cmd);
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

    gchar* cmd = garcon_menu_item_get_command_expanded(item);

    (START_MENU_ITEM(start_menu_item))->priv->cmd_argv =
        true_shell_parse_argv(cmd);

    g_free(cmd);

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
        gtk_widget_add_style_class(
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
        gchar* real_comment = g_str_set_suffix(comment, ".");

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
        // FIXME: Debugging
        //
        gchar* argv_debug = g_strjoinv(" ", start_menu_item->priv->cmd_argv);
        g_message(argv_debug);
        g_free(argv_debug);

        launch_command(start_menu_item->priv->cmd_argv);
    }
}

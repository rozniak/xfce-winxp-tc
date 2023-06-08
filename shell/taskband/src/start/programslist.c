#include <garcon/garcon.h>
#include <garcon-gtk/garcon-gtk.h>
#include <gio/gdesktopappinfo.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <wintc-comgtk.h>
#include <wintc-exec.h>

#include "../meta.h"
#include "programslist.h"
#include "startmenuitem.h"
#include "util.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _ProgramsListClass
{
    GtkMenuBarClass __parent__;
};

struct _ProgramsList
{
    GtkMenuBar __parent__;
};

//
// FORWARD DECLARATIONS
//
static void programs_list_finalize(
    GObject* object
);

static void programs_list_append_all_programs_item(
    ProgramsList* programs_list
);
static void programs_list_append_default_items(
    ProgramsList* programs_list
);
static void programs_list_append_separator(
    ProgramsList* programs_list,
    const gchar*  style_class
);
static void programs_list_append_top_items(
    ProgramsList* programs_list,
    gint          count
);

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(ProgramsList, programs_list, GTK_TYPE_MENU_BAR)

static void programs_list_class_init(
    ProgramsListClass* klass
)
{
    GObjectClass* gclass = G_OBJECT_CLASS(klass);

    gclass->finalize = programs_list_finalize;
}

static void programs_list_init(
    ProgramsList* self
)
{
    // Set up structure
    //
    gtk_menu_bar_set_pack_direction(
        GTK_MENU_BAR(self),
        GTK_PACK_DIRECTION_TTB
    );

    programs_list_refresh(self);
}

//
// PUBLIC FUNCTIONS
//
void programs_list_refresh(
    ProgramsList* programs_list
)
{
    programs_list_append_default_items(programs_list);

    programs_list_append_separator(programs_list, NULL);

    programs_list_append_top_items(programs_list, 6);

    programs_list_append_separator(programs_list, "xp-start-all-programs");

    programs_list_append_all_programs_item(programs_list);
}

//
// FINALIZE
//
static void programs_list_finalize(
    GObject* object
)
{
    (*G_OBJECT_CLASS(programs_list_parent_class)->finalize) (object);
}

//
// PRIVATE FUNCTIONS
//
static void programs_list_append_all_programs_item(
    ProgramsList* programs_list
)
{
    GtkWidget* item  = gtk_menu_item_new();

    //
    // The layout of the item contents is such that we can have the text and
    // arrow centered
    //
    // We have an outer box, which contains the label and a box that represents
    // the arrow (should the theme decide to style one)
    //

    // Outer box
    //
    GtkWidget* outer_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_widget_set_halign(outer_box, GTK_ALIGN_CENTER);

    // All programs text
    // 
    GtkWidget* all_programs_label = gtk_label_new(_("All Programs"));

    // 'Arrow' box
    //
    GtkWidget* arrow_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    // Set garcon menu
    //
    GarconMenu* programs_menu    = garcon_menu_new_for_path(
                                       "/usr/share/wintc/shell-res/start-menu.menu"
                                   );
    GtkWidget*  programs_submenu = garcon_gtk_menu_new(programs_menu);

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), programs_submenu);

    // Set style
    //
    wintc_widget_add_style_class(item,      "xp-start-all-programs");
    wintc_widget_add_style_class(arrow_box, "arrow");

    // Box up
    //
    gtk_box_pack_start(GTK_BOX(outer_box), all_programs_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(outer_box), arrow_box,          FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(item), outer_box);
    gtk_menu_shell_append(GTK_MENU_SHELL(programs_list), item);
}

static void programs_list_append_default_items(
    ProgramsList* programs_list
)
{
    const gchar* imsg = _("Opens your Internet browser.");
    const gchar* emsg = _("Opens your e-mail program so you can send or read a message.");

    // Load desktop entries and create menu items
    //
    GDesktopAppInfo* internet_entry = wintc_query_mime_handler(
                                          "x-scheme-handler/http",
                                          NULL
                                      );
    GDesktopAppInfo* email_entry    = wintc_query_mime_handler(
                                          "x-scheme-handler/mailto",
                                          NULL
                                      );

    //
    // FIXME: Handle NULL entries here! ATM there is a bodge in startmenuitem.c, which
    //        isn't really the right place for it (since in future, the desktop entry
    //        constructor could be used for pinned items!)
    //
    //        (Also we throw away the error, it should be handled as well ofc, I'm too
    //        lazy to do that this evening)
    //

    GtkWidget* internet_item = start_menu_item_new_from_desktop_entry(
                                   internet_entry,
                                   _("Internet"),
                                   imsg
                               );
    GtkWidget* email_item    = start_menu_item_new_from_desktop_entry(
                                   email_entry,
                                   _("E-mail"),
                                   emsg
                               );

    // FIXME: Shift 32 size to constant
    //
    start_menu_item_set_icon_size(START_MENU_ITEM(internet_item), 32);
    start_menu_item_set_icon_size(START_MENU_ITEM(email_item),    32);

    g_clear_object(&internet_entry);
    g_clear_object(&email_entry);

    gtk_menu_shell_append(GTK_MENU_SHELL(programs_list), internet_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(programs_list), email_item);
}

static void programs_list_append_separator(
    ProgramsList* programs_list,
    const gchar*  style_class
)
{
    GtkWidget* separator = gtk_separator_menu_item_new();

    if (style_class != NULL)
    {
        wintc_widget_add_style_class(separator, style_class);
    }

    gtk_menu_shell_append(GTK_MENU_SHELL(programs_list), separator);
}

static void programs_list_append_top_items(
    ProgramsList* programs_list,
    gint          count
)
{
    GarconMenu* all_entries = garcon_menu_new_for_path(
                                  "/usr/share/wintc/shell-res/all.menu"
                              );
    GError*     load_error  = NULL;

    if (!garcon_menu_load(all_entries, NULL, &load_error))
    {
        wintc_log_error_and_clear(&load_error);
        return;
    }

    // FIXME: In future we will need some >>>algorithm<<< to come up with a 'top'
    //        programs list to display here, and also reserve the last slot for the
    //        latest newly added program, if any
    //
    //        For now we simply grab the first items that are pulled in via the
    //        all.menu file
    //
    GList*      elements    = garcon_menu_get_elements(all_entries);
    GList* li = elements;
    gint   i  = 0;

    GtkWidget* menu_item;

    while (i < count && li != NULL)
    {
        if (
            GARCON_IS_MENU_ITEM(li->data) &&
            !garcon_menu_item_get_no_display(li->data)
        )
        {
            menu_item =
                start_menu_item_new_from_garcon_item(
                    GARCON_MENU_ITEM(li->data),
                    NULL,
                    NULL
                );

            // FIXME: Shift 32 size to constant
            //
            start_menu_item_set_icon_size(START_MENU_ITEM(menu_item), 32);

            gtk_menu_shell_append(GTK_MENU_SHELL(programs_list), menu_item);

            // We only count items we actually added to the increment
            //
            i++;
        }

        li = li->next;
    }

    g_list_free(g_steal_pointer(&elements));
    g_clear_object(&all_entries);
}

#include <errno.h>
#include <garcon/garcon.h>
#include <gio/gdesktopappinfo.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>
#include <wintc/shcommon.h>

#include "progmenu.h"

#define WINTC_COMPONENT_START_MENU "start-menu"

#define K_DIR_ROOT           ""
#define K_DIR_ACCESSORIES    "/Accessories"
#define K_DIR_ACCESSIBILITY  "/Accessories/Accessibility"
#define K_DIR_COMMUNICATIONS "/Accessories/Communications"
#define K_DIR_ENTERTAINMENT  "/Accessories/Entertainment"
#define K_DIR_SYSTEM_TOOLS   "/Accessories/System Tools"
#define K_DIR_GAMES          "/Games"
#define K_DIR_STARTUP        "/Startup"

#define K_DIR_GNOME "/GNOME"
#define K_DIR_KDE   "/KDE"

#define K_DIR_DOOM         "/DOOM"
#define K_DIR_LIBREOFFICE  "/LibreOffice"
#define K_DIR_QT_DEV_TOOLS "/Qt Developer Tools"

//
// LOCAL TYPEDEFS
//
typedef const gchar* (*DesktopAppInfoFilterFunc) (
    GDesktopAppInfo* entry
);

//
// FORWARD DECLARATIONS
//
static void wintc_toolbar_start_progmenu_build_fs(void);
static const gchar* wintc_toolbar_start_progmenu_filter_entry(
    GDesktopAppInfo* entry
);
static GMenu* wintc_toolbar_start_progmenu_menu_from_filelist(
    GList*       files,
    GHashTable** map_dir_to_menu
);

static gboolean create_symlink(
    const gchar* rel_path,
    const gchar* entry_name,
    const gchar* target
);

static void action_launch(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);

static const gchar* filter_doom(
    GDesktopAppInfo* entry
);
static const gchar* filter_libreoffice(
    GDesktopAppInfo* entry
);
static const gchar* filter_qt_dev_tools(
    GDesktopAppInfo* entry
);

//
// STATIC DATA
//
static gboolean S_INIT_DONE = FALSE;

static gchar* S_DIR_START_MENU = NULL;

static GHashTable* S_KNOWN_MAPPINGS_TABLE  = NULL;
static GHashTable* S_VENDOR_MAPPINGS_TABLE = NULL;

static gchar* S_KNOWN_MAPPINGS[] = {
    "explorer.desktop",    K_DIR_ACCESSORIES,
    "firefox.desktop",     K_DIR_ROOT,
    "firefox-esr.desktop", K_DIR_ROOT,
    "iexplore.desktop",    K_DIR_ROOT,
    "mspaint.desktop",     K_DIR_ACCESSORIES,
    "notepad.desktop",     K_DIR_ACCESSORIES
};

static const gchar* S_EXCLUDED_CATEGORIES[] = {
    "Screensaver",
    "Settings",
    "X-XFCE"
};

static const gchar* S_VENDOR_MAPPINGS[] = {
    "GNOME", K_DIR_GNOME,
    "KDE",   K_DIR_KDE
};

static DesktopAppInfoFilterFunc S_ENTRY_FILTERS[] = {
    &filter_doom,
    &filter_libreoffice,
    &filter_qt_dev_tools
};

// // // //

static GMenu* S_MENU_PROGRAMS = NULL;

static GHashTable* S_MAP_DIR_TO_MENU = NULL;

static GActionEntry S_ACTIONS[] = {
    {
        .name           = "launch",
        .activate       = action_launch,
        .parameter_type = "s",
        .state          = NULL,
        .change_state   = NULL
    }
};

//
// PUBLIC FUNCTIONS
//
gboolean wintc_toolbar_start_progmenu_init(
    GError** error
)
{
    if (S_INIT_DONE)
    {
        return TRUE;
    }

    // Sort out profile
    //
    S_DIR_START_MENU =
        wintc_profile_get_path(WINTC_COMPONENT_START_MENU, "");

    if (!wintc_profile_ensure_exists(WINTC_COMPONENT_START_MENU, error))
    {
        return FALSE;
    }

    // Set up known desktop entry mappings
    //
    S_KNOWN_MAPPINGS_TABLE  = g_hash_table_new(g_str_hash, g_str_equal);
    S_VENDOR_MAPPINGS_TABLE = g_hash_table_new(g_str_hash, g_str_equal);

    wintc_hash_table_insert_from_array(
        S_KNOWN_MAPPINGS_TABLE,
        (void**) S_KNOWN_MAPPINGS,
        G_N_ELEMENTS(S_KNOWN_MAPPINGS)
    );
    wintc_hash_table_insert_from_array(
        S_VENDOR_MAPPINGS_TABLE,
        (void**) S_VENDOR_MAPPINGS,
        G_N_ELEMENTS(S_VENDOR_MAPPINGS)
    );

    // Attempt to pull the structure from here
    //
    GList* files =
        wintc_sh_fs_get_names_as_list(
            S_DIR_START_MENU,
            TRUE,
            G_FILE_TEST_IS_REGULAR,
            TRUE,
            error
        );

    if (!files)
    {
        wintc_toolbar_start_progmenu_build_fs();

        files =
            wintc_sh_fs_get_names_as_list(
                S_DIR_START_MENU,
                TRUE,
                G_FILE_TEST_IS_REGULAR,
                TRUE,
                error
            );
    }

    files = g_list_sort(files, (GCompareFunc) g_ascii_strcasecmp);

    // Construct the menu
    //
    S_MENU_PROGRAMS =
        wintc_toolbar_start_progmenu_menu_from_filelist(
            files,
            &S_MAP_DIR_TO_MENU
        );

    g_list_free_full(files, g_free);

    S_INIT_DONE = TRUE;

    return TRUE;
}

GtkWidget* wintc_toolbar_start_progmenu_new_gtk_menu()
{
    if (!S_INIT_DONE)
    {
        g_critical(
            "%s",
            "start menu - progmenu not ready to make a GtkMenu"
        );

        return NULL;
    }

    // Ensure we have an action map for the menu items to call
    //
    static GSimpleActionGroup* s_action_group = NULL;

    if (!s_action_group)
    {
        s_action_group = g_simple_action_group_new();

        g_action_map_add_action_entries(
            G_ACTION_MAP(s_action_group),
            S_ACTIONS,
            G_N_ELEMENTS(S_ACTIONS),
            NULL
        );
    }

    // Create the menu
    //
    GtkWidget* menu = gtk_menu_new_from_model(G_MENU_MODEL(S_MENU_PROGRAMS));

    gtk_widget_insert_action_group(
        menu,
        "progmenu",
        G_ACTION_GROUP(s_action_group)
    );

    return menu;
}

//
// PRIVATE FUNCTIONS
//
static void wintc_toolbar_start_progmenu_build_fs(void)
{
    GList* all_entries =
        wintc_sh_fs_get_names_as_list(
            WINTC_RT_PREFIX "/share/applications",
            TRUE,
            G_FILE_TEST_IS_REGULAR,
            TRUE,
            NULL // FIXME: Error handling
        );

    for (GList* iter = all_entries; iter; iter = iter->next)
    {
        gchar* entry_path = (gchar*) iter->data;

        WINTC_LOG_DEBUG("start menu - analyse %s", entry_path);

        GDesktopAppInfo* entry =
            g_desktop_app_info_new_from_filename(entry_path);

        if (!entry)
        {
            g_warning("start menu - failed to load %s", entry_path);
            continue;
        }

        const gchar* rel_path =
            wintc_toolbar_start_progmenu_filter_entry(
                entry
            );

        if (rel_path)
        {
            create_symlink(
                rel_path,
                wintc_basename(g_desktop_app_info_get_filename(entry)),
                g_desktop_app_info_get_filename(entry)
            );
        }

        g_object_unref(entry);
    }

    g_list_free_full(all_entries, g_free);
}

static const gchar* wintc_toolbar_start_progmenu_filter_entry(
    GDesktopAppInfo* entry
)
{
    const gchar* basename =
        wintc_basename(g_desktop_app_info_get_filename(entry));

    // Check if this entry should be skipped (NoDisplay)
    //
    if (g_desktop_app_info_get_nodisplay(entry))
    {
        WINTC_LOG_DEBUG(
            "start menu - NoDisplay skipping %s",
            basename
        );

        return NULL;
    }

    // Check if this entry has a direct mapping
    //
    const gchar* known_target =
        g_hash_table_lookup(S_KNOWN_MAPPINGS_TABLE, basename);

    if (known_target)
    {
        WINTC_LOG_DEBUG(
            "start menu - filter - suggest (via known) %s/%s",
            known_target,
            basename
        );

        return known_target;
    }

    // Category checks...
    //
    const gchar* categories_str = g_desktop_app_info_get_categories(entry);

    if (!categories_str)
    {
        WINTC_LOG_DEBUG(
            "start menu - filter - no categories for %s, excluding",
            basename
        );

        return NULL;
    }

    // The categories string is literally just given raw by GLib, so we must
    // deal with the semicolon delimiter here
    //
    gchar** categories = g_strsplit(categories_str, ";", 0);

    // See if entry is in an excluded category
    //
    gchar** iter_cat = categories;

    while (*iter_cat)
    {
        gchar* category = *iter_cat;

        for (gsize i = 0; i < G_N_ELEMENTS(S_EXCLUDED_CATEGORIES); i++)
        {
            if (g_strcmp0(category, S_EXCLUDED_CATEGORIES[i]) == 0)
            {
                WINTC_LOG_DEBUG(
                    "start menu - filter - %s is in excluded category %s",
                    basename,
                    S_EXCLUDED_CATEGORIES[i]
                );

                g_strfreev(categories);

                return NULL;
            }
        }

        iter_cat++;
    }

    // See if there is a filter func for this entry
    //
    for (gsize i = 0; i < G_N_ELEMENTS(S_ENTRY_FILTERS); i++)
    {
        const gchar* fn_target =
            (*S_ENTRY_FILTERS[i]) (entry);

        if (fn_target)
        {
            g_strfreev(categories);
            return fn_target;
        }
    }

    // See if there is a vendor-specific category we can filter
    //
    iter_cat = categories;

    while (*iter_cat)
    {
        gchar* category = *iter_cat;

        const gchar* vendor_target =
            g_hash_table_lookup(S_VENDOR_MAPPINGS_TABLE, category);

        if (vendor_target)
        {
            WINTC_LOG_DEBUG(
                "start menu - filter - suggest (via vendor) %s/%s",
                vendor_target,
                basename
            );

            g_strfreev(categories);

            return vendor_target;
        }

        iter_cat++;
    }

    g_strfreev(categories);

    // Didn't find anything, plop it in the root
    //
    g_message(
        "start menu - filter - suggest (default) %s/%s",
        K_DIR_ROOT,
        basename
    );

    return K_DIR_ROOT;
}

static GMenu* wintc_toolbar_start_progmenu_menu_from_filelist(
    GList*       files,
    GHashTable** map_dir_to_menu
)
{
    GMenu* menu = g_menu_new();

    *map_dir_to_menu = g_hash_table_new(g_str_hash, g_str_equal);

    // Add the top level menu to start with...
    //
    g_hash_table_insert(
        *map_dir_to_menu,
        "",
        menu
    );

    // Construct the menus from this file list
    //
    gint len_root = strlen(S_DIR_START_MENU);

    for (GList* iter = files; iter; iter = iter->next)
    {
        const gchar*     entry_path = (gchar*) iter->data;
        GDesktopAppInfo* entry =
            g_desktop_app_info_new_from_filename(entry_path);

        WINTC_LOG_DEBUG("start menu - found: %s", (gchar*) iter->data);

        // Pull the directory components out of the path to check the submenu
        // that should own this item
        //
        const gchar* dir_end   = strrchr(entry_path, G_DIR_SEPARATOR);
        const gchar* dir_start = entry_path + len_root;

        gchar* rel_dir = g_malloc0((dir_end - dir_start) + 1);

        if (dir_start != dir_end)
        {
            memcpy(rel_dir, dir_start, dir_end - dir_start);
        }

        // Fetch the menu
        //
        GMenu* menu_owner =
            g_hash_table_lookup(*map_dir_to_menu, rel_dir);

        if (!menu_owner)
        {
            menu_owner = g_menu_new();

            g_hash_table_insert(
                *map_dir_to_menu,
                rel_dir,
                menu_owner
            );
        }
        else
        {
            g_free(rel_dir); // No longer needed
        }

        // Set up the cmdline
        //
        gchar* cmd = wintc_desktop_app_info_get_command(entry);

        // Create the menu item
        //
        GMenuItem* new_item = g_menu_item_new(NULL, NULL);
        GVariant*  variant  = g_variant_new_string(cmd);

        g_menu_item_set_action_and_target_value(
            new_item,
            "progmenu.launch",
            variant
        );
        g_menu_item_set_icon(
            new_item,
            g_app_info_get_icon(G_APP_INFO(entry))
        );
        g_menu_item_set_label(
            new_item,
            g_app_info_get_name(G_APP_INFO(entry))
        );

        g_menu_append_item(menu_owner, new_item);

        g_free(cmd);
        g_object_unref(new_item);
        g_object_unref(entry);
    }

    // Link up submenus
    //
    GList* submenu_keys = g_hash_table_get_keys(*map_dir_to_menu);

    for (GList* iter = submenu_keys; iter; iter = iter->next)
    {
        gchar* this_dir = (gchar*) iter->data;

        if (g_strcmp0(this_dir, "") == 0)
        {
            continue;
        }

        // Hold onto this menu
        //
        GMenu* this_menu =
            g_hash_table_lookup(
                *map_dir_to_menu,
                this_dir
            );

        // Nav up dir components to find parent menus, it could be that a menu
        // does not exist for a parent yet!
        //
        // strdup simplifies things so we can just free()
        //
        GMenu* cur_menu = this_menu;
        gchar* iter     = g_strdup(this_dir);

        while (iter)
        {
            // Retrieve the parent path
            //
            const gchar* dir_end = strrchr(iter, G_DIR_SEPARATOR);

            gchar* parent = g_malloc0(dir_end - iter + 1);

            memcpy(parent, iter, dir_end - iter);

            // Create the menu item for the submenu
            //
            // FIXME: The icon will not show up until a custom menu tracker
            //        implementation is made - GTK3 itself does not bind the
            //        icon attribute for menu items with a submenu -_-
            //
            static GIcon* s_icon_programs = NULL;

            if (!s_icon_programs)
            {
                s_icon_programs = g_themed_icon_new("applications-other");
            }

            GMenuItem* submenu_item = g_menu_item_new(NULL, NULL);

            g_menu_item_set_icon(
                submenu_item,
                g_themed_icon_new("add")
            );
            g_menu_item_set_label(
                submenu_item,
                dir_end + 1
            );
            g_menu_item_set_submenu(
                submenu_item,
                G_MENU_MODEL(cur_menu)
            );

            // Check if the parent exists
            //
            GMenu* parent_menu =
                g_hash_table_lookup(
                    *map_dir_to_menu,
                    parent
                );

            if (parent_menu)
            {
                g_menu_prepend_item(
                    parent_menu,
                    submenu_item
                );

                // No more work required
                g_free(parent);
                g_object_unref(submenu_item);
                break;
            }

            // Parent doesn't exist, create it...
            //
            parent_menu = g_menu_new();

            g_hash_table_insert(
                *map_dir_to_menu,
                parent,
                parent_menu
            );

            // ...add us to it and continue iterating, because this new menu
            // will need to be added to its parent too
            //
            g_menu_prepend_item(
                parent_menu,
                submenu_item
            );

            g_object_unref(submenu_item);

            cur_menu = parent_menu;

            g_free(iter);
            iter = parent;
        }

        g_free(iter);
    }

    g_list_free(submenu_keys);

    return menu;
}

static gboolean create_symlink(
    const gchar* rel_path,
    const gchar* entry_name,
    const gchar* target
)
{
    gboolean ret = TRUE;

    // Ensure the dir exists
    //
    gchar* full_dir_path =
        g_build_path(G_DIR_SEPARATOR_S, S_DIR_START_MENU, rel_path, NULL);

    if (
        mkdir(
            full_dir_path,
            S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
        ) < 0
    )
    {
        // EEXIST is fine, anything else is a problem
        //
        if (errno != EEXIST)
        {
            g_warning(
                "Start menu mapping: unable to create dir %s (err %d)",
                full_dir_path,
                errno
            );

            g_free(full_dir_path);
            return FALSE;
        }
    }

    // Attempt to create the symlink now
    //
    gchar* full_link_path =
        g_build_path(G_DIR_SEPARATOR_S, full_dir_path, entry_name, NULL);

    if (
        symlink(
            target,
            full_link_path
        ) < 0
    )
    {
        // Even EEXIST is a bit odd here... maybe the user already created a
        // link with this name
        //
        g_warning(
            "Start menu mapping: unable to create link %s (err %d)",
            full_link_path,
            errno
        );

        ret = FALSE;
    }

    g_free(full_dir_path);
    g_free(full_link_path);

    return ret;
}

//
// CALLBACKS
//
static void action_launch(
    WINTC_UNUSED(GSimpleAction* action),
    GVariant* parameter,
    WINTC_UNUSED(gpointer user_data)
)
{
    GError* error = NULL;

    if (
        !wintc_launch_command(
            g_variant_get_string(parameter, NULL),
            &error
        )
    )
    {
        wintc_display_error_and_clear(&error, NULL);
    }
}

static const gchar* filter_doom(
    GDesktopAppInfo* entry
)
{
    // Check if 'doom' is found in the keywords
    //
    const gchar* const* iter =
        g_desktop_app_info_get_keywords(entry);

    if (!iter)
    {
        return NULL;
    }

    for (; *iter; iter++)
    {
        const gchar* keyword = *iter;

        if (g_strcmp0(keyword, "doom") == 0)
        {
            return K_DIR_DOOM;
        }
    }

    // Dunno if it's a packaging bug, but for some reason Crispy Doom and
    // Chocolate Doom do not have the keyword 'doom'... ( -_- )
    //
    // IDK, just catch everything else that ends with -doom
    //
    const gchar* cmdline =
        g_app_info_get_commandline(G_APP_INFO(entry));

    if (g_str_has_suffix(cmdline, "-doom"))
    {
        return K_DIR_DOOM;
    }

    return NULL;
}

static const gchar* filter_libreoffice(
    GDesktopAppInfo* entry
)
{
    if (
        g_str_has_prefix(
            g_app_info_get_name(G_APP_INFO(entry)),
            "LibreOffice"
        )
    )
    {
        return K_DIR_LIBREOFFICE;
    }

    return NULL;
}

static const gchar* filter_qt_dev_tools(
    GDesktopAppInfo* entry
)
{
    // Assume stuff that's bundled with Qt is for the dev tools
    //
    // Under like /usr/lib/qtX/bin
    //
    const gchar* cmdline =
        g_app_info_get_commandline(G_APP_INFO(entry));

    if (g_str_has_prefix(cmdline, WINTC_RT_PREFIX "/lib/qt"))
    {
        return K_DIR_QT_DEV_TOOLS;
    }

    // Or... could be qtcreator itself
    //
    if (g_str_has_prefix(cmdline, "qtcreator"))
    {
        return K_DIR_QT_DEV_TOOLS;
    }

    return NULL;
}

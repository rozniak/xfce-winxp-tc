#include <errno.h>
#include <garcon/garcon.h>
#include <gio/gdesktopappinfo.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>
#include <wintc/shcommon.h>

#include "progmenu.h"
#include "mfu.h"

#define WINTC_COMPONENT_START_MENU "start-menu"

#define PROGMENU_MAP_FILENAME "progmenu-map"

#define K_DIR_ROOT           ""
#define K_DIR_ACCESSORIES    "/Accessories"
#define K_DIR_ACCESSIBILITY  "/Accessories/Accessibility"
#define K_DIR_COMMUNICATIONS "/Accessories/Communications"
#define K_DIR_ENTERTAINMENT  "/Accessories/Entertainment"
#define K_DIR_SYSTEM_TOOLS   "/Accessories/System Tools"
#define K_DIR_GAMES          "/Games"
#define K_DIR_STARTUP        "/Startup"

#define K_DIR_GNOME     "/GNOME"
#define K_DIR_KDE       "/KDE"
#define K_DIR_WINTC_DEV "/WinTC Developer"

#define K_DIR_DOOM         "/DOOM"
#define K_DIR_LIBREOFFICE  "/LibreOffice"
#define K_DIR_QT_DEV_TOOLS "/Qt Developer Tools"

//
// PRIVATE ENUMS
//
typedef enum
{
    WINTC_PROGMENU_SRC_HOME   = 0,
    WINTC_PROGMENU_SRC_LOCAL  = 5,
    WINTC_PROGMENU_SRC_SYSTEM = 10,
    WINTC_PROGMENU_SRC_WINE   = 17
} WinTCProgMenuSource;

//
// LOCAL TYPEDEFS
//
typedef const gchar* (*DesktopAppInfoFilterFunc) (
    GDesktopAppInfo* entry
);

//
// PRIVATE STRUCTURES
//
typedef struct _WinTCProgMenuMapping
{
    WinTCProgMenuSource src_id;
    gchar*              mapped_path;
} WinTCProgMenuMapping;

//
// FORWARD DECLARATIONS
//
static void wintc_toolbar_start_progmenu_dispose(
    GObject* object
);

static void wintc_toolbar_start_progmenu_build_fs(
    WinTCToolbarStartProgmenu* progmenu
);

static gboolean wintc_toolbar_start_progmenu_delete_entry(
    WinTCToolbarStartProgmenu* progmenu,
    const gchar*               entry_path,
    WinTCProgMenuSource        src_id
);
static const gchar* wintc_toolbar_start_progmenu_filter_entry(
    GDesktopAppInfo* entry
);
static const gchar* wintc_toolbar_start_progmenu_get_src_path(
    WinTCProgMenuSource src_id
);
static void wintc_toolbar_start_progmenu_load_mappings(
    WinTCToolbarStartProgmenu* progmenu
);
static void wintc_toolbar_start_progmenu_mapping_free(
    WinTCProgMenuMapping* mapping
);
static gsize wintc_toolbar_start_progmenu_mappings_to_text(
    GHashTable* map,
    gchar*      buf
);
static void wintc_toolbar_start_progmenu_new_entry(
    WinTCToolbarStartProgmenu* progmenu,
    const gchar*               entry_path,
    WinTCProgMenuSource        src_id
);
static void wintc_toolbar_start_progmenu_save_mappings(
    WinTCToolbarStartProgmenu* progmenu
);

static void wintc_toolbar_start_progmenu_menu_delete_entry(
    WinTCToolbarStartProgmenu* progmenu,
    const gchar*               entry_path
);
static GMenu* wintc_toolbar_start_progmenu_menu_from_filelist(
    WinTCToolbarStartProgmenu* progmenu,
    GList*                     files
);
static void wintc_toolbar_start_progmenu_menu_insert_sorted(
    GMenu*     menu,
    GMenuItem* menu_item
);
static void wintc_toolbar_start_progmenu_menu_new_entry(
    WinTCToolbarStartProgmenu* progmenu,
    const gchar*               entry_path
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

static void on_file_monitor_dir_programs_changed(
    GObject*          monitor,
    GFile*            file,
    GFile*            other_file,
    GFileMonitorEvent event_type,
    gpointer          user_data
);
static void on_file_monitor_dir_start_menu_changed(
    WinTCShDirMonitorRecursive* monitor,
    GFile*                      file,
    GFile*                      other_file,
    GFileMonitorEvent           event_type,
    gpointer                    user_data
);

//
// STATIC DATA
//
static GActionEntry S_ACTIONS[] = {
    {
        .name           = "launch",
        .activate       = action_launch,
        .parameter_type = "s",
        .state          = NULL,
        .change_state   = NULL
    }
};

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
    "GNOME",       K_DIR_GNOME,
    "KDE",         K_DIR_KDE,
    "X-WinTC-Dev", K_DIR_WINTC_DEV
};

static DesktopAppInfoFilterFunc S_ENTRY_FILTERS[] = {
    &filter_doom,
    &filter_libreoffice,
    &filter_qt_dev_tools
};

static GQuark S_QUARK_PROGMENU_SRC = 0;

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCToolbarStartProgmenu
{
    GObject __parent__;

    GMenu* menu_programs;

    GHashTable* map_dir_to_menu;
    GHashTable* map_src_rel_path_to_mapping;
    GHashTable* map_path_to_entry;

    WinTCShDirMonitorRecursive* monitor_start;
    GSList*                     monitors;
} WinTCToolbarStartProgmenu;

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCToolbarStartProgmenu,
    wintc_toolbar_start_progmenu,
    G_TYPE_OBJECT
)

static void wintc_toolbar_start_progmenu_class_init(
    WinTCToolbarStartProgmenuClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = wintc_toolbar_start_progmenu_dispose;

    // Sort out profile
    //
    S_DIR_START_MENU =
        wintc_profile_get_path(WINTC_COMPONENT_START_MENU, "");

    if (!wintc_profile_ensure_exists(WINTC_COMPONENT_START_MENU, NULL))
    {
        return;
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

    // Set up quark used for FS monitors
    //
    S_QUARK_PROGMENU_SRC = g_quark_from_static_string("progmenu-src");
}

static void wintc_toolbar_start_progmenu_init(
    WinTCToolbarStartProgmenu* self
)
{
    // Load existing mappings
    //
    wintc_toolbar_start_progmenu_load_mappings(self);

    // Attempt to pull the structure from here
    //
    GList* files;

    wintc_toolbar_start_progmenu_build_fs(self);

    files =
        wintc_sh_fs_get_names_as_list(
            S_DIR_START_MENU,
            TRUE,
            G_FILE_TEST_IS_REGULAR,
            TRUE,
            NULL
        );

    // Create mapping for entries
    //
    self->map_path_to_entry =
        g_hash_table_new_full(
            g_str_hash,
            g_str_equal,
            (GDestroyNotify) g_free,
            (GDestroyNotify) g_object_unref
        );

    // Construct the menu
    //
    self->menu_programs =
        wintc_toolbar_start_progmenu_menu_from_filelist(
            self,
            files
        );

    g_list_free_full(files, g_free);

    // Set up file monitor for the start menu dir
    //
    GFile* start_menu_dir = g_file_new_for_path(S_DIR_START_MENU);

    self->monitor_start =
        wintc_sh_fs_monitor_directory_recursive(
            start_menu_dir,
            G_FILE_MONITOR_NONE,
            NULL,
            NULL
        );

    if (self->monitor_start)
    {
        g_signal_connect(
            self->monitor_start,
            "changed",
            G_CALLBACK(on_file_monitor_dir_start_menu_changed),
            self
        );
    }

    g_object_unref(start_menu_dir);

    // Set up file monitors for application directories
    //
    WinTCProgMenuSource locations[] = {
        WINTC_PROGMENU_SRC_HOME,
        WINTC_PROGMENU_SRC_LOCAL,
        WINTC_PROGMENU_SRC_SYSTEM,
        WINTC_PROGMENU_SRC_WINE
    };

    for (gsize i = 0; i < G_N_ELEMENTS(locations); i++)
    {
        WinTCProgMenuSource src_id = locations[i];

        const gchar* monitor_path =
            wintc_toolbar_start_progmenu_get_src_path(src_id);

        if (!monitor_path)
        {
            continue;
        }

        GFile* monitor_file = g_file_new_for_path(monitor_path);

        GObject* monitor = NULL;

        if (src_id == WINTC_PROGMENU_SRC_WINE)
        {
            monitor =
                G_OBJECT(
                    wintc_sh_fs_monitor_directory_recursive(
                        monitor_file,
                        G_FILE_MONITOR_NONE,
                        NULL,
                        NULL // FIXME: Error handling
                    )
                );
        }
        else
        {
            monitor =
                G_OBJECT(
                    g_file_monitor_directory(
                        monitor_file,
                        G_FILE_MONITOR_NONE,
                        NULL,
                        NULL // FIXME: Error handling
                    )
                );
        }

        g_object_set_qdata(
            monitor,
            S_QUARK_PROGMENU_SRC,
            GINT_TO_POINTER(src_id)
        );

        self->monitors = g_slist_append(self->monitors, monitor);

        g_signal_connect(
            monitor,
            "changed",
            G_CALLBACK(on_file_monitor_dir_programs_changed),
            self
        );

        g_object_unref(monitor_file);
    }
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_toolbar_start_progmenu_dispose(
    GObject* object
)
{
    WinTCToolbarStartProgmenu* progmenu =
        WINTC_TOOLBAR_START_PROGMENU(object);

    // Save mappings first
    //
    wintc_toolbar_start_progmenu_save_mappings(progmenu);

    // Continue with the disposal
    //
    // Note that we do not dispose menu_programs explicitly here, it is
    // actually owned by map_dir_to_menu which tracks all GMenu objects
    //
    g_clear_object(&(progmenu->monitor_start));
    g_clear_slist(&(progmenu->monitors), (GDestroyNotify) g_object_unref);

    g_hash_table_destroy(
        g_steal_pointer(&(progmenu->map_dir_to_menu))
    );
    g_hash_table_destroy(
        g_steal_pointer(&(progmenu->map_src_rel_path_to_mapping))
    );
    g_hash_table_destroy(
        g_steal_pointer(&(progmenu->map_path_to_entry))
    );

    (G_OBJECT_CLASS(wintc_toolbar_start_progmenu_parent_class))
        ->dispose(object);
}

//
// PUBLIC FUNCTIONS
//
WinTCToolbarStartProgmenu* wintc_toolbar_start_progmenu_new(void)
{
    return WINTC_TOOLBAR_START_PROGMENU(
        g_object_new(
            WINTC_TYPE_TOOLBAR_START_PROGMENU,
            NULL
        )
    );
}

GtkWidget* wintc_toolbar_start_progmenu_new_gtk_menu(
    WinTCToolbarStartProgmenu* progmenu,
    WinTCCtlMenuBinding**      menu_binding
)
{
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
    GtkWidget* menu = gtk_menu_new();

    gtk_menu_set_reserve_toggle_size(GTK_MENU(menu), FALSE);

    *menu_binding =
        wintc_ctl_menu_binding_new(
            GTK_MENU_SHELL(menu),
            G_MENU_MODEL(progmenu->menu_programs)
        );

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
static void wintc_toolbar_start_progmenu_build_fs(
    WinTCToolbarStartProgmenu* progmenu
)
{
    const WinTCProgMenuSource sources[] = {
        WINTC_PROGMENU_SRC_HOME,
        WINTC_PROGMENU_SRC_LOCAL,
        WINTC_PROGMENU_SRC_SYSTEM,
        WINTC_PROGMENU_SRC_WINE
    };

    GList*       all_entries;
    const gchar* src_path;

    for (gulong i = 0; i < G_N_ELEMENTS(sources); i++)
    {
        src_path =
            wintc_toolbar_start_progmenu_get_src_path(
                sources[i]
            );

        if (!src_path)
        {
            continue;
        }

        all_entries =
            wintc_sh_fs_get_names_as_list(
                src_path,
                TRUE,
                G_FILE_TEST_IS_REGULAR,
                sources[i] == WINTC_PROGMENU_SRC_WINE,
                NULL // FIXME: Error handling
            );

        for (GList* iter = all_entries; iter; iter = iter->next)
        {
            gchar* entry_path = (gchar*) iter->data;

            wintc_toolbar_start_progmenu_new_entry(
                progmenu,
                entry_path,
                sources[i]
            );
        }

        g_list_free_full(all_entries, g_free);
    }
}

static gboolean wintc_toolbar_start_progmenu_delete_entry(
    WinTCToolbarStartProgmenu* progmenu,
    const gchar*               entry_path,
    WinTCProgMenuSource        src_id
)
{
    gboolean     delete = FALSE;
    const gchar* src_path = wintc_toolbar_start_progmenu_get_src_path(src_id);

    // Retrieve the mapping
    //
    const gchar* src_rel_path = entry_path + g_utf8_strlen(src_path, -1);

    WinTCProgMenuMapping* mapping =
        g_hash_table_lookup(
            progmenu->map_src_rel_path_to_mapping,
            src_rel_path
        );

    if (!mapping)
    {
        return TRUE;
    }

    // Validate mapping is ours
    //
    GError* error = NULL;

    gchar*     dst_path  = g_build_path(
                               G_DIR_SEPARATOR_S,
                               S_DIR_START_MENU,
                               mapping->mapped_path,
                               wintc_basename(entry_path),
                               NULL
                           );
    GFile*     file      = g_file_new_for_path(dst_path);
    GFileInfo* file_info = g_file_query_info(
                               file,
                               G_FILE_ATTRIBUTE_STANDARD_SYMLINK_TARGET,
                               G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                               NULL,
                               &error
                           );

    if (!file_info)
    {
        // FIXME: Proper error handling here
        //
        wintc_log_error_and_clear(&error);
        goto cleanup;
    }

    // Do the comparison
    //
    const gchar* link = g_file_info_get_symlink_target(file_info);

    WINTC_LOG_DEBUG("start menu - delete mapping - source is %s",  entry_path);
    WINTC_LOG_DEBUG("start menu - delete mapping - symlink is %s", link);

    if (!link)
    {
        delete = TRUE;
    }
    else if (link == entry_path)
    {
        if (unlink(dst_path) < 0)
        {
            g_warning(
                "start menu - delete mapping - couldn't unlink %s (%d)",
                dst_path,
                errno
            );

            goto cleanup;
        }

        delete = TRUE;
    }

    if (delete)
    {
        g_hash_table_remove(
            progmenu->map_src_rel_path_to_mapping,
            src_rel_path
        );
    }

cleanup:
    if (file_info) { g_object_unref(file_info); }
    g_object_unref(file);
    g_free(dst_path);

    return delete;
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

    // Is this a WINE program?
    //
    if (
        g_str_has_prefix(
            g_desktop_app_info_get_filename(entry),
            wintc_toolbar_start_progmenu_get_src_path(WINTC_PROGMENU_SRC_WINE)
        )
    )
    {
        static gchar* rel_path = NULL;

        g_free(g_steal_pointer(&rel_path));

        const gchar* full_path = g_desktop_app_info_get_filename(entry);

        const gchar* end_wine =
            g_utf8_strrchr(
                full_path,
                -1,
                G_DIR_SEPARATOR
            );

        const gchar* start_wine =
            full_path +
            g_utf8_strlen(
                wintc_toolbar_start_progmenu_get_src_path(
                    WINTC_PROGMENU_SRC_WINE
                ),
                -1
            );

        rel_path =  wintc_substr(start_wine, end_wine);

        WINTC_LOG_DEBUG(
            "start menu - filter - suggest (via WINE), %s/%s",
            rel_path,
            basename
        );

        return rel_path;
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

static const gchar* wintc_toolbar_start_progmenu_get_src_path(
    WinTCProgMenuSource src_id
)
{
    switch (src_id)
    {
        case WINTC_PROGMENU_SRC_HOME:
        {
            static const char* s_src_home = NULL;

            if (!s_src_home)
            {
                s_src_home =
                    g_build_path(
                        G_DIR_SEPARATOR_S,
                        g_get_home_dir(),
                        ".local",
                        "share",
                        "applications",
                        NULL
                    );
            }

            return s_src_home;
        }

        case WINTC_PROGMENU_SRC_LOCAL:
        {
#ifdef WINTC_PKGMGR_BSDPKG
            return NULL;
#else
            static const gchar* s_src_local = NULL;

            if (!s_src_local)
            {
                s_src_local =
                    g_build_path(
                        G_DIR_SEPARATOR_S,
                        G_DIR_SEPARATOR_S,
                        "usr",
                        "local",
                        "share",
                        "applications",
                        NULL
                    );
            }

            return s_src_local;
#endif
        }

        case WINTC_PROGMENU_SRC_SYSTEM:
        {
            static const gchar* s_src_system = NULL;

            if (!s_src_system)
            {
                s_src_system =
                    g_build_path(
                        G_DIR_SEPARATOR_S,
                        G_DIR_SEPARATOR_S,
                        "usr",
#ifdef WINTC_PKGMGR_BSDPKG
                        "local",
#endif
                        "share",
                        "applications",
                        NULL
                    );
            }

            return s_src_system;
        }

        case WINTC_PROGMENU_SRC_WINE:
        {
            static const gchar* s_src_wine = NULL;

            if (!s_src_wine)
            {
                s_src_wine =
                    g_build_path(
                        G_DIR_SEPARATOR_S,
                        g_get_home_dir(),
                        ".local",
                        "share",
                        "applications",
                        "wine",
                        "Programs",
                        NULL
                    );
            }

            return s_src_wine;
        }

        default:
            g_critical(
                "taskband - progmenu - unknown src dir %d",
                src_id
            );

            return NULL;
    }
}

static void wintc_toolbar_start_progmenu_mapping_free(
    WinTCProgMenuMapping* mapping
)
{
    g_free(mapping->mapped_path);
    g_free(mapping);
}

static void wintc_toolbar_start_progmenu_load_mappings(
    WinTCToolbarStartProgmenu* progmenu
)
{
    GError* error    = NULL;
    gchar*  map_text = NULL;

    progmenu->map_src_rel_path_to_mapping =
        g_hash_table_new_full(
            g_str_hash,
            g_str_equal,
            (GDestroyNotify) g_free,
            (GDestroyNotify) wintc_toolbar_start_progmenu_mapping_free
        );

    if (
        !wintc_profile_get_file_contents(
            WINTC_COMPONENT_SHELL,
            PROGMENU_MAP_FILENAME,
            &map_text,
            NULL,
            &error
        )
    )
    {
        // FIXME: Handle this in the UI properly
        //        Also, ignore 'file not found', because that could just be
        //        that this is the first time the taskband has started up
        //
        wintc_log_error_and_clear(&error);
        return;
    }

    // Read through the mappings
    //
    const gchar* line = map_text;

    while (line)
    {
        gchar* src_rel_path = wintc_strdup_nextchr(line, -1, ',',  &line);
        gchar* src_id_str   = wintc_strdup_nextchr(line, -1, ';',  &line);
        gchar* dst_rel_path = wintc_strdup_nextchr(line, -1, '\n', &line);

        if (
            !src_rel_path ||
            !src_id_str   ||
            !dst_rel_path
        )
        {
            g_free(src_rel_path);
            g_free(src_id_str);
            g_free(dst_rel_path);

            break;
        }

        gint src_id = strtol(src_id_str, NULL, 10);

        WinTCProgMenuMapping* mapping = g_new(WinTCProgMenuMapping, 1);

        mapping->src_id      = src_id;
        mapping->mapped_path = dst_rel_path;

        g_hash_table_insert(
            progmenu->map_src_rel_path_to_mapping,
            src_rel_path,
            mapping
        );

        g_free(src_id_str);
    }

    g_free(map_text);
}

static gsize wintc_toolbar_start_progmenu_mappings_to_text(
    GHashTable* map,
    gchar*      buf
)
{
    GHashTableIter iter;
    gpointer       key;
    gpointer       value;

    glong pos = 0;

    g_hash_table_iter_init(&iter, map);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        const WinTCProgMenuMapping* mapping      = value;
        const gchar*                src_rel_path = key;

        gchar* src_id_str = g_strdup_printf("%d", mapping->src_id);

        glong len_src_rel_path = g_utf8_strlen(src_rel_path,         -1);
        glong len_src_id       = g_utf8_strlen(src_id_str,           -1);
        glong len_mapped       = g_utf8_strlen(mapping->mapped_path, -1);

        wintc_memcpy_ref((void*) buf, pos, src_rel_path, len_src_rel_path);
        pos += len_src_rel_path;

        if (buf) { buf[pos] = ','; }
        pos++;

        wintc_memcpy_ref((void*) buf, pos, src_id_str, len_src_id);
        pos += len_src_id;

        if (buf) { buf[pos] = ';'; }
        pos++;

        wintc_memcpy_ref((void*) buf, pos, mapping->mapped_path, len_mapped);
        pos += len_mapped;

        if (buf) { buf[pos] = '\n'; }
        pos++;

        g_free(src_id_str);
    }

    // Null terminator
    //
    if (!pos)
    {
        pos++;
    }

    if (buf) { buf[pos - 1] = '\0'; }

    return (gsize) pos;
}

static void wintc_toolbar_start_progmenu_new_entry(
    WinTCToolbarStartProgmenu* progmenu,
    const gchar*               entry_path,
    WinTCProgMenuSource        src_id
)
{
    WINTC_LOG_DEBUG("start menu - analyse %s", entry_path);

    // Is this actually an entry?
    //
    if (!g_str_has_suffix(entry_path, ".desktop"))
    {
        WINTC_LOG_DEBUG(
            "start menu - new entry - not a .desktop: %s (skipped)",
            entry_path
        );

        return;
    }

    // Check relative path - do we know of this entry already?
    //
    const gchar* src_path = wintc_toolbar_start_progmenu_get_src_path(src_id);
    const gchar* rel_path = entry_path + g_utf8_strlen(src_path, -1);

    WinTCProgMenuMapping* mapping =
        g_hash_table_lookup(
            progmenu->map_src_rel_path_to_mapping,
            rel_path
        );

    if (mapping)
    {
        // If the existing mapping is higher priority then drop out
        //
        if (mapping->src_id < src_id)
        {
            return;
        }

        // If the mapping exists is lower priority, bin it
        //
        if (mapping->src_id > src_id)
        {
            if (
                !wintc_toolbar_start_progmenu_delete_entry(
                    progmenu,
                    entry_path,
                    src_id
                )
            )
            {
                g_warning(
                    "%s",
                    "start menu - new entry - couldn't bin superseded entry"
                );

                return;
            }
        }
    }

    // Add the entry now
    //
    GDesktopAppInfo* entry =
        g_desktop_app_info_new_from_filename(entry_path);

    if (!entry)
    {
        g_warning("start menu - failed to load %s", entry_path);
        return;
    }

    const gchar* dst_rel_path =
        wintc_toolbar_start_progmenu_filter_entry(
            entry
        );

    if (dst_rel_path)
    {
        if (mapping)
        {
            // If mapped already¬
            //   - If the destination is different, delete the mapping
            //   - Otherwise, nothing to do!
            //
            if (g_strcmp0(mapping->mapped_path, dst_rel_path) != 0)
            {
                if (
                    !wintc_toolbar_start_progmenu_delete_entry(
                        progmenu,
                        entry_path,
                        src_id
                    )
                )
                {
                    g_warning(
                        "%s",
                        "start menu - new entry - couldn't bin old entry"
                    );

                    goto cleanup;
                }
            }
            else
            {
                goto cleanup;
            }
        }

        if (
            create_symlink(
                dst_rel_path,
                wintc_basename(g_desktop_app_info_get_filename(entry)),
                g_desktop_app_info_get_filename(entry)
            )
        )
        {
            WinTCProgMenuMapping* new_map = g_new0(WinTCProgMenuMapping, 1);

            new_map->src_id      = src_id;
            new_map->mapped_path = g_strdup(dst_rel_path);

            g_hash_table_insert(
                progmenu->map_src_rel_path_to_mapping,
                g_strdup(rel_path),
                new_map
            );
        }
        else
        {
            g_warning(
                "start menu - new entry - could not make symlink for %s",
                entry_path
            );
        }
    }

cleanup:
    g_object_unref(entry);
}

static void wintc_toolbar_start_progmenu_save_mappings(
    WinTCToolbarStartProgmenu* progmenu
)
{
    gchar*  buf;
    gsize   buf_size;
    GError* error = NULL;

    buf_size =
        wintc_toolbar_start_progmenu_mappings_to_text(
            progmenu->map_src_rel_path_to_mapping,
            NULL
        );

    buf = g_malloc(buf_size);

    wintc_toolbar_start_progmenu_mappings_to_text(
        progmenu->map_src_rel_path_to_mapping,
        buf
    );

    if (
        !wintc_profile_set_file_contents(
            WINTC_COMPONENT_SHELL,
            PROGMENU_MAP_FILENAME,
            buf,
            -1,
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
    }

    g_free(buf);
}

static void wintc_toolbar_start_progmenu_menu_delete_entry(
    WinTCToolbarStartProgmenu* progmenu,
    const gchar*               entry_path
)
{
    // Do we even know about this entry?
    //
    GDesktopAppInfo* entry =
        g_hash_table_lookup(progmenu->map_path_to_entry, entry_path);

    if (!entry)
    {
        return;
    }

    // Pull the menu that owns the item
    //
    const gchar* dir_start = entry_path + g_utf8_strlen(S_DIR_START_MENU, -1);
    const gchar* dir_end   = g_utf8_strrchr(entry_path, -1, G_DIR_SEPARATOR);

    gchar* rel_dir = wintc_substr(dir_start, dir_end);

    GMenu* menu = g_hash_table_lookup(progmenu->map_dir_to_menu, rel_dir);

    g_free(rel_dir);

    if (!menu)
    {
        g_critical(
            "start menu - lost track of %s, report this!",
            entry_path
        );

        return;
    }

    // Begin search after submenus
    //
    gint n_items = g_menu_model_get_n_items(G_MENU_MODEL(menu));

    gint idx_end   = n_items - 1;
    gint idx_start = 0;

    for (; idx_start < n_items; idx_start++)
    {
        if (
            !g_menu_model_get_item_link(
                G_MENU_MODEL(menu),
                idx_start,
                G_MENU_LINK_SUBMENU
            )
        )
        {
            break;
        }
    }

    // Search for the item now
    //
    gchar* cmp_name = NULL;
    gchar* our_name = NULL;
    gint   idx_mid;
    gint   res;

    our_name =
        g_utf8_casefold(
            g_app_info_get_display_name(G_APP_INFO(entry)),
            -1
        );

    while(idx_end - idx_start > 1)
    {
        idx_mid = idx_end - ((idx_end - idx_start) / 2);

        g_menu_model_get_item_attribute(
            G_MENU_MODEL(menu),
            idx_mid,
            G_MENU_ATTRIBUTE_LABEL,
            "s",
            &cmp_name
        );

        WINTC_UTF8_TRANSFORM(cmp_name, g_utf8_casefold);

        res = g_utf8_collate(our_name, cmp_name);

        if (res < 0)
        {
            idx_end = idx_mid;
        }
        else if (res > 0)
        {
            idx_start = idx_mid;
        }
        else
        {
            idx_start = idx_mid;
            idx_end   = idx_mid;
        }

        g_free(cmp_name);
    }

    // Confirm the index
    //
    gint idx_found = -1;

    g_menu_model_get_item_attribute(
        G_MENU_MODEL(menu),
        idx_start,
        G_MENU_ATTRIBUTE_LABEL,
        "s",
        &cmp_name
    );

    WINTC_UTF8_TRANSFORM(cmp_name, g_utf8_casefold);

    if (g_utf8_collate(our_name, cmp_name) == 0)
    {
        idx_found = idx_start;
    }
    else
    {
        g_free(cmp_name);

        g_menu_model_get_item_attribute(
            G_MENU_MODEL(menu),
            idx_end,
            G_MENU_ATTRIBUTE_LABEL,
            "s",
            &cmp_name
        );

        WINTC_UTF8_TRANSFORM(cmp_name, g_utf8_casefold);

        if (g_utf8_collate(our_name, cmp_name) == 0)
        {
            idx_found = idx_end;
        }
    }

    g_free(cmp_name);
    g_free(our_name);

    // Destroy the item
    //
    if (idx_found < 0)
    {
        g_critical("start menu - failed to locate %s in menu", entry_path);
        return;
    }

    g_menu_remove(menu, idx_found);
    g_hash_table_remove(progmenu->map_path_to_entry, entry_path);
}

static GMenu* wintc_toolbar_start_progmenu_menu_from_filelist(
    WinTCToolbarStartProgmenu* progmenu,
    GList*                     files
)
{
    GMenu* menu = g_menu_new();

    progmenu->map_dir_to_menu =
        g_hash_table_new_full(
            g_str_hash,
            g_str_equal,
            (GDestroyNotify) g_free,
            (GDestroyNotify) g_object_unref
        );

    // Add the top level menu to start with...
    //
    g_hash_table_insert(
        progmenu->map_dir_to_menu,
        g_strdup(""),
        menu
    );

    // Construct the menus from this file list
    //
    for (GList* iter = files; iter; iter = iter->next)
    {
        const gchar* entry_path = (gchar*) iter->data;

        WINTC_LOG_DEBUG("start menu - found: %s", (gchar*) iter->data);

        wintc_toolbar_start_progmenu_menu_new_entry(
            progmenu,
            entry_path
        );
    }

    return menu;
}

static void wintc_toolbar_start_progmenu_menu_insert_sorted(
    GMenu*     menu,
    GMenuItem* menu_item
)
{
    gint idx_start = 0;
    gint idx_end   = 0;
    gint n_items   = g_menu_model_get_n_items(G_MENU_MODEL(menu));

    // Easy case - no items? Prepend
    //
    if (!n_items)
    {
        g_menu_prepend_item(menu, menu_item);
        return;
    }

    // Find the end of submenu items
    //
    gint n_submenu_items = 0;

    for (; n_submenu_items < n_items; n_submenu_items++)
    {
        // Hit a normal item? We've already hit the end then
        //
        if (
            !g_menu_model_get_item_link(
                G_MENU_MODEL(menu),
                n_submenu_items,
                G_MENU_LINK_SUBMENU
            )
        )
        {
            break;
        }
    }

    // Set the start/end depending on whether we need to insert a normal item
    // or submenu item
    //
    if (g_menu_item_get_link(menu_item, G_MENU_LINK_SUBMENU))
    {
        // If this is the first submenu item, it's always first in the menu
        //
        if (!n_submenu_items)
        {
            g_menu_prepend_item(menu, menu_item);
            return;
        }

        idx_end = n_submenu_items - 1;
    }
    else
    {
        if (n_submenu_items)
        {
            idx_start = n_submenu_items;
        }

        idx_end = n_items - 1;
    }

    // Binary search and insert
    //
    gint   idx_mid;
    gchar* cmp_name = NULL;
    gchar* our_name = NULL;
    gint   res;

    g_menu_item_get_attribute(
        menu_item,
        G_MENU_ATTRIBUTE_LABEL,
        "s",
        &our_name
    );

    WINTC_LOG_DEBUG("start menu - inserting %s", our_name);

    WINTC_UTF8_TRANSFORM(our_name, g_utf8_casefold);

    while (idx_end - idx_start > 1)
    {
        idx_mid = idx_start + ((idx_end - idx_start) / 2);

        g_menu_model_get_item_attribute(
            G_MENU_MODEL(menu),
            idx_mid,
            G_MENU_ATTRIBUTE_LABEL,
            "s",
            &cmp_name
        );

        WINTC_UTF8_TRANSFORM(cmp_name, g_utf8_casefold);

        res = g_utf8_collate(our_name, cmp_name);

        if (res < 0)
        {
            idx_end = idx_mid;
        }
        else if (res > 0)
        {
            idx_start = idx_mid;
        }
        else
        {
            idx_start = idx_mid;
            idx_end   = idx_mid;
        }

        g_free(cmp_name);
    }

    // Otherwise determine between start and end
    //
    g_menu_model_get_item_attribute(
        G_MENU_MODEL(menu),
        idx_start,
        G_MENU_ATTRIBUTE_LABEL,
        "s",
        &cmp_name
    );

    WINTC_UTF8_TRANSFORM(cmp_name, g_utf8_casefold);

    if (g_utf8_collate(our_name, cmp_name) < 0)
    {
        g_menu_insert_item(
            menu,
            idx_start,
            menu_item
        );
    }
    else
    {
        g_free(cmp_name);

        g_menu_model_get_item_attribute(
            G_MENU_MODEL(menu),
            idx_end,
            G_MENU_ATTRIBUTE_LABEL,
            "s",
            &cmp_name
        );

        WINTC_UTF8_TRANSFORM(cmp_name, g_utf8_casefold);

        if (g_utf8_collate(our_name, cmp_name) < 0)
        {
            g_menu_insert_item(
                menu,
                idx_end,
                menu_item
            );
        }
        else
        {
            g_menu_insert_item(
                menu,
                idx_end + 1,
                menu_item
            );
        }
    }

    g_free(cmp_name);
    g_free(our_name);
}

static void wintc_toolbar_start_progmenu_menu_new_entry(
    WinTCToolbarStartProgmenu* progmenu,
    const gchar*               entry_path
)
{
    GDesktopAppInfo* entry =
        g_desktop_app_info_new_from_filename(entry_path);

    if (!entry)
    {
        g_warning(
            "start menu - new menu item - not a desktop entry: %s",
            entry_path
        );

        return;
    }

    // Retrieve the launch command - some entries don't have one, which is
    // weird... we just ignore those
    //
    gchar* cmd = wintc_desktop_app_info_get_command(entry);

    if (!cmd)
    {
        WINTC_LOG_DEBUG(
            "start menu - no command for %s, skipping",
            entry_path
        );

        g_object_unref(entry);
        return;
    }

    // Create the menu item for the entry
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

    g_free(cmd);

    // Store the entry for later bookkeeping
    //
    g_hash_table_insert(
        progmenu->map_path_to_entry,
        g_strdup(entry_path),
        entry
    );

    // Pull the directory components out of the path to check the submenu
    // that should own this item
    //
    const gchar* dir_end   = g_utf8_strrchr(
                                 entry_path,
                                 -1,
                                 G_DIR_SEPARATOR
                             );
    const gchar* dir_start = entry_path + g_utf8_strlen(S_DIR_START_MENU, -1);

    gchar* rel_dir = wintc_substr(dir_start, dir_end);

    // Fetch the menu
    //
    GMenu* menu_owner =
        g_hash_table_lookup(
            progmenu->map_dir_to_menu,
            rel_dir
        );

    if (!menu_owner)
    {
        WINTC_LOG_DEBUG("start menu - new menu at %s", rel_dir);

        menu_owner = g_menu_new();

        g_hash_table_insert(
            progmenu->map_dir_to_menu,
            g_strdup(rel_dir),
            menu_owner
        );

        // Follow back up the chain if we need to create parent submenus
        //
        gboolean found_parent = FALSE;
        gchar*   iter_dir;
        GMenu*   menu_iter = menu_owner;

        while (!found_parent)
        {
            iter_dir =
                wintc_substr(
                    rel_dir,
                    g_utf8_strrchr(rel_dir, -1, G_DIR_SEPARATOR)
                );

            // Create the submenu item
            //
            static GIcon* s_icon_programs = NULL;

            if (!s_icon_programs)
            {
                s_icon_programs = g_themed_icon_new("applications-other");
            }

            GMenuItem* submenu_item = g_menu_item_new(NULL, NULL);

            g_menu_item_set_icon(
                submenu_item,
                s_icon_programs
            );
            g_menu_item_set_label(
                submenu_item,
                g_utf8_strrchr(rel_dir, -1, G_DIR_SEPARATOR) + 1
            );
            g_menu_item_set_submenu(
                submenu_item,
                G_MENU_MODEL(menu_iter)
            );

            // Locate the parent menu
            //
            menu_iter =
                g_hash_table_lookup(
                    progmenu->map_dir_to_menu,
                    iter_dir
                );

            if (!menu_iter)
            {
                menu_iter = g_menu_new();

                g_hash_table_insert(
                    progmenu->map_dir_to_menu,
                    g_strdup(iter_dir),
                    menu_iter
                );

                wintc_toolbar_start_progmenu_menu_insert_sorted(
                    menu_iter,
                    submenu_item
                );
            }
            else
            {
                wintc_toolbar_start_progmenu_menu_insert_sorted(
                    menu_iter,
                    submenu_item
                );

                found_parent = TRUE;
            }

            g_object_unref(submenu_item);

            g_free(rel_dir);
            rel_dir = iter_dir;
        }
    }

    wintc_toolbar_start_progmenu_menu_insert_sorted(
        menu_owner,
        new_item
    );

    g_object_unref(new_item);
    g_free(rel_dir);
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
        g_mkdir_with_parents(
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
    const gchar* cmdline = g_variant_get_string(parameter, NULL);
    GError*      error = NULL;

    if (!wintc_launch_command(cmdline, &error))
    {
        wintc_display_error_and_clear(&error, NULL);
        return;
    }

    wintc_start_mfu_tracker_bump_cmdline(
        wintc_start_mfu_tracker_get_default(),
        cmdline
    );
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

static void on_file_monitor_dir_programs_changed(
    GObject*          monitor,
    GFile*            file,
    WINTC_UNUSED(GFile* other_file),
    GFileMonitorEvent event_type,
    gpointer          user_data
)
{
    WinTCToolbarStartProgmenu* progmenu =
        WINTC_TOOLBAR_START_PROGMENU(user_data);

    WinTCProgMenuSource src_id =
        GPOINTER_TO_INT(
            g_object_get_qdata(monitor, S_QUARK_PROGMENU_SRC)
        );

    switch (event_type)
    {
        case G_FILE_MONITOR_EVENT_CREATED:
            WINTC_LOG_DEBUG(
                "start menu - monitor progs - new entry: %s",
                g_file_peek_path(file)
            );

            wintc_toolbar_start_progmenu_new_entry(
                progmenu,
                g_file_peek_path(file),
                src_id
            );

            break;

        case G_FILE_MONITOR_EVENT_DELETED:
            WINTC_LOG_DEBUG(
                "start menu - monitor progs - deleted entry: %s",
                g_file_peek_path(file)
            );

            wintc_toolbar_start_progmenu_delete_entry(
                progmenu,
                g_file_peek_path(file),
                src_id
            );

            break;

        default:
            WINTC_LOG_DEBUG(
                "start menu - monitor progs - not handled event %d",
                event_type
            );
            break;
    }
}

static void on_file_monitor_dir_start_menu_changed(
    WINTC_UNUSED(WinTCShDirMonitorRecursive* monitor),
    GFile*            file,
    WINTC_UNUSED(GFile* other_file),
    GFileMonitorEvent event_type,
    gpointer          user_data
)
{
    WinTCToolbarStartProgmenu* progmenu =
        WINTC_TOOLBAR_START_PROGMENU(user_data);

    switch (event_type)
    {
        case G_FILE_MONITOR_EVENT_CREATED:
            WINTC_LOG_DEBUG(
                "start menu - monitor menu - new start menu entry: %s",
                g_file_peek_path(file)
            );

            wintc_toolbar_start_progmenu_menu_new_entry(
                progmenu,
                g_file_peek_path(file)
            );

            break;

        case G_FILE_MONITOR_EVENT_DELETED:
            WINTC_LOG_DEBUG(
                "start menu - monitor menu - deleted start menu entry: %s",
                g_file_peek_path(file)
            );

            wintc_toolbar_start_progmenu_menu_delete_entry(
                progmenu,
                g_file_peek_path(file)
            );

            break;

        default:
            WINTC_LOG_DEBUG(
                "start menu - monitor menu - not handled event %d",
                event_type
            );
            break;
    }
}

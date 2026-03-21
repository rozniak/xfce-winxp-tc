#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>
#include <wintc/shcommon.h>
#include <wintc/shellext.h>

#include "../intapi.h"
#include "toolbar.h"

#define WINTC_COMPONENT_QUICK_ACCESS "quick-launch"

//
// FORWARD DECLARATIONS
//
static void wintc_toolbar_quick_access_constructed(
    GObject* object
);
static void wintc_toolbar_quick_access_dispose(
    GObject* object
);

static void wintc_toolbar_quick_access_create_button_for_file(
    WinTCToolbarQuickAccess* toolbar_qaccess,
    const gchar*             path
);
static void wintc_toolbar_quick_access_destroy_button_for_file(
    WinTCToolbarQuickAccess* toolbar_qaccess,
    const gchar*             path
);
static void wintc_toolbar_quick_access_scan_for_files(
    WinTCToolbarQuickAccess* toolbar_qaccess
);

static void on_monitor_dir_changed(
    GFileMonitor*     self,
    GFile*            file,
    GFile*            other_file,
    GFileMonitorEvent event_type,
    gpointer          user_data
);
static void on_qaccess_button_clicked(
    GtkButton* self,
    gpointer   user_data
);

//
// STATIC DATA
//
static gchar* S_DIR_QUICK_ACCESS = NULL;
static GQuark S_QUARK_PATH_HASH  = 0;

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCToolbarQuickAccess
{
    WinTCShextUIController __parent__;

    // UI stuff
    //
    GtkWidget* box_programs;

    // State stuff
    //
    GHashTable*   map_hash_to_path;
    GFileMonitor* monitor_dir;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCToolbarQuickAccess,
    wintc_toolbar_quick_access,
    WINTC_TYPE_SHEXT_UI_CONTROLLER
)

static void wintc_toolbar_quick_access_class_init(
    WinTCToolbarQuickAccessClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_toolbar_quick_access_constructed;
    object_class->dispose     = wintc_toolbar_quick_access_dispose;

    // Sort out profile
    //
    S_DIR_QUICK_ACCESS =
        wintc_profile_get_path(WINTC_COMPONENT_QUICK_ACCESS, "");

    if (!wintc_profile_ensure_exists(WINTC_COMPONENT_QUICK_ACCESS, NULL))
    {
        g_warning(
            "taskband: qaccess: failed to create dir: %s",
            S_DIR_QUICK_ACCESS
        );
    }

    // Set up quark used for storing path hash on buttons for later lookup
    //
    S_QUARK_PATH_HASH = g_quark_from_static_string("qaccess-path");
}

static void wintc_toolbar_quick_access_init(
    WinTCToolbarQuickAccess* self
)
{
    GError* error = NULL;

    // Store a mapping of path hashes to paths, the buttons store the hash in
    // qdata and use it to look up the path to execute in their 'clicked'
    // handlers
    //
    self->map_hash_to_path =
        g_hash_table_new_full(
            g_direct_hash,
            g_direct_equal,
            NULL,
            (GDestroyNotify) g_free
        );

    // Initialise UI
    //
    self->box_programs = 
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    // Pull existing directory content
    //
    wintc_toolbar_quick_access_scan_for_files(self);

    // Establish directory monitor
    //
    GFile* file_qaccess = g_file_new_for_path(S_DIR_QUICK_ACCESS);

    self->monitor_dir =
        g_file_monitor_directory(
            file_qaccess,
            G_FILE_MONITOR_NONE,
            NULL,
            &error
        );

    g_object_unref(file_qaccess);

    if (!(self->monitor_dir))
    {
        wintc_log_error_and_clear(&error);
        return;
    }

    g_signal_connect(
        self->monitor_dir,
        "changed",
        G_CALLBACK(on_monitor_dir_changed),
        self
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_toolbar_quick_access_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_toolbar_quick_access_parent_class))
        ->constructed(object);

    WinTCToolbarQuickAccess* toolbar_qaccess =
        WINTC_TOOLBAR_QUICK_ACCESS(object);

    wintc_ishext_ui_host_get_ext_widget(
        wintc_shext_ui_controller_get_ui_host(
            WINTC_SHEXT_UI_CONTROLLER(object)
        ),
        WINTC_TASKBAND_HOSTEXT_TOOLBAR,
        GTK_TYPE_WIDGET,
        toolbar_qaccess->box_programs
    );
}

static void wintc_toolbar_quick_access_dispose(
    GObject* object
)
{
    WinTCToolbarQuickAccess* toolbar_qaccess =
        WINTC_TOOLBAR_QUICK_ACCESS(object);

    g_hash_table_destroy(
        g_steal_pointer(&(toolbar_qaccess->map_hash_to_path))
    );
    g_clear_object(&(toolbar_qaccess->monitor_dir));

    (G_OBJECT_CLASS(wintc_toolbar_quick_access_parent_class))
        ->dispose(object);
}

//
// PRIVATE FUNCTIONS
//
static void wintc_toolbar_quick_access_create_button_for_file(
    WinTCToolbarQuickAccess* toolbar_qaccess,
    const gchar*             path
)
{
    guint hash = g_str_hash(path);

    // Insert hash->path mapping
    //
    if (
        g_hash_table_contains(
            toolbar_qaccess->map_hash_to_path,
            GUINT_TO_POINTER(hash)
        )
    )
    {
        return;
    }

    g_hash_table_insert(
        toolbar_qaccess->map_hash_to_path,
        GUINT_TO_POINTER(hash),
        g_strdup(path)
    );

    // Set up icon
    //
    GIcon*     icon     = wintc_sh_fs_get_file_path_icon(path);
    GtkWidget* img_icon = gtk_image_new_from_gicon(icon, GTK_ICON_SIZE_MENU);

    gtk_image_set_pixel_size(GTK_IMAGE(img_icon), 16);

    g_object_unref(icon);

    // Set up button
    //
    GtkWidget* button  = gtk_button_new();
    gchar*     tooltip = wintc_sh_fs_get_file_path_title(path);

    wintc_widget_add_style_class(button, "flat");

    gtk_widget_set_tooltip_text(
        button,
        tooltip
    );

    g_object_set_qdata(
        G_OBJECT(button),
        S_QUARK_PATH_HASH,
        GUINT_TO_POINTER(g_str_hash(path))
    );

    gtk_container_add(
        GTK_CONTAINER(button),
        img_icon
    );

    g_signal_connect(
        button,
        "clicked",
        G_CALLBACK(on_qaccess_button_clicked),
        toolbar_qaccess
    );

    g_free(tooltip);

    // Append to the toolbar
    //
    gtk_container_add(
        GTK_CONTAINER(toolbar_qaccess->box_programs),
        button
    );

    gtk_widget_show_all(button);
}

static void wintc_toolbar_quick_access_destroy_button_for_file(
    WinTCToolbarQuickAccess* toolbar_qaccess,
    const gchar*             path
)
{
    guint hash = g_str_hash(path);

    // Billy basic linear search through the buttons
    //
    GList* children =
        gtk_container_get_children(
            GTK_CONTAINER(toolbar_qaccess->box_programs)
        );

    for (GList* iter = children; iter; iter = iter->next)
    {
        guint cmp_hash =
            GPOINTER_TO_UINT(
                g_object_get_qdata(
                    G_OBJECT(iter->data),
                    S_QUARK_PATH_HASH
                )
            );

        if (cmp_hash == hash)
        {
            gtk_widget_destroy(GTK_WIDGET(iter->data));

            g_hash_table_remove(
                toolbar_qaccess->map_hash_to_path,
                GUINT_TO_POINTER(hash)
            );

            break;
        }
    }

    g_list_free(children);
}

static void wintc_toolbar_quick_access_scan_for_files(
    WinTCToolbarQuickAccess* toolbar_qaccess
)
{
    GList* entries =
        wintc_sh_fs_get_names_as_list(
            S_DIR_QUICK_ACCESS,
            TRUE,
            G_FILE_TEST_IS_REGULAR,
            FALSE,
            NULL
        );

    for (GList* iter = entries; iter; iter = iter->next)
    {
        wintc_toolbar_quick_access_create_button_for_file(
            toolbar_qaccess,
            (gchar*) iter->data
        );
    }

    g_list_free_full(entries, (GDestroyNotify) g_free);
}

//
// CALLBACKS
//
static void on_monitor_dir_changed(
    WINTC_UNUSED(GFileMonitor* self),
    GFile*            file,
    WINTC_UNUSED(GFile* other_file),
    GFileMonitorEvent event_type,
    gpointer          user_data
)
{
    WinTCToolbarQuickAccess* toolbar_qaccess =
        WINTC_TOOLBAR_QUICK_ACCESS(user_data);

    // Special case for the dir itself, since we need to handle deletion
    //
    if (g_strcmp0(g_file_peek_path(file), S_DIR_QUICK_ACCESS) == 0)
    {
        switch (event_type)
        {
            case G_FILE_MONITOR_EVENT_CREATED:
                wintc_toolbar_quick_access_scan_for_files(toolbar_qaccess);
                break;

            case G_FILE_MONITOR_EVENT_DELETED:
                wintc_container_clear(
                    GTK_CONTAINER(toolbar_qaccess->box_programs),
                    TRUE
                );

                g_hash_table_remove_all(toolbar_qaccess->map_hash_to_path);

                break;

            default: break;
        }

        return;
    }

    // Otherwise, safe to assume it's a file
    //
    switch (event_type)
    {
        case G_FILE_MONITOR_EVENT_CREATED:
            wintc_toolbar_quick_access_create_button_for_file(
                toolbar_qaccess,
                g_file_peek_path(file)
            );
            break;

        case G_FILE_MONITOR_EVENT_DELETED:
            wintc_toolbar_quick_access_destroy_button_for_file(
                toolbar_qaccess,
                g_file_peek_path(file)
            );
            break;

        default: break;
    }
}

static void on_qaccess_button_clicked(
    GtkButton* self,
    gpointer   user_data
)
{
    WinTCToolbarQuickAccess* toolbar_qaccess =
        WINTC_TOOLBAR_QUICK_ACCESS(user_data);

    const gchar* path =
        g_hash_table_lookup(
            toolbar_qaccess->map_hash_to_path,
            g_object_get_qdata(
                G_OBJECT(self),
                S_QUARK_PATH_HASH
            )
        );

    // Try launching now
    //
    GError* error = NULL;

    if (!wintc_launch_command(path, &error))
    {
        wintc_display_error_and_clear(&error, NULL);
    }
}

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
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

static void on_monitor_dir_changed(
    GFileMonitor*     self,
    GFile*            file,
    GFile*            other_file,
    GFileMonitorEvent event_type,
    gpointer          user_data
);

//
// STATIC DATA
//
static gchar* S_DIR_QUICK_ACCESS = NULL;

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCToolbarQuickAccess
{
    WinTCShextUIController __parent__;

    // UI stuff
    //
    GtkWidget* box_programs;

    // Dir monitor stuff
    //
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
}

static void wintc_toolbar_quick_access_init(
    WinTCToolbarQuickAccess* self
)
{
    GError* error        = NULL;
    GFile*  file_qaccess = g_file_new_for_path(S_DIR_QUICK_ACCESS);

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

    toolbar_qaccess->box_programs = 
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    wintc_ishext_ui_host_get_ext_widget(
        wintc_shext_ui_controller_get_ui_host(
            WINTC_SHEXT_UI_CONTROLLER(object)
        ),
        WINTC_TASKBAND_HOSTEXT_TOOLBAR,
        GTK_TYPE_WIDGET,
        toolbar_qaccess->box_programs
    );

    // Test button
    //
    GtkWidget* button = 
        gtk_button_new_from_icon_name("iexplore", GTK_ICON_SIZE_MENU);

    gtk_container_add(
        GTK_CONTAINER(toolbar_qaccess->box_programs),
        button
    );

    gtk_widget_show(button);
}

//
// CALLBACKS
//
static void on_monitor_dir_changed(
    WINTC_UNUSED(GFileMonitor* self),
    GFile*            file,
    WINTC_UNUSED(GFile* other_file),
    GFileMonitorEvent event_type,
    WINTC_UNUSED(gpointer user_data)
)
{
    g_message(
        "toolbar: qaccess: dir monitor update (%d): %s",
        event_type,
        g_file_peek_path(file)
    );
}

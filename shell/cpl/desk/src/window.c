#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>
#include <wintc/shlang.h>

#include "application.h"
#include "monitor.h"
#include "settings.h"
#include "window.h"

//
// FORWARD DECLARATIONS
//
static void wintc_cpl_desk_window_finalize(
    GObject* object
);

static void add_wallpaper_to_list(
    WinTCCplDeskWindow* wnd,
    const gchar*        path
);
static void refresh_wallpaper_list(
    WinTCCplDeskWindow* wnd
);
static void select_wallpaper_from_list(
    WinTCCplDeskWindow* wnd,
    const gchar*        path
);

static void action_apply(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_cancel(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_ok(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);

static void on_listbox_wallpapers_row_selected(
    GtkListBox*    self,
    GtkListBoxRow* row,
    gpointer       user_data
);
static gboolean on_window_map_event(
    GtkWidget*   self,
    GdkEventAny* event,
    gpointer     user_data
);

//
// STATIC DATA
//
static GActionEntry s_window_actions[] = {
    {
        .name           = "apply",
        .activate       = action_apply,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "cancel",
        .activate       = action_cancel,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "ok",
        .activate       = action_ok,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    }
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCCplDeskWindowClass
{
    GtkApplicationWindowClass __parent__;
};

struct _WinTCCplDeskWindow
{
    GtkApplicationWindow __parent__;

    // State
    //
    GSList*               list_wallpapers;
    WinTCCplDeskSettings* settings;

    // UI
    //
    GtkWidget* monitor_desktop;
    GtkWidget* listbox_wallpapers;
    GtkWidget* notebook_main;

    gboolean sync_settings;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCCplDeskWindow,
    wintc_cpl_desk_window,
    GTK_TYPE_APPLICATION_WINDOW
)

static void wintc_cpl_desk_window_class_init(
    WinTCCplDeskWindowClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->finalize = wintc_cpl_desk_window_finalize;
}

static void wintc_cpl_desk_window_init(
    WinTCCplDeskWindow* self
)
{
    GtkBuilder* builder;
    GtkWidget*  main_box;

    gtk_widget_set_size_request(
        GTK_WIDGET(self),
        414, // Approx.
        454
    );
    gtk_window_set_default_size(
        GTK_WINDOW(self),
        414, // Approx.
        454
    );
    gtk_window_set_icon_name(
        GTK_WINDOW(self),
        "preferences-desktop-display"
    );
    gtk_window_set_resizable(
        GTK_WINDOW(self),
        FALSE
    );
    gtk_window_set_skip_taskbar_hint(
        GTK_WINDOW(self),
        TRUE
    );

    // Init settings
    //
    self->settings = wintc_cpl_desk_settings_new();

    // Define GActions
    //
    g_action_map_add_action_entries(
        G_ACTION_MAP(self),
        s_window_actions,
        G_N_ELEMENTS(s_window_actions),
        self
    );

    g_object_bind_property(
        self->settings,
        "settings-changed",
        g_action_map_lookup_action(
            G_ACTION_MAP(self),
            "apply"
        ),
        "enabled",
        0
    );

    // Initialize UI
    //
    g_type_ensure(WINTC_TYPE_DESK_MONITOR);

    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/cpl-desk/desk.ui"
        );

    wintc_lc_builder_preprocess_widget_text(builder);

    main_box = GTK_WIDGET(gtk_builder_get_object(builder, "main-box"));

    gtk_container_add(GTK_CONTAINER(self), main_box);

    // Pull out widgets
    //
    self->notebook_main      = GTK_WIDGET(
                                   gtk_builder_get_object(
                                       builder,
                                       "main-notebook"
                                   )
                               );

    g_object_unref(G_OBJECT(builder));

    // Init pages
    //
    wintc_ctl_cpl_notebook_append_page_from_resource(
        GTK_NOTEBOOK(self->notebook_main),
        "/uk/oddmatics/wintc/cpl-desk/page-themes.ui",
        NULL
    );
    wintc_ctl_cpl_notebook_append_page_from_resource(
        GTK_NOTEBOOK(self->notebook_main),
        "/uk/oddmatics/wintc/cpl-desk/page-desktop.ui",
        "listbox-wallpapers", &(self->listbox_wallpapers),
        "monitor",            &(self->monitor_desktop),
        NULL
    );
    wintc_ctl_cpl_notebook_append_page_from_resource(
        GTK_NOTEBOOK(self->notebook_main),
        "/uk/oddmatics/wintc/cpl-desk/page-scrnsave.ui",
        NULL
    );
    wintc_ctl_cpl_notebook_append_page_from_resource(
        GTK_NOTEBOOK(self->notebook_main),
        "/uk/oddmatics/wintc/cpl-desk/page-appearance.ui",
        NULL
    );
    wintc_ctl_cpl_notebook_append_page_from_resource(
        GTK_NOTEBOOK(self->notebook_main),
        "/uk/oddmatics/wintc/cpl-desk/page-settings.ui",
        NULL
    );

    // Attach signals
    //
    g_signal_connect(
        self,
        "map-event",
        G_CALLBACK(on_window_map_event),
        NULL
    );

    g_signal_connect(
        self->listbox_wallpapers,
        "row-selected",
        G_CALLBACK(on_listbox_wallpapers_row_selected),
        self
    );

    // Initial load stuff
    //
    refresh_wallpaper_list(self);
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_cpl_desk_window_finalize(
    GObject* object
)
{
    WinTCCplDeskWindow* wnd = WINTC_CPL_DESK_WINDOW(object);

    g_slist_free_full(wnd->list_wallpapers, g_free);

    (G_OBJECT_CLASS(wintc_cpl_desk_window_parent_class))->finalize(object);
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_cpl_desk_window_new(
    WinTCCplDeskApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_CPL_DESK_WINDOW,
            "application", GTK_APPLICATION(app),
            "title",       _("Display Properties"),
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void add_wallpaper_to_list(
    WinTCCplDeskWindow* wnd,
    const gchar*        path
)
{
    gchar*     filename = g_path_get_basename(path);
    GtkWidget* label    = gtk_label_new(filename);

    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);

    gtk_list_box_insert(
        GTK_LIST_BOX(wnd->listbox_wallpapers),
        label,
        -1
    );

    g_free(filename);
}

static void refresh_wallpaper_list(
    WinTCCplDeskWindow* wnd
)
{
    g_slist_free_full(wnd->list_wallpapers, g_free);
    wintc_container_clear(GTK_CONTAINER(wnd->listbox_wallpapers));

    // Load up wallpapers
    //
    GError* error     = NULL;
    GSList* iter      = NULL;

    wnd->list_wallpapers =
        wintc_sh_fs_get_names_as_list(
            WINTC_RT_PREFIX "/share/backgrounds",
            TRUE,
            G_FILE_TEST_IS_REGULAR,
            TRUE,
            &error
        );

    if (!wnd->list_wallpapers)
    {
        wintc_nice_error_and_clear(&error);
        return;
    }

    iter = wnd->list_wallpapers;

    while (iter)
    {
        add_wallpaper_to_list(wnd, (gchar*) iter->data);
        iter = iter->next;
    }
}

static void select_wallpaper_from_list(
    WinTCCplDeskWindow* wnd,
    const gchar*        path
)
{
    gint i = 0;

    if (!path)
    {
        return;
    }

    for (GSList* iter = wnd->list_wallpapers; iter; iter = iter->next)
    {
        if (g_strcmp0((gchar*) iter->data, path) == 0)
        {
            gtk_list_box_select_row(
                GTK_LIST_BOX(wnd->listbox_wallpapers),
                gtk_list_box_get_row_at_index(
                    GTK_LIST_BOX(wnd->listbox_wallpapers),
                    i
                )
            );
            return;
        }

        i++;
    }

    // The path isnt in the listbox, so add it and select last item
    //
    add_wallpaper_to_list(wnd, path);

    gtk_list_box_select_row(
        GTK_LIST_BOX(wnd->listbox_wallpapers),
        gtk_list_box_get_row_at_index(
            GTK_LIST_BOX(wnd->listbox_wallpapers),
            g_slist_length(wnd->list_wallpapers) - 1
        )
    );
}

//
// CALLBACKS
//
static void action_apply(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    WinTCCplDeskWindow* wnd = WINTC_CPL_DESK_WINDOW(user_data);

    GError* error = NULL;

    if (!wintc_cpl_desk_settings_apply(wnd->settings, &error))
    {
        wintc_nice_error_and_clear(&error);
    }
}

static void action_cancel(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    gtk_window_close(GTK_WINDOW(user_data));
}

static void action_ok(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    GAction* action_apply =
        g_action_map_lookup_action(
            G_ACTION_MAP(user_data),
            "apply"
        );

    // Try applying settings...
    //
    g_action_activate(
        action_apply,
        NULL
    );

    // ...we can assume it worked if the apply action is disabled
    //
    if (!g_action_get_enabled(action_apply))
    {
        gtk_window_close(GTK_WINDOW(user_data));
    }
}

static void on_listbox_wallpapers_row_selected(
    WINTC_UNUSED(GtkListBox* self),
    GtkListBoxRow* row,
    gpointer       user_data
)
{
    WinTCCplDeskWindow* wnd = WINTC_CPL_DESK_WINDOW(user_data);

    GError*      error = NULL;
    GdkPixbuf*   pixbuf_wallpaper;
    const gchar* wallpaper_path;

    wallpaper_path =
        g_slist_nth_data(
            wnd->list_wallpapers,
            gtk_list_box_row_get_index(row)
        );

    pixbuf_wallpaper =
        gdk_pixbuf_new_from_file(
            wallpaper_path,
            &error
        );

    if (!pixbuf_wallpaper)
    {
        wintc_nice_error_and_clear(&error);
        return;
    }

    wintc_desk_monitor_set_pixbuf(
        WINTC_DESK_MONITOR(wnd->monitor_desktop),
        pixbuf_wallpaper
    );

    if (!wnd->sync_settings)
    {
        wintc_cpl_desk_settings_set_wallpaper(
            wnd->settings,
            wallpaper_path
        );
    }

    g_object_unref(pixbuf_wallpaper);
}

static gboolean on_window_map_event(
    GtkWidget* self,
    WINTC_UNUSED(GdkEventAny* event),
    WINTC_UNUSED(gpointer user_data)
)
{
    WinTCCplDeskWindow* wnd = WINTC_CPL_DESK_WINDOW(self);

    GError* error = NULL;

    if (!wintc_cpl_desk_settings_load(wnd->settings, &error))
    {
        wintc_nice_error_and_clear(&error);
        return FALSE;
    }

    wnd->sync_settings = TRUE;

    select_wallpaper_from_list(
        wnd,
        wintc_cpl_desk_settings_get_wallpaper(wnd->settings)
    );

    wnd->sync_settings = FALSE;

    return FALSE;
}

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/shlang.h>

#include "application.h"
#include "monitor.h"
#include "pagedesk.h"
#include "settings.h"
#include "window.h"

//
// FORWARD DECLARATIONS
//
static void wintc_cpl_desk_window_finalize(
    GObject* object
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
        402, // Approx.
        420
    );
    gtk_window_set_default_size(
        GTK_WINDOW(self),
        402, // Approx.
        420
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
    wintc_cpl_desk_window_append_desktop_page(self);
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
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_cpl_desk_window_finalize(
    GObject* object
)
{
    WinTCCplDeskWindow* wnd = WINTC_CPL_DESK_WINDOW(object);

    wintc_cpl_desk_window_finalize_desktop_page(wnd);

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
        wintc_nice_error_and_clear(&error, GTK_WINDOW(wnd));
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
        wintc_nice_error_and_clear(&error, GTK_WINDOW(wnd));
        return FALSE;
    }

    wnd->sync_settings = TRUE;

    wintc_cpl_desk_window_load_desktop_page(wnd);

    wnd->sync_settings = FALSE;

    return FALSE;
}

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shell.h>
#include <wintc/shellext.h>
#include <wintc/shlang.h>

#include "application.h"
#include "intapi.h"
#include "pageproc.h"
#include "window.h"

//
// FORWARD DECLARATIONS
//
static void wintc_taskmgr_window_ishext_ui_host_interface_init(
    WinTCIShextUIHostInterface* iface
);

static void wintc_taskmgr_window_constructed(
    GObject* object
);
static void wintc_taskmgr_window_dispose(
    GObject* object
);

static GtkWidget* wintc_taskmgr_window_get_ext_widget(
    WinTCIShextUIHost* host,
    guint              ext_id,
    GType              expected_type,
    gpointer           ctx
);

static void action_notimpl(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);

static void action_about(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_exit(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);

//
// STATIC DATA
//
static GActionEntry s_window_actions[] = {
    {
        .name           = "notimpl",
        .activate       = action_notimpl,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "about",
        .activate       = action_about,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "exit",
        .activate       = action_exit,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    }
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCTaskmgrWindowClass
{
    GtkApplicationWindowClass __parent__;
};

struct _WinTCTaskmgrWindow
{
    GtkApplicationWindow __parent__;

    // UI
    //
    GtkWidget* notebook_main;
    GtkWidget* menubar_main;

    WinTCShextUIController* uictl_processes;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCTaskmgrWindow,
    wintc_taskmgr_window,
    GTK_TYPE_APPLICATION_WINDOW,
    G_IMPLEMENT_INTERFACE(
        WINTC_TYPE_ISHEXT_UI_HOST,
        wintc_taskmgr_window_ishext_ui_host_interface_init
    )
)

static void wintc_taskmgr_window_class_init(
    WinTCTaskmgrWindowClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_taskmgr_window_constructed;
    object_class->dispose     = wintc_taskmgr_window_dispose;
}

static void wintc_taskmgr_window_init(
    WinTCTaskmgrWindow* self
)
{
    GtkBuilder* builder;
    GtkWidget*  main_box = NULL;

    // Define GActions
    //
    g_action_map_add_action_entries(
        G_ACTION_MAP(self),
        s_window_actions,
        G_N_ELEMENTS(s_window_actions),
        self
    );

    // Initialize UI
    //
    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/taskmgr/taskmgr.ui"
        );

    wintc_lc_builder_preprocess_widget_text(builder);

    wintc_builder_get_objects(
        builder,
        "main-box",      &main_box,
        "main-menubar",  &(self->menubar_main),
        "main-notebook", &(self->notebook_main),
        NULL
    );

    gtk_container_add(GTK_CONTAINER(self), main_box);

    gtk_widget_set_size_request(
        GTK_WIDGET(self),
        396,
        420
    );

    g_object_unref(builder);
}

static void wintc_taskmgr_window_ishext_ui_host_interface_init(
    WinTCIShextUIHostInterface* iface
)
{
    iface->get_ext_widget = wintc_taskmgr_window_get_ext_widget;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_taskmgr_window_constructed(
    GObject* object
)
{
    WinTCTaskmgrWindow* wnd = WINTC_TASKMGR_WINDOW(object);

    // Init pages
    //
    wnd->uictl_processes =
        wintc_shext_ui_controller_new_from_type(
            WINTC_TYPE_TASKMGR_PAGE_PROCESSES,
            WINTC_ISHEXT_UI_HOST(object)
        );

    (G_OBJECT_CLASS(wintc_taskmgr_window_parent_class))
        ->constructed(object);
}

static void wintc_taskmgr_window_dispose(
    GObject* object
)
{
    WinTCTaskmgrWindow* wnd = WINTC_TASKMGR_WINDOW(object);

    g_clear_object(&(wnd->uictl_processes));

    (G_OBJECT_CLASS(wintc_taskmgr_window_parent_class))
        ->dispose(object);
}

//
// INTERFACE METHODS (WinTCIShextUIHost)
//
static GtkWidget* wintc_taskmgr_window_get_ext_widget(
    WinTCIShextUIHost* host,
    guint              ext_id,
    GType              expected_type,
    gpointer           ctx
)
{
    WinTCTaskmgrWindow* wnd = WINTC_TASKMGR_WINDOW(host);

    switch (ext_id)
    {
        case WINTC_TASKMGR_HOSTEXT_PAGE:
            if (!GTK_IS_BUILDER(ctx))
            {
                g_critical(
                    "%s",
                    "ext_widget: Expected a GtkBuilder!"
                );

                return NULL;
            }

            // Try to pull the necessary widgets
            //
            GtkWidget* label_title = NULL;
            GtkWidget* widget_page = NULL;

            wintc_builder_get_objects(
                GTK_BUILDER(ctx),
                "label-title", &label_title,
                "page",        &widget_page,
                NULL
            );

            if (!label_title || !widget_page)
            {
                g_critical("%s", "ext_widget: Missing required page widgets.");
                return NULL;
            }

            if (G_OBJECT_TYPE(widget_page) != expected_type)
            {
                g_critical("%s", "ext_widget: Expected type mismatch.");
                return NULL;
            }

            gtk_notebook_append_page(
                GTK_NOTEBOOK(wnd->notebook_main),
                widget_page,
                label_title
            );

            return widget_page;

        default:
            g_critical("ext_widget: %u", ext_id);
            return NULL;
    }
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_taskmgr_window_new(
    WinTCTaskmgrApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_TASKMGR_WINDOW,
            "application", GTK_APPLICATION(app),
            "title",       "Windows Task Manager",
            NULL
        )
    );
}

//
// CALLBACKS
//
static void action_notimpl(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    WinTCTaskmgrWindow* wnd = WINTC_TASKMGR_WINDOW(user_data);

    GError* error = NULL;

    g_set_error(
        &error,
        WINTC_GENERAL_ERROR,
        WINTC_GENERAL_ERROR_NOTIMPL,
        "%s",
        "Action not implemented."
    );

    wintc_nice_error_and_clear(&error, GTK_WINDOW(wnd));
}

static void action_about(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    WinTCTaskmgrWindow* wnd = WINTC_TASKMGR_WINDOW(user_data);

    wintc_sh_about(
        GTK_WINDOW(wnd),
        "Windows Task Manager",
        NULL,
        "taskmgr"
    );
}

static void action_exit(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    WinTCTaskmgrWindow* wnd = WINTC_TASKMGR_WINDOW(user_data);

    gtk_widget_destroy(GTK_WIDGET(wnd));
}

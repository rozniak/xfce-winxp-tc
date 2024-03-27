#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shlang.h>

#include "application.h"
#include "window.h"

//
// FORWARD DECLARATIONS
//
static void add_sysdm_page(
    GtkNotebook* notebook_main,
    gchar*       resource_path
);

static void action_close(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);

//
// STATIC DATA
//
static GActionEntry s_window_actions[] = {
    {
        .name           = "close",
        .activate       = action_close,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    }
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCCplSysdmWindowClass
{
    GtkApplicationWindowClass __parent__;
};

struct _WinTCCplSysdmWindow
{
    GtkApplicationWindow __parent__;

    // UI
    //
    GtkWidget* notebook_main;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCCplSysdmWindow,
    wintc_cpl_sysdm_window,
    GTK_TYPE_APPLICATION_WINDOW
)

static void wintc_cpl_sysdm_window_class_init(
    WINTC_UNUSED(WinTCCplSysdmWindowClass* klass)
) {}

static void wintc_cpl_sysdm_window_init(
    WinTCCplSysdmWindow* self
)
{
    GtkBuilder* builder;
    GtkWidget*  main_box;

    gtk_widget_set_size_request(
        GTK_WIDGET(self),
        414, // Approx.
        454
    );
    gtk_window_set_resizable(
        GTK_WINDOW(self),
        FALSE
    );
    gtk_window_set_skip_taskbar_hint(
        GTK_WINDOW(self),
        TRUE
    );

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
            "/uk/oddmatics/wintc/cpl-sysdm/sysdm.ui"
        );

    wintc_lc_builder_preprocess_widget_text(builder);

    main_box = GTK_WIDGET(gtk_builder_get_object(builder, "main-box"));

    gtk_container_add(GTK_CONTAINER(self), main_box);

    // Pull out widgets
    //
    self->notebook_main = GTK_WIDGET(
                              gtk_builder_get_object(builder, "main-notebook")
                          );

    g_object_unref(G_OBJECT(builder));

    // Init pages
    //
    add_sysdm_page(
        GTK_NOTEBOOK(self->notebook_main),
        "/uk/oddmatics/wintc/cpl-sysdm/page-gen.ui"
    );
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_cpl_sysdm_window_new(
    WinTCCplSysdmApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_CPL_SYSDM_WINDOW,
            "application", GTK_APPLICATION(app),
            "title",       _("System Properties"),
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void add_sysdm_page(
    GtkNotebook* notebook_main,
    gchar*       resource_path
)
{
    GtkBuilder* builder;
    GtkWidget*  box_page;
    GtkWidget*  label_title;

    builder = gtk_builder_new_from_resource(resource_path);

    wintc_lc_builder_preprocess_widget_text(builder);

    box_page = GTK_WIDGET(gtk_builder_get_object(builder, "page-box"));
    label_title = GTK_WIDGET(gtk_builder_get_object(builder, "label-title"));

    gtk_notebook_append_page(
        notebook_main,
        box_page,
        label_title
    );

    g_object_unref(G_OBJECT(builder));
}

//
// CALLBACKS
//
static void action_close(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    gtk_window_close(GTK_WINDOW(user_data));
}

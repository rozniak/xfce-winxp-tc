#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/shlang.h>

#include "application.h"
#include "window.h"

//
// FORWARD DECLARATIONS
//
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
    wintc_ctl_cpl_notebook_append_page_from_resource(
        GTK_NOTEBOOK(self->notebook_main),
        "/uk/oddmatics/wintc/cpl-sysdm/page-gen.ui",
        NULL
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

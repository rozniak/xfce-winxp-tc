#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shellext.h>
#include <wintc/shlang.h>

#include "application.h"
#include "intapi.h"
#include "pagegen.h"
#include "window.h"

//
// FORWARD DECLARATIONS
//
static void wintc_cpl_sysdm_window_ishext_ui_host_interface_init(
    WinTCIShextUIHostInterface* iface
);

static void wintc_cpl_sysdm_window_constructed(
    GObject* object
);
static void wintc_cpl_sysdm_window_dispose(
    GObject* object
);

static GtkWidget* wintc_cpl_sysdm_window_get_ext_widget(
    WinTCIShextUIHost* host,
    guint              ext_id,
    GType              expected_type,
    gpointer           ctx
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

    WinTCShextUIController* uictl_pagegen;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCCplSysdmWindow,
    wintc_cpl_sysdm_window,
    GTK_TYPE_APPLICATION_WINDOW,
    G_IMPLEMENT_INTERFACE(
        WINTC_TYPE_ISHEXT_UI_HOST,
        wintc_cpl_sysdm_window_ishext_ui_host_interface_init
    )
)

static void wintc_cpl_sysdm_window_class_init(
    WinTCCplSysdmWindowClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_cpl_sysdm_window_constructed;
    object_class->dispose     = wintc_cpl_sysdm_window_dispose;
}

static void wintc_cpl_sysdm_window_init(
    WinTCCplSysdmWindow* self
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
}

static void wintc_cpl_sysdm_window_ishext_ui_host_interface_init(
    WinTCIShextUIHostInterface* iface
)
{
    iface->get_ext_widget = wintc_cpl_sysdm_window_get_ext_widget;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_cpl_sysdm_window_constructed(
    GObject* object
)
{
    WinTCCplSysdmWindow* wnd = WINTC_CPL_SYSDM_WINDOW(object);

    // Init pages
    //
    wnd->uictl_pagegen =
        wintc_shext_ui_controller_new_from_type(
            WINTC_TYPE_CPL_SYSDM_PAGE_GENERAL,
            WINTC_ISHEXT_UI_HOST(object)
        );

    (G_OBJECT_CLASS(wintc_cpl_sysdm_window_parent_class))
        ->constructed(object);
}

static void wintc_cpl_sysdm_window_dispose(
    GObject* object
)
{
    WinTCCplSysdmWindow* wnd = WINTC_CPL_SYSDM_WINDOW(object);

    g_clear_object(&(wnd->uictl_pagegen));

    (G_OBJECT_CLASS(wintc_cpl_sysdm_window_parent_class))
        ->dispose(object);
}

//
// INTERFACE METHODS (WinTCIShextUIHost)
//
static GtkWidget* wintc_cpl_sysdm_window_get_ext_widget(
    WinTCIShextUIHost* host,
    guint              ext_id,
    GType              expected_type,
    gpointer           ctx
)
{
    WinTCCplSysdmWindow* wnd = WINTC_CPL_SYSDM_WINDOW(host);

    switch (ext_id)
    {
        case WINTC_CPL_SYSDM_HOSTEXT_PAGE:
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

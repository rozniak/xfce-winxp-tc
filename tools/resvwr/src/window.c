#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shlang.h>

#include "application.h"
#include "window.h"

//
// FORWARD DECLARATIONS
//
static void wintc_resvwr_window_finalize(
    GObject* object
);

static void wintc_resvwr_window_refresh(
    WinTCResvwrWindow* wnd
);

static void action_exit(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_open(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_refresh(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);

//
// STATIC DATA
//
static GActionEntry s_window_actions[] = {
    {
        .name           = "exit",
        .activate       = action_exit,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "open",
        .activate       = action_open,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "refresh",
        .activate       = action_refresh,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCResvwrWindow
{
    GtkApplicationWindow __parent__;

    // State
    //
    gchar*     path_resource;
    GtkWidget* scrollwnd_view;
} WinTCResvwrWindow;

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCResvwrWindow,
    wintc_resvwr_window,
    GTK_TYPE_APPLICATION_WINDOW
)

static void wintc_resvwr_window_class_init(
    WinTCResvwrWindowClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->finalize = wintc_resvwr_window_finalize;
}

static void wintc_resvwr_window_init(
    WinTCResvwrWindow* self
)
{
    GtkBuilder* builder;

    // Set up action map
    //
    g_action_map_add_action_entries(
        G_ACTION_MAP(self),
        s_window_actions,
        G_N_ELEMENTS(s_window_actions),
        self
    );

    // Build menu
    //
    GMenuModel* menubar;

    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/tools/resvwr/menubar.ui"
        );

    menubar =
        G_MENU_MODEL(
            g_object_ref(
                gtk_builder_get_object(builder, "menubar")
            )
        );

    g_object_unref(builder);

    // Initialize UI
    //
    GtkWidget* menubar_main;

    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/tools/resvwr/resvwr.ui"
        );

    wintc_lc_builder_preprocess_widget_text(builder);

    wintc_builder_get_objects(
        builder,
        "menubar-main",   &menubar_main,
        "scrollwnd-view", &(self->scrollwnd_view),
        NULL
    );

    gtk_menu_shell_bind_model(
        GTK_MENU_SHELL(menubar_main),
        menubar,
        NULL,
        FALSE
    );

    gtk_container_add(
        GTK_CONTAINER(self),
        GTK_WIDGET(gtk_builder_get_object(builder, "main-box"))
    );

    g_object_unref(builder);
    g_object_unref(menubar);

    gtk_window_set_default_size(
        GTK_WINDOW(self),
        320,
        260
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_resvwr_window_finalize(
    GObject* object
)
{
    WinTCResvwrWindow* wnd = WINTC_RESVWR_WINDOW(object);

    g_free(wnd->path_resource);

    (G_OBJECT_CLASS(wintc_resvwr_window_parent_class))
        ->finalize(object);
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_resvwr_window_new(
    WinTCResvwrApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_RESVWR_WINDOW,
            "application", GTK_APPLICATION(app),
            "title",       "Resource Viewer",
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void wintc_resvwr_window_refresh(
    WinTCResvwrWindow* wnd
)
{
    // Have we got any selection?
    //
    if (!(wnd->path_resource))
    {
        return;
    }

    // Clear existing stuff (if any)
    //
    GtkWidget* child = gtk_bin_get_child(GTK_BIN(wnd->scrollwnd_view));

    if (child)
    {
        gtk_container_remove(GTK_CONTAINER(wnd->scrollwnd_view), child);
    }

    // Create new from file
    //
    GtkBuilder* builder = gtk_builder_new();
    GError*     error   = NULL;

    if (
        gtk_builder_add_from_file(
            builder,
            wnd->path_resource,
            &error
        )
    )
    {
        // Based on what we find in the builder, construct the UI in the
        // scrolled window
        //
        GtkWidget*  label_title;
        GMenuModel* menu;
        GMenuModel* menubar;
        GtkWidget*  main_box;
        GtkWidget*  main_wnd;
        GtkWidget*  page;
        GtkWidget*  toolbar;

        wintc_builder_get_objects(
            builder,
            "label-title", &label_title,
            "main-box",    &main_box,
            "main-wnd",    &main_wnd,
            "menu",        &menu,
            "menubar",     &menubar,
            "page",        &page,
            "toolbar",     &toolbar,
            NULL
        );

        if (main_box)
        {
            gtk_container_add(
                GTK_CONTAINER(wnd->scrollwnd_view),
                main_box
            );
        }
        else if (main_wnd)
        {
            GtkWidget* box_wrapper = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
            GtkWidget* child       = gtk_bin_get_child(GTK_BIN(main_wnd));

            g_object_ref(child);

            gtk_container_remove(
                GTK_CONTAINER(main_wnd),
                child
            );
            gtk_container_add(
                GTK_CONTAINER(box_wrapper),
                child
            );
            gtk_container_add(
                GTK_CONTAINER(wnd->scrollwnd_view),
                box_wrapper
            );

            g_object_unref(child);
        }
        else if (menu || menubar)
        {
            GtkWidget* box_wrapper = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
            GtkWidget* menu_built;

            if (menu) // Floating menu - create menubar to attach to
            {
                GtkWidget* submenu_built = gtk_menu_new_from_model(menu);
                GtkWidget* submenu_item  = gtk_menu_item_new_with_label("_");

                menu_built = gtk_menu_bar_new();

                gtk_menu_item_set_submenu(
                    GTK_MENU_ITEM(submenu_item),
                    submenu_built
                );
                gtk_menu_shell_append(
                    GTK_MENU_SHELL(menu_built),
                    submenu_item
                );
            }
            else // Menubar - attach immediately
            {
                menu_built  = gtk_menu_bar_new_from_model(menubar);
            }

            gtk_container_add(
                GTK_CONTAINER(box_wrapper),
                menu_built
            );
            gtk_container_add(
                GTK_CONTAINER(wnd->scrollwnd_view),
                box_wrapper
            );
        }
        else if (page)
        {
            GtkWidget* notebook = gtk_notebook_new();

            gtk_notebook_append_page(
                GTK_NOTEBOOK(notebook),
                page,
                label_title
            );

            gtk_container_add(
                GTK_CONTAINER(wnd->scrollwnd_view),
                notebook
            );
        }
        else if (toolbar)
        {
            GtkWidget* box_wrapper = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

            gtk_container_add(
                GTK_CONTAINER(box_wrapper),
                toolbar
            );
            gtk_container_add(
                GTK_CONTAINER(wnd->scrollwnd_view),
                box_wrapper
            );
        }
        else
        {
            // FIXME: Probably just append the first item? Who knows...
            //
            wintc_messagebox_show(
                GTK_WINDOW(wnd),
                "Don't know how to deal with this widget tree.",
                "Unrecognised Widget Tree",
                GTK_BUTTONS_OK,
                GTK_MESSAGE_ERROR
            );
        }

        gtk_widget_show_all(wnd->scrollwnd_view);
    }
    else
    {
        wintc_display_error_and_clear(&error, GTK_WINDOW(wnd));
    }

    g_object_unref(builder);
}

//
// CALLBACKS
//
static void action_exit(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant* parameter),
    gpointer user_data
)
{
    gtk_window_close(GTK_WINDOW(user_data));
}

static void action_open(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant* parameter),
    gpointer user_data
)
{
    WinTCResvwrWindow* wnd = WINTC_RESVWR_WINDOW(user_data);

    // Open file dialog stuff
    //
    GtkWidget*     dlg;
    GtkFileFilter* filter_xml;

    filter_xml = gtk_file_filter_new();

    gtk_file_filter_set_name(filter_xml, "XML Files");
    gtk_file_filter_add_mime_type(filter_xml, "text/xml");

    dlg =
        gtk_file_chooser_dialog_new(
            wintc_lc_get_control_text(WINTC_CTLTXT_OPEN, WINTC_PUNC_NONE),
            GTK_WINDOW(wnd),
            GTK_FILE_CHOOSER_ACTION_OPEN,
            wintc_lc_get_control_text(WINTC_CTLTXT_CANCEL, WINTC_PUNC_NONE),
            GTK_RESPONSE_CANCEL,
            wintc_lc_get_control_text(WINTC_CTLTXT_OPEN, WINTC_PUNC_NONE),
            GTK_RESPONSE_ACCEPT,
            NULL
        );

    gtk_file_chooser_add_filter(
        GTK_FILE_CHOOSER(dlg),
        filter_xml
    );

    // Execute dialog and handle result
    //
    gint result = gtk_dialog_run(GTK_DIALOG(dlg));

    if (result == GTK_RESPONSE_ACCEPT)
    {
        wnd->path_resource =
            gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));

        wintc_resvwr_window_refresh(wnd);
    }

    gtk_widget_destroy(dlg);
}

static void action_refresh(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant* parameter),
    gpointer user_data
)
{
    WinTCResvwrWindow* wnd = WINTC_RESVWR_WINDOW(user_data);

    WINTC_LOG_DEBUG("resvwr: refreshing...");

    wintc_resvwr_window_refresh(wnd);
}

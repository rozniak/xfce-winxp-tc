#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shellext.h>
#include <wintc/shlang.h>

#include "webside.h"

//
// PUBLIC CONSTANTS
//
const gchar* WINTC_EXPLORER_SIDEBAR_ID_WEB = "web";

//
// FORWARD DECLARATIONS
//
static void wintc_exp_web_sidebar_constructed(
    GObject* object
);

static void wintc_exp_web_sidebar_connect_signal(
    WinTCExpWebSidebar* sidebar_web
);
static void wintc_exp_web_sidebar_update_actions(
    WinTCExpWebSidebar* sidebar_web
);

static void on_browser_load_changed(
    WinTCShBrowser*         self,
    WinTCShBrowserLoadEvent load_event,
    gpointer                user_data
);
static void on_explorer_window_mode_changed(
    WinTCExplorerWindow* self,
    gpointer             user_data
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCExpWebSidebarClass
{
    WinTCExplorerSidebarClass __parent__;
};

struct _WinTCExpWebSidebar
{
    WinTCExplorerSidebar __parent__;

    gulong sigid_load_changed;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCExpWebSidebar,
    wintc_exp_web_sidebar,
    WINTC_TYPE_EXPLORER_SIDEBAR
)

static void wintc_exp_web_sidebar_class_init(
    WinTCExpWebSidebarClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_exp_web_sidebar_constructed;
}

static void wintc_exp_web_sidebar_init(
    WinTCExpWebSidebar* self
)
{
    WinTCExplorerSidebar* sidebar = WINTC_EXPLORER_SIDEBAR(self);

    GtkBuilder* builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/explorer/webside.ui"
        );

    wintc_builder_get_objects(
        builder,
        "main-box", &(sidebar->root_widget),
        NULL
    );

    g_object_ref(sidebar->root_widget);

    g_object_unref(builder);
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_exp_web_sidebar_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_exp_web_sidebar_parent_class))
        ->constructed(object);

    WinTCExplorerSidebar* sidebar =
        WINTC_EXPLORER_SIDEBAR(object);
    WinTCExpWebSidebar* sidebar_web =
        WINTC_EXP_WEB_SIDEBAR(object);

    // May have to delay loading if Explorer has only just started and hasn't
    // got a browser yet
    //
    if (
        wintc_explorer_window_get_mode(
            WINTC_EXPLORER_WINDOW(sidebar->owner_explorer_wnd)
        ) == WINTC_EXPLORER_WINDOW_MODE_LOCAL
    )
    {
        wintc_exp_web_sidebar_connect_signal(sidebar_web);
        wintc_exp_web_sidebar_update_actions(sidebar_web);
    }
    else
    {
        g_signal_connect_object(
            sidebar->owner_explorer_wnd,
            "mode-changed",
            G_CALLBACK(on_explorer_window_mode_changed),
            object,
            G_CONNECT_DEFAULT
        );
    }
}

//
// PUBLIC FUNCTIONS
//
WinTCExplorerSidebar* wintc_exp_web_sidebar_new(
    WinTCExplorerWindow* wnd
)
{
    return WINTC_EXPLORER_SIDEBAR(
        g_object_new(
            WINTC_TYPE_EXP_WEB_SIDEBAR,
            "owner-explorer", wnd,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void wintc_exp_web_sidebar_connect_signal(
    WinTCExpWebSidebar* sidebar_web
)
{
    WinTCExplorerSidebar* sidebar =
        WINTC_EXPLORER_SIDEBAR(sidebar_web);
    
    sidebar_web->sigid_load_changed =
        g_signal_connect(
            wintc_explorer_window_get_browser(
                WINTC_EXPLORER_WINDOW(sidebar->owner_explorer_wnd)
            ),
            "load-changed",
            G_CALLBACK(on_browser_load_changed),
            sidebar_web
        );
}

static void wintc_exp_web_sidebar_update_actions(
    WinTCExpWebSidebar* sidebar_web
)
{
    WinTCExplorerSidebar* sidebar = WINTC_EXPLORER_SIDEBAR(sidebar_web);

    // Retrieve the view (if we have one)
    //
    WinTCShBrowser* browser =
        wintc_explorer_window_get_browser(
            WINTC_EXPLORER_WINDOW(sidebar->owner_explorer_wnd)
        );

    WinTCIShextView* view =
        wintc_sh_browser_get_current_view(browser);

    // Retrieve the suggested actions model
    //
    GMenuModel* model =
        wintc_ishext_view_get_suggested_actions(view, 0);

    wintc_container_clear(
        GTK_CONTAINER(sidebar->root_widget),
        TRUE
    );

    // Iterate first level (groups)
    //
    gint n_groups = g_menu_model_get_n_items(model);

    for (gint i = 0; i < n_groups; i++)
    {
        // This SHOULD be a <section>, if it isn't then we have a problem
        //
        GMenuModel* section =
            g_menu_model_get_item_link(
                model,
                i,
                G_MENU_LINK_SECTION
            );

        if (!section)
        {
            g_critical("%s", "shell: web sidebar received an invalid menu");
            break;
        }

        // Construct this group
        //
        GVariant* v_label   = g_menu_model_get_item_attribute_value(
                                  model,
                                  i,
                                  "label",
                                  G_VARIANT_TYPE_STRING
                              );
        GVariant* v_icon    = g_menu_model_get_item_attribute_value(
                                  model,
                                  i,
                                  "icon",
                                  G_VARIANT_TYPE_STRING
                              );
        GVariant* v_special = g_menu_model_get_item_attribute_value(
                                  model,
                                  i,
                                  "special",
                                  G_VARIANT_TYPE_STRING
                              );

        const gchar* group_label =
            v_label ? g_variant_get_string(v_label, NULL) : "(no name)";
        const gchar* group_icon =
            v_icon  ? g_variant_get_string(v_icon,  NULL) : NULL;

        // Determine what type of group we're dealing with
        //  - Label only = normal group
        //  - Label and icon = 'hot' group
        //  - Special = file details or some other known special
        //
        GtkBuilder* builder =
            gtk_builder_new_from_resource(
                "/uk/oddmatics/wintc/explorer/websdgrp.ui"
            );

        GtkWidget* box_group     = NULL;
        GtkWidget* box_items     = NULL;
        GtkWidget* img_icon      = NULL;
        GtkWidget* label_heading = NULL;

        wintc_lc_builder_preprocess_widget_text(builder);

        wintc_builder_get_objects(
            builder,
            "box-group",     &box_group,
            "box-items",     &box_items,
            "img-icon",      &img_icon,
            "label-heading", &label_heading,
            NULL
        );

        gtk_label_set_text(
            GTK_LABEL(label_heading),
            group_label
        );
        gtk_image_set_from_icon_name(
            GTK_IMAGE(img_icon),
            group_icon,
            GTK_ICON_SIZE_MENU
        );

        gtk_box_pack_start(
            GTK_BOX(sidebar->root_widget),
            box_group,
            FALSE,
            FALSE,
            0
        );

        if (v_special)
        {
            g_critical("%s", "shell: special item not supported!");
        }
        else
        {
            // Iterate second level (action items)
            //
            gint n_items = g_menu_model_get_n_items(section);

            for (gint j = 0; j < n_items; j++)
            {
                GVariant* v_item_label =
                    g_menu_model_get_item_attribute_value(
                        section,
                        j,
                        "label",
                        G_VARIANT_TYPE_STRING
                    );
                GVariant* v_item_icon =
                    g_menu_model_get_item_attribute_value(
                        section,
                        j,
                        "icon",
                        G_VARIANT_TYPE_STRING
                    );

                const gchar* item_label =
                    g_variant_get_string(v_item_label, NULL);
                const gchar* item_icon =
                    g_variant_get_string(v_item_icon, NULL);

                GtkBuilder* builder_button =
                    gtk_builder_new_from_resource(
                        "/uk/oddmatics/wintc/explorer/websditm.ui"
                    );

                GtkWidget* button_item      = NULL;
                GtkWidget* box_inner        = NULL;
                GtkWidget* img_item_icon    = NULL;
                GtkWidget* label_item_label = NULL;

                wintc_builder_get_objects(
                    builder_button,
                    "button-item",      &button_item,
                    "box-inner",        &box_inner,
                    "img-item-icon",    &img_item_icon,
                    "label-item-label", &label_item_label,
                    NULL
                );

                gtk_label_set_text(
                    GTK_LABEL(label_item_label),
                    item_label
                );
                gtk_image_set_from_icon_name(
                    GTK_IMAGE(img_item_icon),
                    item_icon,
                    GTK_ICON_SIZE_MENU
                );

                gtk_box_pack_start(
                    GTK_BOX(box_items),
                    button_item,
                    FALSE,
                    FALSE,
                    0
                );

                g_object_unref(builder_button);

                wintc_clear_variant(&v_item_label);
                wintc_clear_variant(&v_item_icon);
            }
        }

        gtk_widget_show_all(box_group);

        g_object_unref(builder);

        wintc_clear_variant(&v_label);
        wintc_clear_variant(&v_icon);
        wintc_clear_variant(&v_special);
    }

    g_object_unref(model);
}

//
// CALLBACKS
//
static void on_browser_load_changed(
    WINTC_UNUSED(WinTCShBrowser* self),
    WinTCShBrowserLoadEvent load_event,
    gpointer                user_data
)
{
    WinTCExpWebSidebar* sidebar_web =
        WINTC_EXP_WEB_SIDEBAR(user_data);

    if (load_event != WINTC_SH_BROWSER_LOAD_FINISHED)
    {
        return;
    }

    wintc_exp_web_sidebar_update_actions(sidebar_web);
}

static void on_explorer_window_mode_changed(
    WinTCExplorerWindow* self,
    gpointer             user_data
)
{
    WinTCExpWebSidebar* sidebar_web =
        WINTC_EXP_WEB_SIDEBAR(user_data);

    if (
        sidebar_web->sigid_load_changed ||
        wintc_explorer_window_get_mode(self) !=
            WINTC_EXPLORER_WINDOW_MODE_LOCAL
    )
    {
        return;
    }

    wintc_exp_web_sidebar_connect_signal(sidebar_web);
}

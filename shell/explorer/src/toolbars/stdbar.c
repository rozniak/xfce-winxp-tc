#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../sidebars/favside.h"
#include "../sidebars/fldrside.h"
#include "../sidebars/srchside.h"
#include "../toolbar.h"
#include "../window.h"
#include "stdbar.h"

//
// FORWARD DECLARATIONS
//
static void wintc_exp_standard_toolbar_constructed(
    GObject* object
);

static GtkToolItem* create_toolbar_item_from_char(
    WinTCExpStandardToolbar* toolbar_std,
    gchar                    c
);
static GtkToolItem* find_toolbar_item(
    WinTCExpStandardToolbar* toolbar_std,
    gchar                    c
);
static void populate_toolbar(
    WinTCExpStandardToolbar* toolbar_std,
    const gchar*             config_str
);
static void repopulate_toolbar(
    WinTCExpStandardToolbar* toolbar_std
);
static void setup_toolbar_button(
    GtkToolItem* tool_button,
    gboolean     is_important,
    const gchar* label,
    const gchar* icon_name
);
static void sync_sidebar_buttons(
    WinTCExpStandardToolbar* toolbar_std
);
static void toggle_sidebar(
    WinTCExpStandardToolbar* toolbar_std,
    const gchar*             sidebar_id
);

static void on_owner_explorer_wnd_notify_active_sidebar(
    GObject*    self,
    GParamSpec* pspec,
    gpointer    user_data
);
static void on_owner_explorer_wnd_mode_changed(
    GtkWidget* self,
    gpointer   user_data
);
static void on_toolbar_button_favorites_toggled(
    GtkToggleToolButton* self,
    gpointer             user_data
);
static void on_toolbar_button_folders_toggled(
    GtkToggleToolButton* self,
    gpointer             user_data
);
static void on_toolbar_button_search_toggled(
    GtkToggleToolButton* self,
    gpointer             user_data
);
static void on_toolbar_style_updated(
    GtkWidget* self,
    gpointer   user_data
);

//
// STATIC DATA
//
static const gchar* S_LAYOUT_LOCAL    = "bfu|sd|v";
static const gchar* S_LAYOUT_INTERNET = "bfSRh|sFH|MPEC";

static const gint S_TOOLBAR_ICON_SIZE = 24;

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCExpStandardToolbarClass
{
    WinTCExplorerToolbarClass __parent__;
};

struct _WinTCExpStandardToolbar
{
    WinTCExplorerToolbar __parent__;

    // State
    //
    gboolean synchronizing;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCExpStandardToolbar,
    wintc_exp_standard_toolbar,
    WINTC_TYPE_EXPLORER_TOOLBAR
)

static void wintc_exp_standard_toolbar_class_init(
    WinTCExpStandardToolbarClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_exp_standard_toolbar_constructed;
}

static void wintc_exp_standard_toolbar_init(
    WinTCExpStandardToolbar* self
)
{
    WinTCExplorerToolbar* toolbar = WINTC_EXPLORER_TOOLBAR(self);

    gtk_toolbar_set_style(
        GTK_TOOLBAR(toolbar->toolbar),
        GTK_TOOLBAR_BOTH_HORIZ
    );

    g_signal_connect(
        toolbar->toolbar,
        "style-updated",
        G_CALLBACK(on_toolbar_style_updated),
        self
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_exp_standard_toolbar_constructed(
    GObject* object
)
{
    WinTCExplorerToolbar* toolbar = WINTC_EXPLORER_TOOLBAR(object);

    g_signal_connect(
        toolbar->owner_explorer_wnd,
        "mode-changed",
        G_CALLBACK(on_owner_explorer_wnd_mode_changed),
        object
    );
    g_signal_connect(
        toolbar->owner_explorer_wnd,
        "notify::active-sidebar",
        G_CALLBACK(on_owner_explorer_wnd_notify_active_sidebar),
        object
    );

    (G_OBJECT_CLASS(wintc_exp_standard_toolbar_parent_class))
        ->constructed(object);
}

//
// PUBLIC FUNCTIONS
//
WinTCExplorerToolbar* wintc_exp_standard_toolbar_new(
    WinTCExplorerWindow* wnd
)
{
    return WINTC_EXPLORER_TOOLBAR(
        g_object_new(
            WINTC_TYPE_EXP_STANDARD_TOOLBAR,
            "owner-explorer", wnd,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static GtkToolItem* create_toolbar_item_from_char(
    WinTCExpStandardToolbar* toolbar_std,
    gchar                    c
)
{
    GtkToolItem* ret = NULL;

    //
    // FIXME: Localise strings when possible
    //

    switch (c)
    {
        // TODO: Implement this
        //
        case 'b':
            // TODO: The tooltip for the back button should be the first
            //       item in the history
            //
            ret = gtk_menu_tool_button_new(NULL, NULL);

            setup_toolbar_button(
                ret,
                TRUE,
                "Back",
                wintc_icon_name_first_available(
                    S_TOOLBAR_ICON_SIZE,
                    "go-previous",
                    "history-back",
                    NULL
                )
            );

            gtk_actionable_set_action_name(
                GTK_ACTIONABLE(ret),
                "win.nav-back"
            );

            break;

        // TODO: Implement this
        //
        case 'C':
            ret = gtk_tool_button_new(NULL, NULL);

            setup_toolbar_button(
                ret,
                FALSE,
                "Messenger",
                wintc_icon_name_first_available(
                    S_TOOLBAR_ICON_SIZE,
                    "face-wink",
                    "windows-messenger",
                    "internet-group-chat",
                    NULL
                )
            );

            break;

        // TODO: Implement this
        //
        case 'd':
            ret = gtk_toggle_tool_button_new();

            setup_toolbar_button(
                ret,
                TRUE,
                "Folders",
                wintc_icon_name_first_available(
                    S_TOOLBAR_ICON_SIZE,
                    "inode-directory",
                    "folders",
                    NULL
                )
            );

            g_signal_connect(
                ret,
                "toggled",
                G_CALLBACK(on_toolbar_button_folders_toggled),
                toolbar_std
            );

            break;

        // TODO: Implement this
        //
        case 'E':
            ret = gtk_tool_button_new(NULL, NULL);

            setup_toolbar_button(
                ret,
                FALSE,
                "Edit with Notepad", // FIXME: Might not always be notepad?
                wintc_icon_name_first_available(
                    S_TOOLBAR_ICON_SIZE,
                    "document-properties",
                    "document-open-ide",
                    NULL
                )
            );

            break;

        // TODO: Implement this
        // 
        case 'F':
            ret = gtk_toggle_tool_button_new();

            setup_toolbar_button(
                ret,
                TRUE,
                "Favorites",
                "emblem-favorite"
            );

            g_signal_connect(
                ret,
                "toggled",
                G_CALLBACK(on_toolbar_button_favorites_toggled),
                toolbar_std
            );

            break;

        // TODO: Implement this
        //
        case 'f':
            // TODO: See back button tooltip about history, but for Forward,
            //       since it has no label, the tooltip defaults to Forward
            ret = gtk_tool_button_new(NULL, NULL);

            setup_toolbar_button(
                ret,
                FALSE,
                "Forward",
                wintc_icon_name_first_available(
                    S_TOOLBAR_ICON_SIZE,
                    "go-next",
                    "history-forward",
                    NULL
                )
            );

            gtk_actionable_set_action_name(
                GTK_ACTIONABLE(ret),
                "win.nav-forward"
            );

            break;

        // TODO: Implement this
        //
        case 'H':
            ret = gtk_toggle_tool_button_new();

            setup_toolbar_button(
                ret,
                FALSE,
                "History",
                wintc_icon_name_first_available(
                    S_TOOLBAR_ICON_SIZE,
                    "media-seek-backward",
                    "history",
                    NULL
                )
            );

            break;

        // TODO: Implement this
        //
        case 'h':
            ret = gtk_tool_button_new(NULL, NULL);

            setup_toolbar_button(
                ret,
                FALSE,
                "Home",
                "go-home"
            );

            break;

        // TODO: Implement this
        // 
        case 'M':
            ret = gtk_tool_button_new(NULL, NULL);

            setup_toolbar_button(
                ret,
                FALSE,
                "Mail",
                wintc_icon_name_first_available(
                    S_TOOLBAR_ICON_SIZE,
                    "emblem-mail",
                    "internet-mail",
                    NULL
                )
            );

            break;

        // TODO: Implement this
        //
        case 'P':
            ret = gtk_tool_button_new(NULL, NULL);

            setup_toolbar_button(
                ret,
                FALSE,
                "Print",
                "printer"
            );

            break;

        // TODO: Implement this
        //
        case 'R':
            ret = gtk_tool_button_new(NULL, NULL);

            setup_toolbar_button(
                ret,
                FALSE,
                "Refresh",
                wintc_icon_name_first_available(
                    S_TOOLBAR_ICON_SIZE,
                    "view-refresh",
                    "document-refresh",
                    NULL
                )
            );

            break;

        // TODO: Implement this
        //
        case 'S':
            ret = gtk_tool_button_new(NULL, NULL);

            setup_toolbar_button(
                ret,
                FALSE,
                "Stop",
                wintc_icon_name_first_available(
                    S_TOOLBAR_ICON_SIZE,
                    "process-stop",
                    "document-stop",
                    NULL
                )
            );

            break;

        // TODO: Implement this
        //
        case 's':
            ret = gtk_toggle_tool_button_new();

            setup_toolbar_button(
                ret,
                TRUE,
                "Search",
                wintc_icon_name_first_available(
                    S_TOOLBAR_ICON_SIZE,
                    "system-search",
                    "search",
                    NULL
                )
            );

            g_signal_connect(
                ret,
                "toggled",
                G_CALLBACK(on_toolbar_button_search_toggled),
                toolbar_std
            );

            break;

        case 'u':
            ret = gtk_tool_button_new(NULL, NULL);

            setup_toolbar_button(
                ret,
                FALSE,
                "Up",
                wintc_icon_name_first_available(
                    S_TOOLBAR_ICON_SIZE,
                    "go-up",
                    "go-parent-directory",
                    NULL
                )
            );

            gtk_actionable_set_action_name(
                GTK_ACTIONABLE(ret),
                "win.nav-up"
            );

            break;

        // TODO: Implement this
        //
        case 'v':
            ret = gtk_menu_tool_button_new(NULL, NULL);

            setup_toolbar_button(
                ret,
                FALSE,
                "Views",
                wintc_icon_name_first_available(
                    S_TOOLBAR_ICON_SIZE,
                    "video-display",
                    "views",
                    NULL
                )
            );

            break;

        case '|':
            ret = gtk_separator_tool_item_new();

            gtk_separator_tool_item_set_draw(
                GTK_SEPARATOR_TOOL_ITEM(ret),
                TRUE
            );

            break;

        default:
            g_warning("explorer: std toolbar unknown button %c", c);
            break;
    }

    return ret;
}

static GtkToolItem* find_toolbar_item(
    WinTCExpStandardToolbar* toolbar_std,
    gchar                    c
)
{
    WinTCExplorerToolbar* toolbar = WINTC_EXPLORER_TOOLBAR(toolbar_std);

    // We locate the button based on its position in the toolbar string
    //
    gint pos = -1;

    WinTCExplorerWindowMode mode =
        wintc_explorer_window_get_mode(
            WINTC_EXPLORER_WINDOW(toolbar->owner_explorer_wnd)
        );

    switch (mode)
    {
        case WINTC_EXPLORER_WINDOW_MODE_LOCAL:
            pos = strchr(S_LAYOUT_LOCAL, c) - S_LAYOUT_LOCAL;
            break;

        case WINTC_EXPLORER_WINDOW_MODE_INTERNET:
            pos = strchr(S_LAYOUT_INTERNET, c) - S_LAYOUT_INTERNET;
            break;

        default:
            g_critical("explorer: stdbar unknown explorer mode %d", mode);
            break;
    }

    if (pos < 0)
    {
        return NULL;
    }

    // Grab the toolbar widget
    //
    GList*       children = gtk_container_get_children(
                                GTK_CONTAINER(toolbar->toolbar)
                            );
    GtkToolItem* found    = GTK_TOOL_ITEM(
                                g_list_nth_data(children, (guint) pos)
                            );

    g_list_free(children);

    return found;
}

static void populate_toolbar(
    WinTCExpStandardToolbar* toolbar_std,
    const gchar*             config_str
)
{
    GtkWidget* toolbar = (WINTC_EXPLORER_TOOLBAR(toolbar_std))->toolbar;

    wintc_container_clear(GTK_CONTAINER(toolbar));

    WINTC_LOG_DEBUG("explorer: populating std toolbar with %s", config_str);

    for (const gchar* p = config_str; *p; p++)
    {
        gtk_toolbar_insert(
            GTK_TOOLBAR(toolbar),
            create_toolbar_item_from_char(toolbar_std, *p),
            -1
        );
    }

    gtk_widget_show_all(toolbar);
}

static void repopulate_toolbar(
    WinTCExpStandardToolbar* toolbar_std
)
{
    WinTCExplorerToolbar* toolbar = WINTC_EXPLORER_TOOLBAR(toolbar_std);

    // Populate toolbar based on mode
    //
    WinTCExplorerWindowMode mode =
        wintc_explorer_window_get_mode(
            WINTC_EXPLORER_WINDOW(toolbar->owner_explorer_wnd)
        );

    switch (mode)
    {
        case WINTC_EXPLORER_WINDOW_MODE_LOCAL:
            populate_toolbar(toolbar_std, S_LAYOUT_LOCAL);
            break;

        case WINTC_EXPLORER_WINDOW_MODE_INTERNET:
            populate_toolbar(toolbar_std, S_LAYOUT_INTERNET);
            break;

        default:
            g_critical("explorer: stdbar unknown explorer mode %d", mode);
            break;
    }

    sync_sidebar_buttons(toolbar_std);
}

static void setup_toolbar_button(
    GtkToolItem* tool_button,
    gboolean     is_important,
    const gchar* label,
    const gchar* icon_name
)
{
    if (icon_name)
    {
        gtk_tool_button_set_icon_name(
            GTK_TOOL_BUTTON(tool_button),
            icon_name
        );
    }

    if (label)
    {
        gtk_tool_button_set_label(
            GTK_TOOL_BUTTON(tool_button),
            label
        );
        gtk_widget_set_tooltip_text(
            GTK_WIDGET(tool_button),
            label
        );
    }

    gtk_tool_item_set_is_important(tool_button, is_important);
}

static void sync_sidebar_buttons(
    WinTCExpStandardToolbar* toolbar_std
)
{
    WinTCExplorerToolbar* toolbar = WINTC_EXPLORER_TOOLBAR(toolbar_std);

    toolbar_std->synchronizing = TRUE;

    // Identify the corresponding toolbar button, if any
    //
    gchar*       active_sidebar_id;
    GtkToolItem* to_activate = NULL;

    g_object_get(
        toolbar->owner_explorer_wnd,
        "active-sidebar", &active_sidebar_id,
        NULL
    );

    if (
        g_strcmp0(active_sidebar_id, WINTC_EXPLORER_SIDEBAR_ID_FAVORITES) == 0
    )
    {
        to_activate = find_toolbar_item(toolbar_std, 'F');
    }
    else if (
        g_strcmp0(active_sidebar_id, WINTC_EXPLORER_SIDEBAR_ID_FOLDERS) == 0
    )
    {
        to_activate = find_toolbar_item(toolbar_std, 'd');
    }
    else if (
        g_strcmp0(active_sidebar_id, WINTC_EXPLORER_SIDEBAR_ID_SEARCH) == 0
    )
    {
        to_activate = find_toolbar_item(toolbar_std, 's');
    }

    g_free(active_sidebar_id);

    // Iterate over the toolbar items to sync the toggle buttons
    //
    GList* children =
        gtk_container_get_children(GTK_CONTAINER(toolbar->toolbar));

    for (GList* iter = children; iter; iter = iter->next)
    {
        if (iter->data == to_activate)
        {
            continue;
        }

        if (GTK_IS_TOGGLE_TOOL_BUTTON(iter->data))
        {
            gtk_toggle_tool_button_set_active(
                GTK_TOGGLE_TOOL_BUTTON(iter->data),
                FALSE
            );
        }
    }

    g_list_free(children);

    if (to_activate)
    {
        gtk_toggle_tool_button_set_active(
            GTK_TOGGLE_TOOL_BUTTON(to_activate),
            TRUE
        );
    }
    
    toolbar_std->synchronizing = FALSE;
}

static void toggle_sidebar(
    WinTCExpStandardToolbar* toolbar_std,
    const gchar*             sidebar_id
)
{
    WinTCExplorerToolbar* toolbar = WINTC_EXPLORER_TOOLBAR(toolbar_std);

    if (toolbar_std->synchronizing)
    {
        return;
    }

    wintc_explorer_window_toggle_sidebar(
        WINTC_EXPLORER_WINDOW(toolbar->owner_explorer_wnd),
        sidebar_id
    );
}

//
// CALLBACKS
//
static void on_owner_explorer_wnd_notify_active_sidebar(
    WINTC_UNUSED(GObject* self),
    WINTC_UNUSED(GParamSpec* pspec),
    gpointer user_data
)
{
    WinTCExpStandardToolbar* toolbar_std =
        WINTC_EXP_STANDARD_TOOLBAR(user_data);

    sync_sidebar_buttons(toolbar_std);
}

static void on_owner_explorer_wnd_mode_changed(
    WINTC_UNUSED(GtkWidget* self),
    gpointer user_data
)
{
    // Just repopulate toolbar
    //
    repopulate_toolbar(WINTC_EXP_STANDARD_TOOLBAR(user_data));
}

static void on_toolbar_button_favorites_toggled(
    WINTC_UNUSED(GtkToggleToolButton* self),
    gpointer user_data
)
{
    WinTCExpStandardToolbar* toolbar_std =
        WINTC_EXP_STANDARD_TOOLBAR(user_data);

    toggle_sidebar(toolbar_std, WINTC_EXPLORER_SIDEBAR_ID_FAVORITES);
}

static void on_toolbar_button_folders_toggled(
    WINTC_UNUSED(GtkToggleToolButton* self),
    gpointer user_data
)
{
    WinTCExpStandardToolbar* toolbar_std =
        WINTC_EXP_STANDARD_TOOLBAR(user_data);

    toggle_sidebar(toolbar_std, WINTC_EXPLORER_SIDEBAR_ID_FOLDERS);
}

static void on_toolbar_button_search_toggled(
    WINTC_UNUSED(GtkToggleToolButton* self),
    gpointer user_data
)
{
    WinTCExpStandardToolbar* toolbar_std =
        WINTC_EXP_STANDARD_TOOLBAR(user_data);

    toggle_sidebar(toolbar_std, WINTC_EXPLORER_SIDEBAR_ID_SEARCH);
}

static void on_toolbar_style_updated(
    WINTC_UNUSED(GtkWidget* self),
    gpointer user_data
)
{
    // We repopulate the toolbar upon style change, potentially the icon theme
    // changed which requires us to look up the icons again etc. it's easier
    // to just reload the whole thing (I'm a lazy git once again)
    //
    repopulate_toolbar(WINTC_EXP_STANDARD_TOOLBAR(user_data));
}

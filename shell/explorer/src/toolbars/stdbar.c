#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../toolbar.h"
#include "../window.h"
#include "stdbar.h"

//
// FORWARD DECLARATIONS
//
static void wintc_exp_standard_toolbar_constructed(
    GObject* object
);

GtkToolItem* create_toolbar_button(
    gboolean     is_important,
    gboolean     is_menu_button,
    const gchar* label,
    const gchar* icon_name
);
GtkToolItem* create_toolbar_item_from_char(
    gchar c
);
void populate_toolbar(
    GtkWidget*   toolbar,
    const gchar* config_str
);
void repopulate_toolbar(
    WinTCExpStandardToolbar* toolbar_std
);

static void on_owner_explorer_wnd_mode_changed(
    GtkWidget* self,
    gpointer   user_data
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
GtkToolItem* create_toolbar_button(
    gboolean     is_important,
    gboolean     is_menu_button,
    const gchar* label,
    const gchar* icon_name
)
{
    GtkToolItem* tb;

    if (is_menu_button)
    {
        tb = gtk_menu_tool_button_new(NULL, NULL);
    }
    else
    {
        tb = gtk_tool_button_new(NULL, NULL);
    }

    if (icon_name)
    {
        gtk_tool_button_set_icon_name(
            GTK_TOOL_BUTTON(tb),
            icon_name
        );
    }

    if (label)
    {
        gtk_tool_button_set_label(
            GTK_TOOL_BUTTON(tb),
            label
        );
        gtk_widget_set_tooltip_text(
            GTK_WIDGET(tb),
            label
        );
    }

    gtk_tool_item_set_is_important(tb, is_important);

    return tb;
}

GtkToolItem* create_toolbar_item_from_char(
    gchar c
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
            ret =
                create_toolbar_button(
                    TRUE,
                    TRUE,
                    "Back",
                    wintc_icon_name_first_available(
                        S_TOOLBAR_ICON_SIZE,
                        "go-previous",
                        "history-back",
                        NULL
                    )
                );
            break;

        // TODO: Implement this
        //
        case 'C':
            ret =
                create_toolbar_button(
                    FALSE,
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
            ret =
                create_toolbar_button(
                    TRUE,
                    FALSE,
                    "Folders",
                    wintc_icon_name_first_available(
                        S_TOOLBAR_ICON_SIZE,
                        "inode-directory",
                        "folders",
                        NULL
                    )
                );
            break;

        // TODO: Implement this
        //
        case 'E':
            ret =
                create_toolbar_button(
                    FALSE,
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
            ret =
                create_toolbar_button(
                    TRUE,
                    FALSE,
                    "Favorites",
                    "emblem-favorite"
                );
            break;

        // TODO: Implement this
        //
        case 'f':
            // TODO: See back button tooltip about history, but for Forward,
            //       since it has no label, the tooltip defaults to Forward
            ret =
                create_toolbar_button(
                    FALSE,
                    FALSE,
                    "Forward",
                    wintc_icon_name_first_available(
                        S_TOOLBAR_ICON_SIZE,
                        "go-next",
                        "history-forward",
                        NULL
                    )
                );
            break;

        // TODO: Implement this
        //
        case 'H':
            ret =
                create_toolbar_button(
                    FALSE,
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
            ret =
                create_toolbar_button(
                    FALSE,
                    FALSE,
                    "Home",
                    "go-home"
                );
            break;

        // TODO: Implement this
        // 
        case 'M':
            ret =
                create_toolbar_button(
                    FALSE,
                    TRUE,
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
            ret =
                create_toolbar_button(
                    FALSE,
                    FALSE,
                    "Print",
                    "printer"
                );
            break;

        // TODO: Implement this
        //
        case 'R':
            ret =
                create_toolbar_button(
                    FALSE,
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
            ret =
                create_toolbar_button(
                    FALSE,
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
            ret =
                create_toolbar_button(
                    TRUE,
                    FALSE,
                    "Search",
                    wintc_icon_name_first_available(
                        S_TOOLBAR_ICON_SIZE,
                        "system-search",
                        "search",
                        NULL
                    )
                );
            break;

        case 'u':
            ret =
                create_toolbar_button(
                    FALSE,
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
            ret =
                create_toolbar_button(
                    FALSE,
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

void populate_toolbar(
    GtkWidget*   toolbar,
    const gchar* config_str
)
{
    wintc_container_clear(GTK_CONTAINER(toolbar));

    WINTC_LOG_DEBUG("explorer: populating std toolbar with %s", config_str);

    for (const gchar* p = config_str; *p; p++)
    {
        gtk_toolbar_insert(
            GTK_TOOLBAR(toolbar),
            create_toolbar_item_from_char(*p),
            -1
        );
    }

    gtk_widget_show_all(toolbar);
}

void repopulate_toolbar(
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
            populate_toolbar(GTK_WIDGET(toolbar->toolbar), S_LAYOUT_LOCAL);
            break;

        case WINTC_EXPLORER_WINDOW_MODE_INTERNET:
            populate_toolbar(GTK_WIDGET(toolbar->toolbar), S_LAYOUT_INTERNET);
            break;

        default:
            g_critical("explorer: stdbar unknown explorer mode %d", mode);
            break;
    }
}

//
// CALLBACKS
//
static void on_owner_explorer_wnd_mode_changed(
    WINTC_UNUSED(GtkWidget* self),
    gpointer user_data
)
{
    // Just repopulate toolbar
    //
    repopulate_toolbar(WINTC_EXP_STANDARD_TOOLBAR(user_data));
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

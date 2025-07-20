#include <garcon/garcon.h>
#include <garcon-gtk/garcon-gtk.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <pwd.h>
#include <sys/types.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>
#include <wintc/shelldpa.h>
#include <wintc/shlang.h>

#include "../toolbar.h"
#include "menumod.h"
#include "personal.h"
#include "progmenu.h"
#include "shared.h"
#include "toolbar.h"
#include "util.h"

#define DEFAULT_ITEM_COUNT 2
#define MAX_MFU_ITEM_COUNT 6
#define TOTAL_PROGRAMS_ITEM_COUNT (DEFAULT_ITEM_COUNT + MAX_MFU_ITEM_COUNT)

#define PLACE_ICON_SIZE   24
#define PROGRAM_ICON_SIZE 32

//
// PRIVATE STRUCTS
//
typedef struct _StartSignalTuple
{
    WinTCToolbarStart* toolbar_start;
    gboolean           is_action;
    gpointer           user_data;
} StartSignalTuple;

//
// FORWARD DECLARATIONS
//
static void connect_menu_shell_signals(
    GtkMenuShell*      menu_shell,
    WinTCToolbarStart* toolbar_start
);
static GtkWidget* create_personal_menu_item(
    GtkMenuShell* menu_shell,
    const gchar*  icon_name,
    const gchar*  program_name,
    const gchar*  comment,
    const gchar*  generic_name
);
static GtkWidget* create_personal_menu_item_from_desktop_entry(
    GDesktopAppInfo*  entry,
    StartSignalTuple* signal_tuple,
    const gchar*      comment,
    const gchar*      generic_name
);
static GtkWidget* create_personal_menu_item_from_garcon_item(
    GarconMenuItem*   garcon_item,
    StartSignalTuple* signal_tuple
);
static void refresh_personal_menu(
    WinTCToolbarStart* toolbar_start
);
static void refresh_userpic(
    WinTCToolbarStart* toolbar_start
);

static void clear_signal_tuple(
    StartSignalTuple* tuple
);

static gboolean recent_filter_exclude_directories(
    const GtkRecentFilterInfo* filter_info,
    gpointer                   user_data
);

static void on_button_power_clicked(
    GtkButton* button,
    gpointer   user_data
);
static void on_event_box_userpic_clicked(
    GtkWidget*      widget,
    GdkEventButton* event,
    gpointer        user_data
);
static void on_menu_item_launcher_activate(
    GtkMenuItem* self,
    gpointer     user_data
);
static void on_menu_item_recent_activated(
    GtkRecentChooser* self,
    gpointer          user_data
);
static void on_menu_item_with_submenu_deselect(
    GtkMenuItem* self,
    gpointer     user_data
);
static void on_menu_shell_selection_done(
    GtkMenuShell* self,
    gpointer      user_data
);
static void on_menu_shell_submenu_selection_done(
    GtkMenuShell* self,
    gpointer      user_data
);
static void on_personal_menu_hide(
    GtkWidget* self,
    gpointer   user_data
);

//
// INTERNAL FUNCTIONS
//
void close_personal_menu(
    WinTCToolbarStart* toolbar_start
)
{
    gtk_widget_hide(toolbar_start->personal.popup_menu);
}

void create_personal_menu(
    WinTCToolbarStart* toolbar_start
)
{
    GtkBuilder* builder;
    WinTCTaskbandToolbar* toolbar = WINTC_TASKBAND_TOOLBAR(toolbar_start);

    // Set default states
    //
    toolbar_start->personal.sync_menu_refresh = TRUE;

    // Init GArrays for signal tuples
    //
    toolbar_start->personal.tuples_places   = g_array_new(
                                                  FALSE,
                                                  TRUE,
                                                  sizeof(StartSignalTuple)
                                              );
    toolbar_start->personal.tuples_programs = g_array_new(
                                                  FALSE,
                                                  TRUE,
                                                  sizeof(StartSignalTuple)
                                              );

    g_array_set_clear_func(
        toolbar_start->personal.tuples_programs,
        (GDestroyNotify) clear_signal_tuple
    );

    // Ensure our modded menu type is known to GObject
    //
    g_type_ensure(WINTC_TYPE_MENU_MODDED);

    // Construct menu from XML
    //
    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/taskband/personal-menu.ui"
        );

    wintc_lc_builder_preprocess_widget_text(builder);

    // Pull UI elements we store for later usage
    //
    toolbar_start->personal.button_logoff =
        GTK_WIDGET(gtk_builder_get_object(builder, "button-logoff"));
    toolbar_start->personal.button_shutdown =
        GTK_WIDGET(gtk_builder_get_object(builder, "button-shutdown"));
    toolbar_start->personal.eventbox_userpic =
        GTK_WIDGET(gtk_builder_get_object(builder, "eventbox-userpic"));
    toolbar_start->personal.menubar_places =
        GTK_WIDGET(gtk_builder_get_object(builder, "menubar-places"));
    toolbar_start->personal.menubar_programs =
        GTK_WIDGET(gtk_builder_get_object(builder, "menubar-programs"));
    toolbar_start->personal.menuitem_all_programs =
        GTK_WIDGET(gtk_builder_get_object(builder, "menuitem-all-programs"));
    toolbar_start->personal.separator_all_programs =
        GTK_WIDGET(gtk_builder_get_object(builder, "separator-all-programs"));

    // Sort out user pic
    //
    toolbar_start->personal.style_userpic =
        GTK_STYLE_PROVIDER(gtk_css_provider_new());

    gtk_style_context_add_provider(
        gtk_widget_get_style_context(
            gtk_widget_get_parent(toolbar_start->personal.eventbox_userpic)
        ),
        toolbar_start->personal.style_userpic,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    gtk_widget_set_events(
        toolbar_start->personal.eventbox_userpic,
        GDK_BUTTON_PRESS_MASK
    );

    g_signal_connect(
        toolbar_start->personal.eventbox_userpic,
        "button-press-event",
        G_CALLBACK(on_event_box_userpic_clicked),
        NULL
    );

    refresh_userpic(toolbar_start);

    // Set username labels
    //
    struct passwd* user_pwd = getpwuid(getuid());

    gtk_label_set_text(
        GTK_LABEL(gtk_builder_get_object(builder, "label-username-horz")),
        user_pwd->pw_name
    );
    gtk_label_set_text(
        GTK_LABEL(gtk_builder_get_object(builder, "label-username-vert")),
        user_pwd->pw_name
    );

    // Attach Recent Documents submenu
    //
    GtkWidget*       menu_recents  = gtk_recent_chooser_menu_new();
    GtkRecentFilter* recent_filter = gtk_recent_filter_new();

    gtk_recent_filter_add_custom(
        recent_filter,
        GTK_RECENT_FILTER_MIME_TYPE,
        (GtkRecentFilterFunc) recent_filter_exclude_directories,
        NULL,
        NULL
    );

    gtk_recent_chooser_set_filter(
        GTK_RECENT_CHOOSER(menu_recents),
        recent_filter
    );
    gtk_recent_chooser_set_sort_type(
        GTK_RECENT_CHOOSER(menu_recents),
        GTK_RECENT_SORT_MRU
    );

    gtk_menu_item_set_submenu(
        GTK_MENU_ITEM(gtk_builder_get_object(builder, "menuitem-recent-docs")),
        menu_recents
    );

    g_signal_connect(
        menu_recents,
        "item-activated",
        G_CALLBACK(on_menu_item_recent_activated),
        toolbar_start
    );

    // Attach All Programs submenu
    //
    gtk_menu_item_set_submenu(
        GTK_MENU_ITEM(toolbar_start->personal.menuitem_all_programs),
        wintc_toolbar_start_progmenu_new_gtk_menu(
            toolbar_start->progmenu,
            &(toolbar_start->personal.all_programs_binding)
        )
    );

    // Transfer to popup
    //
    toolbar_start->personal.popup_menu =
        wintc_dpa_create_popup(toolbar->widget_root, TRUE);

    wintc_widget_add_style_class(
        toolbar_start->personal.popup_menu,
        "wintc-personal-menu"
    );

    gtk_container_add(
        GTK_CONTAINER(toolbar_start->personal.popup_menu),
        GTK_WIDGET(gtk_builder_get_object(builder, "main-box"))
    );

    g_object_unref(G_OBJECT(builder));

    g_signal_connect(
        toolbar_start->personal.popup_menu,
        "hide",
        G_CALLBACK(on_personal_menu_hide),
        toolbar_start
    );

    // Take extra references to All Programs items, so we can safely take
    // them out of the menu bar whilst refreshing without them being disposed
    //
    g_object_ref(toolbar_start->personal.menuitem_all_programs);
    g_object_ref(toolbar_start->personal.separator_all_programs);

    // Attach signals
    //
    connect_menu_shell_signals(
        GTK_MENU_SHELL(toolbar_start->personal.menubar_places),
        toolbar_start
    );
    connect_menu_shell_signals(
        GTK_MENU_SHELL(toolbar_start->personal.menubar_programs),
        toolbar_start
    );

    // Attach actions for places - the order of WinTCAction is the same as
    // these Start menu items
    //
    StartSignalTuple* tuple;
    GList* elements;
    GList* li;
    gint   i = 0;

    elements =
        gtk_container_get_children(
            GTK_CONTAINER(toolbar_start->personal.menubar_places)
        );
    li       = elements;

    g_array_set_size(
        toolbar_start->personal.tuples_places,
        g_list_length(elements)
    );

    while (li)
    {
        GtkMenuItem* menu_item = GTK_MENU_ITEM(li->data);

        // Skip separators
        //
        if (!GTK_IS_SEPARATOR_MENU_ITEM(menu_item))
        {
            // Only connect activation for items without submenus
            //
            if (!gtk_menu_item_get_submenu(menu_item))
            {
                tuple =
                    &g_array_index(
                        toolbar_start->personal.tuples_places,
                        StartSignalTuple,
                        i
                    );

                tuple->is_action     = TRUE;
                tuple->toolbar_start = toolbar_start;
                tuple->user_data     = GINT_TO_POINTER(i);

                g_signal_connect(
                    menu_item,
                    "activate",
                    G_CALLBACK(on_menu_item_launcher_activate),
                    tuple
                );
            }

            i++;
        }

        li = li->next;
    }

    g_list_free(elements);

    // Attach power button signals
    //
    g_signal_connect(
        toolbar_start->personal.button_logoff,
        "clicked",
        G_CALLBACK(on_button_power_clicked),
        GINT_TO_POINTER(WINTC_ACTION_LOGOFF)
    );
    g_signal_connect(
        toolbar_start->personal.button_shutdown,
        "clicked",
        G_CALLBACK(on_button_power_clicked),
        GINT_TO_POINTER(WINTC_ACTION_SHUTDOWN)
    );
}

void destroy_personal_menu(
    WinTCToolbarStart* toolbar_start
)
{
    // Unref extra references on all programs items
    //
    g_object_unref(
        g_steal_pointer(&(toolbar_start->personal.menuitem_all_programs))
    );
    g_object_unref(
        g_steal_pointer(&(toolbar_start->personal.separator_all_programs))
    );

    // Clear all programs menu model binding object
    //
    g_clear_object(
        &(toolbar_start->personal.all_programs_binding)
    );

    // Clear signal tuple data
    //
    g_array_unref(
        g_steal_pointer(&(toolbar_start->personal.tuples_places))
    );
    g_array_unref(
        g_steal_pointer(&(toolbar_start->personal.tuples_programs))
    );

    // Destroy popup
    //
    gtk_widget_destroy(
        g_steal_pointer(&(toolbar_start->personal.popup_menu))
    );
}

void open_personal_menu(
    WinTCToolbarStart* toolbar_start
)
{
    WinTCTaskbandToolbar* toolbar = WINTC_TASKBAND_TOOLBAR(toolbar_start);

    // Reset should close flag, so that selection-done knows no item has
    // actually been 'activated' yet
    //
    toolbar_start->sync_menu_should_close = FALSE;

    if (toolbar_start->personal.sync_menu_refresh)
    {
        refresh_personal_menu(toolbar_start);
    }

    wintc_dpa_show_popup(
        toolbar_start->personal.popup_menu,
        toolbar->widget_root
    );
}

//
// PRIVATE FUNCTIONS
//
static void connect_menu_shell_signals(
    GtkMenuShell*      menu_shell,
    WinTCToolbarStart* toolbar_start
)
{
    GtkWidget* child;
    GList*     children = gtk_container_get_children(
                              GTK_CONTAINER(menu_shell)
                          );
    GList*     li       = children;

    while (li)
    {
        child = li->data;

        // Connect enter/leave events for auto-focusing the menu shell
        //
        g_signal_connect(
            child,
            "enter-notify-event",
            G_CALLBACK(wintc_menu_shell_select_on_enter),
            menu_shell
        );
        g_signal_connect(
            child,
            "leave-notify-event",
            G_CALLBACK(wintc_menu_shell_deselect_on_leave),
            menu_shell
        );

        // If the item has a sub-menu, apply a hacks to deal with the funny
        // pop-up menu behaviour (if we deselect the menu ourselves, it avoids
        // the issue of the submenu somehow 'detaching' itself)
        //
        if (gtk_menu_item_get_submenu(GTK_MENU_ITEM(child)))
        {
            g_signal_connect(
                child,
                "deselect",
                G_CALLBACK(on_menu_item_with_submenu_deselect),
                NULL
            );
            g_signal_connect(
                gtk_menu_item_get_submenu(GTK_MENU_ITEM(child)),
                "selection-done",
                G_CALLBACK(on_menu_shell_submenu_selection_done),
                toolbar_start
            );
        }

        li = li->next;
    }

    g_list_free(g_steal_pointer(&children));

    g_signal_connect(
        menu_shell,
        "selection-done",
        G_CALLBACK(on_menu_shell_selection_done),
        toolbar_start
    );
}

static GtkWidget* create_personal_menu_item(
    GtkMenuShell* menu_shell,
    const gchar*  icon_name,
    const gchar*  program_name,
    const gchar*  comment,
    const gchar*  generic_name
)
{
    GtkWidget* box           = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget* image_icon    = gtk_image_new();
    GtkWidget* label_program = gtk_label_new(program_name);
    GtkWidget* menu_item     = gtk_menu_item_new();

    // Attempt to load the icon...
    //
    GdkPixbuf* pixbuf_icon =
        gtk_icon_theme_load_icon(
            gtk_icon_theme_get_default(),
            icon_name ? icon_name : "application-x-generic",
            PROGRAM_ICON_SIZE,
            GTK_ICON_LOOKUP_FORCE_SIZE,
            NULL
        );

    if (!pixbuf_icon)
    {
        gtk_icon_theme_load_icon(
            gtk_icon_theme_get_default(),
            "application-x-generic",
            PROGRAM_ICON_SIZE,
            GTK_ICON_LOOKUP_FORCE_SIZE,
            NULL
        );
    }

    if (pixbuf_icon)
    {
        gtk_image_set_from_pixbuf(
            GTK_IMAGE(image_icon),
            pixbuf_icon
        );

        g_object_unref(pixbuf_icon);
    }

    // Set up label properties
    //
    gtk_label_set_line_wrap(
        GTK_LABEL(label_program),
        TRUE
    );
    gtk_label_set_max_width_chars(
        GTK_LABEL(label_program),
        24
    );
    gtk_label_set_xalign(
        GTK_LABEL(label_program),
        0.0
    );

    // Ensure image widget always requests the right size, in case no icon was
    // loaded
    //
    gtk_widget_set_size_request(
        image_icon,
        PROGRAM_ICON_SIZE,
        PROGRAM_ICON_SIZE
    );

    // Pack icon first...
    //
    gtk_box_pack_start(
        GTK_BOX(box),
        image_icon,
        FALSE,
        FALSE,
        0
    );

    // ...then pack the program name/heading...
    //
    if (generic_name)
    {
        GtkWidget* inner_box     = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        GtkWidget* label_generic = gtk_label_new(generic_name);

        wintc_widget_add_style_class(
            menu_item,
            "start-default-item"
        );

        gtk_label_set_xalign(
            GTK_LABEL(label_generic),
            0.0
        );
        gtk_widget_set_valign(inner_box, GTK_ALIGN_CENTER);

        gtk_box_pack_start(
            GTK_BOX(inner_box),
            label_generic,
            TRUE,
            TRUE,
            0
        );
        gtk_box_pack_start(
            GTK_BOX(inner_box),
            label_program,
            TRUE,
            TRUE,
            0
        );

        gtk_box_pack_start(
            GTK_BOX(box),
            inner_box,
            TRUE,
            TRUE,
            0
        );
    }
    else
    {
        gtk_box_pack_start(
            GTK_BOX(box),
            label_program,
            FALSE,
            FALSE,
            0
        );
    }

    gtk_container_add(
        GTK_CONTAINER(menu_item),
        box
    );

    // ...finally add the comment to the menu item tooltip
    //
    if (comment)
    {
        gchar* real_comment = wintc_str_set_suffix(comment, ".");

        gtk_widget_set_tooltip_text(menu_item, real_comment);

        g_free(real_comment);
    }

    // Add enter/leave signals for auto-selecting menushell
    //
    g_signal_connect(
        menu_item,
        "enter-notify-event",
        G_CALLBACK(wintc_menu_shell_select_on_enter),
        menu_shell
    );
    g_signal_connect(
        menu_item,
        "leave-notify-event",
        G_CALLBACK(wintc_menu_shell_deselect_on_leave),
        menu_shell
    );

    return menu_item;
}

static GtkWidget* create_personal_menu_item_from_desktop_entry(
    GDesktopAppInfo*  entry,
    StartSignalTuple* signal_tuple,
    const gchar*      comment,
    const gchar*      generic_name
)
{
    GtkWidget* menu_item;

    if (entry)
    {
        GAppInfo* app_info  = G_APP_INFO(entry);
        gchar*    icon_name = g_desktop_app_info_get_string(entry, "Icon");

        if (!icon_name) // If no icon, try the executable name...
        {
            icon_name =
                g_path_get_basename(
                    g_app_info_get_executable(app_info)
                );
        }

        menu_item =
            create_personal_menu_item(
                GTK_MENU_SHELL(
                    signal_tuple->toolbar_start->personal.menubar_programs
                ),
                icon_name,
                g_app_info_get_name(app_info),
                comment,
                generic_name
            );

        signal_tuple->is_action = FALSE;
        signal_tuple->user_data = wintc_desktop_app_info_get_command(entry);

        g_signal_connect(
            menu_item,
            "activate",
            G_CALLBACK(on_menu_item_launcher_activate),
            signal_tuple
        );

        g_free(icon_name);
    }
    else
    {
        menu_item =
            create_personal_menu_item(
                GTK_MENU_SHELL(
                    signal_tuple->toolbar_start->personal.menubar_programs
                ),
                "dialog-warning",
                _("Click to specify a default"),
                comment,
                generic_name
            );

        signal_tuple->is_action = TRUE;
        signal_tuple->user_data = GINT_TO_POINTER(WINTC_ACTION_MIMEMGMT);

        g_signal_connect(
            menu_item,
            "activate",
            G_CALLBACK(on_menu_item_launcher_activate),
            signal_tuple
        );
    }

    return menu_item;
}

static GtkWidget* create_personal_menu_item_from_garcon_item(
    GarconMenuItem*   garcon_item,
    StartSignalTuple* signal_tuple
)
{
    GtkWidget* menu_item =
        create_personal_menu_item(
            GTK_MENU_SHELL(
                signal_tuple->toolbar_start->personal.menubar_programs
            ),
            garcon_menu_item_get_icon_name(garcon_item),
            garcon_menu_item_get_name(garcon_item),
            garcon_menu_item_get_comment(garcon_item),
            NULL
        );

    signal_tuple->is_action = FALSE;
    signal_tuple->user_data =
        garcon_menu_item_get_command_expanded(garcon_item);

    g_signal_connect(
        menu_item,
        "activate",
        G_CALLBACK(on_menu_item_launcher_activate),
        signal_tuple
    );

    return menu_item;
}

static void refresh_personal_menu(
    WinTCToolbarStart* toolbar_start
)
{
    // Temporarily remove All Programs items from meubar (safe because we
    // took extra references to them already)
    //
    gtk_container_remove(
        GTK_CONTAINER(toolbar_start->personal.menubar_programs),
        toolbar_start->personal.menuitem_all_programs
    );
    gtk_container_remove(
        GTK_CONTAINER(toolbar_start->personal.menubar_programs),
        toolbar_start->personal.separator_all_programs
    );

    // Clear existing items
    //
    wintc_container_clear(
        GTK_CONTAINER(toolbar_start->personal.menubar_programs)
    );

    // Set up signal tuple array
    //
    g_array_remove_range(
        toolbar_start->personal.tuples_programs,
        0,
        toolbar_start->personal.tuples_programs->len
    );

    g_array_set_size(
        toolbar_start->personal.tuples_programs,
        TOTAL_PROGRAMS_ITEM_COUNT
    );

    // Add default items
    //
    GDesktopAppInfo*  entry_internet = wintc_query_mime_handler(
                                           "x-scheme-handler/http",
                                           NULL
                                       );
    GDesktopAppInfo*  entry_email    = wintc_query_mime_handler(
                                           "x-scheme-handler/mailto",
                                           NULL
                                       );
    StartSignalTuple* tuple;

    tuple =
        &g_array_index(
            toolbar_start->personal.tuples_programs,
            StartSignalTuple,
            0
        );
    tuple->toolbar_start = toolbar_start;

    gtk_menu_shell_append(
        GTK_MENU_SHELL(toolbar_start->personal.menubar_programs),
        create_personal_menu_item_from_desktop_entry(
            entry_internet,
            tuple,
            _("Opens your Internet browser."),
            _("Internet")
        )
    );

    tuple =
        &g_array_index(
            toolbar_start->personal.tuples_programs,
            StartSignalTuple,
            1
        );
    tuple->toolbar_start = toolbar_start;

    gtk_menu_shell_append(
        GTK_MENU_SHELL(toolbar_start->personal.menubar_programs),
        create_personal_menu_item_from_desktop_entry(
            entry_email,
            tuple,
            _("Opens your e-mail program so you can send or read a message."),
            _("E-mail")
        )
    );

    g_object_unref(entry_internet);
    g_object_unref(entry_email);

    // Add separator between defaults & MFU
    //
    gtk_menu_shell_append(
        GTK_MENU_SHELL(toolbar_start->personal.menubar_programs),
        gtk_separator_menu_item_new()
    );

    // Add MFU items
    // FIXME: In future we will need some >>>algorithm<<< to come up with a
    //        'top' programs list to display here, and also reserve the last
    //        slot for the latest newly added program, if any
    //
    //        For now we simply grab the first items that are pulled in via the
    //        all.menu file
    //
    GarconMenu* all_entries = garcon_menu_new_for_path(
                                  WINTC_ASSETS_DIR "/shell-res/all.menu"
                              );
    GError*     error       = NULL;

    if (!garcon_menu_load(all_entries, NULL, &error))
    {
        wintc_display_error_and_clear(&error, NULL);
        return;
    }

    // Loop over to add the items
    //
    GList*      elements = garcon_menu_get_elements(all_entries);
    GList* li = elements;
    gint   i  = 0;

    while (i < MAX_MFU_ITEM_COUNT && li != NULL)
    {
        tuple =
            &g_array_index(
                toolbar_start->personal.tuples_programs,
                StartSignalTuple,
                DEFAULT_ITEM_COUNT + i
            );
        tuple->toolbar_start = toolbar_start;

        gtk_menu_shell_append(
            GTK_MENU_SHELL(toolbar_start->personal.menubar_programs),
            create_personal_menu_item_from_garcon_item(
                GARCON_MENU_ITEM(li->data),
                tuple
            )
        );

        i++;
        li = li->next;
    }

    g_list_free(g_steal_pointer(&elements));

    // Re-append All Programs items
    //
    gtk_menu_shell_append(
        GTK_MENU_SHELL(toolbar_start->personal.menubar_programs),
        toolbar_start->personal.separator_all_programs
    );
    gtk_menu_shell_append(
        GTK_MENU_SHELL(toolbar_start->personal.menubar_programs),
        toolbar_start->personal.menuitem_all_programs
    );

    // Successfully refreshed the menu
    //
    toolbar_start->personal.sync_menu_refresh = FALSE;
}

static void refresh_userpic(
    WinTCToolbarStart* toolbar_start
)
{
    // TODO: Read from AccountsService or whatever on DBus? Default to the
    //       flower pic -- for now we're just displaying the FPO image
    //
    static const gchar* css =
        "* { background-image: url('"
        WINTC_ASSETS_DIR "/shell-res/fpo-userpic.png"
        "'); }";

    // Give GTK a bump that we want to update the pic
    //
    gtk_css_provider_load_from_data(
        GTK_CSS_PROVIDER(toolbar_start->personal.style_userpic),
        css,
        -1,
        NULL
    );
}

//
// CALLBACKS
//
static void clear_signal_tuple(
    StartSignalTuple* tuple
)
{
    if (!(tuple->is_action))
    {
        g_clear_pointer(&(tuple->user_data), g_free);
    }
}

static gboolean recent_filter_exclude_directories(
    const GtkRecentFilterInfo* filter_info,
    WINTC_UNUSED(gpointer user_data)
)
{
    return g_strcmp0(filter_info->mime_type, "inode/directory") != 0;
}

static void on_button_power_clicked(
    GtkButton* button,
    gpointer   user_data
)
{
    GError* error = NULL;

    gtk_widget_hide(
        GTK_WIDGET(
            wintc_widget_get_toplevel_window(GTK_WIDGET(button))
        )
    );

    if (!wintc_launch_action(GPOINTER_TO_INT(user_data), &error))
    {
        if (
            error->domain == WINTC_EXEC_ERROR &&
            error->code   == WINTC_EXEC_ERROR_FELLTHRU
        )
        {
            g_clear_error(&error);

            // Localise
            //
            wintc_messagebox_show(
                NULL,
                "Unable to find any programs for exiting the session.",
                "Windows",
                GTK_BUTTONS_OK,
                GTK_MESSAGE_ERROR
            );
        }
        else
        {
            wintc_nice_error_and_clear(&error, NULL);
        }
    }
}

static void on_event_box_userpic_clicked(
    WINTC_UNUSED(GtkWidget* widget),
    GdkEventButton* event,
    WINTC_UNUSED(gpointer   user_data)
)
{
    //
    // TODO: Implement this when the user pic cpl is done!
    //
    if (event->button > 1)
    {
        return;
    }

    GError* error = NULL;

    g_set_error(
        &error,
        WINTC_GENERAL_ERROR,
        WINTC_GENERAL_ERROR_NOTIMPL,
        "Cannot edit user pic yet!"
    );

    wintc_nice_error_and_clear(&error, NULL);
}

static void on_menu_item_launcher_activate(
    WINTC_UNUSED(GtkMenuItem* self),
    gpointer user_data
)
{
    StartSignalTuple* tuple = (StartSignalTuple*) user_data;

    // Raise flag to let Start menu know to close, and launch the action
    //
    tuple->toolbar_start->sync_menu_should_close = TRUE;

    // Launch either the associated action or cmdline depending on what's in
    // the tuple
    //
    GError* error = NULL;

    if (tuple->is_action)
    {
        wintc_launch_action(GPOINTER_TO_INT(tuple->user_data), &error);
    }
    else
    {
        wintc_launch_command(tuple->user_data, &error);
    }

    if (error)
    {
        wintc_nice_error_and_clear(&error, NULL);
    }
}

static void on_menu_item_recent_activated(
    GtkRecentChooser* self,
    gpointer          user_data
)
{
    WinTCToolbarStart* toolbar_start = WINTC_TOOLBAR_START(user_data);

    toolbar_start->sync_menu_should_close = TRUE;

    // Launch the specified URI
    //
    GError* error = NULL;
    gchar*  uri   = gtk_recent_chooser_get_current_uri(self);

    wintc_launch_command(uri, &error);

    g_free(uri);
}

static void on_menu_item_with_submenu_deselect(
    GtkMenuItem* self,
    WINTC_UNUSED(gpointer user_data)
)
{
    // HACK: I suppose infighting with select-on-enter... we popdown the menu
    //       ourselves when the item is deselected, otherwise for some asinine
    //       reason the menu has a chance to get stuck open, in a detached
    //       state
    //
    //       This is still a bit glitchy if someone spam clicks the submenu
    //       item, but it's better than the alternative
    //
    gtk_menu_popdown(
        GTK_MENU(gtk_menu_item_get_submenu(self))
    );
}

static void on_menu_shell_selection_done(
    WINTC_UNUSED(GtkMenuShell* self),
    gpointer user_data
)
{
    WinTCToolbarStart* toolbar_start = WINTC_TOOLBAR_START(user_data);

    if (!toolbar_start->sync_menu_should_close)
    {
        return;
    }

    gtk_widget_hide(
        toolbar_start->personal.popup_menu
    );
}

static void on_menu_shell_submenu_selection_done(
    WINTC_UNUSED(GtkMenuShell* self),
    gpointer user_data
)
{
    WinTCToolbarStart* toolbar_start = WINTC_TOOLBAR_START(user_data);

    toolbar_start->sync_menu_should_close = TRUE;
}

static void on_personal_menu_hide(
    WINTC_UNUSED(GtkWidget* self),
    gpointer   user_data
)
{
    WinTCTaskbandToolbar* toolbar       = WINTC_TASKBAND_TOOLBAR(user_data);
    WinTCToolbarStart*    toolbar_start = WINTC_TOOLBAR_START(user_data);

    // Track the last closed time, important for toggling the menu properly
    //   (see toolbar.c)
    //
    toolbar_start->time_menu_closed = g_get_monotonic_time();

    // Sync Start button state
    //
    toolbar_start->sync_button = TRUE;
    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(toolbar->widget_root),
        FALSE
    );
    toolbar_start->sync_button = FALSE;
}

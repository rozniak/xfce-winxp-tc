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

#include "menumod.h"
#include "mfu.h"
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
static void refresh_personal_menu(
    WinTCToolbarStart* toolbar_start
);
static void refresh_userpic(
    WinTCToolbarStart* toolbar_start
);
static void update_personal_menu_mfu_items(
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
static void on_mfu_tracker_updated(
    WinTCStartMfuTracker* self,
    gpointer              user_data
);
static void on_personal_menu_hide(
    GtkWidget* self,
    gpointer   user_data
);

//
// STATIC DATA
//
static GQuark S_QUARK_PERSONAL_ITEM_TUPLE = 0;

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

    if (!S_QUARK_PERSONAL_ITEM_TUPLE)
    {
        S_QUARK_PERSONAL_ITEM_TUPLE =
            g_quark_from_static_string("personal-tuple");
    }

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
        wintc_dpa_create_popup(
            toolbar_start->start_button,
            TRUE
        );

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
        toolbar_start->start_button
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
    if (icon_name)
    {
        GdkPixbuf* pixbuf_icon =
            gtk_icon_theme_load_icon(
                gtk_icon_theme_get_default(),
                icon_name,
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
    gtk_image_set_pixel_size(
        GTK_IMAGE(image_icon),
        PROGRAM_ICON_SIZE
    );
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

    g_clear_object(&entry_internet);
    g_clear_object(&entry_email);

    // Add separator between defaults & MFU
    //
    gtk_menu_shell_append(
        GTK_MENU_SHELL(toolbar_start->personal.menubar_programs),
        gtk_separator_menu_item_new()
    );

    // Loop over to add the MFU items
    //
    for (gint i = 0; i < MAX_MFU_ITEM_COUNT; i++)
    {
        GtkWidget* personal_item =
            create_personal_menu_item(
                GTK_MENU_SHELL(
                    toolbar_start->personal.menubar_programs
                ),
                NULL,
                NULL,
                NULL,
                NULL
            );

        tuple =
            &g_array_index(
                toolbar_start->personal.tuples_programs,
                StartSignalTuple,
                DEFAULT_ITEM_COUNT + i
            );

        tuple->is_action     = FALSE;
        tuple->toolbar_start = toolbar_start;

        g_object_set_qdata(
            G_OBJECT(personal_item),
            S_QUARK_PERSONAL_ITEM_TUPLE,
            tuple
        );

        g_signal_connect(
            personal_item,
            "activate",
            G_CALLBACK(on_menu_item_launcher_activate),
            tuple
        );

        gtk_menu_shell_append(
            GTK_MENU_SHELL(toolbar_start->personal.menubar_programs),
            personal_item
        );
    }

    update_personal_menu_mfu_items(toolbar_start);

    g_signal_connect(
        wintc_start_mfu_tracker_get_default(),
        "mfu-updated",
        G_CALLBACK(on_mfu_tracker_updated),
        toolbar_start
    );

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
    static const gchar* s_path_face = NULL;

    if (!s_path_face)
    {
        s_path_face =
            g_build_path(
                G_DIR_SEPARATOR_S,
                g_get_home_dir(),
                ".face",
                NULL
            );
    }

    // Update the pic CSS if we have a face, otherwise default to the built-in
    // pic
    //
    const gchar* actual_path;
    gchar*       css;

    if (g_file_test(s_path_face, G_FILE_TEST_IS_REGULAR))
    {
        actual_path = s_path_face;
    }
    else
    {
        actual_path = "resource:///uk/oddmatics/wintc/taskband/userpic.bmp";
    }

    css =
        g_strdup_printf(
            "* { background-image: url('%s'); }",
            actual_path
        );

    gtk_css_provider_load_from_data(
        GTK_CSS_PROVIDER(toolbar_start->personal.style_userpic),
        css,
        -1,
        NULL
    );

    g_free(css);
}

static void update_personal_menu_mfu_items(
    WinTCToolbarStart* toolbar_start
)
{
    // Find the first MFU item
    //
    GList* li_mfu        = NULL;
    GList* list_children =
        gtk_container_get_children(
            GTK_CONTAINER(toolbar_start->personal.menubar_programs)
        );

    for (GList* iter = list_children; iter; iter = iter->next)
    {
        if (
            g_object_get_qdata(
                G_OBJECT(iter->data),
                S_QUARK_PERSONAL_ITEM_TUPLE
            )
        )
        {
            li_mfu = iter;
            break;
        }
    }

    if (!li_mfu)
    {
        g_critical("%s", "taskband: somehow found no mfu items");
        g_list_free(list_children);
        return;
    }

    // Start updating the items
    //
    GList* list_mfu =
        wintc_start_mfu_tracker_get_mfu_list(
            wintc_start_mfu_tracker_get_default()
        );

    for (
        GList* iter = list_mfu;
        iter;
        iter = iter->next, li_mfu = li_mfu->next
    )
    {
        StartSignalTuple* tuple =
            g_object_get_qdata(
                G_OBJECT(li_mfu->data),
                S_QUARK_PERSONAL_ITEM_TUPLE
            );

        // Protect against the case where there could be more MFU items tracked
        // than personal menu items
        //
        if (!tuple)
        {
            break;
        }

        // Update the menu item and tuple
        //
        GarconMenuItem* garcon_item = GARCON_MENU_ITEM(iter->data);
        const gchar*    icon_name   = garcon_menu_item_get_icon_name(
                                          garcon_item
                                      );
        GList*          list_menu_children;
        GtkWidget*      menu_item   = GTK_WIDGET(li_mfu->data);
        GdkPixbuf*      pixbuf_icon = NULL;

        list_menu_children =
            gtk_container_get_children(
                GTK_CONTAINER(
                    gtk_bin_get_child(GTK_BIN(menu_item))
                )
            );

        if (icon_name)
        {
            pixbuf_icon =
                gtk_icon_theme_load_icon(
                    gtk_icon_theme_get_default(),
                    icon_name,
                    PROGRAM_ICON_SIZE,
                    GTK_ICON_LOOKUP_FORCE_SIZE,
                    NULL
                );
        }

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

        gtk_image_set_from_pixbuf(
            GTK_IMAGE(list_menu_children->data),
            pixbuf_icon
        );
        gtk_label_set_text(
            GTK_LABEL(list_menu_children->next->data),
            garcon_menu_item_get_name(garcon_item)
        );
        gtk_widget_set_tooltip_text(
            menu_item,
            garcon_menu_item_get_comment(garcon_item)
        );

        g_free(tuple->user_data);
        tuple->user_data =
            garcon_menu_item_get_command_expanded(garcon_item);

        if (pixbuf_icon)
        {
            g_object_unref(pixbuf_icon);
        }

        g_list_free(list_menu_children);
    }

    g_list_free(list_mfu);
    g_list_free(list_children);
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
        if (wintc_launch_command(tuple->user_data, &error))
        {
            wintc_start_mfu_tracker_bump_cmdline(
                wintc_start_mfu_tracker_get_default(),
                tuple->user_data
            );
        }
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

static void on_mfu_tracker_updated(
    WINTC_UNUSED(WinTCStartMfuTracker* self),
    gpointer user_data
)
{
    WinTCToolbarStart* toolbar_start = WINTC_TOOLBAR_START(user_data);

    update_personal_menu_mfu_items(toolbar_start);
}

static void on_personal_menu_hide(
    WINTC_UNUSED(GtkWidget* self),
    gpointer   user_data
)
{
    WinTCToolbarStart* toolbar_start = WINTC_TOOLBAR_START(user_data);

    // Track the last closed time, important for toggling the menu properly
    //   (see toolbar.c)
    //
    toolbar_start->time_menu_closed = g_get_monotonic_time();

    // Sync Start button state
    //
    toolbar_start->sync_button = TRUE;
    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(toolbar_start->start_button),
        FALSE
    );
    toolbar_start->sync_button = FALSE;
}

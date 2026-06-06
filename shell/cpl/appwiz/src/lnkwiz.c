#include <gio/gdesktopappinfo.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shlang.h>
#include <wintc/wizard97.h>

#include "lnkwiz.h"

#define K_INI_GROUP_DESKTOP_ENTRY "Desktop Entry"

#define K_TITLE_CREATE_SHORTCUT "Create Shortcut"
#define K_TITLE_SELECT_TITLE    "Select a Title for the Program"

//
// PRIVATE ENUMS
//
enum
{
    PROP_NULL,
    PROP_PATH,
    N_PROPERTIES,

    OVERRIDE_PROP_CAN_NEXT,
};

enum
{
    WIZPAGE_INTRO,
    WIZPAGE_FINISH
};

//
// FORWARD DECLARATIONS
//
static void wintc_cpl_appwiz_new_link_wizard_constructed(
    GObject* object
);
static void wintc_cpl_appwiz_new_link_wizard_dispose(
    GObject* object
);
static void wintc_cpl_appwiz_new_link_wizard_finalize(
    GObject* object
);
static void wintc_cpl_appwiz_new_link_wizard_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_cpl_appwiz_new_link_wizard_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void wintc_cpl_appwiz_new_link_wizard_constructing_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page_num,
    GtkBuilder*          builder
);
static gboolean wintc_cpl_appwiz_new_link_wizard_finish(
    WinTCWizard97Window* wiz_wnd,
    guint                current_page
);
static void wintc_cpl_appwiz_new_link_wizard_presenting_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page_num
);
static gboolean wintc_cpl_appwiz_new_link_wizard_validate(
    WinTCWizard97Window* wiz_wnd,
    guint                current_page
);

static gchar* wintc_cpl_appwiz_new_link_wizard_get_available_name(
    WinTCCplAppwizNewLinkWizard* lnkwiz
);
static void wintc_cpl_appwiz_new_link_wizard_update_state(
    WinTCCplAppwizNewLinkWizard* lnkwiz,
    guint                        page_num
);

static void on_button_browse_clicked(
    GtkWidget* button,
    gpointer   user_data
);
static void on_entry_name_changed(
    GtkEditable* editable,
    gpointer     user_data
);
static void on_entry_target_changed(
    GtkEditable* editable,
    gpointer     user_data
);
static gboolean on_window_map_event(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);

//
// STATIC DATA
//
static GParamSpec*
    wintc_cpl_appwiz_new_link_wizard_properties[N_PROPERTIES] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCCplAppwizNewLinkWizardClass
{
    WinTCWizard97WindowClass __parent__;
};

struct _WinTCCplAppwizNewLinkWizard
{
    WinTCWizard97Window __parent__;

    // State
    //
    gboolean can_next;
    gboolean done;

    gchar*             dest_name;
    GFile*             file;
    GError*            file_error;
    GFileOutputStream* file_stream;
    gchar*             path;

    // UI stuff
    //
    GtkWidget* button_browse;
    GtkWidget* entry_target;

    GtkWidget* entry_name;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCCplAppwizNewLinkWizard,
    wintc_cpl_appwiz_new_link_wizard,
    WINTC_TYPE_WIZARD97_WINDOW
)

static void wintc_cpl_appwiz_new_link_wizard_class_init(
    WinTCCplAppwizNewLinkWizardClass* klass
)
{
    GObjectClass*             object_class = G_OBJECT_CLASS(klass);
    WinTCWizard97WindowClass* wizard_class =
        WINTC_WIZARD97_WINDOW_CLASS(klass);

    object_class->constructed  = wintc_cpl_appwiz_new_link_wizard_constructed;
    object_class->dispose      = wintc_cpl_appwiz_new_link_wizard_dispose;
    object_class->finalize     = wintc_cpl_appwiz_new_link_wizard_finalize;
    object_class->get_property = wintc_cpl_appwiz_new_link_wizard_get_property;
    object_class->set_property = wintc_cpl_appwiz_new_link_wizard_set_property;
    wizard_class->constructing_page =
        wintc_cpl_appwiz_new_link_wizard_constructing_page;
    wizard_class->finish            =
        wintc_cpl_appwiz_new_link_wizard_finish;
    wizard_class->presenting_page   =
        wintc_cpl_appwiz_new_link_wizard_presenting_page;
    wizard_class->validate          =
        wintc_cpl_appwiz_new_link_wizard_validate;


    // Set up properties
    //
    wintc_cpl_appwiz_new_link_wizard_properties[PROP_PATH] =
        g_param_spec_string(
            "path",
            "Path",
            "The path to the shortcut being created.",
            NULL,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_cpl_appwiz_new_link_wizard_properties
    );

    g_object_class_override_property(
        object_class,
        OVERRIDE_PROP_CAN_NEXT,
        "can-next"
    );

    // Configure wizard
    //
    wintc_wizard97_window_class_setup_from_resources(
        wizard_class,
        WINTC_WIZARD97_STYLE_OLD,
        "/uk/oddmatics/wintc/appwiz/watermk.png",
        NULL,
        "/uk/oddmatics/wintc/appwiz/lnkwizp1.ui",
        "/uk/oddmatics/wintc/appwiz/lnkwizp2.ui",
        NULL
    );
}

static void wintc_cpl_appwiz_new_link_wizard_init(
    WinTCCplAppwizNewLinkWizard* self
)
{
    wintc_wizard97_window_init_wizard(
        WINTC_WIZARD97_WINDOW(self)
    );

    g_signal_connect(
        self,
        "map-event",
        G_CALLBACK(on_window_map_event),
        self
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_cpl_appwiz_new_link_wizard_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_cpl_appwiz_new_link_wizard_parent_class))
        ->constructed(object);

    WinTCCplAppwizNewLinkWizard* lnkwiz =
        WINTC_CPL_APPWIZ_NEW_LINK_WIZARD(object);

    // Attempt to get ourselves a file
    //
    gint    i;
    gchar*  try_path = NULL;

    for (i = 0; i <= G_MAXINT16; i++)
    {
        try_path =
            i == 0 ?
                g_strdup(lnkwiz->path) :
                g_strdup_printf("%s (%d)", lnkwiz->path, i);

        lnkwiz->file = g_file_new_for_path(try_path);

        lnkwiz->file_stream =
            g_file_create(
                lnkwiz->file,
                G_FILE_CREATE_NONE,
                NULL,
                &(lnkwiz->file_error)
            );

        g_free(try_path);

        if (!(lnkwiz->file_stream))
        {
            g_clear_object(&(lnkwiz->file));

            // We're fine to progress if it's file exists, other errors should
            // bomb out here
            //
            if (
                !g_error_matches(
                    lnkwiz->file_error,
                    G_FILE_ERROR,
                    G_FILE_ERROR_EXIST
                ) &&
                !g_error_matches(
                    lnkwiz->file_error,
                    G_IO_ERROR,
                    G_IO_ERROR_EXISTS
                )
            )
            {
                break;
            }

            g_clear_error(&(lnkwiz->file_error));
            continue;
        }

        break;
    }
}

static void wintc_cpl_appwiz_new_link_wizard_dispose(
    GObject* object
)
{
    WinTCCplAppwizNewLinkWizard* lnkwiz =
        WINTC_CPL_APPWIZ_NEW_LINK_WIZARD(object);

    if (lnkwiz->file_stream)
    {
        g_output_stream_close(
            G_OUTPUT_STREAM(lnkwiz->file_stream),
            NULL,
            NULL
        );

        lnkwiz->file_stream = NULL;
    }

    if (!(lnkwiz->done) && lnkwiz->file)
    {
        // Delete the temp file if the wizard was closed early
        //
        g_file_delete(lnkwiz->file, NULL, NULL);
    }

    g_clear_object(&(lnkwiz->file));

    (G_OBJECT_CLASS(wintc_cpl_appwiz_new_link_wizard_parent_class))
        ->dispose(object);
}

static void wintc_cpl_appwiz_new_link_wizard_finalize(
    GObject* object
)
{
    WinTCCplAppwizNewLinkWizard* lnkwiz =
        WINTC_CPL_APPWIZ_NEW_LINK_WIZARD(object);

    g_free(lnkwiz->dest_name);
    g_free(lnkwiz->path);

    (G_OBJECT_CLASS(wintc_cpl_appwiz_new_link_wizard_parent_class))
        ->finalize(object);
}

static void wintc_cpl_appwiz_new_link_wizard_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCCplAppwizNewLinkWizard* lnkwiz =
        WINTC_CPL_APPWIZ_NEW_LINK_WIZARD(object);

    switch (prop_id)
    {
        case OVERRIDE_PROP_CAN_NEXT:
            g_value_set_boolean(value, lnkwiz->can_next);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_cpl_appwiz_new_link_wizard_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCCplAppwizNewLinkWizard* lnkwiz =
        WINTC_CPL_APPWIZ_NEW_LINK_WIZARD(object);

    switch (prop_id)
    {
        case PROP_PATH:
            lnkwiz->path = g_value_dup_string(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_cpl_appwiz_new_link_wizard_constructing_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page_num,
    GtkBuilder*          builder
)
{
    WinTCCplAppwizNewLinkWizard* lnkwiz =
        WINTC_CPL_APPWIZ_NEW_LINK_WIZARD(wiz_wnd);

    switch (page_num)
    {
        case WIZPAGE_INTRO:
            wintc_builder_get_objects(
                builder,
                "button-browse", &(lnkwiz->button_browse),
                "entry-target",  &(lnkwiz->entry_target),
                NULL
            );

            g_signal_connect(
                lnkwiz->button_browse,
                "clicked",
                G_CALLBACK(on_button_browse_clicked),
                lnkwiz
            );
            g_signal_connect(
                lnkwiz->entry_target,
                "changed",
                G_CALLBACK(on_entry_target_changed),
                lnkwiz
            );

            break;

        case WIZPAGE_FINISH:
            wintc_builder_get_objects(
                builder,
                "entry-name", &(lnkwiz->entry_name),
                NULL
            );

            g_signal_connect(
                lnkwiz->entry_name,
                "changed",
                G_CALLBACK(on_entry_name_changed),
                lnkwiz
            );

            break;

        default: break;
    }
}

static gboolean wintc_cpl_appwiz_new_link_wizard_finish(
    WinTCWizard97Window* wiz_wnd,
    WINTC_UNUSED(guint current_page)
)
{
    WinTCCplAppwizNewLinkWizard* lnkwiz =
        WINTC_CPL_APPWIZ_NEW_LINK_WIZARD(wiz_wnd);

    const gchar* target =
        gtk_entry_get_text(GTK_ENTRY(lnkwiz->entry_target));

    // Figure out what we're linking to
    //
    gboolean is_url      = TRUE;
    gboolean terminal    = FALSE;
    gchar*   true_target = NULL;

    if (strlen(target) > 0 && *target == '/')
    {
        if (
            g_file_test(target, G_FILE_TEST_IS_REGULAR) &&
            g_file_test(target, G_FILE_TEST_IS_EXECUTABLE)
        )
        {
            is_url = FALSE;

            // Okay it's an executable -- does it need a terminal?
            //
            gchar* possible_entry =
                g_strdup_printf("%s.desktop", wintc_basename(target));

            GDesktopAppInfo* app_info =
                g_desktop_app_info_new(possible_entry);

            if (app_info)
            {
                terminal =
                    g_desktop_app_info_get_boolean(
                        app_info,
                        "Terminal"
                    );
            }

            g_free(possible_entry);
        }
        else
        {
            true_target =
                g_strdup_printf("file://%s", target);
        }
    }

    if (!true_target)
    {
        true_target = g_strdup(target);
    }

    // Set up our desktop entry
    //
    GKeyFile* ini_link = g_key_file_new();

    g_key_file_set_string(
        ini_link,
        K_INI_GROUP_DESKTOP_ENTRY,
        "Name",
        gtk_entry_get_text(GTK_ENTRY(lnkwiz->entry_name))
    );

    g_key_file_set_string(
        ini_link,
        K_INI_GROUP_DESKTOP_ENTRY,
        "Type",
        is_url ? "Link" : "Application"
    );

    g_key_file_set_string(
        ini_link,
        K_INI_GROUP_DESKTOP_ENTRY,
        is_url ? "URL" : "Exec",
        true_target
    );

    g_key_file_set_boolean(
        ini_link,
        K_INI_GROUP_DESKTOP_ENTRY,
        "StartupNotify",
        TRUE
    );

    g_key_file_set_string(
        ini_link,
        K_INI_GROUP_DESKTOP_ENTRY,
        "Path",
        g_get_home_dir()
    );

    g_key_file_set_boolean(
        ini_link,
        K_INI_GROUP_DESKTOP_ENTRY,
        "Terminal",
        terminal
    );

    // Attempt to write it out
    //
    gchar*  data;
    gchar*  dest;
    GFile*  dest_file;
    gchar*  dir;
    gsize   len;
    GError* error = NULL;

    dir  = g_path_get_dirname(lnkwiz->path);
    data = g_key_file_to_data(ini_link, &len, &error);

    dest =
        g_build_path(
            G_DIR_SEPARATOR_S,
            dir,
            lnkwiz->dest_name,
            NULL
        );

    dest_file = g_file_new_for_path(dest);

    if (!data)
    {
        wintc_display_error_and_clear(&error, GTK_WINDOW(lnkwiz));
        goto cleanup;
    }

    if (
        !g_output_stream_write_all(
            G_OUTPUT_STREAM(lnkwiz->file_stream),
            (void*) data,
            len,
            NULL,
            NULL,
            &error
        )
    )
    {
        wintc_display_error_and_clear(&error, GTK_WINDOW(lnkwiz));
        goto cleanup;
    }

    g_output_stream_close(
        G_OUTPUT_STREAM(lnkwiz->file_stream),
        NULL,
        NULL
    );

    g_file_delete(dest_file, NULL, NULL);

    if (
        !g_file_set_display_name(
            lnkwiz->file,
            lnkwiz->dest_name,
            NULL,
            &error
        )
    )
    {
        wintc_display_error_and_clear(&error, GTK_WINDOW(lnkwiz));
        goto cleanup;
    }

    lnkwiz->done = TRUE;

cleanup:
    g_free(data);
    g_free(dir);
    g_free(dest);
    g_object_unref(dest_file);
    g_output_stream_close(
        G_OUTPUT_STREAM(lnkwiz->file_stream),
        NULL,
        NULL
    );

    return TRUE;
}

static void wintc_cpl_appwiz_new_link_wizard_presenting_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page_num
)
{
    WinTCCplAppwizNewLinkWizard* lnkwiz =
        WINTC_CPL_APPWIZ_NEW_LINK_WIZARD(wiz_wnd);

    wintc_cpl_appwiz_new_link_wizard_update_state(lnkwiz, page_num);

    // Update the suggested name in the entry
    //
    if (page_num == WIZPAGE_FINISH)
    {
        gchar* suggested_name =
            wintc_cpl_appwiz_new_link_wizard_get_available_name(lnkwiz);

        gtk_entry_set_text(
            GTK_ENTRY(lnkwiz->entry_name),
            suggested_name
        );

        g_free(suggested_name);
    }

    switch (page_num)
    {
        case WIZPAGE_INTRO:
            gtk_window_set_title(GTK_WINDOW(wiz_wnd), K_TITLE_CREATE_SHORTCUT);
            break;

        case WIZPAGE_FINISH:
            gtk_window_set_title(GTK_WINDOW(wiz_wnd), K_TITLE_SELECT_TITLE);
            break;

        default: break;
    }
}

static gboolean wintc_cpl_appwiz_new_link_wizard_validate(
    WinTCWizard97Window* wiz_wnd,
    guint                current_page
)
{
    WinTCCplAppwizNewLinkWizard* lnkwiz =
        WINTC_CPL_APPWIZ_NEW_LINK_WIZARD(wiz_wnd);

    switch (current_page)
    {
        case WIZPAGE_INTRO:
        {
            const gchar* target =
                gtk_entry_get_text(GTK_ENTRY(lnkwiz->entry_target));

            // Special for Linux -- accept any URI since desktop entries are
            // fine with this
            //
            if (
                g_regex_match(
                    wintc_regex_uri_scheme(NULL),
                    target,
                    G_REGEX_MATCH_DEFAULT,
                    NULL
                )
            )
            {
                return TRUE;
            }

            // Otherwise we assume that we've got a file path, only move on
            // if we have it
            //
            if (!g_file_test(target, G_FILE_TEST_EXISTS))
            {
                gchar* msg =
                    g_strdup_printf("The file %s cannot be found.", target);

                wintc_messagebox_show(
                    GTK_WINDOW(lnkwiz),
                    msg,
                    K_TITLE_CREATE_SHORTCUT,
                    GTK_BUTTONS_OK,
                    GTK_MESSAGE_WARNING
                );

                g_free(msg);

                return FALSE;
            }

            return TRUE;
        }

        case WIZPAGE_FINISH:
        {
            gchar*       dest;
            gchar*       dir;
            const gchar* name;
            gboolean     success = FALSE;

            dir  = g_path_get_dirname(lnkwiz->path);
            name = gtk_entry_get_text(GTK_ENTRY(lnkwiz->entry_name));

            dest =
                g_strdup_printf(
                    "%s/%s.desktop",
                    dir,
                    name
                );

            if (g_file_test(dest, G_FILE_TEST_EXISTS))
            {
                gchar* msg =
                    g_strdup_printf(
                        "A shortcut named %s already exists in this folder. "
                        "Do you want to replace it?",
                        name
                    );

                gint response =
                    wintc_messagebox_show(
                        GTK_WINDOW(lnkwiz),
                        msg,
                        K_TITLE_SELECT_TITLE,
                        GTK_BUTTONS_YES_NO,
                        GTK_MESSAGE_ERROR
                    );

                g_free(msg);

                if (response == GTK_RESPONSE_NO)
                {
                    goto cleanup_finish;
                }
            }

            lnkwiz->dest_name = g_strdup_printf("%s.desktop", name);

            success = TRUE;

cleanup_finish:
            g_free(dir);
            g_free(dest);

            return success;
        }

        default:
            return TRUE;
    }
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_cpl_appwiz_new_link_wizard_new(
    const gchar* path
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_CPL_APPWIZ_NEW_LINK_WIZARD,
            "path",  path,
            "title", K_TITLE_CREATE_SHORTCUT,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static gchar* wintc_cpl_appwiz_new_link_wizard_get_available_name(
    WinTCCplAppwizNewLinkWizard* lnkwiz
)
{
    const gchar* small_name;
    const gchar* target;

    target     = gtk_entry_get_text(GTK_ENTRY(lnkwiz->entry_target));
    small_name = wintc_basename(target);

    if (!small_name)
    {
        // Realistically this shouldn't happen because the prior page
        // requires a fully qualified path or URL
        //
        small_name = target;
    }

    // Try to find an available name on FS for this
    //
    gchar* dest_path = NULL;
    gchar* test_name = NULL;

    for (gint i = 0; i <= G_MAXINT16; i++)
    {
        test_name =
            i == 0 ?
                g_strdup(small_name) :
                g_strdup_printf("%s (%d)", small_name, i);

        dest_path =
            g_strdup_printf("%s/%s", lnkwiz->path, test_name);

        if (!g_file_test(dest_path, G_FILE_TEST_EXISTS))
        {
            break;
        }

        g_clear_pointer(&test_name, (GDestroyNotify) g_free);
        g_free(dest_path);
    }

    g_free(dest_path);

    if (!test_name)
    {
        test_name = g_strdup("New Shortcut");
    }

    return test_name;
}

static void wintc_cpl_appwiz_new_link_wizard_update_state(
    WinTCCplAppwizNewLinkWizard* lnkwiz,
    guint                        page_num
)
{
    gboolean can_next = TRUE;

    switch (page_num)
    {
        case WIZPAGE_INTRO:
            can_next =
                strlen(
                    gtk_entry_get_text(GTK_ENTRY(lnkwiz->entry_target))
                ) > 0;

            break;

        case WIZPAGE_FINISH:
            can_next =
                strlen(
                    gtk_entry_get_text(GTK_ENTRY(lnkwiz->entry_name))
                ) > 0;

            break;

        default: break;
    }

    lnkwiz->can_next = can_next;

    g_object_notify(
        G_OBJECT(lnkwiz),
        "can-next"
    );
}

//
// CALLBACKS
//
static void on_button_browse_clicked(
    WINTC_UNUSED(GtkWidget* button),
    gpointer user_data
)
{
    WinTCCplAppwizNewLinkWizard* lnkwiz =
        WINTC_CPL_APPWIZ_NEW_LINK_WIZARD(user_data);

    // Try to figure out where we should open
    //
    const char* target   = gtk_entry_get_text(GTK_ENTRY(lnkwiz->entry_target));
    gchar*      open_dir = NULL;

    if (strlen(target) > 0 && *target == '/')
    {
        open_dir = g_path_get_dirname(target);
    }
    else
    {
        open_dir =
            g_strdup(g_get_user_special_dir(G_USER_DIRECTORY_DOCUMENTS));
    }

    // Set up All Files filter
    //
    GtkFileFilter* filter_all_files = gtk_file_filter_new();

    gtk_file_filter_set_name(filter_all_files, "All Files");
    gtk_file_filter_add_pattern(filter_all_files, "*");

    // Set up the file dialog
    //
    GtkWidget* dlg =
        gtk_file_chooser_dialog_new(
            wintc_lc_get_control_text(WINTC_CTLTXT_BROWSE, WINTC_PUNC_NONE),
            wintc_widget_get_toplevel_window(button),
            GTK_FILE_CHOOSER_ACTION_OPEN,
            wintc_lc_get_control_text(WINTC_CTLTXT_CANCEL, WINTC_PUNC_NONE),
            GTK_RESPONSE_CANCEL,
            wintc_lc_get_control_text(WINTC_CTLTXT_OPEN, WINTC_PUNC_NONE),
            GTK_RESPONSE_ACCEPT,
            NULL
        );

    gtk_file_chooser_add_filter(
        GTK_FILE_CHOOSER(dlg),
        filter_all_files
    );
    gtk_file_chooser_set_current_folder(
        GTK_FILE_CHOOSER(dlg),
        open_dir
    );

    // Launch dialog now
    //
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT)
    {
        gtk_entry_set_text(
            GTK_ENTRY(lnkwiz->entry_target),
            gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg))
        );
    }

    gtk_widget_destroy(dlg);

    g_free(open_dir);
}

static void on_entry_name_changed(
    WINTC_UNUSED(GtkEditable* editable),
    gpointer user_data
)
{
    WinTCCplAppwizNewLinkWizard* lnkwiz =
        WINTC_CPL_APPWIZ_NEW_LINK_WIZARD(user_data);

    wintc_cpl_appwiz_new_link_wizard_update_state(
        lnkwiz,
        WIZPAGE_FINISH
    );
}

static void on_entry_target_changed(
    WINTC_UNUSED(GtkEditable* editable),
    gpointer user_data
)
{
    WinTCCplAppwizNewLinkWizard* lnkwiz =
        WINTC_CPL_APPWIZ_NEW_LINK_WIZARD(user_data);

    wintc_cpl_appwiz_new_link_wizard_update_state(
        lnkwiz,
        WIZPAGE_INTRO
    );
}

static gboolean on_window_map_event(
    WINTC_UNUSED(GtkWidget* widget),
    WINTC_UNUSED(GdkEvent*  event),
    gpointer user_data
)
{
    WinTCCplAppwizNewLinkWizard* lnkwiz =
        WINTC_CPL_APPWIZ_NEW_LINK_WIZARD(user_data);

    // If we don't have a file to work on, throw the error now
    //
    if (!(lnkwiz->file))
    {
        if (lnkwiz->file_error)
        {
            wintc_display_error_and_clear(
                &(lnkwiz->file_error),
                GTK_WINDOW(lnkwiz)
            );
        }
        else
        {
            wintc_messagebox_show(
                GTK_WINDOW(lnkwiz),
                "The operation could not be completed.", // FIXME: localise
                K_TITLE_CREATE_SHORTCUT,
                GTK_BUTTONS_OK,
                GTK_MESSAGE_ERROR
            );
        }

        gtk_window_close(GTK_WINDOW(lnkwiz));
    }

    return FALSE;
}

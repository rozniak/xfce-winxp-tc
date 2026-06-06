#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shlang.h>
#include <wintc/wizard97.h>

#include "lnkwiz.h"

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
static void wintc_cpl_appwiz_new_link_wizard_presenting_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page_num
);
static gboolean wintc_cpl_appwiz_new_link_wizard_validate(
    WinTCWizard97Window* wiz_wnd,
    guint                current_page
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

    GFile* file;
    gchar* path;

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
    self->can_next = FALSE;

    wintc_wizard97_window_init_wizard(
        WINTC_WIZARD97_WINDOW(self)
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

    // Touch the file
    //
    lnkwiz->file = g_file_new_for_path(lnkwiz->path);
}

static void wintc_cpl_appwiz_new_link_wizard_dispose(
    GObject* object
)
{
    WinTCCplAppwizNewLinkWizard* lnkwiz =
        WINTC_CPL_APPWIZ_NEW_LINK_WIZARD(object);

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

static void wintc_cpl_appwiz_new_link_wizard_presenting_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page_num
)
{
    wintc_cpl_appwiz_new_link_wizard_update_state(
        WINTC_CPL_APPWIZ_NEW_LINK_WIZARD(wiz_wnd),
        page_num
    );
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

            if (!g_file_test(target, G_FILE_TEST_EXISTS))
            {
                gchar* msg =
                    g_strdup_printf("The file %s cannot be found.", target);

                wintc_messagebox_show(
                    GTK_WINDOW(lnkwiz),
                    msg,
                    "Create Shortcut",
                    GTK_BUTTONS_OK,
                    GTK_MESSAGE_WARNING
                );

                g_free(msg);

                return FALSE;
            }

            return TRUE;
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
            "path", path,
            "title", "Create Shortcut",
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
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
    GtkWidget* button,
    gpointer   user_data
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

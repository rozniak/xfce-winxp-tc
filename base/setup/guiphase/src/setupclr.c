#include <glib.h>
#include <gio/gunixinputstream.h>
#include <wintc/comgtk.h>

#include "netwiz.h"
#include "perwiz.h"
#include "phase.h"
#include "setupapi.h"
#include "setupclr.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_NULL,
    PROP_SETUP_WINDOW,
    N_PROPERTIES
};

enum
{
    SIGNAL_SETUP_DONE = 0,
    N_SIGNALS
};

enum
{
    PHASE_INSTALL_DEVICES,
    PHASE_PERWIZ,
    PHASE_INSTALL_NETWORK,
    PHASE_NETWIZ,
    PHASE_COPY_FILES,
    PHASE_SAVE_SETTINGS,
    PHASE_DONE
};

//
// FORWARD DECLARATIONS
//
static void wintc_setup_controller_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_setup_controller_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void wintc_setup_controller_go_to_phase(
    WinTCSetupController* setup,
    guint                 phase
);
static void wintc_setup_controller_install_files(
    WinTCSetupController* setup
);

static GList* collect_packages(
    GKeyFile*    ini_complist,
    const gchar* group,
    GList*       list_packages
);
static gchar** key_file_get_csv(
    GKeyFile*    key_file,
    const gchar* group,
    const gchar* key,
    GError**     error
);

static void cb_setup_act_done(
    gpointer user_data
);
static void cb_setup_act_error(
    GError** error,
    gpointer user_data
);
static void cb_setup_act_progress(
    gdouble  progress,
    gpointer user_data
);

static void on_netwiz_destroyed(
    GtkWidget* widget,
    gpointer   user_data
);
static void on_perwiz_destroyed(
    GtkWidget* widget,
    gpointer   user_data
);

//
// STATIC DATA
//
static GParamSpec* wintc_setup_controller_properties[N_PROPERTIES] = { 0 };
static gint        wintc_setup_controller_signals[N_SIGNALS]       = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCSetupControllerClass
{
    GObjectClass __parent__;
};

struct _WinTCSetupController
{
    GObject __parent__;

    WinTCSetupWindow* wnd_setup;
    guint             current_phase;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCSetupController,
    wintc_setup_controller,
    G_TYPE_OBJECT
)

static void wintc_setup_controller_class_init(
    WinTCSetupControllerClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = wintc_setup_controller_get_property;
    object_class->set_property = wintc_setup_controller_set_property;

    wintc_setup_controller_properties[PROP_SETUP_WINDOW] =
        g_param_spec_object(
            "setup-window",
            "SetupWindow",
            "The setup window.",
            WINTC_TYPE_SETUP_WINDOW,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_setup_controller_properties
    );

    wintc_setup_controller_signals[SIGNAL_SETUP_DONE] =
        g_signal_new(
            "setup-done",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0
        );
}

static void wintc_setup_controller_init(
    WINTC_UNUSED(WinTCSetupController* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_setup_controller_get_property(
    GObject*    object,
    guint       prop_id,
    WINTC_UNUSED(GValue* value),
    GParamSpec* pspec
)
{
    //WinTCSetupController* setup_ctl = WINTC_SETUP_CONTROLLER(object);

    switch (prop_id)
    {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_setup_controller_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCSetupController* setup_ctl = WINTC_SETUP_CONTROLLER(object);

    switch (prop_id)
    {
        case PROP_SETUP_WINDOW:
            setup_ctl->wnd_setup = g_value_dup_object(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
WinTCSetupController* wintc_setup_controller_new(
    WinTCSetupWindow* wnd_setup
)
{
    return WINTC_SETUP_CONTROLLER(
        g_object_new(
            WINTC_TYPE_SETUP_CONTROLLER,
            "setup-window", wnd_setup,
            NULL
        )
    );
}

void wintc_setup_controller_begin(
    WINTC_UNUSED(WinTCSetupController* setup)
)
{
    wintc_setup_act_init();

    //
    // FIXME: This needs to load the previous state of setup, if possible, in
    //        case power was lost or something
    //

    wintc_setup_controller_go_to_phase(
        setup,
        PHASE_INSTALL_DEVICES
    );
}

//
// PRIVATE FUNCTIONS
//
static void wintc_setup_controller_go_to_phase(
    WinTCSetupController* setup,
    guint                 phase
)
{
    setup->current_phase = phase;

    switch (phase)
    {
        case PHASE_INSTALL_DEVICES:
            //
            // FIXME: Skipping for now - should handle things like backlight
            //        controllers that need kernel params (like
            //        samsung_backlight)
            //
            //        Potentially also should cover things that need firmware
            //        install (like NVIDIA GPU or WLAN cards like Realtek)
            //
            wintc_setup_controller_go_to_phase(
                setup,
                PHASE_PERWIZ
            );
            break;

        case PHASE_PERWIZ:
        {
            // Setup window properties
            //
            wintc_setup_window_disable_billboards(setup->wnd_setup);
            wintc_setup_window_disable_throbbers(setup->wnd_setup);

            wintc_setup_window_set_completion_minutes_approx(
                setup->wnd_setup,
                33
            );

            wintc_setup_window_set_current_step(
                setup->wnd_setup,
                WINTC_SETUP_STEP_INSTALLING_WINDOWS
            );
            wintc_setup_window_set_current_step_progress(
                setup->wnd_setup,
                NULL,
                0.0f
            );

            // Spawn the personalize wizard
            //
            GtkWidget* wizard = wintc_setup_personalize_wizard_new();

            gtk_window_set_transient_for(
                GTK_WINDOW(wizard),
                GTK_WINDOW(setup->wnd_setup)
            );

            g_signal_connect(
                wizard,
                "destroy",
                G_CALLBACK(on_perwiz_destroyed),
                setup
            );

            gtk_widget_show_all(wizard);

            break;
        }

        case PHASE_INSTALL_NETWORK:
            //
            // FIXME: Another one we're skipping for now, not sure what would
            //        be done during this phase anyhow
            //
            wintc_setup_window_enable_billboards(setup->wnd_setup);
            wintc_setup_window_enable_throbbers(setup->wnd_setup);

            wintc_setup_window_set_completion_minutes_approx(
                setup->wnd_setup,
                33
            );

            wintc_setup_window_set_current_step(
                setup->wnd_setup,
                WINTC_SETUP_STEP_INSTALLING_WINDOWS
            );
            wintc_setup_window_set_current_step_progress(
                setup->wnd_setup,
                "Installing Network...",
                1.0f
            );

            wintc_setup_controller_go_to_phase(
                setup,
                PHASE_NETWIZ
            );

            break;

        case PHASE_NETWIZ:
        {
            // Setup window properties
            //
            wintc_setup_window_disable_billboards(setup->wnd_setup);
            wintc_setup_window_disable_throbbers(setup->wnd_setup);

            wintc_setup_window_set_completion_minutes_approx(
                setup->wnd_setup,
                33
            );

            wintc_setup_window_set_current_step(
                setup->wnd_setup,
                WINTC_SETUP_STEP_INSTALLING_WINDOWS
            );
            wintc_setup_window_set_current_step_progress(
                setup->wnd_setup,
                NULL,
                0.0f
            );

            // Spawn the network wizard
            //
            GtkWidget* wizard = wintc_setup_network_wizard_new();

            gtk_window_set_transient_for(
                GTK_WINDOW(wizard),
                GTK_WINDOW(setup->wnd_setup)
            );

            g_signal_connect(
                wizard,
                "destroy",
                G_CALLBACK(on_netwiz_destroyed),
                setup
            );

            gtk_widget_show_all(wizard);

            break;
        }

        case PHASE_COPY_FILES:
        {
            wintc_setup_window_enable_billboards(setup->wnd_setup);
            wintc_setup_window_enable_throbbers(setup->wnd_setup);

            wintc_setup_window_set_completion_minutes_approx(
                setup->wnd_setup,
                20
            );

            wintc_setup_window_set_current_step(
                setup->wnd_setup,
                WINTC_SETUP_STEP_INSTALLING_WINDOWS
            );
            wintc_setup_window_set_current_step_progress(
                setup->wnd_setup,
                NULL,
                0.0f
            );

            // Kick off package install
            //
            wintc_setup_controller_install_files(setup);

            break;
        }

        case PHASE_SAVE_SETTINGS:
            wintc_setup_window_enable_billboards(setup->wnd_setup);
            wintc_setup_window_enable_throbbers(setup->wnd_setup);

            wintc_setup_window_set_completion_minutes_approx(
                setup->wnd_setup,
                10
            );

            wintc_setup_window_set_current_step(
                setup->wnd_setup,
                WINTC_SETUP_STEP_INSTALLING_WINDOWS
            );
            wintc_setup_window_set_current_step_progress(
                setup->wnd_setup,
                NULL,
                0.0f
            );

            // Kick off system config
            //
            wintc_setup_act_prepare_system(
                cb_setup_act_done,
                cb_setup_act_error,
                cb_setup_act_progress,
                setup
            );
            break;

        case PHASE_DONE:
        {
            wintc_setup_phase_set(WINTC_SETUP_PHASE_OOBE);

            g_signal_emit(
                setup,
                wintc_setup_controller_signals[SIGNAL_SETUP_DONE],
                0
            );
            break;
        };
    }
}

static void wintc_setup_controller_install_files(
    WinTCSetupController* setup
)
{
    GError* error         = NULL;
    GList*  list_packages = NULL;

    //
    // FIXME: We're just iterating over all packages listed in the
    //        component list and installing everything
    //
    GKeyFile* ini_complist = g_key_file_new();

    if (
        !g_key_file_load_from_file(
            ini_complist,
            WINTC_SETUP_ACT_ROOT_DIR "/complist.ini",
            G_KEY_FILE_NONE,
            &error
        )
    )
    {
        // FIXME: We probably want a more specific error for this
        //
        wintc_display_error_and_clear(
            &error,
            GTK_WINDOW(setup->wnd_setup)
        );

        g_signal_emit(
            setup,
            wintc_setup_controller_signals[SIGNAL_SETUP_DONE],
            0
        );
        return;
    }

    // Add from top level
    //
    list_packages =
        collect_packages(
            ini_complist,
            "TopLevel",
            list_packages
        );

    // Install everything
    //
    if (
        !wintc_setup_act_install_packages(
            list_packages,
            cb_setup_act_done,
            cb_setup_act_error,
            cb_setup_act_progress,
            setup,
            &error
        )
    )
    {
        wintc_display_error_and_clear(
            &error,
            GTK_WINDOW(setup->wnd_setup)
        );
    }

    g_list_free_full(
        list_packages,
        (GDestroyNotify) g_free
    );
}

static GList* collect_packages(
    GKeyFile*    ini_complist,
    const gchar* group,
    GList*       list_packages
)
{
    GError* error = NULL;

    gchar* group_name = g_strdup_printf("Group.%s", group);

    if (!g_key_file_has_group(ini_complist, group_name))
    {
        WINTC_LOG_DEBUG(
            "setupapi: no such group in complist: %s",
            group_name
        );

        g_free(group_name);

        return list_packages;
    }

    // Find components referenced by the group and add their packages
    //
    gchar** components =
        key_file_get_csv(
            ini_complist,
            group_name,
            "Components",
            &error
        );

    if (components)
    {
        for (gint i = 0; components[i]; i++)
        {
            if (strlen(components[i]) == 0)
            {
                continue;
            }

            gchar* component_name =
                g_strdup_printf("Component.%s", components[i]);

            gchar** packages =
                key_file_get_csv(
                    ini_complist,
                    component_name,
                    "Packages",
                    &error
                );

            if (packages)
            {
                for (gint j = 0; packages[j]; j++)
                {
                    if (strlen(packages[j]) == 0)
                    {
                        continue;
                    }

                    list_packages =
                        g_list_prepend(
                            list_packages,
                            g_strdup_printf(
                                "%s%s%s.deb",
                                WINTC_SETUP_ACT_PKG_PATH,
                                G_DIR_SEPARATOR_S,
                                packages[j]
                            )
                        );
                }

                g_strfreev(packages);
            }
            else
            {
                wintc_log_error_and_clear(&error);
            }

            g_free(component_name);
        }

        g_strfreev(components);
    }
    else
    {
        wintc_log_error_and_clear(&error);
    }

    // Iterate over subgroups
    //
    gchar** subgroups =
        key_file_get_csv(
            ini_complist,
            group_name,
            "SubGroups",
            &error
        );

    if (subgroups)
    {
        for (gint i = 0; subgroups[i]; i++)
        {
            if (strlen(subgroups[i]) == 0)
            {
                continue;
            }

            list_packages =
                collect_packages(
                    ini_complist,
                    subgroups[i],
                    list_packages
                );
        }

        g_strfreev(subgroups);
    }
    else
    {
        wintc_log_error_and_clear(&error);
    }

    return list_packages;
}

static gchar** key_file_get_csv(
    GKeyFile*    key_file,
    const gchar* group,
    const gchar* key,
    GError**     error
)
{
    gchar* key_value =
        g_key_file_get_string(
            key_file,
            group,
            key,
            error
        );

    if (!key_value)
    {
        return NULL;
    }

    gchar** split = g_strsplit(key_value, ",", -1);

    g_free(key_value);

    return split;
}

//
// CALLBACKS
//
static void cb_setup_act_done(
    gpointer user_data
)
{
    WinTCSetupController* setup = WINTC_SETUP_CONTROLLER(user_data);

    guint next_phase = PHASE_DONE;

    switch (setup->current_phase)
    {
        case PHASE_COPY_FILES:    next_phase = PHASE_SAVE_SETTINGS; break;
        case PHASE_SAVE_SETTINGS: next_phase = PHASE_DONE;          break;
        default: break;
    }

    wintc_setup_controller_go_to_phase(
        setup,
        next_phase
    );
}

static void cb_setup_act_error(
    GError** error,
    gpointer user_data
)
{
    WinTCSetupController* setup = WINTC_SETUP_CONTROLLER(user_data);

    //
    // FIXME: Need to be far more robust here
    //

    wintc_display_error_and_clear(
        error,
        GTK_WINDOW(setup->wnd_setup)
    );

    wintc_setup_controller_go_to_phase(
        setup,
        PHASE_DONE
    );
}

static void cb_setup_act_progress(
    gdouble  progress,
    gpointer user_data
)
{
    WinTCSetupController* setup = WINTC_SETUP_CONTROLLER(user_data);

    const gchar* progress_label = NULL;

    switch (setup->current_phase)
    {
        case PHASE_COPY_FILES:
            progress_label = "Copying Files...";
            break;

        case PHASE_SAVE_SETTINGS:
            progress_label = "Saving Settings...";
            break;

        default: break;
    }

    wintc_setup_window_set_current_step_progress(
        setup->wnd_setup,
        progress_label,
        progress / 100.0f
    );
}

static void on_netwiz_destroyed(
    WINTC_UNUSED(GtkWidget* widget),
    gpointer user_data
)
{
    WinTCSetupController* setup = WINTC_SETUP_CONTROLLER(user_data);

    wintc_setup_controller_go_to_phase(
        setup,
        PHASE_COPY_FILES
    );
}

static void on_perwiz_destroyed(
    WINTC_UNUSED(GtkWidget* widget),
    gpointer user_data
)
{
    WinTCSetupController* setup = WINTC_SETUP_CONTROLLER(user_data);

    wintc_setup_controller_go_to_phase(
        setup,
        PHASE_INSTALL_NETWORK
    );
}

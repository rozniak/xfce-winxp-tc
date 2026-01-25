#include <glib.h>
#include <gio/gunixinputstream.h>
#include <wintc/comgtk.h>

#include "netwiz.h"
#include "perwiz.h"
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
static void wintc_setup_controller_test_install_gedit(
    WinTCSetupController* setup
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
            wintc_setup_controller_test_install_gedit(setup);

            //
            // FIXME: Implement this later...
            //
            wintc_setup_controller_go_to_phase(
                setup,
                PHASE_DONE
            );
            break;
        }

        case PHASE_DONE:
        {
            g_signal_emit(
                setup,
                wintc_setup_controller_signals[SIGNAL_SETUP_DONE],
                0
            );
            break;
        };
    }
}

static void wintc_setup_controller_test_install_gedit(
    WINTC_UNUSED(WinTCSetupController* setup)
)
{
    gchar* argv[] = {
        "/usr/bin/apt-get",
        "install",
        "-y",
        "-o",
        "APT::Status-Fd=1",
        "gedit"
    };

    GError* error  = NULL;
    gint    fd_out = -1;

    if (
        !g_spawn_async_with_pipes(
            NULL,
            argv,
            NULL,
            0,
            NULL,
            NULL,
            NULL,
            NULL,
            &fd_out,
            NULL,
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
        return;
    }

    // TESTING READING STDOUT
    //
    GInputStream*     fd_stream = g_unix_input_stream_new(fd_out, FALSE);
    GDataInputStream* stream    = g_data_input_stream_new(fd_stream);

    gchar* line;

    while ((line = g_data_input_stream_read_line(stream, NULL, NULL, &error)))
    {
        g_message("FROM APT: %s", line);
        g_free(line);
    }

    if (error)
    {
        wintc_log_error_and_clear(&error);
        return;
    }
}

//
// CALLBACKS
//
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

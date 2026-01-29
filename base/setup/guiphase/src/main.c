#include <glib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>
#include <wintc/shelldpa.h>

#include "arm.h"
#include "setupclr.h"
#include "setupwnd.h"

//
// FORWARD DECLARATIONS
//
static gboolean idle_setup_begin_cb(
    gpointer user_data
);

static void on_setup_controller_done(
    WinTCSetupController* setup,
    gpointer              user_data
);

//
// STATIC DATA
//
static gboolean S_OPTION_ARM  = FALSE;
static gboolean S_OPTION_TEST = FALSE;

static GOptionEntry S_ENTRIES[] = {
    {
        "arm",
        'a',
        0,
        G_OPTION_ARG_NONE,
        &S_OPTION_ARM,
        "Arm the system to boot into graphical-mode setup.",
        NULL
    },
    {
        "test",
        't',
        0,
        G_OPTION_ARG_NONE,
        &S_OPTION_TEST,
        "Rory's testing mode.",
        NULL
    },
    G_OPTION_ENTRY_NULL
};

//
// ENTRY POINT
//
int main(
    int argc,
    char* argv[]
)
{
    GError* error = NULL;

    //
    // Parse startup options
    //
    GOptionContext* ctx = g_option_context_new(NULL);

    g_option_context_add_main_entries(
        ctx,
        S_ENTRIES,
        NULL
    );
    g_option_context_add_group(
        ctx,
        gtk_get_option_group(FALSE)
    );

    if (!g_option_context_parse(ctx, &argc, &argv, &error))
    {
        wintc_log_error_and_clear(&error);
        return EXIT_FAILURE;
    }

    //
    // Handle options
    //
    if (S_OPTION_ARM)
    {
        gboolean res_arm = wintc_setup_arm_system();

        if (res_arm)
        {
            g_message("%s", "Setup prepared, reboot to begin setup.");
        }
        else
        {
            g_critical("%s", "Setup preparation was unsuccessful.");
        }

        return res_arm ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    //
    // Continue to normal graphical mode setup
    //

    // Fire up XFWM4
    //
    if (
        !S_OPTION_TEST &&
        !wintc_launch_command("xfwm4 --compositor=on", &error)
    )
    {
        wintc_log_error_and_clear(&error);
        return EXIT_FAILURE;
    }

    // Spawn GTK
    //
    gtk_init(&argc, &argv);

    if (!wintc_init_display_protocol_apis())
    {
        g_critical("%s", "Failed to resolve display protocol APIs.");
        return EXIT_FAILURE;
    }

    wintc_ctl_install_default_styles();

    // Set GtkSettings, because xsettings never seems to work -_-
    //
    GtkSettings* settings = gtk_settings_get_default();

    g_object_set(
        settings,
        "gtk-theme-name",        "Windows Classic style",
        "gtk-cursor-theme-name", "standard-with-shadow",
        NULL
    );

    // Set up styling
    //
    GtkCssProvider* css = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(
        css,
        "/uk/oddmatics/wintc/wsetupx/setupwnd.css"
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    // Create setup background window
    //
    GtkWidget* wnd_setup = wintc_setup_window_new();

    wintc_setup_window_enable_billboards(WINTC_SETUP_WINDOW(wnd_setup));
    wintc_setup_window_enable_throbbers(WINTC_SETUP_WINDOW(wnd_setup));

    wintc_setup_window_set_completion_minutes_approx(
        WINTC_SETUP_WINDOW(wnd_setup),
        45
    );

    wintc_setup_window_set_current_step(
        WINTC_SETUP_WINDOW(wnd_setup),
        WINTC_SETUP_STEP_INSTALLING_WINDOWS
    );

    // Set up the chain to start the setup controller
    //
    g_idle_add(
        (GSourceFunc) idle_setup_begin_cb,
        wnd_setup
    );

    // Launch!
    //
    gtk_widget_show_all(wnd_setup);
    gtk_main();

    return EXIT_SUCCESS;
}

//
// CALLBACKS
//
static gboolean idle_setup_begin_cb(
    gpointer user_data
)
{
    WinTCSetupController* controller =
        wintc_setup_controller_new(
            WINTC_SETUP_WINDOW(user_data)
        );

    wintc_setup_controller_begin(controller);

    g_signal_connect(
        controller,
        "setup-done",
        G_CALLBACK(on_setup_controller_done),
        NULL
    );

    return G_SOURCE_REMOVE;
}

static void on_setup_controller_done(
    WINTC_UNUSED(WinTCSetupController* setup),
    WINTC_UNUSED(gpointer              user_data)
)
{
    GError* error = NULL;

    // Reboot
    //
    // FIXME: likely debian specific!!
    //
    gchar* argv[] = {
        "/usr/sbin/reboot",
        NULL
    };

    if (
        !g_spawn_sync(
            NULL,
            argv,
            NULL,
            0,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
    }

    gtk_main_quit();
}

#include <glib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>
#include <wintc/shelldpa.h>

#include "arm.h"
#include "setupwnd.h"

//
// FORWARD DECLARATIONS
//
static void on_window_destroyed(
    GtkWidget* widget,
    gpointer   user_data
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
    gtk_init(&argc, &argv);

    if (!wintc_init_display_protocol_apis())
    {
        g_critical("%s", "Failed to resolve display protocol APIs.");
        return EXIT_FAILURE;
    }

    // Fire up xfwm4
    //
    if (
        !S_OPTION_TEST &&
        !wintc_launch_command(
            "xfwm4 --compositor=on",
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
        return EXIT_FAILURE;
    }

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

    // Create a billy basic test window (will be replaced by a wizard
    // eventually
    //
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(GTK_WINDOW(window), "Hello World!");
    gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(wnd_setup));

    g_signal_connect(
        window,
        "destroy",
        G_CALLBACK(on_window_destroyed),
        NULL
    );

    // Launch!
    //
    gtk_widget_show_all(wnd_setup);
    gtk_widget_show_all(window);
    gtk_main();

    return EXIT_SUCCESS;
}

//
// CALLBACKS
//
static void on_window_destroyed(
    WINTC_UNUSED(GtkWidget* widget),
    WINTC_UNUSED(gpointer   user_data)
)
{
    gtk_main_quit();
}

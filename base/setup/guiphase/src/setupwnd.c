#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/shelldpa.h>
#include <wintc/shlang.h>

#include "setupwnd.h"

#define NUM_THROBBERS 5
#define THROBBER_FRAMERATE 12

//
// FORWARD DECLARATIONS
//
static void on_animation_throbber_cycled(
    WinTCCtlAnimation* anim,
    gpointer           user_data
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCSetupWindowClass
{
    WinTCDpaDesktopWindowClass __parent__;
};

struct _WinTCSetupWindow
{
    WinTCDpaDesktopWindow __parent__;

    // UI
    //
    GtkWidget* box_bottom;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCSetupWindow,
    wintc_setup_window,
    WINTC_TYPE_DPA_DESKTOP_WINDOW
)

static void wintc_setup_window_class_init(
    WINTC_UNUSED(WinTCSetupWindowClass* klass)
) {}

static void wintc_setup_window_init(
    WinTCSetupWindow* self
)
{
    GtkBuilder* builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/wsetupx/setupwnd.ui"
        );

    GtkWidget* main_box = NULL;

    wintc_lc_builder_preprocess_widget_text(builder);

    wintc_builder_get_objects(
        GTK_BUILDER(builder),
        "main-box", &main_box,
        "box-bottom", &(self->box_bottom),
        NULL
    );

    gtk_container_add(
        GTK_CONTAINER(self),
        main_box
    );

    g_object_unref(builder);

    // Add the style context
    //
    GtkCssProvider*  css = gtk_css_provider_new();
    GtkStyleContext* ctx = gtk_widget_get_style_context(
                               GTK_WIDGET(self)
                           );

    gtk_css_provider_load_from_resource(
        css,
        "/uk/oddmatics/wintc/wsetupx/setupwnd.css"
    );
    gtk_style_context_add_provider(
        ctx,
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_setup_window_new(void)
{
    GdkMonitor* monitor =
        gdk_display_get_primary_monitor(
            gdk_screen_get_display(
                gdk_screen_get_default()
            )
        );

    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_SETUP_WINDOW,
            "type",    GTK_WINDOW_TOPLEVEL,
            "monitor", monitor,
            NULL
        )
    );
}

void wintc_setup_window_disable_throbbers(
    WinTCSetupWindow* wnd
)
{
    wintc_container_clear(
        GTK_CONTAINER(wnd->box_bottom)
    );
}

void wintc_setup_window_enable_throbbers(
    WinTCSetupWindow* wnd
)
{
    GList*  children = gtk_container_get_children(
                           GTK_CONTAINER(wnd->box_bottom)
                       );
    GError* error    = NULL;

    if (g_list_length(children) > 0)
    {
        return;
    }

    g_list_free(children);

    // Load the throbber pixbuf
    //
    GdkPixbuf* pixbuf_throbber =
        gdk_pixbuf_new_from_resource(
            "/uk/oddmatics/wintc/wsetupx/throbber.png",
            &error
        );

    // Create the throbbers - created back to front because they are packed at
    // the end of the box
    //
    for (gint i = NUM_THROBBERS; i > 0; i--)
    {
        GtkWidget* anim_throbber = wintc_ctl_animation_new();

        gtk_widget_set_valign(anim_throbber, GTK_ALIGN_CENTER);

        wintc_ctl_animation_add_framesheet(
            WINTC_CTL_ANIMATION(anim_throbber),
            pixbuf_throbber,
            8
        );

        g_signal_connect(
            anim_throbber,
            "cycled",
            G_CALLBACK(on_animation_throbber_cycled),
            wnd
        );

        gtk_box_pack_end(
            GTK_BOX(wnd->box_bottom),
            anim_throbber,
            FALSE,
            FALSE,
            0
        );

        if (i == 1)
        {
            wintc_ctl_animation_play(
                WINTC_CTL_ANIMATION(anim_throbber),
                1,
                THROBBER_FRAMERATE,
                1
            );
        }
        else
        {
            wintc_ctl_animation_play(
                WINTC_CTL_ANIMATION(anim_throbber),
                1,
                0,
                WINTC_CTL_ANIMATION_INFINITE
            );
        }
    }

    g_object_unref(pixbuf_throbber);
}

//
// CALLBACKS
//
static void on_animation_throbber_cycled(
    WINTC_UNUSED(WinTCCtlAnimation* anim),
    gpointer user_data
)
{
    static guint current_throbber = 0;

    WinTCSetupWindow* wnd = WINTC_SETUP_WINDOW(user_data);

    // Iterate to next throbber
    //
    guint next_throbber = current_throbber + 1;

    if (next_throbber >= NUM_THROBBERS)
    {
        next_throbber = 0;
    }

    // Update the throbbers
    //
    GList* children =
        gtk_container_get_children(GTK_CONTAINER(wnd->box_bottom));

    wintc_ctl_animation_play(
        WINTC_CTL_ANIMATION(
            g_list_nth_data(children, current_throbber)
        ),
        1,
        0,
        WINTC_CTL_ANIMATION_INFINITE
    );

    wintc_ctl_animation_play(
        WINTC_CTL_ANIMATION(
            g_list_nth_data(children, next_throbber)
        ),
        1,
        THROBBER_FRAMERATE,
        1
    );

    g_list_free(children);

    current_throbber = next_throbber;
}

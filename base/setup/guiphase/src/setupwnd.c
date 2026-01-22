#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/shelldpa.h>
#include <wintc/shlang.h>

#include "setupwnd.h"

#define SECONDS_PER_BILLBOARD 35
#define NUM_THROBBERS 5
#define THROBBER_FRAMERATE 12

//
// FORWARD DECLARATIONS
//
static void wintc_setup_window_set_billboard(
    WinTCSetupWindow* wnd,
    guint             id
);

static void on_animation_throbber_cycled(
    WinTCCtlAnimation* anim,
    gpointer           user_data
);
static gboolean on_timeout_billboard(
    gpointer user_data
);

//
// STATIC DATA
//
const gchar* S_BILLBOARDS[] = {
    "Welcome to WinTC Release Preview",
    "Thanks for installing and trying out Windows Total Conversion Release Preview! It has been a long road to reach this first test release, and there's still plenty left to do.\n\nPlease report any bugs, feedback, or technical contributions you may have as this would be beneficial to improving the project.",

    "Supporting your choice of system",
    "One of the primary project goals is to support the many combinations of Linux and BSD systems people may be using. Windows Total Conversion should install, run, and adapt to your choice of distribution and system software automatically.\n\nYou can help by testing this release against your own personally configured setups, or simply against uncommon distributions or computers.",

    "Familiar and fun, enjoy computing again",
    "Software does not have to be so lifeless, the Windows Total Conversion project is driven by enthusiasm for fun and usefulness in computing.\n\nFeel free to toy around and personalize your environment, and know your bug reports will be taken seriously!",

    "Help develop towards the next release",
    "If you are a developer interested in Windows and Linux, consider playing with the source and chipping in. Developer guides are on the GitHub Wiki with information on building from source and getting familiar with making changes. Become familiar with the code inside-and-out, and enhance your knowlege of how desktop environments work.\n\nAsk any questions on GitHub Issues or Discussion boards - this is no big-cheese project so don't worry about getting your head bitten off! Interest in contribution or just plain playing with the source is welcome.",

    "Thank you, and enjoy!",
    "I sincerely hope you enjoy using Windows Total Conversion. Whilst this first release may be at the bottom of the mountain, it should be much less fuss for you to try out!\n\nThank you and have fun!\n\nRory"
};

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
    GtkWidget* grid_steps;
    GtkWidget* label_approx;
    GtkWidget* label_billboard_body;
    GtkWidget* label_billboard_title;
    GtkWidget* label_progress;
    GtkWidget* progress_step;

    guint id_timeout_billboards;
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
        "main-box",              &main_box,
        "box-bottom",            &(self->box_bottom),
        "grid-steps",            &(self->grid_steps),
        "label-approx",          &(self->label_approx),
        "label-billboard-body",  &(self->label_billboard_body),
        "label-billboard-title", &(self->label_billboard_title),
        "label-progress",        &(self->label_progress),
        "progress-step",         &(self->progress_step),
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

void wintc_setup_window_disable_billboards(
    WinTCSetupWindow* wnd
)
{
    if (!(wnd->id_timeout_billboards))
    {
        return;
    }

    g_source_remove(wnd->id_timeout_billboards);
    wnd->id_timeout_billboards = 0;

    gtk_widget_set_visible(wnd->label_billboard_title, FALSE);
    gtk_widget_set_visible(wnd->label_billboard_body,  FALSE);
}

void wintc_setup_window_disable_throbbers(
    WinTCSetupWindow* wnd
)
{
    wintc_container_clear(
        GTK_CONTAINER(wnd->box_bottom),
        TRUE
    );
}

void wintc_setup_window_enable_billboards(
    WinTCSetupWindow* wnd
)
{
    if (wnd->id_timeout_billboards)
    {
        return;
    }

    wintc_setup_window_set_billboard(wnd, 0);

    gtk_widget_set_visible(wnd->label_billboard_title, TRUE);
    gtk_widget_set_visible(wnd->label_billboard_body,  TRUE);

    wnd->id_timeout_billboards =
        g_timeout_add_seconds(
            SECONDS_PER_BILLBOARD,
            (GSourceFunc) on_timeout_billboard,
            wnd
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

void wintc_setup_window_set_completion_minutes_approx(
    WinTCSetupWindow* wnd,
    guint             minutes
)
{
    gchar* text = g_strdup_printf("%d minutes", minutes);

    gtk_label_set_text(
        GTK_LABEL(wnd->label_approx),
        text
    );

    g_free(text);
}

void wintc_setup_window_set_current_step(
    WinTCSetupWindow* wnd,
    gint              step
)
{
    if (step > N_WINTC_SETUP_STEPS)
    {
        g_critical("wsetupx: invalid step %d", step);
        return;
    }

    // Iterate over and update steps
    //
    GList* children = gtk_container_get_children(
                          GTK_CONTAINER(wnd->grid_steps)
                      );
    GList* iter     = g_list_reverse(children); // Grid is back to front

    for (gint i = 0; i < N_WINTC_SETUP_STEPS; i++)
    {
        GtkWidget* image_bullet = GTK_WIDGET(iter->data);
        GtkWidget* label_step   = GTK_WIDGET(iter->next->data);

        GtkStyleContext* ctx = gtk_widget_get_style_context(label_step);

        gtk_style_context_remove_class(ctx, "setup-step-complete");
        gtk_style_context_remove_class(ctx, "setup-step-current");

        if (i < step)
        {
            gtk_image_set_from_resource(
                GTK_IMAGE(image_bullet),
                "/uk/oddmatics/wintc/wsetupx/bulldone.png"
            );

            gtk_style_context_add_class(ctx, "setup-step-complete");
        }
        else if (i == step)
        {
            gtk_image_set_from_resource(
                GTK_IMAGE(image_bullet),
                "/uk/oddmatics/wintc/wsetupx/bullcur.png"
            );

            gtk_style_context_add_class(ctx, "setup-step-current");
        }
        else
        {
            gtk_image_set_from_resource(
                GTK_IMAGE(image_bullet),
                "/uk/oddmatics/wintc/wsetupx/bullnull.png"
            );
        }

        iter = iter->next->next;
    }

    g_list_free(children);
}

void wintc_setup_window_set_current_step_progress(
    WinTCSetupWindow* wnd,
    const gchar*      step_descr,
    gdouble           fraction
)
{
    if (fraction == 0.0f)
    {
        gtk_widget_set_visible(wnd->label_progress, FALSE);
        gtk_widget_set_visible(wnd->progress_step,  FALSE);

        return;
    }

    gtk_label_set_text(
        GTK_LABEL(wnd->label_progress),
        step_descr
    );
    gtk_progress_bar_set_fraction(
        GTK_PROGRESS_BAR(wnd->progress_step),
        fraction
    );

    gtk_widget_set_visible(wnd->label_progress, TRUE);
    gtk_widget_set_visible(wnd->progress_step,  TRUE);
}

//
// PRIVATE FUNCTIONS
//
static void wintc_setup_window_set_billboard(
    WinTCSetupWindow* wnd,
    guint             id
)
{
    guint base_id = id * 2;

    if (base_id >= G_N_ELEMENTS(S_BILLBOARDS))
    {
        g_critical("wsetupx: invalid billboard requested: %u", id);
        return;
    }

    gtk_label_set_text(
        GTK_LABEL(wnd->label_billboard_title),
        S_BILLBOARDS[base_id]
    );
    gtk_label_set_text(
        GTK_LABEL(wnd->label_billboard_body),
        S_BILLBOARDS[base_id + 1]
    );
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

static gboolean on_timeout_billboard(
    gpointer user_data
)
{
    static guint current_billboard = 0;

    WinTCSetupWindow* wnd = WINTC_SETUP_WINDOW(user_data);

    // Iterate to next billboard
    //
    guint next_billboard = current_billboard + 1;

    if (next_billboard * 2 >= G_N_ELEMENTS(S_BILLBOARDS))
    {
        next_billboard = 0;
    }

    wintc_setup_window_set_billboard(wnd, next_billboard);

    current_billboard = next_billboard;

    return G_SOURCE_CONTINUE;
}

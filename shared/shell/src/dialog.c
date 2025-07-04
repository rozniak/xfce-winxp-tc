#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <pwd.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/shlang.h>
#include <wintc/winbrand.h>

#include "../public/dialog.h"

//
// FORWARD DECLARATIONS
//
static void action_sh_about_ok(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);

//
// PUBLIC FUNCTIONS
//
void wintc_sh_about(
    GtkWindow*   parent_wnd,
    const gchar* app_name,
    WINTC_UNUSED(const gchar* other_stuff),
    const gchar* icon_name
)
{
    static const GActionEntry k_actions[] = {
        {
            .name           = "ok",
            .activate       = action_sh_about_ok,
            .parameter_type = NULL,
            .state          = NULL,
            .change_state   = NULL
        }
    };

    // Instance dialog from builder
    //
    GtkBuilder* builder;

    GtkWidget* img_banner;
    GtkWidget* img_strip;
    GtkWidget* img_icon;
    GtkWidget* label_appname;
    GtkWidget* label_kernel;
    GtkWidget* label_stats;
    GtkWidget* label_user;
    GtkWidget* wnd;

    wintc_ctl_install_default_styles();

    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/shell/dlgabout.ui"
        );

    wintc_lc_builder_preprocess_widget_text(builder);

    wintc_builder_get_objects(
        builder,
        "main-wnd",      &wnd,
        "img-banner",    &img_banner,
        "img-strip",     &img_strip,
        "img-icon",      &img_icon,
        "label-appname", &label_appname,
        "label-kernel",  &label_kernel,
        "label-user",    &label_user,
        "label-stats",   &label_stats,
        NULL
    );

    g_object_ref(wnd);

    g_object_unref(builder);

    // Set up window
    //
    GdkPixbuf* pixbuf_banner = wintc_brand_get_brand_pixmap(
                                   WINTC_BRAND_PART_BANNER,
                                   NULL
                               );
    GdkPixbuf* pixbuf_strip  = wintc_brand_get_brand_pixmap(
                                   WINTC_BRAND_PART_STRIP_STATIC,
                                   NULL
                               );

    struct utsname kernel_info;
    struct sysinfo stats;
    struct passwd* user_pwd = getpwuid(getuid());

    uname(&kernel_info);
    sysinfo(&stats);

    gtk_image_set_from_pixbuf(
        GTK_IMAGE(img_banner),
        pixbuf_banner
    );
    gtk_image_set_from_pixbuf(
        GTK_IMAGE(img_strip),
        pixbuf_strip
    );

    wintc_widget_printf(
        wnd,
        app_name
    );
    wintc_widget_printf(
        label_appname,
        app_name
    );
    wintc_widget_printf(
        label_kernel,
        kernel_info.release,
        wintc_build_query(WINTC_VER_TAG),
        wintc_build_is_debug() ? " (Debug)" : ""
    );
    wintc_widget_printf(
        label_user,
        user_pwd->pw_name
    );
    wintc_widget_printf(
        label_stats,
#ifdef WINTC_PKGMGR_BSDPKG
        stats.totalram
#else
        stats.totalram / 1024
#endif
    );

    if (icon_name)
    {
        gtk_image_set_from_icon_name(
            GTK_IMAGE(img_icon),
            icon_name,
            GTK_ICON_SIZE_DND
        );
    }

    if (parent_wnd)
    {
        gtk_window_set_transient_for(
            GTK_WINDOW(wnd),
            GTK_WINDOW(parent_wnd)
        );
        gtk_window_set_modal(
            GTK_WINDOW(wnd),
            TRUE
        );
    }

    // Set up actions
    //
    g_action_map_add_action_entries(
        G_ACTION_MAP(wnd),
        k_actions,
        G_N_ELEMENTS(k_actions),
        wnd
    );

    // Launch window (for application)
    //
    GApplication* app = g_application_get_default();

    if (app)
    {
        gtk_window_set_application(
            GTK_WINDOW(wnd),
            GTK_APPLICATION(app)
        );
    }

    gtk_widget_show_all(wnd);

    // Clean up
    //
    g_object_unref(pixbuf_banner);
    g_object_unref(pixbuf_strip);
}

//
// PRIVATE FUNCTIONS
//
static void action_sh_about_ok(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant* parameter),
    gpointer user_data
)
{
    gtk_window_close(GTK_WINDOW(user_data));
}

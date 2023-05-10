#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <pwd.h>
#include <wintc-comgtk.h>
#include <wintc-winbrand.h>

//
// FORWARD DECLARATIONS
//
static void apply_box_model_style(
    GtkWidget*   widget,
    const gchar* model_part,
    const gchar* side,
    guint        margin
);
static void apply_winver_margin_style(
    GtkWidget* widget
);
static GtkWidget* create_winver_label(
    const gchar* text
);

static void on_ok_button_clicked(
    GtkButton* button,
    gpointer   user_data
);
static void on_window_destroyed(
    GtkWidget* widget,
    gpointer   user_data
);

//
// ENTRY POINT
//
int main(
    int   argc,
    char* argv[]
)
{
    GtkWidget* banner;
    GtkWidget* banner_strip;
    GtkWidget* box;
    GtkWidget* box_buttons;
    GtkWidget* button_ok;
    GtkWidget* label_copyright;
    GtkWidget* label_eula;
    GtkWidget* label_eula_user;
    GtkWidget* label_memory;
    GtkWidget* label_product;
    GtkWidget* label_version;
    GtkWidget* separator;
    GtkWidget* window;
    struct sysinfo stats;
    struct utsname kernel_info;

    gtk_init(&argc, &argv);

    // Create the window
    //
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_widget_set_size_request(window, 413, 322);
    gtk_window_set_icon(GTK_WINDOW(window), NULL); // FIXME: Icon still present :(
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_window_set_title(GTK_WINDOW(window), "About Windows");
    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_MENU);

    g_signal_connect(
        window,
        "destroy",
        G_CALLBACK(on_window_destroyed),
        NULL
    );

    // Create banner
    //
    GError*    banner_error  = NULL;
    GdkPixbuf* banner_pixbuf = NULL;
    GError*    strip_error   = NULL;
    GdkPixbuf* strip_pixbuf  = NULL;

    banner_pixbuf = wintc_brand_get_banner(&banner_error);
    strip_pixbuf  = wintc_brand_get_progress_strip(&strip_error);

    if (banner_pixbuf != NULL)
    {
        banner = gtk_image_new_from_pixbuf(banner_pixbuf);

        g_object_unref(banner_pixbuf);
    }

    if (strip_pixbuf != NULL)
    {
        banner_strip = gtk_image_new_from_pixbuf(strip_pixbuf);

        apply_box_model_style(banner_strip, "margin", "bottom", 4);

        g_object_unref(strip_pixbuf);
    }

    wintc_log_error_and_clear(&banner_error);
    wintc_log_error_and_clear(&strip_error);

    // Get kernel info
    //
    gchar* kernel_version;

    uname(&kernel_info);
    
    kernel_version =
        g_strdup_printf(
            "%s (%s %s)",
            kernel_info.version,
            kernel_info.sysname,
            kernel_info.release
        );
    
    // Get system info
    //
    gchar* system_stats;

    sysinfo(&stats);

    system_stats =
        g_strdup_printf(
            "Physical memory available to Windows: %lu KB",
            stats.totalram / 1024
        );

    // Create labels
    // 
    struct passwd *user_pwd = getpwuid(getuid());

    label_product   = create_winver_label("Microsoft ® Windows");
    label_version   = create_winver_label(kernel_version);
    label_copyright = create_winver_label("Copyright © 1985-2005 Microsoft Corporation");
    label_eula      = create_winver_label("This product is licensed under the terms of the End User License Agreement to:");
    label_eula_user = create_winver_label(user_pwd->pw_name);
    label_memory    = create_winver_label(system_stats);

    apply_box_model_style(label_product,   "margin", "top",    4);
    apply_box_model_style(label_eula,      "margin", "top",    30);
    apply_box_model_style(label_eula_user, "margin", "left",   60);
    apply_box_model_style(label_eula_user, "margin", "bottom", 16);
    apply_box_model_style(label_memory,    "margin", "top",    4);

    // Create separator
    //
    separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);

    apply_box_model_style(separator, "margin", "right", 8);
    apply_winver_margin_style(separator);

    // Create OK button
    //
    button_ok = gtk_button_new_with_label("OK");

    apply_box_model_style(button_ok, "margin", "right",  8);
    apply_box_model_style(button_ok, "margin", "bottom", 10);
    apply_box_model_style(button_ok, "padding", "left",  26);
    apply_box_model_style(button_ok, "padding", "right", 26);

    g_signal_connect(
        button_ok,
        "clicked",
        G_CALLBACK(on_ok_button_clicked),
        window
    );

    // Build window contents
    //
    box         = gtk_box_new(GTK_ORIENTATION_VERTICAL,   0);
    box_buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_container_add(GTK_CONTAINER(window), box);

    gtk_box_pack_start(GTK_BOX(box), banner,          FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), banner_strip,    FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), label_product,   FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), label_version,   FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), label_copyright, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), label_eula,      FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), label_eula_user, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), separator,       FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), label_memory,    FALSE, FALSE, 0);
    
    gtk_box_pack_end(GTK_BOX(box),         box_buttons, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(box_buttons), button_ok,   FALSE, FALSE, 0);
    
    // Clear mem
    //
    g_free(kernel_version);
    g_free(system_stats);

    // Launch now
    //
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}

//
// PRIVATE FUNCTIONS
//
static void apply_box_model_style(
    GtkWidget*   widget,
    const gchar* model_part,
    const gchar* side,
    guint        margin
)
{
    GtkCssProvider*  provider  = gtk_css_provider_new();
    GtkStyleContext* style     = gtk_widget_get_style_context(widget);
    gchar*           style_css = g_slice_alloc0(
                                     sizeof(char) * 40
                                 );

    g_sprintf(
        style_css,
        "* { %s-%s: %dpx; }",
        model_part,
        side,
        margin
    );

    gtk_css_provider_load_from_data(
        provider,
        style_css,
        -1,
        NULL
    );

    gtk_style_context_add_provider(
        style,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(provider);
    g_slice_free1(sizeof(char) * 40, style_css);
}

static void apply_winver_margin_style(
    GtkWidget* widget
)
{
    apply_box_model_style(widget, "margin", "bottom", 2);
    apply_box_model_style(widget, "margin", "left", 52);
    apply_box_model_style(widget, "margin", "top", 2);
}

static GtkWidget* create_winver_label(
    const gchar* text
)
{
    GtkWidget* label = gtk_label_new(text);

    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(label), 54);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    apply_winver_margin_style(label);

    return label;
}

//
// CALLBACKS
//
static void on_ok_button_clicked(
    WINTC_UNUSED(GtkButton* button),
    gpointer user_data
)
{
    gtk_window_close(
        GTK_WINDOW(user_data)
    );
}

static void on_window_destroyed(
    WINTC_UNUSED(GtkWidget* widget),
    WINTC_UNUSED(gpointer   user_data)
)
{
    gtk_main_quit();
}

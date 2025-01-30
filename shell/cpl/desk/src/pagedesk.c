#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>
#include <wintc/shlang.h>
#include <wintc/syscfg.h>

#include "monitor.h"
#include "pagedesk.h"
#include "settings.h"
#include "window.h"

//
// FORWARD DECLARATIONS
//
static void add_wallpaper_to_list(
    WinTCCplDeskWindow* wnd,
    const gchar*        path,
    gboolean            new_item
);
static void redraw_wallpaper_preview(
    WinTCCplDeskWindow* wnd
);
static void refresh_wallpaper_list(
    WinTCCplDeskWindow* wnd
);
static void select_wallpaper_from_list(
    WinTCCplDeskWindow* wnd,
    const gchar*        path
);

static void action_browse(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);

static void on_combo_style_changed(
    GtkComboBox* self,
    gpointer     user_data
);
static void on_listbox_wallpapers_row_selected(
    GtkListBox*    self,
    GtkListBoxRow* row,
    gpointer       user_data
);

//
// STATIC DATA
//
static GActionEntry s_actions[] = {
    {
        .name           = "browse",
        .activate       = action_browse,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    }
};

//
// PUBLIC FUNCTIONS
//
void wintc_cpl_desk_window_append_desktop_page(
    WinTCCplDeskWindow* wnd
)
{
    wintc_ctl_cpl_notebook_append_page_from_resource(
        GTK_NOTEBOOK(wnd->notebook_main),
        "/uk/oddmatics/wintc/cpl-desk/page-desktop.ui",
        "combo-style",        &(wnd->combo_style),
        "listbox-wallpapers", &(wnd->listbox_wallpapers),
        "monitor",            &(wnd->monitor_desktop),
        NULL
    );

    // Define GActions
    //
    GSimpleActionGroup* action_group = g_simple_action_group_new();

    g_action_map_add_action_entries(
        G_ACTION_MAP(action_group),
        s_actions,
        G_N_ELEMENTS(s_actions),
        wnd
    );

    gtk_widget_insert_action_group(
        wnd->notebook_main,
        "desktop",
        G_ACTION_GROUP(action_group)
    );

    // Connect signals
    //
    g_signal_connect(
        wnd->combo_style,
        "changed",
        G_CALLBACK(on_combo_style_changed),
        wnd
    );
    g_signal_connect(
        wnd->listbox_wallpapers,
        "row-selected",
        G_CALLBACK(on_listbox_wallpapers_row_selected),
        wnd
    );
}

void wintc_cpl_desk_window_load_desktop_page(
    WinTCCplDeskWindow* wnd
)
{
    refresh_wallpaper_list(wnd);

    select_wallpaper_from_list(
        wnd,
        wintc_cpl_desk_settings_get_wallpaper(wnd->settings)
    );

    // Select initial value for style combobox
    //
    GtkTreeIter iter;

    gchar* sz_style_id =
        g_strdup_printf(
            "%d",
            wintc_cpl_desk_settings_get_wallpaper_style(wnd->settings)
        );

    gtk_tree_model_get_iter_from_string(
        gtk_combo_box_get_model(
            GTK_COMBO_BOX(wnd->combo_style)
        ),
        &iter,
        sz_style_id
    );

    gtk_combo_box_set_active_iter(
        GTK_COMBO_BOX(wnd->combo_style),
        &iter
    );

    g_free(sz_style_id);
}

void wintc_cpl_desk_window_finalize_desktop_page(
    WinTCCplDeskWindow* wnd
)
{
    g_list_free_full(wnd->list_wallpapers, g_free);
    g_clear_object(&(wnd->pixbuf_wallpaper));
}

//
// PRIVATE FUNCTIONS
//
static void add_wallpaper_to_list(
    WinTCCplDeskWindow* wnd,
    const gchar*        path,
    gboolean            new_item
)
{
    gchar*     filename = g_path_get_basename(path);
    GtkWidget* label    = gtk_label_new(filename);

    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);

    gtk_list_box_insert(
        GTK_LIST_BOX(wnd->listbox_wallpapers),
        label,
        -1
    );

    gtk_widget_show(label);

    g_free(filename);

    // Do we need to add this item to the backing list?
    //
    if (new_item)
    {
        wnd->list_wallpapers =
            g_list_append(
                wnd->list_wallpapers,
                g_strdup(path)
            );
    }
}

static void redraw_wallpaper_preview(
    WinTCCplDeskWindow* wnd
)
{
    WinTCWallpaperStyle style =
        wintc_cpl_desk_settings_get_wallpaper_style(wnd->settings);

    // The preview image seems to be a 1024x768 desktop, so that's more or less
    // what we want to draw here, and then set on the monitor
    //
    cairo_surface_t* surface = cairo_image_surface_create(
                                   CAIRO_FORMAT_RGB24,
                                   1024,
                                   768
                               );
    cairo_t*         cr      = cairo_create(surface);

    // FIXME: Defaulting to white for now, should be system theme's desktop bg
    //
    cairo_set_source_rgb(cr, 1.0f, 1.0f, 1.0f);
    cairo_paint(cr);

    if (wnd->pixbuf_wallpaper)
    {
        gint wallpaper_w = gdk_pixbuf_get_width(wnd->pixbuf_wallpaper);
        gint wallpaper_h = gdk_pixbuf_get_height(wnd->pixbuf_wallpaper);

        switch (style)
        {
            case WINTC_WALLPAPER_STYLE_CENTER:
                cairo_translate(
                    cr,
                    512.0f - ((gdouble) wallpaper_w / 2),
                    384.0f - ((gdouble) wallpaper_h / 2)
                );
                break;

            case WINTC_WALLPAPER_STYLE_STRETCH:
                cairo_scale(
                    cr,
                    1024.0f / wallpaper_w,
                    768.0f  / wallpaper_h
                );
                break;

            default: break;
        }

        gdk_cairo_set_source_pixbuf(
            cr,
            wnd->pixbuf_wallpaper,
            0,
            0
        );

        if (style == WINTC_WALLPAPER_STYLE_TILED)
        {
            cairo_pattern_set_extend(
                cairo_get_source(cr),
                CAIRO_EXTEND_REPEAT
            );
        }

        cairo_paint(cr);
    }

    // Assign to the monitor now!
    //
    GdkPixbuf* pixbuf =
        gdk_pixbuf_get_from_surface(surface, 0, 0, 1024, 768);

    wintc_desk_monitor_set_pixbuf(
        WINTC_DESK_MONITOR(wnd->monitor_desktop),
        pixbuf
    );

    g_object_unref(pixbuf);
    cairo_surface_destroy(surface);
    cairo_destroy(cr);
}

static void refresh_wallpaper_list(
    WinTCCplDeskWindow* wnd
)
{
    g_list_free_full(wnd->list_wallpapers, g_free);
    wintc_container_clear(GTK_CONTAINER(wnd->listbox_wallpapers));

    // Load up wallpapers
    //
    GError* error = NULL;

    wnd->list_wallpapers =
        wintc_sh_fs_get_names_as_list(
            WINTC_RT_PREFIX "/share/backgrounds",
            TRUE,
            G_FILE_TEST_IS_REGULAR,
            TRUE,
            &error
        );

    if (!wnd->list_wallpapers)
    {
        wintc_nice_error_and_clear(&error, GTK_WINDOW(wnd));
        return;
    }

    for (GList* iter = wnd->list_wallpapers; iter; iter = iter->next)
    {
        add_wallpaper_to_list(wnd, (gchar*) iter->data, FALSE);
    }
}

static void select_wallpaper_from_list(
    WinTCCplDeskWindow* wnd,
    const gchar*        path
)
{
    gint i = 0;

    if (!path)
    {
        return;
    }

    for (GList* iter = wnd->list_wallpapers; iter; iter = iter->next)
    {
        if (g_strcmp0((gchar*) iter->data, path) == 0)
        {
            gtk_list_box_select_row(
                GTK_LIST_BOX(wnd->listbox_wallpapers),
                gtk_list_box_get_row_at_index(
                    GTK_LIST_BOX(wnd->listbox_wallpapers),
                    i
                )
            );

            wintc_list_box_queue_scroll_to_selected(
                GTK_LIST_BOX(wnd->listbox_wallpapers)
            );

            return;
        }

        i++;
    }

    // The path isn't in the listbox, so add it and select last item
    //
    add_wallpaper_to_list(wnd, path, TRUE);

    gtk_list_box_select_row(
        GTK_LIST_BOX(wnd->listbox_wallpapers),
        gtk_list_box_get_row_at_index(
            GTK_LIST_BOX(wnd->listbox_wallpapers),
            g_list_length(wnd->list_wallpapers) - 1
        )
    );

    wintc_list_box_queue_scroll_to_selected(
        GTK_LIST_BOX(wnd->listbox_wallpapers)
    );
}

//
// CALLBACKS
//
static void action_browse(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    WinTCCplDeskWindow* wnd = WINTC_CPL_DESK_WINDOW(user_data);

    //
    // FIXME: No support for Active Desktop at the moment, hence the lack of
    //        Background Files and Web page filters
    //

    // Set up image filters
    //
    static const gchar* k_filters[] = {
        "image/bmp",
        "image/gif",
        "image/jpeg",
        "image/jpg",
        "image/png"
    };

    GtkFileFilter* filter_images = gtk_file_filter_new();

    gtk_file_filter_set_name(
        filter_images,
        "All Picture Files (*.bmp;*.gif;*.jpg;*.jpeg;*.dib;*.png)"
    );

    for (gulong i = 0; i < G_N_ELEMENTS(k_filters); i++)
    {
        gtk_file_filter_add_mime_type(
            filter_images,
            k_filters[i]
        );
    }

    // Set up dialog
    //
    GtkWidget* dlg =
        gtk_file_chooser_dialog_new(
            wintc_lc_get_control_text(WINTC_CTLTXT_BROWSE, WINTC_PUNC_NONE),
            GTK_WINDOW(wnd),
            GTK_FILE_CHOOSER_ACTION_OPEN,
            wintc_lc_get_control_text(WINTC_CTLTXT_CANCEL, WINTC_PUNC_NONE),
            GTK_RESPONSE_CANCEL,
            wintc_lc_get_control_text(WINTC_CTLTXT_OPEN, WINTC_PUNC_NONE),
            GTK_RESPONSE_ACCEPT,
            NULL
        );

    gtk_file_chooser_add_filter(
        GTK_FILE_CHOOSER(dlg),
        filter_images
    );

    // Launch dialog
    //
    gint result = gtk_dialog_run(GTK_DIALOG(dlg));

    if (result == GTK_RESPONSE_ACCEPT)
    {
        select_wallpaper_from_list(
            wnd,
            gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg))
        );
    }

    gtk_widget_destroy(dlg);
}

static void on_combo_style_changed(
    GtkComboBox* self,
    gpointer     user_data
)
{
    WinTCCplDeskWindow* wnd = WINTC_CPL_DESK_WINDOW(user_data);

    gint        dw_style = 0;
    GtkTreeIter iter;

    gtk_combo_box_get_active_iter(self, &iter);

    gtk_tree_model_get(
        gtk_combo_box_get_model(self),
        &iter,
        0, &dw_style,
        -1
    );

    wintc_cpl_desk_settings_set_wallpaper_style(
        wnd->settings,
        dw_style
    );

    redraw_wallpaper_preview(wnd);
}

static void on_listbox_wallpapers_row_selected(
    WINTC_UNUSED(GtkListBox* self),
    GtkListBoxRow* row,
    gpointer       user_data
)
{
    WinTCCplDeskWindow* wnd = WINTC_CPL_DESK_WINDOW(user_data);

    GError*      error = NULL;
    const gchar* wallpaper_path;

    wallpaper_path =
        g_list_nth_data(
            wnd->list_wallpapers,
            gtk_list_box_row_get_index(row)
        );

    g_clear_object(&(wnd->pixbuf_wallpaper));

    wnd->pixbuf_wallpaper =
        gdk_pixbuf_new_from_file(
            wallpaper_path,
            &error
        );

    if (!wnd->pixbuf_wallpaper)
    {
        wintc_nice_error_and_clear(&error, GTK_WINDOW(wnd));
        return;
    }

    redraw_wallpaper_preview(wnd);

    if (!wnd->sync_settings)
    {
        wintc_cpl_desk_settings_set_wallpaper(
            wnd->settings,
            wallpaper_path
        );
    }
}
